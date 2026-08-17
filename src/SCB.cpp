#include "../h/SCB.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"


extern  "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);


static inline  size_t bytesToBlocks(size_t bytes){
    return (bytes+ MEM_BLOCK_SIZE- 1)/ MEM_BLOCK_SIZE;
}

void SCB::operator delete(void *p){
    MemoryAllocator::free(p);
}
void* SCB::operator new(size_t n) {
    return MemoryAllocator::alloc(bytesToBlocks(n));
}

void SCB::enqueue(TCB* tcb) {
    tcb->next = nullptr;
    if (tail) tail->next = tcb;
    else head = tcb;
    tail = tcb;
}

TCB* SCB::dequeue() {
    if (!head) return nullptr;
    TCB* tcb = head;
    head = head->next;
    if (!head) tail = nullptr;
    tcb->next = nullptr;
    return tcb;
}

void SCB::block() {
    TCB* old = TCB::running;
    enqueue(old);
    TCB::running = TCB::pickNext();
    contextSwitch(&old->context, &TCB::running->context);
}

void SCB::deblock(int res) {
    TCB* t = dequeue();
    if (!t) return;
    t->semResult = res;
    Scheduler::put(t);

}

int SCB::wait() {
    if (--val < 0) {
        block();
        return TCB::running->semResult;
    }
    return 0;
}

int SCB::signal() {
    if (++val<= 0) deblock(0);
    return 0;
}

void SCB::closeAll() {
    while (head) {deblock(-1);}
}

