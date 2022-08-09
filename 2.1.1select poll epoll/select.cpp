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
    //select
    fd_set rfds, rset, wfds, wset;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(listenfd, &rfds);
    int max_fd = listenfd;

    while (1) {
        rset = rfds;
        wset = wfds;
        int nready = select(max_fd + 1, &rset, &wset, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) { //
            struct sockaddr_in clt_addr;
            socklen_t len = sizeof(clt_addr);
            if ((connfd = accept(listenfd, (struct sockaddr *) &clt_addr, &len)) == -1) {
                printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
                return 0;
            }
            FD_SET(connfd, &rfds);
            if (connfd > max_fd) max_fd = connfd;
            if (--nready == 0) continue;
        }
        int i = 0;
        for (i = listenfd + 1; i <= max_fd; i++) {
            if (FD_ISSET(i, &rset)) { //
                n = recv(i, buff, MAX_LEN, 0);
                if (n > 0) {
                    buff[n] = '\0';
                    printf("recv msg from client: %s\n", buff);
                    FD_SET(i, &wfds);
                }
                else if (n == 0) { //
                    FD_CLR(i, &rfds);
                    close(i);
                }
                if (--nready == 0) break;
            }
            else if (FD_ISSET(i, &wset)) {
                send(i, buff, n, 0);
                FD_SET(i, &rfds);
                FD_CLR(i, &wfds);
            }
        }
    }
    close(listenfd);
    return 0;
}
