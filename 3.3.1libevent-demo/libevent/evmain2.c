#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <time.h>
#include <signal.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

void socket_read_callback(struct bufferevent *bev, void *arg) {
    // 操作读缓冲当中的数据
    struct evbuffer *evbuf = bufferevent_get_input(bev); // 封装了读缓冲区
    char *msg = evbuffer_readln(evbuf, NULL, EVBUFFER_EOL_LF);
    // 也可以直接用 bufferevent_read 读数据
    bufferevent_read(
    struct bufferevent *bufev,
    void *data, size_t
    size)
    if (!msg) return;

    printf("server read the data: %s\n", msg);

    char reply[4096] = {0};
    sprintf(reply, "recvieced msg: %s\n", msg);//echo
    // -WRN: 需要自己释放资源
    free(msg);
    bufferevent_write(bev, reply, strlen(reply));
}

// stdio标准输入触发读事件时的回调
void stdio_callback(struct bufferevent *bev, void *arg) {
    // 获取读缓冲区并操作读缓冲中的数据
    struct evbuffer *evbuf = bufferevent_get_input(bev); // 封装了读缓冲区
    // 从中读一行，需要指定换行符
//    char *msg = evbuffer_readln(evbuf, NULL, EVBUFFER_EOL_LF);
    char msg[20] = {0};
    int cnt = evbuffer_remove(evbuf, msg, 19);
//    if (!msg) return;

    if (strcmp(msg, "quit") == 0) {
        printf("safe exit!!!\n");
        event_base_loopbreak(arg);
    }

    printf("stdio read the data: %s cnt:%d\n", msg, cnt);
}

// socket出现如错误、关闭等异常事件时的回调
void socket_event_callback(struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_EOF)
        printf("connection closed\n");
    else if (events & BEV_EVENT_ERROR)
        printf("some other error\n");
    else if (events & BEV_EVENT_TIMEOUT)
        printf("timeout\n");
    bufferevent_free(bev); // close
}

// accept的回调函数封装
void listener_callback(struct evconnlistener *listener, evutil_socket_t fd,
                       struct sockaddr *sock, int socklen, void *arg) {
    char ip[32] = {0};
    evutil_inet_ntop(AF_INET, sock, ip, sizeof(ip) - 1);
    printf("accept a client fd:%d ip:%s\n", fd, ip);
    struct event_base *base = (struct event_base *) arg;
    //创建一个bufferevent
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    // 设置读、写、以及异常时的回调函数
    bufferevent_setcb(bev, socket_read_callback, NULL, socket_event_callback, NULL);
    // 使能这个bufferevent开启读事件
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

static void
do_timer(int fd, short events, void *arg) {
    struct event *timer = (struct event *) arg;
    time_t now = time(NULL);
    printf("do_timer %s", (char *) ctime(&now));
    //event_del(timer);
    // struct timeval tv = {1,0};
    // event_add(timer, &tv);
}

static void
do_sig_int(int fd, short event, void *arg) {
    struct event *si = (struct event *) arg;
    event_del(si);
    printf("do_sig_int SIGINT\n");//CTRL + C
}

// 建立连接的事件封装   
int main() {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8080);
    /* 底层IO多路复用抽象 */
    struct event_base *base = event_base_new();
    /* evconnlistener 监听 */
    struct evconnlistener *listener =
            evconnlistener_new_bind(base, listener_callback, base,
                                    LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                    10, (struct sockaddr *) &sin,
                                    sizeof(struct sockaddr_in));

    /* 普通 fd 的 IO 事件管理，此处以标准输入 stdin 为例 */
    struct bufferevent *ioev = bufferevent_socket_new(base, 0, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(ioev, stdio_callback, NULL, NULL, base);
    bufferevent_enable(ioev, EV_READ | EV_PERSIST);
    /* 定时事件 */
    struct event evtimer;
    struct timeval tv = {1, 0}; // {秒, 微秒}
    event_set(&evtimer, -1, 0, do_timer, &evtimer); // tv 为超时时间
    event_base_set(base, &evtimer);
    event_add(&evtimer, &tv);
    /* 信号事件 */
    struct event evint;
    event_set(&evint, SIGINT, EV_SIGNAL, do_sig_int, &evint);
    event_base_set(base, &evint);
    event_add(&evint, NULL);

    /* 开启主循环 */
    event_base_dispatch(base);
    /* 结束释放资源 */
    evconnlistener_free(listener);
    event_base_free(base);
    return 0;
}

/*
gcc evmain2.c -o ev2 -levent
client:
    telnet 127.0.0.1 8080
*/
ev_ssize_t evbuffer_copyout_from(struct evbuffer *buf, const struct evbuffer_ptr *pos,
                      void *data_out, size_t datlen) {
    //...

    if (pos) {
        //要拷贝的被已有的还多
        if (datlen > (size_t) (EV_SSIZE_MAX - pos->pos)) {
            result = -1;
            goto done;
        }
        chain = pos->internal_.chain;
        pos_in_chain = pos->internal_.pos_in_chain;
        if (datlen + pos->pos > buf->total_len)
            datlen = buf->total_len - pos->pos;
    }
    else {
        chain = buf->first;
        pos_in_chain = 0;
        if (datlen > buf->total_len)
            datlen = buf->total_len;
    }


    if (datlen == 0)
        goto done;

    if (buf->freeze_start) {
        result = -1;
        goto done;
    }

    nread = datlen;

    while (datlen && datlen >= chain->off - pos_in_chain) {
        size_t copylen = chain->off - pos_in_chain;
        memcpy(data,
               chain->buffer + chain->misalign + pos_in_chain,
               copylen);
        data += copylen;
        datlen -= copylen;

        chain = chain->next;
        pos_in_chain = 0;
        EVUTIL_ASSERT(chain || datlen == 0);
    }

    if (datlen) {
        EVUTIL_ASSERT(chain);
        EVUTIL_ASSERT(datlen + pos_in_chain <= chain->off);

        memcpy(data, chain->buffer + chain->misalign + pos_in_chain,
               datlen);
    }

    result = nread;
    done:
    EVBUFFER_UNLOCK(buf);
    return result;
}