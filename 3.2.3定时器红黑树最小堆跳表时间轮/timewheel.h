#ifndef _MARK_TIMEWHEEL_
#define _MARK_TIMEWHEEL_

#include <stdint.h>

#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

typedef struct timer_node timer_node_t;
typedef void (*handler_pt) (struct timer_node *node);

struct timer_node {
	struct timer_node *next;
	uint32_t expire;
    handler_pt callback;
    uint8_t cancel;
	int id; // 此时携带参数
};

timer_node_t* add_timer(int time, handler_pt func, int threadid);

void expire_timer(void);

void del_timer(timer_node_t* node);

void init_timer(void);

void clear_timer();

#endif
