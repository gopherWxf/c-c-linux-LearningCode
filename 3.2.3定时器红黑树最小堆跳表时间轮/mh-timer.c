
#include <stdio.h>
#include <sys/epoll.h>
#include "mh-timer.h"

void hello_world(timer_entry_t *te) {
    printf("hello world time = %u\n", te->time);
}

int main() {
    init_timer();

    add_timer(3000, hello_world);

    int epfd = epoll_create(1);
    struct epoll_event events[512];

    for (;;) {
        int nearest = find_nearest_expire_timer();
        int n = epoll_wait(epfd, events, 512, nearest);
        for (int i=0; i < n; i++) {
            // 
        }
        expire_timer();
    }
    return 0;
}

// gcc mh-timer.c minheap.c -o mh -I./