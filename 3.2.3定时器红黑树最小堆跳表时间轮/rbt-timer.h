#ifndef _MARK_RBT_
#define _MARK_RBT_

#include <stdio.h>
#include <stdint.h> // 整数
#include <unistd.h> // usleep
#include <stdlib.h> // malloc 注意需要强转
#include <stddef.h> //offsetof

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <sys/time.h>
#include <mach/task.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

#include "rbtree.h"

ngx_rbtree_t              timer;
static ngx_rbtree_node_t  sentinel;

typedef struct timer_entry_s timer_entry_t;
typedef void (*timer_handler_pt)(timer_entry_t *ev);

struct timer_entry_s {
    ngx_rbtree_node_t rbnode;
    timer_handler_pt handler;
};

static uint32_t
current_time() {
	uint32_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
	struct timespec ti;
	clock_gettime(CLOCK_MONOTONIC, &ti);
	t = (uint32_t)ti.tv_sec * 1000;
	t += ti.tv_nsec / 1000000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint32_t)tv.tv_sec * 1000;
	t += tv.tv_usec / 1000;
#endif
	return t;
}

ngx_rbtree_t * init_timer() {
    ngx_rbtree_init(&timer, &sentinel, ngx_rbtree_insert_timer_value);
    return &timer;
}

void add_timer(uint32_t msec, timer_handler_pt func) {
    timer_entry_t *te = malloc(sizeof(timer_entry_t));
    memset(te, 0, sizeof(timer_entry_t));
    te->handler = func;
    msec += current_time();
    printf("add_timer expire at msec = %u\n", msec);
    te->rbnode.key = msec;
    ngx_rbtree_insert(&timer, &te->rbnode);
}

void del_timer(timer_entry_t *te) {
    ngx_rbtree_delete(&timer, &te->rbnode);
    free(te);
}

int find_nearest_expire_timer() {
    ngx_rbtree_node_t  *node;
    if (timer.root == &sentinel) {
        return -1;
    }
    node = ngx_rbtree_min(timer.root, timer.sentinel);
    int diff = (int)node->key - (int)current_time();
    return diff > 0 ? diff : 0;
}

void expire_timer() {
    timer_entry_t *te;
    ngx_rbtree_node_t *sentinel, *root, *node;
    sentinel = timer.sentinel;
    uint32_t now = current_time();
    for (;;) {
        root = timer.root;
        if (root == sentinel) break;
        node = ngx_rbtree_min(root, sentinel);
        if (node->key > now) break;
        printf("touch timer expire time=%u, now = %u\n", node->key, now);
        te = (timer_entry_t *) ((char *) node - offsetof(timer_entry_t, rbnode));
        te->handler(te);
        ngx_rbtree_delete(&timer, &te->rbnode);
        free(te);
    }
}

#endif