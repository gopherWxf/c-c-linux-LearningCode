#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define BUFFER_LENGTH           4096
#define MAX_EPOLL_EVENTS        1024
#define SERVER_PORT             8081
#define PORT_COUNT              100
#define    GUID    "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef int (*NCALLBACK)(int, int, void *);

struct ntyevent {
    int fd;
    int events;
    void *arg;

    NCALLBACK callback;

    int status;
    char buffer[BUFFER_LENGTH];
    int length;

    int state_machine;
};
//state_machine
enum {
    WS_HANDSHAKE = 0,
    WS_TRANSMISSION = 1,
    WS_END = 2
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

int base64_encode(char *in_str, int in_len, char *out_str) {
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
    size_t size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in_str, in_len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    out_str[bptr->length - 1] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}

int readline(char *all_buffer, int idx, char *line_buffer) {
    int len = strlen(all_buffer);
    for (; idx < len; idx++) {
        if (all_buffer[idx] == '\r' && all_buffer[idx + 1] == '\n') {
            return idx + 2;
        }
        else {
            *(line_buffer++) = all_buffer[idx];
        }
    }
    return -1;
}

int handshake(struct ntyevent *ev) {
    char line_buffer[1024] = {0};
    char sha_key[32] = {0};//实际只需20B
    char sec_key[32] = {0};//实际只需28B
    int idx = 0;
    //找到Sec-WebSocket-Key这一行
    while (!strstr(line_buffer, "Sec-WebSocket-Key")) {
        memset(line_buffer, 0, 1024);
        idx = readline(ev->buffer, idx, line_buffer);
        if (idx == -1)return -1;
    }
    //1. key=KEY+GUID
    //2. sha_key=SHA1(key)
    //3. sec_key=base64_encode(sha_key)

    strcpy(line_buffer, line_buffer + strlen("Sec-WebSocket-Key: "));
    //1. key=KEY+GUID
    strcat(line_buffer, GUID);
    //2.sha_key = SHA1(key)
    SHA1(line_buffer, strlen(line_buffer), sha_key);
    //3. sec_key=base64_encode(sha_key)
    base64_encode(sha_key, strlen(sha_key), sec_key);
    //set head
    memset(ev->buffer, 0, BUFFER_LENGTH);
    ev->length = sprintf(ev->buffer, "HTTP/1.1 101 Switching Protocols\r\n"
                                     "Upgrade: websocket\r\n"
                                     "Connection: Upgrade\r\n"
                                     "Sec-WebSocket-Accept: %s\r\n\r\n", sec_key);

    printf("[handshake response]\n%s\n", ev->buffer);
    return 0;
}

//暂时的小端
typedef struct _ws_ophdr {
    unsigned char opcode: 4,
            rsv3: 1,
            rsv2: 1,
            rsv1: 1,
            fin: 1;
    unsigned char payload_len: 7,
            mask: 1;
} ws_ophdr;

typedef struct _ws_ophdr126 {
    unsigned short payload_len;
    char mask_key[4];
} ws_ophdr126;

typedef struct _ws_ophdr127 {
    long long payload_len;
    char mask_key[4];
} ws_ophdr127;

void umask(char *payload, int length, char *mask_key) {
    int i = 0;
    for (i = 0; i < length; i++) {
        payload[i] ^= mask_key[i % 4];
    }
}

char *decode_packet(struct ntyevent *ev, int *real_len, int *virtual_len) {
    ws_ophdr *hdr = (ws_ophdr *) ev->buffer;
    printf("decode_packet fin:%d rsv1:%d rsv2:%d rsv3:%d opcode:%d mark:%d\n",
           hdr->fin,
           hdr->rsv1,
           hdr->rsv2,
           hdr->rsv3,
           hdr->opcode,
           hdr->mask);
    char *payload = NULL;
    *virtual_len = hdr->payload_len;
    if (hdr->opcode == 8) {
        ev->state_machine = WS_END;
        close(ev->fd);
        return NULL;
    }

    if (hdr->payload_len < 126) {
        payload = ev->buffer + sizeof(ws_ophdr) + 4; // 6  payload length < 126
        if (hdr->mask) {
            umask(payload, hdr->payload_len, ev->buffer + 2);
        }
        *real_len = hdr->payload_len;
    }
    else if (hdr->payload_len == 126) {
        payload = ev->buffer + sizeof(ws_ophdr) + sizeof(ws_ophdr126);
        ws_ophdr126 *hdr126 = (ws_ophdr126 *) (ev->buffer + sizeof(ws_ophdr));
        hdr126->payload_len = ntohs(hdr126->payload_len);
        if (hdr->mask) {
            umask(payload, hdr126->payload_len, hdr126->mask_key);
        }
        *real_len = hdr126->payload_len;
    }
    else if (hdr->payload_len == 127) {
        payload = ev->buffer + sizeof(ws_ophdr) + sizeof(ws_ophdr127);
        ws_ophdr127 *hdr127 = (ws_ophdr127 *) (ev->buffer + sizeof(ws_ophdr));
        if (hdr->mask) {
            umask(payload, hdr127->payload_len, hdr127->mask_key);
        }
        *real_len = hdr127->payload_len;
    }
    printf("virtual len=%d  real_len=%d\n", hdr->payload_len, *real_len);
    return payload;
}

int encode_packet(struct ntyevent *ev, int real_len, int virtual_len, char *buf) {
    ws_ophdr head = {0};
    head.fin = 1;
    head.opcode = 1;
    head.payload_len = virtual_len;
    memcpy(ev->buffer, &head, sizeof(ws_ophdr));

    int head_offset = 0;
    if (virtual_len < 126) {
        head.payload_len = real_len;
        head_offset = sizeof(ws_ophdr);
    }
    else if (virtual_len == 126) {
        ws_ophdr126 hdr126 = {0};
        hdr126.payload_len = htons(real_len);
        memcpy(ev->buffer + sizeof(ws_ophdr), &hdr126, sizeof(unsigned short));//返回不需要mask，中间去掉4B
        head_offset = sizeof(ws_ophdr) + sizeof(unsigned short);
    }
    else if (virtual_len == 127) {
        ws_ophdr127 hdr127 = {0};
        hdr127.payload_len = real_len;
        memcpy(ev->buffer + sizeof(ws_ophdr), &hdr127, sizeof(long long));//返回不需要mask，中间去掉4B
        head_offset = sizeof(ws_ophdr) + sizeof(long long);
    }
    printf("encode_packet fin:%d rsv1:%d rsv2:%d rsv3:%d opcode:%d mark:%d \n",
           head.fin,
           head.rsv1,
           head.rsv2,
           head.rsv3,
           head.opcode,
           head.mask);
    memcpy(ev->buffer + head_offset, buf, real_len);
    return head_offset + real_len;//头+payload
}

int transmission(struct ntyevent *ev) {
    char *payload_buffer = NULL;
    int real_len = 0, virtual_len;
    payload_buffer = decode_packet(ev, &real_len, &virtual_len);

    printf("real_len=[%d] , buf=[%s]\n", real_len, payload_buffer);

    ev->length = encode_packet(ev, real_len, virtual_len, payload_buffer);
}

int websocket_request(struct ntyevent *ev) {
    if (ev->state_machine == WS_HANDSHAKE) {
        handshake(ev);
        ev->state_machine = WS_TRANSMISSION;
    }
    else if (ev->state_machine == WS_TRANSMISSION) {
        transmission(ev);
    }
    else {

    }
}

int recv_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);
    memset(ev->buffer, 0, BUFFER_LENGTH);
#if 0
    long len = recv(fd, ev->buffer, BUFFER_LENGTH, 0); //
#elif 1
    int len = 0;
    int n = 0;
    while (1) {
        n = recv(fd, ev->buffer + len, BUFFER_LENGTH - len, 0);
        printf("[recv data len = %d]\n", n);
        if (n != -1) {
            len += n;
            usleep(10000);
        }
        else {
            break;
        }
    }
#endif

    nty_event_del(reactor->epfd, ev);
    printf("[recv buffer total len=%d]\n", len);
    printf("buffer:[%s]\n", ev->buffer);
    if (len > 0) {
        ev->length = len;
        ev->buffer[len] = '\0';

        websocket_request(ev);

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


int send_cb(int fd, int events, void *arg) {
    struct ntyreactor *reactor = (struct ntyreactor *) arg;
    struct ntyevent *ev = ntyreactor_find_event_idx(reactor, fd);
    printf("[send buffer]\n%s\n", ev->buffer);

    int len = send(fd, ev->buffer, ev->length, 0);
    if (len > 0) {
        nty_event_del(reactor->epfd, ev);
        nty_event_set(ev, fd, recv_cb, reactor);
        nty_event_add(reactor->epfd, EPOLLIN, ev);
    }
    else {
        nty_event_del(reactor->epfd, ev);
        close(ev->fd);
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
    event->status = WS_HANDSHAKE;
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



