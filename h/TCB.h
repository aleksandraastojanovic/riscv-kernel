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
    static void onTimerTick();
    static void initIdle();
    static TCB* createSystemThread(Body body, void* arg);  // interne niti jezgra

    void* operator new(size_t n);
    void operator delete(void* p);

    struct Context {
        uint64 ra;
        uint64 sp;
        uint64 s[12];   // s0-s11
    };
private:
    explicit TCB(Body body, void* arg, void* stack_space, bool start = true, bool system = false);
    static void threadWrapper();
    Body body;
    void*arg;
    void* stackSpace;    // vrednost stigla kroz ABI thread_create (kraj steka);
                         // stek je alocirao C API sa mem_alloc(DEFAULT_STACK_SIZE)
    Context context;
    bool finished;
    TCB* next;
    int semResult;
    unsigned semNeed = 1;
    static uint64 timeSliceCounter;

    static TCB* pickNext();
    // zajednicka putanja za SVAKO oduzimanje procesora tekucoj niti (dispatch,
    // blokiranje na semaforu, uspavljivanje): bira sledecu, dodeljuje joj pun
    // vremenski odsecak i prebacuje kontekst
    static void switchToNext(TCB* old);

    // odlozeno brisanje: nit koja se zavrsila ne sme da oslobodi sopstveni stek
    // dok na njemu radi, pa to radi PRVI kod jezgra koji se izvrsi na steku
    // nove tekuce niti (posle contextSwitch ili na pocetku threadWrapper)
    static TCB* lastSwitchedOut;
    static void reapLastSwitchedOut();
    static void idleBody(void*);
    static TCB* idle;
    time_t sleepLeft = 0;

    friend class Scheduler;
    friend class SCB;
    friend class SleepList;

    bool systemThread;   // telo se izvrsava u S (interne niti jezgra) ili U rezimu


};

#endif //PROJECT_BASE_V1_1_TCB_H
