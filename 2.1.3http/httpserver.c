#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#define BUFFER_LENGTH           4096
#define MAX_EPOLL_EVENTS        1024
#define SERVER_PORT             8080
#define PORT_COUNT              100
#define HTTP_METHOD_GET        0
#define HTTP_METHOD_POST    1
#define HTTP_WEBSERVER_HTML_ROOT    "html"

typedef int (*NCALLBACK)(int, int, void *);

struct ntyevent {
    int fd;
    int events;
    void *arg;

    NCALLBACK callback;

    int status;
    char buffer[BUFFER_LENGTH];
    int length;

    //http param
    int method; //
    char resource[BUFFER_LENGTH];
    int ret_code;
    char Content_Type[512];
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

void nty_event_set(struct ntyevent *ev, int fd, NCALLBACK callback, void *arg) {
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

int readline(char *allbuf, int idx, char *linebuf) {
    uint len = strlen(allbuf);
    for (; idx < len; idx++) {
        if (allbuf[idx] == '\r' && allbuf[idx + 1] == '\n') {
            return idx + 2;
        }
        else {
            *(linebuf++) = allbuf[idx];
        }
    }
    return -1;
}

int http_request(struct ntyevent *ev) {
    char line_buffer[1024] = {0};
    readline(ev->buffer, 0, line_buffer);
    if (strstr(line_buffer, "GET")) {
        ev->method = HTTP_METHOD_GET;
        //uri
        int i = 0;
        while (line_buffer[sizeof("GET ") + i] != ' ') i++;
        line_buffer[sizeof("GET ") + i] = '\0';
        sprintf(ev->resource, "./%s/%s", HTTP_WEBSERVER_HTML_ROOT, line_buffer + sizeof("GET "));
        //Content-Type
        if (strstr(line_buffer + sizeof("GET "), ".")) {
            char *type = strchr(line_buffer + sizeof("GET "), '.') + 1;
            if (strcmp(type, "html") == 0 || strcmp(type, "css") == 0) {
                sprintf(ev->Content_Type, "text/%s", type);
            }
            else if (strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "ico") == 0) {
                sprintf(ev->Content_Type, "image/%s", type);
            }
        }
        else {
            sprintf(ev->Content_Type, "text/html");
        }
    }
}

int recv_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);
    int len = recv(fd, ev->buffer, BUFFER_LENGTH, 0); //
    nty_event_del(reactor->epfd, ev);

    if (len > 0) {
        ev->length = len;
        ev->buffer[len] = '\0';
        printf("[socket fd=%d recv]\n%s\n", fd, ev->buffer); //http

        http_request(ev);
        nty_event_set(ev, fd, send_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLOUT, ev);
    }
    else if (len == 0) {
        close(ev->fd);
    }
    else {
        close(ev->fd);
    }
    return len;
}

int http_response(struct ntyevent *ev) {
    if (ev == NULL) return -1;
    memset(ev->buffer, 0, BUFFER_LENGTH);
    int filefd = open(ev->resource, O_RDONLY);
    if (filefd == -1) { // return 404
        ev->ret_code = 404;
        ev->length = sprintf(ev->buffer,
                             "HTTP/1.1 404 Not Found\r\n"
                             "Content-Type: %s;charset=utf-8\r\n"
                             "Content-Length: 83\r\n\r\n"
                             "<html><head><title>404 Not Found</title></head><body><H1>404</H1></body></html>\r\n\r\n",
                             ev->Content_Type);
    }
    else {
        struct stat stat_buf;
        fstat(filefd, &stat_buf);
        close(filefd);

        if (S_ISDIR(stat_buf.st_mode)) {
            ev->ret_code = 404;
            ev->length = sprintf(ev->buffer,
                                 "HTTP/1.1 404 Not Found\r\n"
                                 "Content-Type: %s;charset=utf-8\r\n"
                                 "Content-Length: 83\r\n\r\n"
                                 "<html><head><title>404 Not Found</title></head><body><H1>404</H1></body></html>\r\n\r\n",
                                 ev->Content_Type);
        }
        else if (S_ISREG(stat_buf.st_mode)) {
            ev->ret_code = 200;
            ev->length = sprintf(ev->buffer,
                                 "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: %s;charset=utf-8\r\n"
                                 "Content-Length: %ld\r\n\r\n",
                                 ev->Content_Type, stat_buf.st_size);
        }
    }
    return ev->length;
}

int send_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);

    http_response(ev);
    printf("[socket fd=%d count=%d]\n%s", fd, ev->length, ev->buffer);
    int res_head_len = send(fd, ev->buffer, ev->length, 0);
    if (res_head_len > 0) {
        if (ev->ret_code == 200) {
            int filefd = open(ev->resource, O_RDONLY);
            struct stat stat_buf;
            fstat(filefd, &stat_buf);
            off_t offset = 0;
            while (offset != stat_buf.st_size) {
                int n = sendfile(fd, filefd, &offset, (stat_buf.st_size - offset));
                if (n == -1 && errno == EAGAIN) {
                    usleep(5000);
                    continue;
                }
            }
            printf("[resource: %s count:%ld]\r\n", ev->resource, offset);
            close(filefd);
        }
        nty_event_del(reactor->epfd, ev);
        nty_event_set(ev, fd, recv_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLIN, ev);
    }
    else {
        nty_event_del(reactor->epfd, ev);
        close(ev->fd);
    }
    return 0;
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

int ntyreactor_addlistener(struct ntyreactor *reactor, int sockfd, NCALLBACK acceptor) {
    if (reactor == NULL) return -1;
    if (reactor->evblk == NULL) return -1;

    struct ntyevent *event = ntyreactor_find_event_idx(reactor, sockfd);

    nty_event_set(event, sockfd, acceptor, reactor);
    nty_event_add(reactor->epfd, EPOLLIN, event);
    return 0;
}

int ntyreactor_run(struct ntyreactor *reactor) {
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
    return 0;
}

int main(int argc, char *argv[]) {
    unsigned short port = SERVER_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    struct ntyreactor *reactor = (struct ntyreactor *) malloc(sizeof(struct ntyreactor));
    ntyreactor_init(reactor);
    int i;
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