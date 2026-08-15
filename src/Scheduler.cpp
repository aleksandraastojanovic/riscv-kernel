#include "../h/Scheduler.h"
#include "../h/TCB.h"

TCB* Scheduler::head = nullptr;
TCB* Scheduler::tail = nullptr;

void Scheduler::put(TCB *thread) {
    if(!thread) return;
    thread->next = nullptr;
    if(tail) tail->next =thread;
    else head = thread;
    tail= thread;
}
TCB* Scheduler::get() {
    if(!head) return nullptr;
    TCB* thread = head;
    head= head->next;
    if(!head) tail = nullptr;
    thread->next = nullptr;
    return thread;
}