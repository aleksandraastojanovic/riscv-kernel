#include "../h/SleepList.h"
#include "../h/TCB.h"
#include "../h/Scheduler.h"

extern "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);

TCB* SleepList::head = nullptr;

void SleepList::put(time_t relativeTime) {
    TCB* t = TCB::running;

    // nadji mesto: trosimo delte prethodnika dok nas neko ne "prestigne"
    TCB* prev = nullptr;
    TCB* cur = head;
    while (cur && relativeTime >= cur->sleepLeft) {
        relativeTime -= cur->sleepLeft;
        prev = cur;
        cur = cur->next;
    }

    t->sleepLeft = relativeTime;          // nasa delta u odnosu na prev
    if (cur) cur->sleepLeft -= relativeTime;  // sledeci sad ceka relativno od NAS

    t->next = cur;
    if (prev) prev->next = t;
    else head = t;

    // uspavaj tekucu nit - identican obrazac kao SCB::block
    TCB::running = TCB::pickNext();
    contextSwitch(&t->context, &TCB::running->context);
}

void SleepList::tick() {
    if (!head) return;
    if (head->sleepLeft > 0) --head->sleepLeft;   // O(1): samo celo lista "stari"
    while (head && head->sleepLeft == 0) {        // probudi SVE kojima je vreme
        TCB* t = head;
        head = head->next;
        t->next = nullptr;
        Scheduler::put(t);
    }
}