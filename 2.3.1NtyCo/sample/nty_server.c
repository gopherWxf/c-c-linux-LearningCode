//
// Created by 68725 on 2022/7/15.
//
#include <malloc.h>
#include "nty_coroutine.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/poll.h>

#define MAX_CLIENT_NUM            1000000
#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

void server_reader(void *arg) {
    int fd = *(int *) arg;
    int ret = 0;

    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    while (1) {
        char buf[1024] = {0};
        ret = nty_recv(fd, buf, 1024, 0);
        if (ret > 0) {
            if (fd > MAX_CLIENT_NUM) {
                printf("100w connect\n");
                assert(0);
            }
            ret = nty_send(fd, buf, strlen(buf), 0);
            if (ret == -1) {
                nty_close(fd);
                break;
            }
        }
        else if (ret == 0) {
            nty_close(fd);
            break;
        }
    }
}

void server(void *arg) {
    unsigned short port = *(unsigned short *) arg;
    free(arg);
    int fd = nty_socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;

    struct sockaddr_in local, remote;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (struct sockaddr *) &local, sizeof(struct sockaddr_in));
    listen(fd, 128);
    printf("listen port : %d\n", port);

    struct timeval tv_begin;
    gettimeofday(&tv_begin, NULL);

    while (1) {
        socklen_t len = sizeof(struct sockaddr_in);
        int cli_fd = nty_accept(fd, (struct sockaddr *) &remote, &len);
        if (cli_fd % 1000 == 999) {

            struct timeval tv_cur;
            memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));

            gettimeofday(&tv_begin, NULL);
            int time_used = TIME_SUB_MS(tv_begin, tv_cur);

            printf("client fd : %d, time_used: %d\n", cli_fd, time_used);
        }
        nty_coroutine *read_co;
        nty_coroutine_create(&read_co, server_reader, &cli_fd);
    }
}


int main(int argc, char *argv[]) {
    nty_coroutine *co = NULL;
    int i = 0;
    unsigned short base_port = 8080;
    for (i = 0; i < 100; i++) {
        unsigned short *port = calloc(1, sizeof(unsigned short));
        *port = base_port + i;
        nty_coroutine_create(&co, server, port);
    }
    nty_schedule_run(); //run
    return 0;
}
