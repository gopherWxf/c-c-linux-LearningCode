#include "minheap.h"

#define min_heap_elem_greater(a, b) \
    ((a)->time > (b)->time)

void min_heap_ctor_(min_heap_t* s) { s->p = 0; s->n = 0; s->a = 0; }
void min_heap_dtor_(min_heap_t* s) { if (s->p) free(s->p); }
void min_heap_elem_init_(timer_entry_t* e) { e->min_heap_idx = -1; }
int min_heap_empty_(min_heap_t* s) { return 0u == s->n; }
unsigned min_heap_size_(min_heap_t* s) { return s->n; }
timer_entry_t* min_heap_top_(min_heap_t* s) { return s->n ? *s->p : 0; }

int min_heap_push_(min_heap_t* s, timer_entry_t* e)
{
    if (min_heap_reserve_(s, s->n + 1))
        return -1;
    min_heap_shift_up_(s, s->n++, e);
    return 0;
}

timer_entry_t* min_heap_pop_(min_heap_t* s)
{
    if (s->n)
    {
        timer_entry_t* e = *s->p;
        min_heap_shift_down_(s, 0u, s->p[--s->n]);
        e->min_heap_idx = -1;
        return e;
    }
    return 0;
}

int min_heap_elt_is_top_(const timer_entry_t *e)
{
    return e->min_heap_idx == 0;
}

int min_heap_erase_(min_heap_t* s, timer_entry_t* e)
{
    if (-1 != e->min_heap_idx)
    {
        timer_entry_t *last = s->p[--s->n];
        unsigned parent = (e->min_heap_idx - 1) / 2;
        /* we replace e with the last element in the heap.  We might need to
           shift it upward if it is less than its parent, or downward if it is
           greater than one or both its children. Since the children are known
           to be less than the parent, it can't need to shift both up and
           down. */
        if (e->min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], last))
            min_heap_shift_up_unconditional_(s, e->min_heap_idx, last);
        else
            min_heap_shift_down_(s, e->min_heap_idx, last);
        e->min_heap_idx = -1;
        return 0;
    }
    return -1;
}

int min_heap_adjust_(min_heap_t *s, timer_entry_t *e)
{
    if (-1 == e->min_heap_idx) {
        return min_heap_push_(s, e);
    } else {
        unsigned parent = (e->min_heap_idx - 1) / 2;
        /* The position of e has changed; we shift it up or down
         * as needed.  We can't need to do both. */
        if (e->min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], e))
            min_heap_shift_up_unconditional_(s, e->min_heap_idx, e);
        else
            min_heap_shift_down_(s, e->min_heap_idx, e);
        return 0;
    }
}

int min_heap_reserve_(min_heap_t* s, unsigned n)
{
    if (s->a < n)
    {
        timer_entry_t** p;
        unsigned a = s->a ? s->a * 2 : 8;
        if (a < n)
            a = n;
        if (!(p = (timer_entry_t**)realloc(s->p, a * sizeof *p)))
            return -1;
        s->p = p;
        s->a = a;
    }
    return 0;
}

void min_heap_shift_up_unconditional_(min_heap_t* s, unsigned hole_index, timer_entry_t* e)
{
    unsigned parent = (hole_index - 1) / 2;
    do
    {
    (s->p[hole_index] = s->p[parent])->min_heap_idx = hole_index;
    hole_index = parent;
    parent = (hole_index - 1) / 2;
    } while (hole_index && min_heap_elem_greater(s->p[parent], e));
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

void min_heap_shift_up_(min_heap_t* s, unsigned hole_index, timer_entry_t* e)
{
    unsigned parent = (hole_index - 1) / 2;
    while (hole_index && min_heap_elem_greater(s->p[parent], e))
    {
    (s->p[hole_index] = s->p[parent])->min_heap_idx = hole_index;
    hole_index = parent;
    parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, timer_entry_t* e)
{
    unsigned min_child = 2 * (hole_index + 1);
    while (min_child <= s->n)
    {
    min_child -= min_child == s->n || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);
    if (!(min_heap_elem_greater(e, s->p[min_child])))
        break;
    (s->p[hole_index] = s->p[min_child])->min_heap_idx = hole_index;
    hole_index = min_child;
    min_child = 2 * (hole_index + 1);
    }
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}