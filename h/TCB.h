//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_TCB_H
#define PROJECT_BASE_V1_1_TCB_H


#include "../lib/hw.h"
class TCB{
public:
    using Body = void(*)(void*);
    static TCB* createThread(Body body,void* arg,void* stack_space);
    static void dispatch();


    bool isFinished()const{return finished;}
    void setFinished(bool value) {finished =value;}
    static TCB* running;
    ~TCB();

    void* operator new(size_t n);
    void operator delete(void* p);

    struct Context {
        uint64 ra;
        uint64 sp;
        uint64 s[12];   // s0-s11
    };
private:
    explicit TCB(Body body,void* arg, void* stack_space);

    static void threadWrapper();
    Body body;
    void*arg;
    uint64 *stackBegin;
    Context context;
    bool finished;
    TCB* next;
    int semResult;

    friend class Scheduler;
    friend class SCB;


};

#endif //PROJECT_BASE_V1_1_TCB_H
