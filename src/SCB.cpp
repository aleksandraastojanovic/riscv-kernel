#include "../h/SCB.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"


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
    TCB::switchToNext(old);
}

void SCB::deblock(int res) {
    TCB* t = dequeue();
    if (!t) return;
    t->semResult = res;
    Scheduler::put(t);

}

int SCB::wait() {
    return wait_n(1);
}

int SCB::signal() {
    return signal_n(1);
}

int SCB::wait_n(unsigned n) {
    if (val>= (int) n) {
        val-=(int) n;
        return 0;
    }
    TCB::running->semNeed = n;
    block();
    return TCB::running->semResult;
}

int SCB::signal_n(unsigned n) {
    val+= (int) n;
    while (head && val >= (int) head->semNeed) {
        val-= (int) head->semNeed;
        deblock(0);
    }
    return 0;
}

void SCB::closeAll() {
    while (head) {deblock(-1);}
}

