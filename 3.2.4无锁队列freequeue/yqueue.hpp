/*
    Copyright (c) 2007-2013 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_YQUEUE_HPP_INCLUDED__
#define __ZMQ_YQUEUE_HPP_INCLUDED__

#include <stdlib.h>
#include <stddef.h>

// #include "err.hpp"
#include "atomic_ptr.hpp"

//  yqueue is an efficient queue implementation. The main goal is
//  to minimise number of allocations/deallocations needed. Thus yqueue
//  allocates/deallocates elements in batches of N.
//
//  yqueue allows one thread to use push/back function and another one
//  to use pop/front functions. However, user must ensure that there's no
//  pop on the empty queue and that both threads don't access the same
//  element in unsynchronised manner.
//
//  T is the type of the object in the queue. 队列中元素的类型
//  N is granularity(粒度) of the queue (how many pushes have to be done till  actual memory allocation is required).
//  即是yqueue_t一个结点可以装载N个T类型的元素， yqueue_t的一个结点是一个数组
template<typename T, int N>
class yqueue_t {
public:
    //  创建队列.
    inline yqueue_t() {
        begin_chunk = (chunk_t *) malloc(sizeof(chunk_t));
        alloc_assert(begin_chunk);
        begin_pos = 0;
        back_chunk = NULL; //back_chunk总是指向队列中最后一个元素所在的chunk，现在还没有元素，所以初始为空
        back_pos = 0;
        end_chunk = begin_chunk; //end_chunk总是指向链表的最后一个chunk
        end_pos = 0;
    }

    //  销毁队列.
    inline ~yqueue_t() {
        while (true) {
            if (begin_chunk == end_chunk) {
                free(begin_chunk);
                break;
            }
            chunk_t *o = begin_chunk;
            begin_chunk = begin_chunk->next;
            free(o);
        }

        chunk_t *sc = spare_chunk.xchg(NULL);
        free(sc);
    }

    //  Returns reference to the front element of the queue.
    //  If the queue is empty, behaviour is undefined.
    // 返回队列头部元素的引用，调用者可以通过该引用更新元素，结合pop实现出队列操作。
    inline T &front() // 返回的是引用，是个左值，调用者可以通过其修改容器的值
    {
        return begin_chunk->values[begin_pos];
    }

    //  Returns reference to the back element of the queue.
    //  If the queue is empty, behaviour is undefined.
    // 返回队列尾部元素的引用，调用者可以通过该引用更新元素，结合push实现插入操作。
    // 如果队列为空，该函数是不允许被调用。
    inline T &back() // 返回的是引用，是个左值，调用者可以通过其修改容器的值
    {
        return back_chunk->values[back_pos];
    }

    //  Adds an element to the back end of the queue.
    inline void push() {
        back_chunk = end_chunk;
        back_pos = end_pos; //

        if (++end_pos != N) //end_pos!=N表明这个chunk节点还没有满
            return;

        chunk_t *sc = spare_chunk.xchg(NULL); // 为什么设置为NULL？ 因为如果把之前值取出来了则没有spare chunk了，所以设置为NULL
        if (sc)                               // 如果有spare chunk则继续复用它
        {
            end_chunk->next = sc;
            sc->prev = end_chunk;
        }
        else // 没有则重新分配
        {
            // static int s_cout = 0;
            // printf("s_cout:%d\n", ++s_cout);
            end_chunk->next = (chunk_t *) malloc(sizeof(chunk_t)); // 分配一个chunk
            alloc_assert(end_chunk->next);
            end_chunk->next->prev = end_chunk;
        }
        end_chunk = end_chunk->next;
        end_pos = 0;
    }

    //  Removes element from the back end of the queue. In other words
    //  it rollbacks last push to the queue. Take care: Caller is
    //  responsible for destroying the object being unpushed.
    //  The caller must also guarantee that the queue isn't empty when
    //  unpush is called. It cannot be done automatically as the read
    //  side of the queue can be managed by different, completely
    //  unsynchronised thread.
    // 必须要保证队列不为空，参考ypipe_t的uwrite
    inline void unpush() {
        //  First, move 'back' one position backwards.
        if (back_pos) // 从尾部删除元素
            --back_pos;
        else {
            back_pos = N - 1; // 回退到前一个chunk
            back_chunk = back_chunk->prev;
        }

        //  Now, move 'end' position backwards. Note that obsolete end chunk
        //  is not used as a spare chunk. The analysis shows that doing so
        //  would require free and atomic operation per chunk deallocated
        //  instead of a simple free.
        if (end_pos) // 意味着当前的chunk还有其他元素占有
            --end_pos;
        else {
            end_pos = N - 1; // 当前chunk没有元素占用，则需要将整个chunk释放
            end_chunk = end_chunk->prev;
            free(end_chunk->next);
            end_chunk->next = NULL;
        }
    }

    //  Removes an element from the front end of the queue.
    inline void pop() {
        if (++begin_pos == N) // 删除满一个chunk才回收chunk
        {
            chunk_t *o = begin_chunk;
            begin_chunk = begin_chunk->next;
            begin_chunk->prev = NULL;
            begin_pos = 0;

            //  'o' has been more recently used than spare_chunk,
            //  so for cache reasons we'll get rid of the spare and
            //  use 'o' as the spare.
            chunk_t *cs = spare_chunk.xchg(o); //由于局部性原理，总是保存最新的空闲块而释放先前的空闲快
            free(cs);
        }
    }

private:
    //  Individual memory chunk to hold N elements.
    // 链表结点称之为chunk_t
    struct chunk_t {
        T values[N]; //每个chunk_t可以容纳N个T类型的元素，以后就以一个chunk_t为单位申请内存
        chunk_t *prev;
        chunk_t *next;
    };

    //  Back position may point to invalid memory if the queue is empty,
    //  while begin & end positions are always valid. Begin position is
    //  accessed exclusively be queue reader (front/pop), while back and
    //  end positions are accessed exclusively by queue writer (back/push).
    chunk_t *begin_chunk; // 链表头结点
    int begin_pos;        // 起始点
    chunk_t *back_chunk;  // 队列中最后一个元素所在的链表结点
    int back_pos;         // 尾部
    chunk_t *end_chunk;   // 拿来扩容的，总是指向链表的最后一个结点
    int end_pos;

    //  People are likely to produce and consume at similar rates.  In
    //  this scenario holding onto the most recently freed chunk saves
    //  us from having to call malloc/free.
    atomic_ptr_t<chunk_t> spare_chunk; //空闲块（把所有元素都已经出队的块称为空闲块），读写线程的共享变量

    //  Disable copying of yqueue.
    yqueue_t(const yqueue_t &);

    const yqueue_t &operator=(const yqueue_t &);
};

#endif

