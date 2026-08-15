//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_SYSCALL_C_H
#define PROJECT_BASE_V1_1_SYSCALL_C_H
// c api sloj

#include "../lib/hw.h"
void* mem_alloc(size_t size);
int mem_free(void* ptr);

#endif //PROJECT_BASE_V1_1_SYSCALL_C_H
