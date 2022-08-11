#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <time.h>
#include <atomic>
#include <list>
#include <memory>
#include <atomic>
#include <unistd.h>

// static int s_queue_item_num = 10000000;   // 每个线程插入的元素个数
static int s_queue_item_num = 2000000; // 每个线程插入的元素个数
static int s_producer_thread_num = 1;   // 生产者线程数量
static int s_consumer_thread_num = 1;   // 消费线程数量
// static int s_counter = 0;

// 有锁队列，直接使用list
static std::list<int> s_list;
static std::mutex s_mutex;

static int s_count_push = 0;
static int s_count_pop = 0;

#define PRINT_THREAD_INTO() printf("%s %lu into\n", __FUNCTION__, pthread_self())
#define PRINT_THREAD_LEAVE() printf("%s %lu leave\n", __FUNCTION__, pthread_self())

#define PRINT_THREAD_INTO()
#define PRINT_THREAD_LEAVE()

int64_t get_current_millisecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t) tv.tv_sec * 1000 + (int64_t) tv.tv_usec / 1000);
}

// using namespace lock_free;

typedef void *(*thread_func_t)(void *argv);

static int lxx_atomic_add(int *ptr, int increment) {
    int old_value = *ptr;
    __asm__ volatile("lock; xadd %0, %1 \n\t"
    : "=r"(old_value), "=m"(*ptr)
    : "0"(increment), "m"(*ptr)
    : "cc", "memory");
    return *ptr;
}

class BlockQueue {
public:
    BlockQueue() {
    }

    ~BlockQueue() {
    }

    // 插入packet，需要指明音视频类型
    // 返回0说明正常
    int Enqueue(int val) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(val);

        cond_.notify_one();
        return 0;
    }

    // 取出packet，并也取出对应的类型
    // 返回值: -1 abort; 1 获取到消息; 0 超时
    int Dequeue(int &val, const int timeout = 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) { // 等待唤醒
            // return如果返回false，继续wait, 如果返回true退出wait
            cond_.wait_for(lock, std::chrono::milliseconds(timeout),
                           [this] { return !queue_.empty() | abort_request_; });
        }
        if (abort_request_) {
            printf("abort request");
            return -1;
        }
        if (queue_.empty()) {
            return 0;
        }

        // 真正干活
        val = queue_.front(); //读取队列首部元素，这里还没有真正出队列
        queue_.pop();
        return 1;
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // 唤醒在等待的线程
    void Abort() {
        std::lock_guard<std::mutex> lock(mutex_);
        abort_request_ = true;
        cond_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<int> queue_;

    bool abort_request_ = false;
};

void *mutexqueue_producer_thread(void *argv) {
    PRINT_THREAD_INTO();
    for (int i = 0; i < s_queue_item_num; i++) {
        s_mutex.lock();
        s_list.push_back(s_count_push);
        s_count_push++;
        s_mutex.unlock();
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *mutexqueue_consumer_thread(void *argv) {
    int value = 0;
    int last_value = 0;
    int nodata = 0;
    PRINT_THREAD_INTO();
    while (true) {
        s_mutex.lock();
        if (s_list.size() > 0) {
            value = s_list.front();
            s_list.pop_front();
            last_value = value;
            s_count_pop++;
            nodata = 0;
        }
        else {
            nodata = 1;
        }
        s_mutex.unlock();
        if (nodata) {
            // usleep(1000);
            sched_yield();
        }
        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
        else {
            // printf("s_count_pop:%d, %d, %d\n", s_count_pop, s_queue_item_num, s_producer_thread_num);
        }
    }
    printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num,
           s_consumer_thread_num);
    PRINT_THREAD_LEAVE();
    return NULL;
}

#include "SimpleLockFreeQueue.h"

static SimpleLockFreeQueue<int> s_lockfreequeue;

void *lockfreequeue_producer_thread(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    for (int i = 0; i < s_queue_item_num; i++) {
        count = lxx_atomic_add(&s_count_push, 1); // 只是保证enqueue数据唯一性
        s_lockfreequeue.enqueue(count);           // enqueue的顺序是无法保证的，我们只能计算enqueue的个数
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *lockfreequeue_consumer_thread(void *argv) {
    int last_value = 0;
    static int s_pid_count = 0;
    s_pid_count++;
    // int pid = s_pid_count;
    PRINT_THREAD_INTO();
    while (true) {
        int value = 0;
        if (s_lockfreequeue.try_dequeue(value)) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%d, -> value:%d, expected:%d\n", pid, value, last_value);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
            // printf("pid:%d, null\n", pid);
            // usleep(10);
            sched_yield();
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {

            break;
        }
    }
    printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num,
           s_consumer_thread_num);
    PRINT_THREAD_LEAVE();
    return NULL;
}

static BlockQueue s_blockqueue;

void *blockqueue_producer_thread(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    for (int i = 0; i < s_queue_item_num; i++) {
        count = lxx_atomic_add(&s_count_push, 1); // 只是保证enqueue数据唯一性
        s_blockqueue.Enqueue(count);              // enqueue的顺序是无法保证的，我们只能计算enqueue的个数
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *blockqueue_consumer_thread(void *argv) {
    int last_value = 0;
    PRINT_THREAD_INTO();

    while (true) {
        int value = 0;
        if (s_blockqueue.Dequeue(value, 30) == 1) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
            // printf("pid:%d, null\n", pid);
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

#include "ArrayLockFreeQueue.h"

ArrayLockFreeQueue<int> arraylockfreequeue;

void *arraylockfreequeue_producer_thread(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    int write_failed_count = 0;

    for (int i = 0; i < s_queue_item_num;) {
        if (arraylockfreequeue.enqueue(count)) // enqueue的顺序是无法保证的，我们只能计算enqueue的个数
        {
            count = lxx_atomic_add(&s_count_push, 1);
            i++;
        }
        else {
            write_failed_count++;
            // printf("%s %lu enqueue failed, q:%d\n", __FUNCTION__,  pthread_self(), arraylockfreequeue.size());
            sched_yield();
            // usleep(10000);
        }
    }
    // printf("%s %lu write_failed_count:%d\n", __FUNCTION__, pthread_self(), write_failed_count)
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *arraylockfreequeue_consumer_thread(void *argv) {
    int last_value = 0;
    PRINT_THREAD_INTO();
    int value = 0;
    int read_failed_count = 0;
    while (true) {

        if (arraylockfreequeue.dequeue(value)) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
            read_failed_count++;
            // printf("%s %lu no data, s_count_pop:%d, value:%d\n", __FUNCTION__, pthread_self(), s_count_pop, value);
            // usleep(100);
            sched_yield();
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
    }
    // printf("%s %lu read_failed_count:%d\n", __FUNCTION__, pthread_self(), read_failed_count)
    PRINT_THREAD_LEAVE();
    return NULL;
}

#include "ypipe.hpp"

ypipe_t<int, 10000> yqueue;

void *yqueue_producer_thread(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    for (int i = 0; i < s_queue_item_num;) {
        yqueue.write(count, false); // enqueue的顺序是无法保证的，我们只能计算enqueue的个数
        yqueue.flush();

        count = lxx_atomic_add(&s_count_push, 1);
        i++;
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *yqueue_consumer_thread(void *argv) {
    int last_value = 0;
    PRINT_THREAD_INTO();

    while (true) {
        int value = 0;
        if (yqueue.read(&value)) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value + 1);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
            // printf("%s %lu no data, s_count_pop:%d\n", __FUNCTION__, pthread_self(), s_count_pop);
            usleep(100);
//             sched_yield();
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *yqueue_consumer_thread_batch(void *argv) {
    int last_value = 0;
    PRINT_THREAD_INTO();

    while (true) {
        int value = 0;
        if (yqueue.read(&value)) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value + 1);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
             sched_yield();
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *yqueue_producer_thread_batch(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    int item_num = s_queue_item_num / 10;
    for (int i = 0; i < item_num;) {
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, true);
        count = lxx_atomic_add(&s_count_push, 1);
        yqueue.write(count, false);
        count = lxx_atomic_add(&s_count_push, 1);
        i++;
        yqueue.flush();
    }
    PRINT_THREAD_LEAVE();
    return NULL;
}

std::mutex ypipe_mutex_;
std::condition_variable ypipe_cond_;

void *yqueue_producer_thread_condition(void *argv) {
    PRINT_THREAD_INTO();
    int count = 0;
    for (int i = 0; i < s_queue_item_num;) {
        yqueue.write(count, false); // enqueue的顺序是无法保证的，我们只能计算enqueue的个数
        count = lxx_atomic_add(&s_count_push, 1);
        i++;
        // yqueue.flush();
        if (!yqueue.flush()) {
//             printf("notify_one %d\n",i);
            std::unique_lock<std::mutex> lock(ypipe_mutex_);
            ypipe_cond_.notify_one();
        }
    }
    std::unique_lock<std::mutex> lock(ypipe_mutex_);
    ypipe_cond_.notify_one();
    PRINT_THREAD_LEAVE();
    return NULL;
}

void *yqueue_consumer_thread_condition(void *argv) {
    int last_value = 0;
    PRINT_THREAD_INTO();

    while (true) {
        int value = 0;
        if (yqueue.read(&value)) {
            if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 &&
                (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            {
                // printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value + 1);
            }
            lxx_atomic_add(&s_count_pop, 1);
            last_value = value;
        }
        else {
//            printf("wait_one\n");

            std::unique_lock<std::mutex> lock(ypipe_mutex_);
            ypipe_cond_.wait(lock);
        }

        if (s_count_pop >= s_queue_item_num * s_producer_thread_num) {
            // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
            break;
        }
    }
    printf("%s dequeue: last_value:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__, last_value, s_count_pop,
           s_queue_item_num, s_consumer_thread_num);
    PRINT_THREAD_LEAVE();
    return NULL;
}

int test_queue(thread_func_t func_push, thread_func_t func_pop, char **argv) {
    int64_t start = get_current_millisecond();
    pthread_t tid_push[s_producer_thread_num] = {0};
    for (int i = 0; i < s_producer_thread_num; i++) {
        int ret = pthread_create(&tid_push[i], NULL, func_push, argv);
        if (0 != ret) {
            printf("create thread failed\n");
        }
    }
    pthread_t tid_pop[s_consumer_thread_num] = {0};
    for (int i = 0; i < s_consumer_thread_num; i++) {
        int ret = pthread_create(&tid_pop[i], NULL, func_pop, argv);
        if (0 != ret) {
            printf("create thread failed\n");
        }
    }

    for (int i = 0; i < s_producer_thread_num; i++) {
        pthread_join(tid_push[i], NULL);
    }
    for (int i = 0; i < s_consumer_thread_num; i++) {
        pthread_join(tid_pop[i], NULL);
    }
    int64_t end = get_current_millisecond();
    int64_t temp = s_count_push;
    int64_t ops = (temp * 1000) / (end - start);
    printf("spend time : %ldms\t, push:%d, pop:%d, ops:%lu\n", (end - start), s_count_push, s_count_pop, ops);
    return 0;
}

// 多尝试几次 g++ -o 4_test_lock_free_queue 4_test_lock_free_queue.cpp -lpthread  -std=c++11
int main(int argc, char **argv) {
    if (argc >= 4 && atoi(argv[3]) > 0)
        s_queue_item_num = atoi(argv[3]);
    if (argc >= 3 && atoi(argv[2]) > 0)
        s_consumer_thread_num = atoi(argv[2]);
    if (argc >= 2 && atoi(argv[1]) > 0)
        s_producer_thread_num = atoi(argv[1]);

    printf("\nthread num - producer:%d, consumer:%d,  push:%d\n\n", s_producer_thread_num, s_consumer_thread_num,
           s_queue_item_num);
    for (int i = 0; i < 1; i++) {
        s_count_push = 0;
        s_count_pop = 0;
        printf("\n\n--------->i:%d\n", i);
#if 0
        printf("use mutexqueue ----------->\n");

        test_queue(mutexqueue_producer_thread, mutexqueue_consumer_thread, NULL);

        s_count_push = 0;
        s_count_pop = 0;
        printf("\nuse blockqueue ----------->\n");
        test_queue(blockqueue_producer_thread, blockqueue_consumer_thread, NULL);

        s_count_push = 0;
        s_count_pop = 0;

        printf("\nuse lockfreequeue ----------->\n"); // 指针
        test_queue(lockfreequeue_producer_thread, lockfreequeue_consumer_thread, NULL);

        s_count_push = 0;
        s_count_pop = 0;
        printf("\nuse arraylockfreequeue ----------->\n");
        test_queue(arraylockfreequeue_producer_thread, arraylockfreequeue_consumer_thread, NULL);
#endif
        if (s_consumer_thread_num == 1 && s_producer_thread_num == 1) {
            s_count_push = 0;
            s_count_pop = 0;
            //一次写就提交
            printf("\nuse ypipe_t ----------->\n");
            test_queue(yqueue_producer_thread, yqueue_consumer_thread, NULL);
            s_count_push = 0;
            s_count_pop = 0;
            //批量10次写提交
            printf("\nuse ypipe_t batch ----------->\n");
            test_queue(yqueue_producer_thread_batch, yqueue_consumer_thread_batch, NULL);

            s_count_push = 0;
            s_count_pop = 0;
            //cond
            printf("\nuse ypipe_t condition ----------->\n");
            test_queue(yqueue_producer_thread_condition, yqueue_consumer_thread_condition, NULL);
        }
        else {
            printf("\nypipe_t only support one write one read thread, bu you write %d thread and read %d thread so no test it ----------->\n",
                   s_producer_thread_num, s_consumer_thread_num);
        }
    }

    printf("finish\n");
    return 0;
}
