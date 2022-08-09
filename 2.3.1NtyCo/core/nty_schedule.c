//
// Created by 68725 on 2022/7/15.
//
#include "nty_coroutine.h"

//红黑树比较规则cmp
static inline int nty_coroutine_sleep_cmp(nty_coroutine *co1, nty_coroutine *co2) {
    if (co1->sleep_usecs < co2->sleep_usecs) {
        return -1;
    }
    if (co1->sleep_usecs == co2->sleep_usecs) {
        return 0;
    }
    return 1;
}

static inline int nty_coroutine_wait_cmp(nty_coroutine *co1, nty_coroutine *co2) {
    if (co1->fd < co2->fd) {
        return -1;
    }
    if (co1->fd == co2->fd) {
        return 0;
    }
    return 1;
}

RB_GENERATE(_nty_coroutine_rbtree_sleep, _nty_coroutine, sleep_node, nty_coroutine_sleep_cmp);

RB_GENERATE(_nty_coroutine_rbtree_wait, _nty_coroutine, wait_node, nty_coroutine_wait_cmp);

//释放调度器的内存
void nty_schedule_free(nty_schedule *sched) {
    if (sched->epfd > 0) {
        close(sched->epfd);
    }
    if (sched->eventfd > 0) {
        close(sched->eventfd);
    }
    free(sched);
    assert(pthread_setspecific(global_sched_key, NULL) == 0);
}

// 创建调度器
int nty_schedule_create(int stack_size) {
    int sched_stack_size = stack_size ? stack_size : NTY_CO_MAX_STACKSIZE;
    nty_schedule * sched = (nty_schedule *) calloc(1, sizeof(nty_schedule));
    if (sched == NULL) {
        printf("Failed to initialize scheduler\n");
        return -1;
    }
    assert(pthread_setspecific(global_sched_key, sched) == 0);

    sched->epfd = nty_epoller_create();
    if (sched->epfd == -1) {
        printf("Failed to initialize epoller\n");
        nty_schedule_free(sched);
        return -2;
    }

    nty_epoller_ev_register_trigger();

    sched->stack_size = sched_stack_size;
    sched->page_size = getpagesize();

    sched->spawned_coroutines = 0;
    sched->default_timeout = 3000000u;

    RB_INIT(&sched->sleeping);
    RB_INIT(&sched->waiting);

    sched->birth = nty_coroutine_usec_now();

    TAILQ_INIT(&sched->ready);

    bzero(&sched->ctx, sizeof(nty_cpu_ctx));
}

//加入sleep集合，添加sleep状态（不会走这里的）
void nty_schedule_sched_sleepdown(nty_coroutine *co, uint64_t msecs) {
    uint64_t usecs = msecs * 1000u;

    nty_coroutine *co_tmp = RB_FIND(_nty_coroutine_rbtree_sleep, &co->sched->sleeping, co);
    if (co_tmp != NULL) {
        RB_REMOVE(_nty_coroutine_rbtree_sleep, &co->sched->sleeping, co_tmp);
    }

    co->sleep_usecs = nty_coroutine_diff_usecs(co->sched->birth, nty_coroutine_usec_now()) + usecs;

    while (msecs) {
        co_tmp = RB_INSERT(_nty_coroutine_rbtree_sleep, &co->sched->sleeping, co);
        if (co_tmp) {
            co->sleep_usecs++;
            continue;
        }
        co->status |= BIT(NTY_COROUTINE_STATUS_SLEEPING);
        break;
    }
    //yield
}

//加入wait集合，添加wait状态
void nty_schedule_sched_wait(nty_coroutine *co, int fd, uint32_t events, uint64_t timeout) {
    if (co->status & BIT(NTY_COROUTINE_STATUS_WAIT_READ) || co->status & BIT(NTY_COROUTINE_STATUS_WAIT_WRITE)) {
        printf("Unexpected event. lt id %lu fd %d already in %d state\n", co->id, co->fd, co->status);
        assert(0);
    }
    //添加状态
    if (events & POLLIN) {
        co->status |= NTY_COROUTINE_STATUS_WAIT_READ;
    }
    else if (events & POLLOUT) {
        co->status |= NTY_COROUTINE_STATUS_WAIT_WRITE;
    }
    else {
        printf("events : %d\n", events);
        assert(0);
    }
    co->fd = fd;
    co->events = events;
    //加入waiting集合
    nty_coroutine *co_tmp = RB_INSERT(_nty_coroutine_rbtree_wait, &co->sched->waiting, co);
    assert(co_tmp == NULL);
    //这里其实限死了都会return的，不会走下面的函数
    if (timeout == 1) {
        return;
    }
    //加入sleep集合，添加sleep状态（不会走这里的）
    nty_schedule_sched_sleepdown(co, timeout);
}

//移除wait集合，移除wait状态
nty_coroutine *nty_schedule_desched_wait(int fd) {
    nty_coroutine find_it = {0};
    find_it.fd = fd;
    nty_schedule * sched = nty_coroutine_get_sched();
    nty_coroutine *co = RB_FIND(_nty_coroutine_rbtree_wait, &sched->waiting, &find_it);
    if (co != NULL) {
        RB_REMOVE(_nty_coroutine_rbtree_wait, &co->sched->waiting, co);
    }

    co->status &= CLEARBIT(NTY_COROUTINE_STATUS_WAIT_READ);
    co->status &= CLEARBIT(NTY_COROUTINE_STATUS_WAIT_WRITE);
    nty_schedule_desched_sleepdown(co);

    return co;
}

//移除sleep集合，移除sleep状态
void nty_schedule_desched_sleepdown(nty_coroutine *co) {
    if (co->status & BIT(NTY_COROUTINE_STATUS_SLEEPING)) {
        RB_REMOVE(_nty_coroutine_rbtree_sleep, &co->sched->sleeping, co);
        co->status &= CLEARBIT(NTY_COROUTINE_STATUS_SLEEPING);
    }
}

// 判读集合中是否还有协程
static inline int nty_schedule_isdone(nty_schedule *sched) {
    return (RB_EMPTY(&sched->waiting) &&
            RB_EMPTY(&sched->sleeping) &&
            TAILQ_EMPTY(&sched->ready));
}

//从sleep集合中取出超时的协程
static nty_coroutine *nty_schedule_expired(nty_schedule *sched) {
    uint64_t t_diff_usecs = nty_coroutine_diff_usecs(sched->birth, nty_coroutine_usec_now());
    nty_coroutine *co = RB_MIN(_nty_coroutine_rbtree_sleep, &sched->sleeping);
    if (co == NULL) return NULL;
    if (co->sleep_usecs <= t_diff_usecs) {
        RB_REMOVE(_nty_coroutine_rbtree_sleep, &co->sched->sleeping, co);
        return co;
    }
    return NULL;
}

// 最小超时了多久
static uint64_t nty_schedule_min_timeout(nty_schedule *sched) {
    uint64_t t_diff_usecs = nty_coroutine_diff_usecs(sched->birth, nty_coroutine_usec_now());
    uint64_t min = sched->default_timeout;
    nty_coroutine *co = RB_MIN(_nty_coroutine_rbtree_sleep, &sched->sleeping);
    if (!co) return min;
    min = co->sleep_usecs;
    if (min > t_diff_usecs) {
        return min - t_diff_usecs;
    }
    return 0;
}

// 调度器调度epoll_wait
static int nty_schedule_epoll(nty_schedule *sched) {
    sched->num_new_events = 0;
    struct timespec t = {0, 0};
    uint64_t usecs = nty_schedule_min_timeout(sched);
    if (usecs && TAILQ_EMPTY(&sched->ready)) {
        t.tv_sec = usecs / 1000000u;
        if (t.tv_sec != 0) {
            t.tv_nsec = (usecs % 1000u) * 1000u;
        }
        else {
            t.tv_nsec = usecs * 1000u;
        }
    }
    else {
        return 0;
    }
    int nready = 0;
    while (1) {
        nready = nty_epoller_wait(t);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            }
            else {
                assert(0);
            }
        }
        break;
    }
    sched->num_new_events = nready;
    return 0;
}

//找一个wait状态的协程
nty_coroutine *nty_schedule_search_wait(int fd) {
    nty_coroutine find_it = {0};
    find_it.fd = fd;
    nty_schedule * sched = nty_coroutine_get_sched();
    nty_coroutine *co = RB_FIND(_nty_coroutine_rbtree_wait, &sched->waiting, &find_it);
    co->status = 0;
    return co;
}

//schedule loop
void nty_schedule_run(void) {
    nty_schedule * sched = nty_coroutine_get_sched();
    if (sched == NULL) return;

    while (!nty_schedule_isdone(sched)) {
        // 1. expired --> sleep rbtree
        nty_coroutine *expired = NULL;
        while ((expired = nty_schedule_expired(sched)) != NULL) {
            nty_coroutine_resume(expired);
        }
        // 2. ready queue
        nty_coroutine *last_co_ready = TAILQ_LAST(&sched->ready, _nty_coroutine_queue);
        while (!TAILQ_EMPTY(&sched->ready)) {
            nty_coroutine *co = TAILQ_FIRST(&sched->ready);
            TAILQ_REMOVE(&co->sched->ready, co, ready_node);
            if (co->status & BIT(NTY_COROUTINE_STATUS_FDEOF)) {
                nty_coroutine_free(co);
                break;
            }
            nty_coroutine_resume(co);
            if (co == last_co_ready) break;
        }
        // 3. wait rbtree
        nty_schedule_epoll(sched);
        while (sched->num_new_events) {
            int idx = --sched->num_new_events;
            struct epoll_event *ev = sched->eventlist + idx;

            int fd = ev->data.fd;
            int is_eof = ev->events & EPOLLHUP;
            if (is_eof) errno = ECONNRESET;

            nty_coroutine *co = nty_schedule_search_wait(fd);
            if (co != NULL) {
                if (is_eof) {
                    co->status |= BIT(NTY_COROUTINE_STATUS_FDEOF);
                }
                nty_coroutine_resume(co);
            }
            is_eof = 0;
        }
    }
    nty_schedule_free(sched);
    return;
}

























