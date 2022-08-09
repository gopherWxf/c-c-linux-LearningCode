//
// Created by 68725 on 2022/7/25.
//

#include <pthread.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>


//头插法
#define LL_ADD(item, list)do{   \
    item->prev=NULL;            \
    item->next=list;            \
    if(list!=NULL){             \
        list->prev=item;        \
    }                           \
    list=item;                  \
}while(0)

#define LL_REMOVE(item, list)do{        \
    if(item->prev!=NULL){               \
        item->prev->next=item->next;    \
    }                                   \
    if(item->next!=NULL){               \
        item->next->prev=item->prev;    \
    }                                   \
    if(list==item){                     \
        list=item->next;                \
    }                                   \
    item->prev=item->next=NULL;         \
}while(0)

//执行队列
typedef struct NWORKER {
    pthread_t id;
    int termination;
    struct NTHREADPOLL *thread_poll;

    struct NWORKER *prev;
    struct NWORKER *next;
} worker_t;

//任务队列
typedef struct NTASK {
    void (*task_func)(struct NTASK *arg);

    void *user_data;

    struct NTASK *prev;
    struct NTASK *next;
} task_t;

//池管理组件
typedef struct NTHREADPOLL {
    worker_t *workers;
    task_t *tasks;

    pthread_cond_t cond;
    pthread_mutex_t mutex;
} thread_poll_t;

task_t *get_task(worker_t *worker) {
    while (1) {
        pthread_mutex_lock(&worker->thread_poll->mutex);
        while (worker->thread_poll->workers == NULL) {
            if (worker->termination)break;
            pthread_cond_wait(&worker->thread_poll->cond, &worker->thread_poll->mutex);
        }
        if (worker->termination) {
            pthread_mutex_unlock(&worker->thread_poll->mutex);
            return NULL;
        }
        task_t *task = worker->thread_poll->tasks;
        if (task) {
            LL_REMOVE(task, worker->thread_poll->tasks);
        }
        pthread_mutex_unlock(&worker->thread_poll->mutex);
        if (task != NULL) {
            return task;
        }
    }
}

void *thread_callback(void *arg) {
    worker_t *worker = (worker_t *) arg;
    while (1) {
        task_t *task = get_task(worker);
        if (task == NULL) {
            free(worker);
            pthread_exit("thread termination\n");
        }
        task->task_func(task);
    }
}

//return access create thread num;
int thread_poll_create(thread_poll_t *thread_poll, int thread_num) {
    if (thread_num < 1)thread_num = 1;
    memset(thread_poll, 0, sizeof(thread_poll_t));
    //init cond
    thread_poll->cond=PTHREAD_COND_INITIALIZER;
    //init mutex
    thread_poll->mutex=PTHREAD_MUTEX_INITIALIZER;
    // one thread one worker
    int idx;
    for (idx = 0; idx < thread_num; idx++) {
        worker_t *worker = malloc(sizeof(worker_t));
        if (worker == NULL) {
            perror("worker malloc err\n");
            return idx;
        }
        memset(worker, 0, sizeof(worker_t));
        worker->thread_poll = thread_poll;

        int ret = pthread_create(&worker->id, NULL, thread_callback, worker);
        if (ret) {
            perror("pthread_create err\n");
            free(worker);
            return idx;
        }
        LL_ADD(worker, thread_poll->workers);
    }
    return idx;
}

void thread_poll_push_task(thread_poll_t *thread_poll, task_t *task) {
    pthread_mutex_lock(&thread_poll->mutex);
    LL_ADD(task, thread_poll->tasks);
    pthread_cond_signal(&thread_poll->cond);
    pthread_mutex_unlock(&thread_poll->mutex);
}

void thread_destroy(thread_poll_t *thread_poll) {
    worker_t *worker = NULL;
    for (worker = thread_poll->workers; worker != NULL; worker = worker->next) {
        worker->termination = 1;
    }
    pthread_mutex_lock(&thread_poll->mutex);
    pthread_cond_broadcast(&thread_poll->cond);
    pthread_mutex_unlock(&thread_poll->mutex);
}

void counter(task_t *task) {
    int idx = *(int *) task->user_data;
    printf("idx:%d  pthread_id:%llu\n", idx, pthread_self());
    free(task->user_data);
    free(task);
}

#define THREAD_COUNT 10
#define TASK_COUNT 1000

int main() {
    thread_poll_t thread_poll = {0};
    int ret = thread_poll_create(&thread_poll, THREAD_COUNT);
    if (ret != THREAD_COUNT) {
        thread_destroy(&thread_poll);
    }
    int i;
    for (i = 0; i < TASK_COUNT; i++) {
        //create task
        task_t *task = (task_t *) malloc(sizeof(task_t));
        if (task == NULL) {
            perror("task malloc err\n");
            exit(1);
        }
        task->task_func = counter;
        task->user_data = malloc(sizeof(int));
        *(int *) task->user_data = i;
        //push task
        thread_poll_push_task(&thread_poll, task);
    }
    getchar();
    thread_destroy(&thread_poll);
}