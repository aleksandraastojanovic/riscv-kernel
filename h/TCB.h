//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_TCB_H
#define PROJECT_BASE_V1_1_TCB_H


#include "../lib/hw.h"
class TCB{
public:
    using Body = void(*)();
    static TCB* createThread(Body body);

    bool isFinished()const{return finished;}
    void setFinished(bool value) {finished =value;}
    static TCB* running;
    ~TCB();

    void* operator new(size_t n);
    void operator delete(void* p);
private:
    explicit TCB(Body body);
    struct Context{
        uint64 ra;
        uint64 sp;
    };

    Body body;
    uint64 *stack;
    Context context;
    bool finished;
    TCB* next;

    friend class Scheduler;

    static const uint64 STACK_SIZE = DEFAULT_STACK_SIZE;
};

#endif //PROJECT_BASE_V1_1_TCB_H
