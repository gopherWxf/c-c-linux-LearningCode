#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string.h>
#include <time.h>
#include "event.h"

static struct event evtimer;

static void
do_accept(int fd, short events, void *arg) {
    struct event_base *ev = (struct event_base *) arg;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(addr);
    int clientfd = accept(fd, (struct sockaddr *) &addr, &len);
    if (clientfd != -1)
        close(clientfd);

    printf("accept fd = %d\n", clientfd);
}

static void
do_timer(int fd, short events, void *arg) {
    time_t now = time(NULL);
    printf("do_timer %s", (char *) ctime(&now));
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    event_add(&evtimer, &tv);
}

int socket_listen(uint16_t port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
        return -1;

    if (listen(listenfd, 5) == -1)
        return -1;

    printf("server start listening port:%d\n", port);

    return listenfd;
}

int main() {

    struct event_base *ev = event_init();

    struct event evlisten;

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int listenfd = socket_listen(8989);
    if (listenfd == -1) return -2;

    event_set(&evlisten, listenfd, EV_READ | EV_PERSIST, do_accept, ev);
    event_base_set(ev, &evlisten);
    event_add(&evlisten, NULL);

    event_set(&evtimer, -1, 0, do_timer, ev);
    event_base_set(ev, &evtimer);
    event_add(&evtimer, &tv);

    event_base_dispatch(ev);

    event_base_free(ev);
    return 0;
}

/*
// gcc main-event.c -o main-event -I../libev/ -L../libev/.libs/ -lev
*/

