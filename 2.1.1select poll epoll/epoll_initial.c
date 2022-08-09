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

#define POLL_SIZE 1024
#define MAX_LEN  4096

int main(int argc, char **argv) {
    int listenfd, connfd, n;
    char buff[MAX_LEN];
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(8081);

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

    int epfd = epoll_create(1); //int size

    struct epoll_event events[POLL_SIZE] = {0};
    struct epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    while (1) {
        int nready = epoll_wait(epfd, events, POLL_SIZE, 5);
        if (nready == -1) {
            continue;
        }
        int i = 0;
        for (i = 0; i < nready; i++) {
            int actFd = events[i].data.fd;
            if (actFd == listenfd) {
                struct sockaddr_in cli_addr;
                socklen_t len = sizeof(cli_addr);
                if ((connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &len)) == -1) {
                    printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
                    return 0;
                }
                printf("accept\n");
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (events[i].events & EPOLLIN) {
                n = recv(actFd, buff, MAX_LEN, 0);
                if (n > 0) {
                    buff[n] = '\0';
                    printf("recv msg from client: %s\n", buff);

                    send(actFd, buff, n, 0);
                }
                else if (n == 0) { //
                    epoll_ctl(epfd, EPOLL_CTL_DEL, actFd, NULL);
                    close(actFd);
                }
            }
        }
    }
    return 0;
}




















































































