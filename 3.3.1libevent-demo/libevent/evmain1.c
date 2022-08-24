#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include <netinet/in.h>
#include "event2/event.h"

void socket_read_cb(int fd, short events, void *arg);

void socket_accept_cb(int fd, short events, void *arg) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    evutil_socket_t clientfd = accept(fd, (struct sockaddr *) &addr, &len);
    evutil_make_socket_nonblocking(clientfd);
    printf("accept a client %d\n", clientfd);
    struct event_base *base = (struct event_base *) arg;
    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(ev, base, clientfd, EV_READ | EV_PERSIST,
                 socket_read_cb, (void *) ev);
    event_add(ev, NULL);
}

void socket_read_cb(int fd, short events, void *arg) {
    char msg[4096];
    struct event *ev = (struct event *) arg;
    int len = read(fd, msg, sizeof(msg) - 1);
    if (len <= 0) {
        printf("client fd:%d disconnect\n", fd);
        event_free(ev);
        close(fd);
        return;
    }

    msg[len] = '\0';
    printf("recv the client msg: %s", msg);

    char reply_msg[4096] = "recvieced msg: ";
    strcat(reply_msg + strlen(reply_msg), msg);
    write(fd, reply_msg, strlen(reply_msg));
}

int socket_listen(int port) {
    int errno_save;

    evutil_socket_t listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        return -1;

    evutil_make_listen_socket_reuseable(listenfd);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        evutil_closesocket(listenfd);
        return -1;
    }

    if (listen(listenfd, 5) < 0) {
        evutil_closesocket(listenfd);
        return -1;
    }

    evutil_make_socket_nonblocking(listenfd);

    return listenfd;
}

int main(int argc, char **argv) {
    int listenfd = socket_listen(8080);
    if (listenfd == -1) {
        printf("socket_listen error\n");
        return -1;
    }
    struct event_base *base = event_base_new();

    struct event *ev_listen = event_new(base, listenfd, EV_READ | EV_PERSIST,
                                        socket_accept_cb, base);
    /*
    event_new 等价于
    struct event ev_listen;
    event_set(&ev_listen, listenfd, EV_READ | EV_PERSIST, socket_accept_cb, base);
    event_base_set(base, &ev_listen);
    */

    event_add(ev_listen, NULL);

    event_base_dispatch(base);
    return 0;
}

/*
gcc evmain1.c -o ev1 -levent
client:
    telnet 127.0.0.1 8080
*/
