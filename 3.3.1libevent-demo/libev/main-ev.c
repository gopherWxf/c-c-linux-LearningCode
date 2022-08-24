#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#include <string.h>
#include <time.h>
#include <ev.h>

static void
do_accept(struct ev_loop *reactor, ev_io *w, int events) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(addr);
    int clientfd = accept(w->fd, (struct sockaddr *) &addr, &len);
    if (clientfd != -1)
        close(clientfd);

    printf("accept fd = %d\n", clientfd);
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

static void
do_timer(struct ev_loop *loop, ev_timer *timer_w, int e) {
    time_t now = time(NULL);
    printf("do_timer %s", (char *) ctime(&now));
    // ev_timer_stop(loop, timer_w);
}

int main() {
    struct ev_loop *loop = ev_loop_new(0);

    struct ev_timer t1;
    ev_timer_init(&t1, do_timer, 1, 1);
    ev_timer_start(loop, &t1);

    int listenfd = socket_listen(8989);
    if (listenfd == -1) return -2;
    struct ev_io i1;
    ev_io_init(&i1, do_accept, listenfd, EV_READ);
    ev_io_start(loop, &i1);
    ev_run(loop, 0);

    ev_loop_destroy(loop);
    return 0;
}

/*
gcc main-ev.c -o main-ev -I../libev/ -L../libev/.libs/ -lev
*/
