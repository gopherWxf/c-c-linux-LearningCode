#ifndef MARK_MINHEAP_H
#define MARK_MINHEAP_H

#include <stdint.h>
#include <stdlib.h>

typedef struct timer_entry_s timer_entry_t;
typedef void (*timer_handler_pt)(timer_entry_t *ev);

struct timer_entry_s {
    uint32_t time;
    uint32_t min_heap_idx;
    timer_handler_pt handler;
    void *privdata;
};

typedef struct min_heap {
    timer_entry_t **p;
    uint32_t n, a; // n 为实际元素个数  a 为容量
} min_heap_t;

void            min_heap_ctor_(min_heap_t* s);
void            min_heap_dtor_(min_heap_t* s);
void            min_heap_elem_init_(timer_entry_t* e);
int             min_heap_elt_is_top_(const timer_entry_t *e);
int             min_heap_empty_(min_heap_t* s);
unsigned        min_heap_size_(min_heap_t* s);
timer_entry_t*  min_heap_top_(min_heap_t* s);
int             min_heap_reserve_(min_heap_t* s, unsigned n);
int             min_heap_push_(min_heap_t* s, timer_entry_t* e);
timer_entry_t*  min_heap_pop_(min_heap_t* s);
int             min_heap_adjust_(min_heap_t *s, timer_entry_t* e);
int             min_heap_erase_(min_heap_t* s, timer_entry_t* e);
void            min_heap_shift_up_(min_heap_t* s, unsigned hole_index, timer_entry_t* e);
void            min_heap_shift_up_unconditional_(min_heap_t* s, unsigned hole_index, timer_entry_t* e);
void            min_heap_shift_down_(min_heap_t* s, unsigned hole_index, timer_entry_t* e);

#endif // MARK_MINHEAP_H