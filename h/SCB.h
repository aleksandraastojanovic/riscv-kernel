//
// Created by os on 8/16/26.
//

#ifndef PROJECT_BASE_V1_1_SCB_H
#define PROJECT_BASE_V1_1_SCB_H

#include "../lib/hw.h"
class TCB;
class SCB {
    public:
    explicit SCB(int init): val(init), head(nullptr), tail(nullptr) {}

    int wait();
    int signal();
    void closeAll();

    void* operator new(size_t n);
    void operator delete(void* p);
private:
    void block();
    void deblock(int res);
    void enqueue(TCB* t);
    TCB* dequeue();

    int val;
    TCB* head;
    TCB* tail;
};

#endif //PROJECT_BASE_V1_1_SCB_H
