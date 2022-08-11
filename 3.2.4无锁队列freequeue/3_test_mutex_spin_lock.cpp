#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <atomic>
#include <unistd.h>
#include <sys/time.h>


#define MAX_THREAD_NUM 2
#define FOR_LOOP_COUNT 2000
#define FOR_ADD_COUNT 100000
static int counter = 0;
static pthread_spinlock_t spinlock;
static pthread_mutex_t mutex;

typedef void *(*thread_func_t)(void *argv);

int64_t get_current_millisecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long long) tv.tv_sec * 1000 + (long) tv.tv_usec / 1000);
}


void do_for_add(int count) {
    long sum = 0;
    for (int i = 0; i < count; i++) {
        sum += i;
    }
}


class atomic_flag_spinlock {
    std::atomic_flag flag;
public:
    atomic_flag_spinlock() :
            flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

static atomic_flag_spinlock s_atomic_flag_spinlock;

static int lxx_atomic_add(int *ptr, int increment) {
    int old_value = *ptr;
    __asm__ volatile("lock; xadd %0, %1 \n\t"
    : "=r"(old_value), "=m"(*ptr)
    : "0"(increment), "m"(*ptr)
    : "cc", "memory");
    return *ptr;
}


void *mutex_thread_main(void *argv) {
    for (int i = 0; i < FOR_LOOP_COUNT; i++) {
        pthread_mutex_lock(&mutex);
        // counter++;
        do_for_add(FOR_ADD_COUNT);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *atomic_thread_main(void *argv) {
    for (int i = 0; i < FOR_LOOP_COUNT; i++) {
        lxx_atomic_add(&counter, 1);
        // counter++;
    }
    return NULL;
}

void *spin_thread_main(void *argv) {
    for (int i = 0; i < FOR_LOOP_COUNT; i++) {
        pthread_spin_lock(&spinlock);
        counter++;
        // do_for_add(FOR_ADD_COUNT);
        pthread_spin_unlock(&spinlock);
    }
    return NULL;
}

void *atomic_flag_spinlock_thread_main(void *argv) {
    for (int i = 0; i < FOR_LOOP_COUNT; i++) {
        s_atomic_flag_spinlock.lock();
        counter++;
        // do_for_add(FOR_ADD_COUNT);
        s_atomic_flag_spinlock.unlock();
    }
    return NULL;
}

int test_lock(thread_func_t func, char **argv) {
    int64_t start = get_current_millisecond();
    pthread_t tid[MAX_THREAD_NUM] = {0};
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        int ret = pthread_create(&tid[i], NULL, func, argv);
        if (0 != ret) {
            printf("create thread failed\n");
        }
    }
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        pthread_join(tid[i], NULL);
    }
    int64_t end = get_current_millisecond();
    printf("spend clock : %ldms, ", (end - start));
    return 0;
}

// 多尝试几次
// g++ -o 3_test_mutex_spin_lock 3_test_mutex_spin_lock.cpp -lpthread -std=c++11

int main(int argc, char **argv) {
    printf("THREAD_NUM:%d\n\n", MAX_THREAD_NUM);
    counter = 0;
    printf("use mutex ----------->\n");
    test_lock(mutex_thread_main, NULL);
    printf("counter = %d\n", counter);

    counter = 0;
    printf("\nuse atomic ----------->\n");
    test_lock(atomic_thread_main, NULL);
    printf("counter = %d\n", counter);

    counter = 0;
    printf("\nuse spin ----------->\n");
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    test_lock(spin_thread_main, NULL);
    printf("counter = %d\n", counter);

    counter = 0;
    printf("\nuse atomic_flag_spinlock ----------->\n");
    test_lock(atomic_flag_spinlock_thread_main, NULL);
    printf("counter = %d\n", counter);

    return 0;
}