#include "../h/TCB.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"

extern  "C" void pushRegisters();
extern  "C" void popRegisters();
extern  "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);

TCB* TCB::running = nullptr;

static inline  size_t bytesToBlocks(size_t bytes){
    return (bytes+ MEM_BLOCK_SIZE- 1)/ MEM_BLOCK_SIZE;
}

void TCB::operator delete(void *p){
    MemoryAllocator::free(p);
}
void* TCB::operator new(size_t n) {
    return MemoryAllocator::alloc(bytesToBlocks(n));
}

TCB::TCB(Body b):
    body(b),stack(b?(uint64*) MemoryAllocator::alloc(bytesToBlocks(STACK_SIZE)):nullptr),
    context({b?(uint64) &threadWrapper:0,stack?(uint64)&stack[STACK_SIZE/sizeof(uint64)]:0}),
    finished(false),
    next(nullptr)
{
    if(b)Scheduler::put(this);
}

TCB::~TCB() {
if(stack) MemoryAllocator::free(stack);
}

TCB* TCB::createThread(Body body) {
    return new TCB(body);
}
void TCB::yield() {
    pushRegisters();
    dispatch();
    popRegisters();
}

void TCB::dispatch() {
    TCB* old= running;
    if (!old->isFinished())Scheduler::put(old);
    running= Scheduler::get();
    contextSwitch(&old->context,&running->context);
}

void TCB::threadWrapper() {
    running->body();
    running->setFinished(true);
    yield();
}