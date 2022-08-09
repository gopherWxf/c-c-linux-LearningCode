//
// Created by 68725 on 2022/7/14.
//
#include <sys/eventfd.h>
#include "nty_coroutine.h"

// 创建epoll
int nty_epoller_create(void) {
    return epoll_create(1024);
}
// epoll_wait包装
int nty_epoller_wait(struct timespec t) {
    nty_schedule *sched = nty_coroutine_get_sched();
    return epoll_wait(sched->epfd, sched->eventlist, NTY_CO_MAX_EVENTS, t.tv_sec * 1000.0 + t.tv_nsec / 1000000.0);
}
//多线程做准备
int nty_epoller_ev_register_trigger(void) {
    nty_schedule *sched = nty_coroutine_get_sched();
    if (!sched->eventfd) {
        sched->eventfd = eventfd(0, EFD_NONBLOCK);
        assert(sched->eventfd != -1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sched->eventfd;
    int ret = epoll_ctl(sched->epfd, EPOLL_CTL_ADD, sched->eventfd, &ev);
    assert(ret != -1);
}
