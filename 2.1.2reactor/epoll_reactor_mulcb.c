#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <pthread.h>

#define POLL_SIZE           1024
#define MAX_LEN             4096
#define BUFFER_LENGTH       1024
#define MAX_EPOLL_EVENT     1024


#define NOSET_CB       0
#define READ_CB        1
#define WRITE_CB       2
#define ACCEPT_CB      3


typedef int (*NtyCallBack)(int fd, int event, void *arg);

int read_callback(int fd, int event, void *arg);

int write_callback(int fd, int event, void *arg);

int accept_callback(int listenfd, int event, void *arg);

struct nitem {//fd
    int fd;
    int events;
    void *arg;

    NtyCallBack read_cb;   // epollin
    NtyCallBack write_cb;  // epollout
    NtyCallBack accept_cb; // epollin

    unsigned char sbuffer[BUFFER_LENGTH];
    int slength;
    unsigned char rbuffer[BUFFER_LENGTH];
    int rlength;
};


struct itemblock {
    struct itemblock *next;
    struct nitem *items;
};
struct reactor {
    int epfd;
    struct itemblock *head;
};

int init_reactor(struct reactor *r);

//单例模式
struct reactor *instance = NULL;

struct reactor *getInstance() {
    if (instance == NULL) {
        instance = malloc(sizeof(struct reactor));
        if (instance == NULL)return NULL;
        memset(instance, 0, sizeof(struct reactor));
        if (init_reactor(instance) < 0) {
            free(instance);
            return NULL;
        }
    }
    return instance;
}

//初始化反应堆
int init_reactor(struct reactor *r) {
    if (r == NULL)return -1;
    int epfd = epoll_create(1);
    r->epfd = epfd;
    r->head = (struct itemblock *) malloc(sizeof(struct itemblock));
    if (r->head == NULL) {
        close(epfd);
        return -2;
    }
    memset(r->head, 0, sizeof(struct itemblock));

    r->head->items = malloc(MAX_EPOLL_EVENT * sizeof(struct nitem));
    if (r->head->items == NULL) {
        free(r->head);
        close(epfd);
        return -2;
    }
    memset(r->head->items, 0, (MAX_EPOLL_EVENT * sizeof(struct nitem)));
    r->head->next = NULL;
    return 0;
}

//初始化服务器
int init_server(int port) {
    int listenfd;
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(port);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (bind(listenfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    printf("init server access\n");
    return listenfd;
}

//反应堆设置事件
int reactor_set_event(int fd, NtyCallBack cb, int event, void *arg) {
    struct reactor *r = getInstance();
    struct epoll_event ev = {0};
    ev.data.ptr = &r->head->items[fd];

    r->head->items[fd].fd = fd;
    r->head->items[fd].arg = arg;

    if (event == READ_CB) {
        r->head->items[fd].read_cb = cb;
        ev.events = EPOLLIN;
    }
    else if (event == WRITE_CB) {
        r->head->items[fd].write_cb = cb;
        ev.events = EPOLLOUT;
    }
    else if (event == ACCEPT_CB) {
        r->head->items[fd].accept_cb = cb;
        ev.events = EPOLLIN;
    }

    if (r->head->items[fd].events == NOSET_CB) {
        if (epoll_ctl(r->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
            printf("epoll_ctl EPOLL_CTL_ADD failed, %d\n", errno);
            return -1;
        }
        r->head->items[fd].events = event;
    }
    else if (r->head->items[fd].events != event) {
        if (epoll_ctl(r->epfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
            printf("epoll_ctl EPOLL_CTL_MOD failed\n");
            return -1;
        }
        r->head->items[fd].events = event;
    }
    printf("reactor_set_event access\n");

    return 0;
}

//反应堆删除事件
int reactor_del_event(int fd, NtyCallBack cb, int event, void *arg) {
    struct reactor *r = getInstance();
    struct epoll_event ev = {0};
    ev.data.ptr = arg;
    epoll_ctl(r->epfd, EPOLL_CTL_DEL, fd, &ev);
    r->head->items[fd].events = NOSET_CB;
    return 0;
}

//反应堆反应
_Noreturn int reactor_loop(int listenfd) {
    struct reactor *r = getInstance();
    struct epoll_event events[POLL_SIZE] = {0};

    while (1) {
        int nready = epoll_wait(r->epfd, events, POLL_SIZE, 5);
        if (nready == -1) {
            continue;
        }
        int i = 0;
        for (i = 0; i < nready; i++) {
            struct nitem *item = (struct nitem *) events[i].data.ptr;
            int actFd = item->fd;
            if (actFd == listenfd) {
                item->accept_cb(actFd, 0, NULL);
            }
            else {
                if (events[i].events & EPOLLIN) {
                    item->read_cb(actFd, 0, NULL);
                }
                if (events[i].events & EPOLLOUT) {
                    item->write_cb(actFd, 0, NULL);
                }
            }

        }
    }
}

//接收的回调
int accept_callback(int listenfd, int event, void *arg) {
    int connfd;
    struct sockaddr_in cli_addr = {0};
    socklen_t len = sizeof(cli_addr);
    if ((connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &len)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    printf("%d\n",connfd);

    reactor_set_event(connfd, read_callback, READ_CB, NULL);
}

//读回调
int read_callback(int fd, int event, void *arg) {
    struct reactor *r = getInstance();
    unsigned char *buffer = r->head->items[fd].rbuffer;
    //LT
    int ret = recv(fd, buffer, BUFFER_LENGTH, 0);
    if (ret == 0) { // fin
        reactor_del_event(fd, NULL, 0, NULL);
        close(fd);
    }
    else if (ret > 0) {
        unsigned char *sbuffer = r->head->items[fd].sbuffer;
        memcpy(sbuffer, buffer, ret);
        r->head->items[fd].slength = ret;
        printf("readcb: %s\n", sbuffer);
        reactor_set_event(fd, write_callback, WRITE_CB, NULL);
    }
}

//写回调
int write_callback(int fd, int event, void *arg) {
    struct reactor *r = getInstance();
    unsigned char *sbuffer = r->head->items[fd].sbuffer;
    int length = r->head->items[fd].slength;

    int ret = send(fd, sbuffer, length, 0);
    if (ret < length) {
        reactor_set_event(fd, write_callback, WRITE_CB, NULL);
    }
    else {
        reactor_set_event(fd, read_callback, READ_CB, NULL);
    }
    return 0;
}

int main1232(int argc, char **argv) {
    int listenfd = init_server(8082);
    if (listenfd == 0) {
        return -1;
    }
    printf("%d\n",listenfd);

    reactor_set_event(listenfd, accept_callback, ACCEPT_CB, NULL);
    reactor_loop(listenfd);
    return 0;
}




















































































