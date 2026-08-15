//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_SCHEDULER_H
#define PROJECT_BASE_V1_1_SCHEDULER_H

//fifo red za niti

class TCB;

class Scheduler{
public:
    static void put(TCB* thread);
    static TCB* get();
private:
    static TCB* head;
    static TCB* tail;
};

#endif //PROJECT_BASE_V1_1_SCHEDULER_H
