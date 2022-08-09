#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


#define BUFFER_LENGTH           4096
#define MAX_EPOLL_EVENTS        1024
#define SERVER_PORT             8082

typedef int (*NtyCallBack)(int, int, void *);

struct ntyevent {
    int fd;
    int events;
    void *arg;

    NtyCallBack callback;

    int status;//1 mod o add
    char buffer[BUFFER_LENGTH];
    int length;
    long last_active;
};


struct ntyreactor {
    int epfd;
    struct ntyevent *events;
};


int recv_cb(int fd, int events, void *arg);

int send_cb(int fd, int events, void *arg);

int accept_cb(int fd, int events, void *arg);

void nty_event_set(struct ntyevent *ev, int fd, NtyCallBack callback, void *arg) {
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
    ev->last_active = time(NULL);
}

int nty_event_add(int epfd, int events, struct ntyevent *ntyev) {
    struct epoll_event ev = {0, {0}};
    ev.data.ptr = ntyev;
    ev.events = ntyev->events = events;
    int op;
    if (ntyev->status == 1) {
        op = EPOLL_CTL_MOD;
    }
    else {
        op = EPOLL_CTL_ADD;
        ntyev->status = 1;
    }

    if (epoll_ctl(epfd, op, ntyev->fd, &ev) < 0) {
        printf("event add failed [fd=%d], events[%d],err:%s,err:%d\n", ntyev->fd, events,strerror(errno),errno);
        return -1;
    }
    return 0;
}

int nty_event_del(int epfd, struct ntyevent *ev) {
    struct epoll_event ep_ev = {0, {0}};
    if (ev->status != 1) {
        return -1;
    }
    ep_ev.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ep_ev);
    //epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, NULL);
    return 0;
}

int recv_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ntyev = &reactor->events[fd];

    int len = recv(fd, ntyev->buffer, BUFFER_LENGTH, 0);
    nty_event_del(reactor->epfd, ntyev);

    if (len > 0) {
        ntyev->length = len;
        ntyev->buffer[len] = '\0';

        printf("C[%d]:%s\n", fd, ntyev->buffer);

        nty_event_set(ntyev, fd, send_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLOUT, ntyev);
    }
    else if (len == 0) {
        close(ntyev->fd);
        printf("[fd=%d] pos[%ld], closed\n", fd, ntyev - reactor->events);
    }
    else {
        close(ntyev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));

    }

    return len;
}

int send_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ntyev = &reactor->events[fd];

    int len = send(fd, ntyev->buffer, ntyev->length, 0);
    if (len > 0) {
        printf("send[fd=%d], [%d]%s\n", fd, len, ntyev->buffer);
        nty_event_del(reactor->epfd, ntyev);
        nty_event_set(ntyev, fd, recv_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLIN, ntyev);
    }
    else {
        close(ntyev->fd);
        nty_event_del(reactor->epfd, ntyev);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }
    return len;
}

int accept_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    if (reactor == NULL) return -1;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int clientfd;

    if ((clientfd = accept(fd, (struct sockaddr *) &client_addr, &len)) == -1) {
        printf("accept: %s\n", strerror(errno));
        return -1;
    }
    printf("client fd = %d\n",clientfd);
    int flag = 0;
    if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
        return -1;
    }
    nty_event_set(&reactor->events[clientfd], clientfd, recv_cb, reactor);
    nty_event_add(reactor->epfd, EPOLLIN, &reactor->events[clientfd]);

    printf("new connect [%s:%d][time:%ld], pos[%d]\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), reactor->events[clientfd].last_active, clientfd);

    return 0;
}

int init_sock(short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (listen(fd, 20) < 0) {
        printf("listen failed : %s\n", strerror(errno));
    }
    return fd;
}


int ntyreactor_init(struct ntyreactor *reactor) {
    if (reactor == NULL) return -1;
    memset(reactor, 0, sizeof(struct ntyreactor));
    reactor->epfd = epoll_create(1);
    printf("ep fd=%d\n",reactor->epfd);
    if (reactor->epfd <= 0) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        return -2;
    }
    reactor->events = (struct ntyevent *) malloc((MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));
    memset(reactor->events, 0, (MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));

    if (reactor->events == NULL) {
        printf("create epll events in %s err %s\n", __func__, strerror(errno));
        close(reactor->epfd);
        return -3;
    }
    return 0;
}

int ntyreactor_destory(struct ntyreactor *reactor) {
    close(reactor->epfd);
    free(reactor->events);
}

int ntyreactor_addlistener(struct ntyreactor *reactor, int sockfd, NtyCallBack acceptor) {
    if (reactor == NULL) return -1;
    if (reactor->events == NULL) return -1;
    printf("listen sockfd=%d\n", sockfd);
    nty_event_set(&reactor->events[sockfd], sockfd, acceptor, reactor);
    nty_event_add(reactor->epfd, EPOLLIN, &reactor->events[sockfd]);
    return 0;
}

int ntyreactor_run(struct ntyreactor *reactor) {
    if (reactor == NULL) return -1;
    if (reactor->epfd < 0) return -1;
    if (reactor->events == NULL) return -1;

    struct epoll_event events[MAX_EPOLL_EVENTS];
    int checkpos = 0, i;

    while (1) {
        //心跳包 60s 超时则断开连接
        long now = time(NULL);
        for (i = 0; i < 100; i++, checkpos++) {
            if (checkpos == MAX_EPOLL_EVENTS) {
                checkpos = 0;
            }
            if (reactor->events[checkpos].status != 1 || checkpos == 3) {
                continue;
            }
            long duration = now - reactor->events[checkpos].last_active;

            if (duration >= 60) {
                close(reactor->events[checkpos].fd);
                printf("[fd=%d] timeout\n", reactor->events[checkpos].fd);
                nty_event_del(reactor->epfd, &reactor->events[checkpos]);
            }
        }


        int nready = epoll_wait(reactor->epfd, events, MAX_EPOLL_EVENTS, 1000);
        if (nready < 0) {
            printf("epoll_wait error, exit\n");
            continue;
        }
        for (i = 0; i < nready; i++) {
            struct ntyevent *ev = (struct ntyevent *) events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
                ev->callback(ev->fd, events[i].events, ev->arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
                ev->callback(ev->fd, events[i].events, ev->arg);
            }
        }
    }
}

int main123(int argc, char *argv[]) {
    int sockfd = init_sock(SERVER_PORT);

    struct ntyreactor *reactor = (struct ntyreactor *) malloc(sizeof(struct ntyreactor));
    if (ntyreactor_init(reactor) != 0) {
        return -1;
    }

    ntyreactor_addlistener(reactor, sockfd, accept_cb);
    ntyreactor_run(reactor);

    ntyreactor_destory(reactor);
    close(sockfd);
    return 0;
}



