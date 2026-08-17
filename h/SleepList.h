//
// Created by os on 8/17/26.
//

#ifndef KERNEL_FRE_SLEEPLIST_H
#define KERNEL_FRE_SLEEPLIST_H



#include "../lib/hw.h"
class TCB;

// Lista uspavanih niti, sortirana po vremenu budjenja.
// Svaki element pamti vreme RELATIVNO u odnosu na prethodnika
class SleepList {
public:
    static void put(time_t relativeTime);  //uspavljivanje tekuceniti
    static void tick();
private:
    static TCB* head;
};


#endif //KERNEL_FRE_SLEEPLIST_H
