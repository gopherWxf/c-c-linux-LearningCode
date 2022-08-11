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

#ifndef __ZMQ_ATOMIC_PTR_HPP_INCLUDED__
#define __ZMQ_ATOMIC_PTR_HPP_INCLUDED__

#if defined __GNUC__
#define likely(x) __builtin_expect ((x), 1)
#define unlikely(x) __builtin_expect ((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#if (defined __i386__ || defined __x86_64__) && defined __GNUC__
#define ZMQ_ATOMIC_PTR_X86              // 应该走这里去执行
#elif defined __tile__
#define ZMQ_ATOMIC_PTR_TILE
#elif (defined ZMQ_HAVE_SOLARIS || defined ZMQ_HAVE_NETBSD)
#define ZMQ_ATOMIC_PTR_ATOMIC_H
#endif


#if defined ZMQ_ATOMIC_PTR_ATOMIC_H
#include <atomic.h>
#elif defined ZMQ_ATOMIC_PTR_TILE
#include <arch/atomic.h>
#endif

#define alloc_assert(x) \
    do {\
        if (unlikely (!x)) {\
            fprintf (stderr, "FATAL ERROR: OUT OF MEMORY (%s:%d)\n",\
                __FILE__, __LINE__);\
        }\
    } while (false)


//  This class encapsulates several atomic operations on pointers.


template<typename T>
class atomic_ptr_t {
public:

    //  Initialise atomic pointer
    inline atomic_ptr_t() {
        ptr = NULL;
    }

    //  Destroy atomic pointer
    inline ~atomic_ptr_t() {
    }

    //  Set value of atomic pointer in a non-threadsafe way
    //  Use this function only when you are sure that at most one
    //  thread is accessing the pointer at the moment.
    inline void set(T *ptr_)//非原子操作
    {
        this->ptr = ptr_;
    }

    //  Perform atomic 'exchange pointers' operation. Pointer is set
    //  to the 'val' value. Old value is returned.
    // 设置新值，返回旧值
    inline T *xchg(T *val_)    //原子操
    {
#if defined ZMQ_ATOMIC_PTR_ATOMIC_H
        return (T*) atomic_swap_ptr (&ptr, val_);
#elif defined ZMQ_ATOMIC_PTR_TILE
        return (T*) arch_atomic_exchange (&ptr, val_);
#elif defined ZMQ_ATOMIC_PTR_X86
        T *old;
        __asm__ volatile (
        "lock; xchg %0, %2"
        : "=r" (old), "=m" (ptr)
        : "m" (ptr), "0" (val_));
        return old;
#elif defined ZMQ_ATOMIC_PTR_MUTEX
        sync.lock ();
        T *old = (T*) ptr;
        ptr = val_;
        sync.unlock ();
        return old;
#else
#error atomic_ptr is not implemented for this platform
#endif
    }

    //  Perform atomic 'compare and swap' operation on the pointer.
    //  The pointer is compared to 'cmp' argument and if they are
    //  equal, its value is set to 'val'. Old value of the pointer
    //  is returned.
    // 原来的值(ptr指向)如果和 comp_的值相同则更新为val_,并返回原来的ptr
    //   ○ 如果相等返回ptr设置之前的值，并把ptr更新为参数val_的值，；
    //   ○ 如果不相等直接返回ptr值。
    inline T *cas(T *cmp_, T *val_)//原子操作
    {
#if defined ZMQ_ATOMIC_PTR_ATOMIC_H
        return (T*) atomic_cas_ptr (&ptr, cmp_, val_);
#elif defined ZMQ_ATOMIC_PTR_TILE
        return (T*) arch_atomic_val_compare_and_exchange (&ptr, cmp_, val_);
#elif defined ZMQ_ATOMIC_PTR_X86
        T *old;
        __asm__ volatile (
        "lock; cmpxchg %2, %3"
        : "=a" (old), "=m" (ptr)
        : "r" (val_), "m" (ptr), "0" (cmp_)
        : "cc");
        return old;
#else
#error atomic_ptr is not implemented for this platform
#endif
    }

private:

    volatile T *ptr;
#if defined ZMQ_ATOMIC_PTR_MUTEX
    mutex_t sync;
#endif

    atomic_ptr_t(const atomic_ptr_t &);

    const atomic_ptr_t &operator=(const atomic_ptr_t &);
};


//  Remove macros local to this file.
#if defined ZMQ_ATOMIC_PTR_WINDOWS
#undef ZMQ_ATOMIC_PTR_WINDOWS
#endif
#if defined ZMQ_ATOMIC_PTR_ATOMIC_H
#undef ZMQ_ATOMIC_PTR_ATOMIC_H
#endif
#if defined ZMQ_ATOMIC_PTR_X86
#undef ZMQ_ATOMIC_PTR_X86
#endif
#if defined ZMQ_ATOMIC_PTR_ARM
#undef ZMQ_ATOMIC_PTR_ARM
#endif
#if defined ZMQ_ATOMIC_PTR_MUTEX
#undef ZMQ_ATOMIC_PTR_MUTEX
#endif

#endif

