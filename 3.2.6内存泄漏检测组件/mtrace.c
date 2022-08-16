//
// Created by 68725 on 2022/8/16.
//
#include <malloc.h>
#include <mcheck.h>

int main() {
    mtrace();

    void *p1 = malloc(10);
    void *p2 = malloc(20);
    void *p3 = malloc(30);
    void *p4 = malloc(40);
    free(p1);
    free(p2);
    free(p4);

    muntrace();
}