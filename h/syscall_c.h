//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_SYSCALL_C_H
#define PROJECT_BASE_V1_1_SYSCALL_C_H
// c api sloj

#include "../lib/hw.h"
void* mem_alloc(size_t size);
int mem_free(void* ptr);
class _thread;
typedef _thread* thread_t;

int  thread_create(thread_t* handle, void (*start_routine)(void*), void* arg);
int  thread_exit();
void thread_dispatch();

#endif //PROJECT_BASE_V1_1_SYSCALL_C_H
