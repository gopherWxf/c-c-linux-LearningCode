//
// Created by 68725 on 2022/8/16.
//
#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <link.h>
#include <unistd.h>
#include <malloc.h>

typedef void *(*malloc_hook_t)(size_t size, const void *caller);

malloc_hook_t malloc_f;

typedef void (*free_hook_t)(void *p, const void *caller);

free_hook_t free_f;

int replaced = 0;

void mem_trace(void);

void mem_untrace(void);

const void *ConvertToVMAToSystem_addr2line(const void *addr, const void *ptr) {
    Dl_info info;
    struct link_map *link_map;
    dladdr1((void *) addr, &info, (void **) &link_map, RTLD_DL_LINKMAP);
    const void *caller = addr - link_map->l_addr;

    char command[256];
    snprintf(command, sizeof(command), "addr2line -f -e %s -a %p > ./mem/%p.mem", info.dli_fname, caller, ptr);
    system(command);

    return caller;
}

void *malloc_hook_f(size_t size, const void *caller) {
    mem_untrace();

    void *ptr = malloc(size);

    caller = ConvertToVMAToSystem_addr2line(caller, ptr);
    printf("[+%p]--->ptr:%p size:%zu\n", caller, ptr, size);

    mem_trace();
    return ptr;
}

void free_hook_f(void *ptr, const void *caller) {
    mem_untrace();

    caller = ConvertToVMAToSystem_addr2line(caller, ptr);
    printf("[-%p]--->ptr:%p\n", caller, ptr);

    char buff[128] = {0};
    sprintf(buff, "./mem/%p.mem", ptr);
    if (unlink(buff) < 0) {
        printf("double kill:%p\n", ptr);
    }
    free(ptr);

    mem_trace();
}

//replaced=1代表正在trace，__malloc_hook走我们自己写的函数
//replaced=0代表没有trace，__malloc_hook走系统自己的函数
void mem_trace(void) { //mtrace
    replaced = 1;
    //让malloc_f指向系统的
    malloc_f = __malloc_hook;
    //free_f
    free_f = __free_hook;
    //让系统的hook指向我们写的函数
    __malloc_hook = malloc_hook_f;
    __free_hook = free_hook_f;
}

void mem_untrace(void) {
    //让系统的hook指向系统的malloc和free
    __malloc_hook = malloc_f;
    __free_hook = free_f;
    replaced = 0;
}

int main() {
    mem_trace();

    void *p1 = malloc(10);
    void *p2 = malloc(20);
    void *p3 = malloc(30);
    void *p4 = malloc(40);
    free(p1);
    free(p2);
    free(p4);

    mem_untrace();
}