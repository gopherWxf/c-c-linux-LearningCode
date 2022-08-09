//
// Created by 68725 on 2022/7/14.
//
#include "nty_coroutine.h"

// 加入epoll，更改状态，加入wait集合，然后yield与resume
static int nty_epoll_inner(struct epoll_event *ev, int ev_num, int timeout) {
    if (timeout < 0) {
        timeout = INT_MAX;
    }
    nty_schedule * sched = nty_coroutine_get_sched();
    nty_coroutine *co = sched->curr_thread;
    int i;
    for (i = 0; i < ev_num; i++) {
        epoll_ctl(sched->epfd, EPOLL_CTL_ADD, ev->data.fd, ev);
        co->events = ev->events;
        //加入wait集合，添加wait状态
        nty_schedule_sched_wait(co, ev->data.fd, ev->events, timeout);
    }
    //yield
    nty_coroutine_yield(co);
    for (i = 0; i < ev_num; i++) {
        epoll_ctl(sched->epfd, EPOLL_CTL_DEL, ev->data.fd, ev);
        //移除wait集合，移除wait状态
        nty_schedule_desched_wait(ev->data.fd);
    }
    return ev_num;
}

// 创建非阻塞的socket
int nty_socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if (fd == -1) {
        printf("Failed to create a new socket\n");
        return -1;
    }
    int ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        close(ret);
        return -1;
    }
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));
    return fd;
}

// 创建协程accept接口
int nty_accept(int fd, struct sockaddr *addr, socklen_t *len) {
    int sockfd;
    while (1) {
        struct epoll_event ev;
        ev.events = POLLIN | POLLERR | POLLHUP;
        ev.data.fd = fd;
        //加入epoll，然后yield
        nty_epoll_inner(&ev, 1, 1);
        //resume
        sockfd = accept(fd, addr, len);
        if (sockfd < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            else if (errno == ECONNABORTED) {
                printf("accept : ECONNABORTED\n");
            }
            else if (errno == EMFILE || errno == ENFILE) {
                printf("accept : EMFILE || ENFILE\n");
            }
            return -1;
        }
        else {
            break;
        }
    }
    int ret = fcntl(sockfd, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        close(sockfd);
        return -1;
    }
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));
    return sockfd;
}

// 创建协程recv接口
ssize_t nty_recv(int fd, void *buf, size_t len, int flags) {
    struct epoll_event ev;
    ev.events = POLLIN | POLLERR | POLLHUP;
    ev.data.fd = fd;
    //加入epoll，然后yield
    nty_epoll_inner(&ev, 1, 1);
    //resume
    ssize_t ret = recv(fd, buf, len, flags);
    return ret;
}

// 创建协程send接口
ssize_t nty_send(int fd, const void *buf, size_t len, int flags) {
    int sent = 0;
    int ret = send(fd, ((char *) buf) + sent, len - sent, flags);
    if (ret == 0) return ret;
    if (ret > 0) sent += ret;
    while (sent < len) {
        struct epoll_event ev;
        ev.events = POLLOUT | POLLERR | POLLHUP;
        ev.data.fd = fd;
        //加入epoll，然后yield
        nty_epoll_inner(&ev, 1, 1);
        ret = send(fd, ((char *) buf) + sent, len - sent, flags);
        //printf("send --> len : %d\n", ret);
        if (ret <= 0) {
            break;
        }
        sent += ret;
    }
    if (ret <= 0 && sent == 0) return ret;
    return sent;
}

// 创建协程close接口
int nty_close(int fd) {
    return close(fd);
}

// 创建协程connect接口
int nty_connect(int fd, struct sockaddr *name, socklen_t len) {
    int ret = 0;
    while (1) {
        struct epoll_event ev;
        ev.events = POLLOUT | POLLERR | POLLHUP;
        ev.data.fd = fd;
        //加入epoll，然后yield
        nty_epoll_inner(&ev, 1, 1);
        //resume
        ret = connect(fd, name, len);
        if (ret == 0) break;
        if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)) {
            continue;
        }
        else {
            break;
        }
    }
    return ret;
}
// 创建协程sendto接口
ssize_t nty_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    int sent = 0;
    while (sent < len) {
        struct epoll_event ev;
        ev.events = POLLOUT | POLLERR | POLLHUP;
        ev.data.fd = fd;
        //加入epoll，然后yield
        nty_epoll_inner(&ev, 1, 1);
        int ret = sendto(fd, ((char*)buf)+sent, len-sent, flags, dest_addr, addrlen);
        if (ret <= 0) {
            if (errno == EAGAIN) continue;
            else if (errno == ECONNRESET) {
                return ret;
            }
            printf("send errno : %d, ret : %d\n", errno, ret);
            assert(0);
        }
        sent += ret;
    }
    return sent;
}
// 创建协程srecvfrom接口
ssize_t nty_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    struct epoll_event ev;
    ev.events = POLLIN | POLLERR | POLLHUP;
    ev.data.fd = fd;
    //加入epoll，然后yield
    nty_epoll_inner(&ev, 1, 1);

    int ret = recvfrom(fd, buf, len, flags, src_addr, addrlen);
    if (ret < 0) {
        if (errno == EAGAIN) {
            return ret;
        }
        if (errno == ECONNRESET) {
            return 0;
        }
        printf("recv error : %d, ret : %d\n", errno, ret);
        assert(0);
    }
    return ret;
}
