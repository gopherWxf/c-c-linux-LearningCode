//
// Created by 68725 on 2022/7/14.
//

#ifndef NTYCO_NTY_COROUTINE_H
#define NTYCO_NTY_COROUTINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <errno.h>
#include "nty_queue.h"
#include "nty_tree.h"

#define NTY_CO_MAX_EVENTS        (1024*1024)
#define NTY_CO_MAX_STACKSIZE    (2*1024)

#define BIT(x)                    (1 << (x))
#define CLEARBIT(x)            ~(1 << (x))

LIST_HEAD(_nty_coroutine_link, _nty_coroutine);
TAILQ_HEAD(_nty_coroutine_queue, _nty_coroutine);

RB_HEAD(_nty_coroutine_rbtree_sleep, _nty_coroutine);
RB_HEAD(_nty_coroutine_rbtree_wait, _nty_coroutine);

typedef struct _nty_coroutine_queue nty_coroutine_queue;
typedef struct _nty_coroutine_rbtree_sleep nty_coroutine_rbtree_sleep;
typedef struct _nty_coroutine_rbtree_wait nty_coroutine_rbtree_wait;


typedef void (*proc_coroutine)(void *);

typedef enum {
    NTY_COROUTINE_STATUS_NEW,//new
    NTY_COROUTINE_STATUS_READY,//ready

    NTY_COROUTINE_STATUS_WAIT_READ,//wait read
    NTY_COROUTINE_STATUS_WAIT_WRITE,//wait write

    NTY_COROUTINE_STATUS_SLEEPING,//sleeping

    NTY_COROUTINE_STATUS_EXITED,//exited
    NTY_COROUTINE_STATUS_FDEOF,//eof
    NTY_COROUTINE_STATUS_DETACH,//detach
} nty_coroutine_status;
//寄存器 cpu上下文
typedef struct _nty_cpu_ctx {
    void *rsp;//栈顶
    void *rbp;//栈底
    void *eip;//CPU通过EIP寄存器读取即将要执行的指令
    void *edi;
    void *esi;
    void *rbx;
    void *r1;
    void *r2;
    void *r3;
    void *r4;
    void *r5;
} nty_cpu_ctx;


typedef struct _nty_schedule {
    // create time
    uint64_t birth;
    //cpu ctx
    nty_cpu_ctx ctx;
    //stack_size
    size_t stack_size;
    //coroutine num
    int spawned_coroutines;
    //default_timeout
    uint64_t default_timeout;
    //当前调度的协程
    struct _nty_coroutine *curr_thread;
    //页大小
    int page_size;
    //epoll fd
    int epfd;
    //线程通知相关，暂未实现
    int eventfd;
    //events
    struct epoll_event eventlist[NTY_CO_MAX_EVENTS];
    int num_new_events;
    //set
    nty_coroutine_queue ready;
    nty_coroutine_rbtree_sleep sleeping;
    nty_coroutine_rbtree_wait waiting;
} nty_schedule;

typedef struct _nty_coroutine {
    //cpu ctx
    nty_cpu_ctx ctx;
    // func
    proc_coroutine func;
    void *arg;
    // create time
    uint64_t birth;
    //stack
    void *stack;
    size_t stack_size;
    size_t last_stack_size;
    //status
    nty_coroutine_status status;
    //root
    nty_schedule *sched;
    //co id
    uint64_t id;
    //fd event
    int fd;
    uint16_t events;
    //sleep time
    uint64_t sleep_usecs;
    //set
    RB_ENTRY(_nty_coroutine) sleep_node;
    RB_ENTRY(_nty_coroutine) wait_node;
    TAILQ_ENTRY(_nty_coroutine) ready_node;
} nty_coroutine;

extern pthread_key_t global_sched_key;
//获得线程唯一的sched
static inline nty_schedule* nty_coroutine_get_sched(void) {
    return pthread_getspecific(global_sched_key);
}

//计算微秒差
static uint64_t nty_coroutine_diff_usecs(uint64_t t1, uint64_t t2) {
    return t2 - t1;
}

//计算现在的微秒
static uint64_t nty_coroutine_usec_now(void) {
    struct timeval t1 = {0, 0};
    gettimeofday(&t1, NULL);
    return t1.tv_sec * 1000000 + t1.tv_usec;
}

//socket

int nty_socket(int domain, int type, int protocol);

int nty_accept(int fd, struct sockaddr *addr, socklen_t *len);

ssize_t nty_recv(int fd, void *buf, size_t len, int flags);

ssize_t nty_send(int fd, const void *buf, size_t len, int flags);

int nty_close(int fd);

int nty_connect(int fd, struct sockaddr *name, socklen_t len);

ssize_t nty_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t nty_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

//nty_coroutine

int nty_coroutine_create(nty_coroutine **new_co, proc_coroutine func, void *arg);

void nty_coroutine_free(nty_coroutine *co);

void nty_coroutine_yield(nty_coroutine *co);

int nty_coroutine_resume(nty_coroutine *co);

void nty_coroutine_sleep(uint64_t msecs);

//nty_schedule

int nty_schedule_create(int stack_size);

void nty_schedule_free(nty_schedule *sched);

void nty_schedule_sched_wait(nty_coroutine *co, int fd, uint32_t events, uint64_t timeout);

nty_coroutine *nty_schedule_desched_wait(int fd);

void nty_schedule_sched_sleepdown(nty_coroutine *co, uint64_t msecs);

void nty_schedule_desched_sleepdown(nty_coroutine *co);

void nty_schedule_run(void);

nty_coroutine *nty_schedule_search_wait(int fd);

//nty_epoller

int nty_epoller_create(void);

int nty_epoller_wait(struct timespec t);

int nty_epoller_ev_register_trigger();

void catstatus(int status);

#endif //NTYCO_NTY_COROUTINE_H


