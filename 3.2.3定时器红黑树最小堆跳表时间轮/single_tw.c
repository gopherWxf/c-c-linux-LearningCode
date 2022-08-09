#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <sys/time.h>
#include <mach/task.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

#define MAX_TIMER ((1<<17)-1)
#define MAX_CONN ((1<<16)-1)

typedef struct conn_node {
    uint8_t used; // 引用    就是这段时间发送的心跳包的个数
    int id;  // fd
} conn_node_t;

typedef struct timer_node {
	struct timer_node *next; // 下一个
    struct conn_node *node; // 本身
    uint32_t idx;
} timer_node_t;

static timer_node_t timer_nodes[MAX_TIMER] = {0};
static conn_node_t conn_nodes[MAX_CONN] = {0};
static uint32_t t_iter = 0;
static uint32_t c_iter = 0;

timer_node_t * get_timer_node() { // 注意：没有检测定时任务数超过 MAX_TIMER 的情况
    t_iter++;
    while (timer_nodes[t_iter & MAX_TIMER].idx > 0) {
        t_iter++;
    }
    timer_nodes[t_iter].idx = t_iter;
    return &timer_nodes[t_iter];
}

conn_node_t * get_conn_node() { // 注意：没有检测连接数超过 MAX_CONN 的情况
    c_iter++;
    while (conn_nodes[c_iter & MAX_CONN].used > 0) {
        c_iter++;
    }
    return &conn_nodes[c_iter];
}

#define TW_SIZE 16
#define EXPIRE 10
#define TW_MASK (TW_SIZE - 1)
static uint32_t tick = 0;

typedef struct link_list {
	timer_node_t head; // 方便遍历
	timer_node_t *tail; // 方便添加
}link_list_t;

void add_conn(link_list_t *tw, conn_node_t *cnode, int delay) {
    link_list_t *list = &tw[(tick+EXPIRE+delay) & TW_MASK];
    timer_node_t * tnode = get_timer_node();
    cnode->used++;
    tnode->node = cnode;
    list->tail->next = tnode;
	list->tail = tnode;
	tnode->next = NULL;
}

void link_clear(link_list_t *list) {
	list->head.next = NULL;
	list->tail = &(list->head);
}

void check_conn(link_list_t *tw) {
    int32_t itick = tick;
    tick++;
    link_list_t *list = &tw[itick & TW_MASK];
    timer_node_t *current = list->head.next;
    while (current) {
		timer_node_t * temp = current;
		current = current->next;
        conn_node_t *cn = temp->node;
        cn->used--;
        temp->idx = 0;
        if (cn->used == 0) {
            printf("fd:%d kill down\n", cn->id);
            temp->next = NULL;
            continue;
        }
        printf("fd:%d used:%d\n", cn->id, cn->used);
	}
    link_clear(list);
}

static time_t
current_time() {
	time_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
	struct timespec ti;
	clock_gettime(CLOCK_MONOTONIC, &ti);
	t = (time_t)ti.tv_sec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (time_t)tv.tv_sec;
#endif
	return t;
}

int main() {
    memset(timer_nodes, 0, MAX_TIMER * sizeof(timer_node_t));
    memset(conn_nodes, 0, MAX_CONN * sizeof(conn_node_t));

    // init link list
    link_list_t tw[TW_SIZE];
    memset(tw, 0, TW_SIZE * sizeof(link_list_t));
    for (int i = 0; i < TW_SIZE; i++) {
        link_clear(&tw[i]);
    }

    // 该测试起始时间为0秒，所以 delay 不能添加超过10的数。
    {
        conn_node_t *node = get_conn_node();
        node->id = 10001;
        add_conn(tw, node, 0);
        add_conn(tw, node, 5);
    }

    {
        conn_node_t *node = get_conn_node();
        node->id = 10002;
        add_conn(tw, node, 0);
    }

    {
        conn_node_t *node = get_conn_node();
        node->id = 10003;
        add_conn(tw, node, 3);
    }

    time_t start = current_time();
    for (;;) {
        time_t now = current_time();
        if (now - start > 0) { // 
            for (int i=0; i<now-start; i++)
                check_conn(tw);
            start = now;
            printf("check conn tick:%d\n", tick);
        }
        // 1. 为什么 usleep 20ms  而时间精度是 1s   50个 20ms
        // check_conn 需要花费时间，随着运行，时间精度越来越差
        // 
        usleep(20000); // 1秒 1000000    1s  1000ms   20ms
    }
    return 0;
}


