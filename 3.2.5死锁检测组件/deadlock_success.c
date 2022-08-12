
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define THREAD_NUM      10

/* ******* ******************Hook****************** ******* */

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mutex);

pthread_mutex_lock_t pthread_mutex_lock_f;

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mutex);

pthread_mutex_unlock_t pthread_mutex_unlock_f;

static int init_hook() {
    pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    pthread_mutex_unlock_f = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

/* ******* ******************Digraph****************** ******* */

#define MAX        100

enum Type {
    PROCESS, RESOURCE
};
//锁与线程的信息
struct pair_t {
    unsigned long int th_id;
    enum Type type;

    unsigned long int lock_id;
    int degress;
};
//顶点
struct vertex_t {
    struct pair_t pair;
    struct vertex_t *next;
};

struct task_graph {
    struct vertex_t vertex_list[MAX];
    int vertex_num;

    struct pair_t lock_list[MAX];
    int lock_num;

    pthread_mutex_t mutex;

    int path[MAX + 1];
    int visited[MAX];
    int k;
    int deadlock;
};

struct task_graph *tg = NULL;


//创建一个vertex
struct vertex_t *create_vertex(struct pair_t pair) {
    struct vertex_t *tex = (struct vertex_t *) malloc(sizeof(struct vertex_t));

    tex->pair = pair;
    tex->next = NULL;
    return tex;
}

//查找vertex在list里面的下标
int search_vertex(struct pair_t pair) {
    int i = 0;

    for (i = 0; i < tg->vertex_num; i++) {
        if (tg->vertex_list[i].pair.type == pair.type && tg->vertex_list[i].pair.th_id == pair.th_id) {
            return i;
        }
    }

    return -1;
}

//把vertex添加到vertex_list里面
void add_vertex(struct pair_t pair) {
    if (search_vertex(pair) == -1) {
        tg->vertex_list[tg->vertex_num].pair = pair;
        tg->vertex_list[tg->vertex_num].next = NULL;
        tg->vertex_num++;
    }
}

//添加边
int add_edge(struct pair_t u, struct pair_t v) {
    add_vertex(u);
    add_vertex(v);

    struct vertex_t *cnt = &(tg->vertex_list[search_vertex(u)]);

    while (cnt->next != NULL) {
        cnt = cnt->next;
    }

    cnt->next = create_vertex(v);
}

//检查边是否存在
int verify_edge(struct pair_t u, struct pair_t v) {
    if (tg->vertex_num == 0) return 0;

    int idx = search_vertex(u);
    if (idx == -1) {
        return 0;
    }

    struct vertex_t *cnt = &(tg->vertex_list[idx]);

    while (cnt != NULL) {
        if (cnt->pair.th_id == v.th_id) {
            return 1;
        }
        cnt = cnt->next;
    }
    return 0;
}

//删除边
int remove_edge(struct pair_t u, struct pair_t v) {

    int idx_u = search_vertex(u);
    int idx_v = search_vertex(v);

    if (idx_u != -1 && idx_v != -1) {

        struct vertex_t *cnt = &tg->vertex_list[idx_u];
        struct vertex_t *remove;

        while (cnt->next != NULL) {
            if (cnt->next->pair.th_id == v.th_id) {
                remove = cnt->next;
                cnt->next = cnt->next->next;
                free(remove);
                break;
            }
            cnt = cnt->next;
        }
    }
}

/* ******* ******************check cycle****************** ******* */

//打印
void print_deadlock(void) {
    int i = 0;
    printf("deadlock : ");
    for (i = 0; i < tg->k - 1; i++) {
        printf("%ld --> ", tg->vertex_list[tg->path[i]].pair.th_id);
    }
    printf("%ld\n", tg->vertex_list[tg->path[i]].pair.th_id);
}

void print_locklist(void) {
    int i = 0;

    printf("-----------print_locklist----------\n");
    for (i = 0; i < tg->lock_num; i++) {
        printf("threadid : %ld, lockid: %ld\n", tg->lock_list[i].th_id, tg->lock_list[i].lock_id);
    }
    printf("-----------------------------------\n");
}

int DFS(int idx) {
    struct vertex_t *ver = &tg->vertex_list[idx];
    if (tg->visited[idx] == 1) {
        tg->path[tg->k++] = idx;
        print_deadlock();
        tg->deadlock = 1;
        return 0;
    }

    tg->visited[idx] = 1;
    tg->path[tg->k++] = idx;

    while (ver->next != NULL) {
        DFS(search_vertex(ver->next->pair));
        tg->k--;
        ver = ver->next;
    }

    return 1;
}

//判断某个顶点是否成环
int search_for_cycle(int idx) {
    struct vertex_t *ver = &tg->vertex_list[idx];
    tg->visited[idx] = 1;
    tg->k = 0;
    tg->path[tg->k++] = idx;

    while (ver->next != NULL) {
        int i = 0;
        for (i = 0; i < tg->vertex_num; i++) {
            if (i == idx) continue;
            tg->visited[i] = 0;
        }

        for (i = 1; i <= MAX; i++) {
            tg->path[i] = -1;
        }
        tg->k = 1;

        DFS(search_vertex(ver->next->pair));
        ver = ver->next;
    }
}

//检查是否死锁
void check_dead_lock(void) {
    printf("-----------check deadlock----------\n");

    int i;
    tg->deadlock = 0;
    for (i = 0; i < tg->vertex_num; i++) {
        if (tg->deadlock == 1) {
            break;
        }
        //从每个点都出发一遍
        search_for_cycle(i);
    }
    if (tg->deadlock == 0) {
        printf("no deadlock\n");
    }

    printf("----------------------------------\n");

}


_Noreturn static void *check_dead_lock_routine(void *args) {
    while (1) {
        sleep(5);
        print_locklist();
        check_dead_lock();
    }
}


void start_check(void) {
    tg = (struct task_graph *) malloc(sizeof(struct task_graph));
    tg->vertex_num = 0;
    tg->lock_num = 0;
    pthread_t tid;

    pthread_create(&tid, NULL, check_dead_lock_routine, NULL);
}


//查找lock所在lock_list的索引位置
int search_lock(unsigned long int lock) {
    int i = 0;
    for (i = 0; i < tg->lock_num; i++) {
        if (tg->lock_list[i].lock_id == lock) {
            return i;
        }
    }
    return -1;
}

//找到一个lock_list为空的位置
int search_empty_lock(unsigned long int lock) {
    int i = 0;
    for (i = 0; i < tg->lock_num; i++) {
        if (tg->lock_list[i].lock_id == 0) {
            return i;
        }
    }
    return tg->lock_num;
}


int inc(const int *value, int add) {
    int old;
    __asm__ volatile(
    "lock;xaddl %2, %1;"
    : "=a"(old)
    : "m"(*value), "a" (add)
    : "cc", "memory"
    );
    return old;
}

/* ******* ******************primitive operations****************** ******* */


void lock_before(unsigned long int thread_id, unsigned long int lock) {
    pthread_mutex_lock_f(&tg->mutex);

    int idx = search_lock(lock);
//    printf("[lock_before] self_id:%lu lock:%lu lock idx:%d \n", thread_id, lock, idx);
    //如果该锁是第一次则什么都不做
    if (idx != -1) {
        //u是想要加锁的线程
        struct pair_t u;
        u.th_id = thread_id;
        u.type = PROCESS;
        //把vertex添加到vertex_list里面
        add_vertex(u);
        //v是锁原来的线程
        struct pair_t v;
        v.th_id = tg->lock_list[idx].th_id;
        tg->lock_list[idx].degress++;
        v.type = PROCESS;
        add_vertex(v);

        if (!verify_edge(u, v)) {
            add_edge(u, v); // 把v加入到vertex_list的u的链表中
        }
    }

    pthread_mutex_unlock_f(&tg->mutex);
}

void lock_after(unsigned long int thread_id, unsigned long int lock) {
    pthread_mutex_lock_f(&tg->mutex);

    int idx = search_lock(lock);
//    printf("[lock_after ] self_id:%lu lock:%lu ", thread_id, lock);

    if (idx == -1) {  // 第一次加锁,找一个空位lock_list，设置th_id和lock
        int empty_lock_idx = search_empty_lock(lock);
        tg->lock_list[empty_lock_idx].th_id = thread_id;
        tg->lock_list[empty_lock_idx].lock_id = lock;
//        printf("分配lock_list位置 idx:%d \n", empty_lock_idx);
        if (empty_lock_idx >= tg->lock_num) {
            inc(&tg->lock_num, 1);
        }
    }
    else {
        //u是想要加锁的线程
        struct pair_t u;
        u.th_id = thread_id;
        u.type = PROCESS;
        //v是锁原来的线程
        struct pair_t v;
        v.th_id = tg->lock_list[idx].th_id;
        tg->lock_list[idx].degress--;
        v.type = PROCESS;
        //删除边
        if (verify_edge(u, v)) {
            remove_edge(u, v);
        }
        //设为本线程
        tg->lock_list[idx].th_id = thread_id;

//        printf("获得 lock idx:%d \n", idx);
    }

    pthread_mutex_unlock_f(&tg->mutex);
}

void unlock_after(unsigned long int thread_id, unsigned long int lock) {
    pthread_mutex_lock_f(&tg->mutex);

    int idx = search_lock(lock);
    //如果入度为0，说明没有别的线程指向该锁，则把这个idx位置置空
    if (tg->lock_list[idx].degress == 0) {
        tg->lock_list[idx].th_id = 0;
        tg->lock_list[idx].lock_id = 0;
    }

    pthread_mutex_unlock_f(&tg->mutex);
}


int pthread_mutex_lock(pthread_mutex_t *mutex) {
    pthread_t self_id = pthread_self();
    lock_before(self_id, (unsigned long int) mutex);
    pthread_mutex_lock_f(mutex);
    lock_after(self_id, (unsigned long int) mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    pthread_t self_id = pthread_self();

    pthread_mutex_unlock_f(mutex);
    unlock_after(self_id, (unsigned long int) mutex);
}


/* ******* ******************Debug 1****************** ******* */
#if 0
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;

void *thread_rountine_1(void *args) {
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 1 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_1);
    sleep(1);
    pthread_mutex_lock(&mutex_2);

    pthread_mutex_unlock(&mutex_2);
    pthread_mutex_unlock(&mutex_1);

    return (void *) (0);
}

void *thread_rountine_2(void *args) {
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 2 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_2);
    sleep(1);
    pthread_mutex_lock(&mutex_3);

    pthread_mutex_unlock(&mutex_3);
    pthread_mutex_unlock(&mutex_2);

    return (void *) (0);
}

void *thread_rountine_3(void *args) {
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 3 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_3);
    sleep(1);
    pthread_mutex_lock(&mutex_4);

    pthread_mutex_unlock(&mutex_4);
    pthread_mutex_unlock(&mutex_3);

    return (void *) (0);
}

void *thread_rountine_4(void *args) {
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 4 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_4);
    sleep(1);
    pthread_mutex_lock(&mutex_1);

    pthread_mutex_unlock(&mutex_1);
    pthread_mutex_unlock(&mutex_4);

    return (void *) (0);
}


int main() {
    init_hook();
    start_check();

    printf("start_check\n");

    pthread_t tid1, tid2, tid3, tid4;
    pthread_create(&tid1, NULL, thread_rountine_1, NULL);
    pthread_create(&tid2, NULL, thread_rountine_2, NULL);
    pthread_create(&tid3, NULL, thread_rountine_3, NULL);
    pthread_create(&tid4, NULL, thread_rountine_4, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);

    return 0;
}

#endif
/* ******* ******************Debug 2****************** ******* */
#if 0

int main() {
    tg = (struct task_graph *) malloc(sizeof(struct task_graph));
    tg->vertex_num = 0;

    struct pair_t v1;
    v1.th_id = 1;
    v1.type = PROCESS;
    add_vertex(v1);

    struct pair_t v2;
    v2.th_id = 2;
    v2.type = PROCESS;
    add_vertex(v2);

    struct pair_t v3;
    v3.th_id = 3;
    v3.type = PROCESS;
    add_vertex(v3);

    struct pair_t v4;
    v4.th_id = 4;
    v4.type = PROCESS;
    add_vertex(v4);


    struct pair_t v5;
    v5.th_id = 5;
    v5.type = PROCESS;
    add_vertex(v5);


    add_edge(v1, v2);
    add_edge(v2, v3);
    add_edge(v3, v4);
    add_edge(v4, v5);
    add_edge(v3, v1);
    add_edge(v5, v1);

    check_dead_lock();
//    search_for_cycle(search_vertex(v1));
}
#endif



