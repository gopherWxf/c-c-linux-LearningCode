


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define __USE_GNU
#include <sched.h>

#include <unistd.h>
#include <pthread.h>
#include <setjmp.h>

#include <sys/syscall.h>

#define THREAD_COUNT		3


pthread_mutex_t mutex;
pthread_spinlock_t spinlock;


int inc(int *value, int add) {

	int old;
	__asm__ volatile (
		"lock; xaddl %2, %1;"
		: "=a" (old)
		: "m" (*value), "a" (add)
		: "cc", "memory"
	);
	return old;

}



void *func(void *arg) {

	int *pcount = (int *)arg;

	int i = 0;

	//
	
	while (i ++ < 100000) {
#if 0
		(*pcount) ++;
#elif 0
// 
		pthread_mutex_lock(&mutex);
		(*pcount) ++;
		pthread_mutex_unlock(&mutex);

#elif 0

		pthread_spin_lock(&spinlock);
		(*pcount) ++;
		pthread_spin_unlock(&spinlock);

#else

		inc(pcount, 1);

#endif
		usleep(1);

	}
	

}


pthread_key_t key;

typedef void *(*thread_cb)(void *);


void print_thread1_key(void) {

	int *p = (int *)pthread_getspecific(key);

	printf("thread 1 : %d\n", *p);

}

void *thread1_proc(void *arg) {

	int i = 5;	
	pthread_setspecific(key, &i);

	print_thread1_key();

}

void print_thread2_key(void) {

	char *ptr = (char *)pthread_getspecific(key);

	printf("thread 2 : %s\n", ptr);

}



void *thread2_proc(void *arg) {

	char *ptr = "thread2_proc";
	pthread_setspecific(key, ptr);
	print_thread2_key();
	
}


struct pair {
	int x;
	int y;
};

void print_thread3_key(void) {

	struct pair *p = (struct pair *)pthread_getspecific(key);

	printf("thread 3  x: %d, y: %d\n", p->x, p->y);

}


void *thread3_proc(void *arg) {

	struct pair p = {1, 2};

	pthread_setspecific(key, &p);
	print_thread3_key();

}


// setjmp / longjmp

struct ExceptionFrame {

	jmp_buf env;
	int count;

	struct ExceptionFrame *next;

};

// stack --> current node save





jmp_buf env;
int count = 0;


#define Try 	count = setjmp(env); if (count == 0) 

#define Catch(type)  else if (count == type) 

#define Throw(type)	  longjmp(env, type);

#define Finally			

void sub_func(int idx) {

	printf("sub_func : %d\n", idx);
	//longjmp(env, idx);

	Throw(idx);

}


void process_affinity(int num) {

	//gettid();
	pid_t selfid = syscall(__NR_gettid);

	cpu_set_t mask;
	CPU_ZERO(&mask);

	CPU_SET(1, &mask);

	//selfid
	sched_setaffinity(0, sizeof(mask), &mask);

	while(1) ;
}


int main() {

	// 8
	int num = sysconf(_SC_NPROCESSORS_CONF);

	int i = 0;
	pid_t pid = 0;
	for (i = 0;i < num/2;i ++) {
		pid = fork();
		if (pid <= (pid_t)0) {
			break;
		}
	}

	if (pid == 0) {
		process_affinity(num);
	}

	while (1) usleep(1);


#if 0
	count = setjmp(env);
	if (count == 0) {
		
		sub_func(++count);
	} else if (count == 1) {
		sub_func(++count);
	} else if (count == 2) {
		sub_func(++count);
	} else if (count == 3) {
		sub_func(++count);
	} 
	{
		
		printf("other item\n");
	}
#elif 0

	Try {

		sub_func(++count);
		
	} Catch (1) {

		sub_func(++count);

	} Catch (2) {

		sub_func(++count);

	} Catch (3) {

		sub_func(++count);

	} Finally {

		printf("other item\n");

	}

#endif
	

	pthread_t thid[THREAD_COUNT] = {0};
	int count = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_spin_init(&spinlock, PTHREAD_PROCESS_SHARED);

	pthread_key_create(&key, NULL);

#if 0
	int i = 0;
	for (i = 0;i < THREAD_COUNT;i ++) {
		pthread_create(&thid[i], NULL, func, &count);
	}
	

	for (i = 0;i < 100;i ++) {
		printf("count --> %d\n", count);
		sleep(1);
	}
#endif
#if 0
	thread_cb callback[THREAD_COUNT] = {
		thread1_proc,
		thread2_proc,
		thread3_proc
	};

	int i = 0;
	for (i = 0;i < THREAD_COUNT;i ++) {
		pthread_create(&thid[i], NULL, callback[i], &count);
	}

	for (i = 0;i < THREAD_COUNT;i ++) {
		pthread_join(thid[i], NULL);
	}
#endif

	 
	

	return 0;
}


