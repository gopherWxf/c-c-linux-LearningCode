#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_LENGTH           4096
#define MAX_EPOLL_EVENTS        1024
#define SERVER_PORT             8081
#define PORT_COUNT              100

typedef int (*NCALLBACK)(int, int, void *);

struct ntyevent {
    int fd;
    int events;
    void *arg;

    NCALLBACK callback;

    int status;
    char buffer[BUFFER_LENGTH];
    int length;
};
struct eventblock {
    struct eventblock *next;
    struct ntyevent *events;
};

struct ntyreactor {
    int epfd;
    int blkcnt;
    struct eventblock *evblk;
};


int recv_cb(int fd, int events, void *arg);

int send_cb(int fd, int events, void *arg);

struct ntyevent *ntyreactor_find_event_idx(struct ntyreactor *reactor, int sockfd);

void nty_event_set(struct ntyevent *ev, int fd, NCALLBACK *callback, void *arg) {
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
}

int nty_event_add(int epfd, int events, struct ntyevent *ev) {
    struct epoll_event ep_ev = {0, {0}};
    ep_ev.data.ptr = ev;
    ep_ev.events = ev->events = events;
    int op;
    if (ev->status == 1) {
        op = EPOLL_CTL_MOD;
    }
    else {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }
    if (epoll_ctl(epfd, op, ev->fd, &ep_ev) < 0) {
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
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
    return 0;
}

int recv_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);
    int len = recv(fd, ev->buffer, BUFFER_LENGTH, 0); //
    nty_event_del(reactor->epfd, ev);

    if (len > 0) {
        ev->length = len;
        ev->buffer[len] = '\0';
//        printf("recv[%d]:%s\n", fd, ev->buffer);
        printf("recv fd=[%d\n", fd);

        nty_event_set(ev, fd, send_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLOUT, ev);
    }
    else if (len == 0) {
        close(ev->fd);
        //printf("[fd=%d] pos[%ld], closed\n", fd, ev-reactor->events);
    }
    else {
        close(ev->fd);
//        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }
    return len;
}


int send_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);

    int len = send(fd, ev->buffer, ev->length, 0);
    if (len > 0) {
//        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);
        printf("send fd=[%d\n", fd);

        nty_event_del(reactor->epfd, ev);
        nty_event_set(ev, fd, recv_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLIN, ev);
    }
    else {
        nty_event_del(reactor->epfd, ev);
        close(ev->fd);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }
    return len;
}

int accept_cb(int fd, int events, void *arg) {//非阻塞
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    if (reactor == NULL) return -1;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int clientfd;
    if ((clientfd = accept(fd, (struct sockaddr *) &client_addr, &len)) == -1) {
        printf("accept: %s\n", strerror(errno));
        return -1;
    }
    if ((fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
        return -1;
    }
    struct ntyevent *event = ntyreactor_find_event_idx(reactor, clientfd);

    nty_event_set(event, clientfd, recv_cb, reactor);
    nty_event_add(reactor->epfd, EPOLLIN, event);

    printf("new connect [%s:%d], pos[%d]\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), clientfd);
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


int ntyreactor_alloc(struct ntyreactor *reactor) {
    if (reactor == NULL) return -1;
    if (reactor->evblk == NULL) return -1;

    struct eventblock *blk = reactor->evblk;
    while (blk->next != NULL) {
        blk = blk->next;
    }

    struct ntyevent *evs = (struct ntyevent *) malloc((MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));
    if (evs == NULL) {
        printf("ntyreactor_alloc ntyevents failed\n");
        return -2;
    }
    memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));

    struct eventblock *block = (struct eventblock *) malloc(sizeof(struct eventblock));
    if (block == NULL) {
        printf("ntyreactor_alloc eventblock failed\n");
        return -2;
    }
    memset(block, 0, sizeof(struct eventblock));

    block->events = evs;
    block->next = NULL;

    blk->next = block;
    reactor->blkcnt++; //
    return 0;
}

struct ntyevent *ntyreactor_find_event_idx(struct ntyreactor *reactor, int sockfd) {
    int blkidx = sockfd / MAX_EPOLL_EVENTS;

    while (blkidx >= reactor->blkcnt) {
        ntyreactor_alloc(reactor);
    }
    int i = 0;
    struct eventblock *blk = reactor->evblk;
    while (i++ < blkidx && blk != NULL) {
        blk = blk->next;
    }
    return &blk->events[sockfd % MAX_EPOLL_EVENTS];
}


int ntyreactor_init(struct ntyreactor *reactor) {
    if (reactor == NULL) return -1;
    memset(reactor, 0, sizeof(struct ntyreactor));

    reactor->epfd = epoll_create(1);
    if (reactor->epfd <= 0) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        return -2;
    }

    struct ntyevent *evs = (struct ntyevent *) malloc((MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));
    if (evs == NULL) {
        printf("ntyreactor_alloc ntyevents failed\n");
        return -2;
    }
    memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));

    struct eventblock *block = (struct eventblock *) malloc(sizeof(struct eventblock));
    if (block == NULL) {
        printf("ntyreactor_alloc eventblock failed\n");
        return -2;
    }
    memset(block, 0, sizeof(struct eventblock));

    block->events = evs;
    block->next = NULL;

    reactor->evblk = block;
    reactor->blkcnt = 1;
    return 0;
}

int ntyreactor_destory(struct ntyreactor *reactor) {
    close(reactor->epfd);
    //free(reactor->events);

    struct eventblock *blk = reactor->evblk;
    struct eventblock *blk_next = NULL;

    while (blk != NULL) {
        blk_next = blk->next;
        free(blk->events);
        free(blk);
        blk = blk_next;
    }
    return 0;
}


int ntyreactor_addlistener(struct ntyreactor *reactor, int sockfd, NCALLBACK *acceptor) {
    if (reactor == NULL) return -1;
    if (reactor->evblk == NULL) return -1;

    struct ntyevent *event = ntyreactor_find_event_idx(reactor, sockfd);

    nty_event_set(event, sockfd, acceptor, reactor);
    nty_event_add(reactor->epfd, EPOLLIN, event);
    return 0;
}


_Noreturn int ntyreactor_run(struct ntyreactor *reactor) {
    if (reactor == NULL) return -1;
    if (reactor->epfd < 0) return -1;
    if (reactor->evblk == NULL) return -1;

    struct epoll_event events[MAX_EPOLL_EVENTS + 1];

    int i;

    while (1) {
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


// <remoteip, remoteport, localip, localport,protocol>
int main(int argc, char *argv[]) {
    unsigned short port = SERVER_PORT; // listen 8081
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    struct ntyreactor *reactor = (struct ntyreactor *) malloc(sizeof(struct ntyreactor));
    ntyreactor_init(reactor);
    int i = 0;
    int sockfds[PORT_COUNT] = {0};
    for (i = 0; i < PORT_COUNT; i++) {
        sockfds[i] = init_sock(port + i);
        ntyreactor_addlistener(reactor, sockfds[i], accept_cb);
    }
    ntyreactor_run(reactor);
    ntyreactor_destory(reactor);
    for (i = 0; i < PORT_COUNT; i++) {
        close(sockfds[i]);
    }
    free(reactor);
    return 0;
}



