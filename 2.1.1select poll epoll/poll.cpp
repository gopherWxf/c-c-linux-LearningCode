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

#define MAX_LEN  4096
#define POLL_SIZE    1024

int main(int argc, char **argv) {
    int listenfd, connfd, n;
    struct sockaddr_in svr_addr;
    char buff[MAX_LEN];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(8081);

    if (bind(listenfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    //poll
    struct pollfd fds[POLL_SIZE] = {0};
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    int max_fd = listenfd;
    int i = 0;
    for (i = 1; i < POLL_SIZE; i++) {
        fds[i].fd = -1;
    }
    while (1) {
        int nready = poll(fds, max_fd + 1, -1);

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client = {};
            socklen_t len = sizeof(client);
            if ((connfd = accept(listenfd, (struct sockaddr *) &client, &len)) == -1) {
                printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
                return 0;
            }
            printf("accept \n");
            fds[connfd].fd = connfd;
            fds[connfd].events = POLLIN;
            if (connfd > max_fd) max_fd = connfd;
            if (--nready == 0) continue;
        }
        //int i = 0;
        for (i = listenfd + 1; i <= max_fd; i++) {
            if (fds[i].revents & POLLIN) {
                n = recv(i, buff, MAX_LEN, 0);
                if (n > 0) {
                    buff[n] = '\0';
                    printf("recv msg from client: %s\n", buff);
                    send(i, buff, n, 0);
                }
                else if (n == 0) { //
                    fds[i].fd = -1;
                    close(i);
                }
                if (--nready == 0) break;
            }
        }
    }
}

