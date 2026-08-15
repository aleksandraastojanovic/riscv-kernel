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

const int EOF = -1;
char getc();                    // ucitava znak; EOF u slucaju greske
void putc(char chr);            // ispisuje znak


class _sem;
typedef _sem* sem_t;

int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);

// uspavljivanje niti (zadatak 4)
int time_sleep(time_t);
#endif //PROJECT_BASE_V1_1_SYSCALL_C_H
