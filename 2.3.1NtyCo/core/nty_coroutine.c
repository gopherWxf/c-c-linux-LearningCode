#include "nty_coroutine.h"

pthread_key_t global_sched_key;
static pthread_once_t sched_key_once = PTHREAD_ONCE_INIT;


//new_ctx[%rdi]:即将运行协程的上下文寄存器列表; cur_ctx[%rsi]:正在运行协程的上下文寄存器列表
int _switch(nty_cpu_ctx *new_ctx, nty_cpu_ctx *cur_ctx);
//默认x86_64
__asm__(
"   .text               \n"
"   .p2align 4,,15      \n"
".globl _switch                                          \n"
".globl __switch                                         \n"
"_switch:                                                \n"
"__switch:                                               \n"
"       movq %rsp, 0(%rsi)      # save stack_pointer     \n"
"       movq %rbp, 8(%rsi)      # save frame_pointer     \n"
"       movq (%rsp), %rax       # save insn_pointer      \n"
"       movq %rax, 16(%rsi)     # save eip               \n"
"       movq %rbx, 24(%rsi)     # save rbx,r12-r15       \n"
"       movq %r12, 32(%rsi)                              \n"
"       movq %r13, 40(%rsi)                              \n"
"       movq %r14, 48(%rsi)                              \n"
"       movq %r15, 56(%rsi)                              \n"

"       movq 56(%rdi), %r15                              \n"
"       movq 48(%rdi), %r14                              \n"
"       movq 40(%rdi), %r13                              \n"
"       movq 32(%rdi), %r12                              \n"
"       movq 24(%rdi), %rbx     # restore rbx,r12-r15    \n"
"       movq 8(%rdi), %rbp      # restore frame_pointer  \n"
"       movq 0(%rdi), %rsp      # restore stack_pointer  \n"
"       movq 16(%rdi), %rax     # restore insn_pointer   \n"
"       movq %rax, (%rsp)                                \n"
"       ret                                              \n"
);

static void nty_coroutine_sched_key_destructor(void *data) {
    free(data);
}

static void nty_coroutine_sched_key_creator(void) {
    assert(pthread_key_create(&global_sched_key, nty_coroutine_sched_key_destructor) == 0);
    assert(pthread_setspecific(global_sched_key, NULL) == 0);
}
// 创建协程
int nty_coroutine_create(nty_coroutine **new_co, proc_coroutine func, void *arg) {
    //单例获取全局唯一的sched
    assert(pthread_once(&sched_key_once, nty_coroutine_sched_key_creator) == 0);
    nty_schedule * sched = nty_coroutine_get_sched();
    //如果sched还没有则创建
    if (sched == NULL) {
        nty_schedule_create(0);
        sched = nty_coroutine_get_sched();
        if (sched == NULL) {
            printf("Failed to create scheduler\n");
            return -1;
        }
    }
    //创建协程
    nty_coroutine *co = calloc(1, sizeof(nty_coroutine));
    if (co == NULL) {
        printf("Failed to allocate memory for new coroutine\n");
        return -2;
    }
    //创建协程的栈
    int ret = posix_memalign(&co->stack, getpagesize(), sched->stack_size);
    if (ret) {
        printf("Failed to allocate stack for new coroutine\n");
        free(co);
        return -3;
    }
    co->sched = sched;
    co->stack_size = sched->stack_size;
    //新创建的协程，状态为new，加入ready队列
    co->status = BIT(NTY_COROUTINE_STATUS_NEW); //new
    co->id = sched->spawned_coroutines++;
    co->func = func;

    co->fd = -1;
    co->events = 0;
    co->arg = arg;
    co->birth = nty_coroutine_usec_now();
    *new_co = co;
    TAILQ_INSERT_TAIL(&co->sched->ready, co, ready_node);
}

// yield让出
void nty_coroutine_yield(nty_coroutine *co) {
    _switch(&co->sched->ctx, &co->ctx);
}
// eip 执行入口
static void _exec(void *lt) {
    nty_coroutine *co = (nty_coroutine *) lt;
    co->func(co->arg);
    co->status |= (BIT( NTY_COROUTINE_STATUS_EXITED) | BIT(NTY_COROUTINE_STATUS_FDEOF) |
                   BIT(NTY_COROUTINE_STATUS_DETACH));
    nty_coroutine_yield(co);
}
// 初始化协程栈
static void nty_coroutine_init(nty_coroutine *co) {
    void **stack = (void **) (co->stack + co->stack_size);

    stack[-3] = NULL;
    stack[-2] = (void *) co;

    co->ctx.rsp = (void *) stack - (4 * sizeof(void *));
    co->ctx.rbp = (void *) stack - (3 * sizeof(void *));
    co->ctx.eip = (void *) _exec;
    co->status = BIT(NTY_COROUTINE_STATUS_READY);
#if 0
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    printf("co.stack start :%p\n",co->stack);
    printf("co.stack end   :%p\n",(void *) stack);
    printf("stack[-3]  :%p\n",&stack[-3]);
    printf("stack[-2]  :%p\n",&stack[-2]);
    printf("co->ctx.rbp  :%p\n",(void *) stack - (3 * sizeof(void *)));
    printf("co->ctx.rsp  :%p\n",(void *) stack - (4 * sizeof(void *)));

    printf("stack[0]  :%p\n",&stack[0]);
    printf("stack[-1]  :%p\n",&stack[-1]);

    printf("stack[0]  :%p\n",stack[0]);
    printf("stack[-1]  :%p\n",stack[-1]);

    int i=0;
    for(i=0;i>-10;i--){
        printf("&stack[%d]:%p  stack[%d]:%p \n",i,&stack[i],i,stack[i]);
    }

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
#endif
}
//销毁协程内存
void nty_coroutine_free(nty_coroutine *co) {
    if (co == NULL) return;
    co->sched->spawned_coroutines--;

    if (co->stack) {
        free(co->stack);
        co->stack = NULL;
    }
    if (co) {
        free(co);
    }
}
// resume协程恢复执行
int nty_coroutine_resume(nty_coroutine *co) {
    if (co->status & BIT(NTY_COROUTINE_STATUS_NEW)) {
        nty_coroutine_init(co);
    }
    nty_schedule * sched = nty_coroutine_get_sched();
    sched->curr_thread = co;


    _switch(&co->ctx, &co->sched->ctx);
    sched->curr_thread = NULL;

    if (co->status & BIT(NTY_COROUTINE_STATUS_EXITED)) {
        if (co->status & BIT(NTY_COROUTINE_STATUS_DETACH)) {
            nty_coroutine_free(co);
        }
        return -1;
    }
    return 0;
}
// 协程sleep
void nty_coroutine_sleep(uint64_t msecs) {
    nty_coroutine *co = nty_coroutine_get_sched()->curr_thread;
    if (msecs == 0) {
        TAILQ_INSERT_TAIL(&co->sched->ready, co, ready_node);
        nty_coroutine_yield(co);
    }
    else {
        nty_schedule_sched_sleepdown(co, msecs);
    }
}