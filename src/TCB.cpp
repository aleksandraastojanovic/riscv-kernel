#include "../h/riscv.h"
#include  "../h/syscall_c.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"


extern  "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);

TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;

static inline  size_t bytesToBlocks(size_t bytes){
    return (bytes+ MEM_BLOCK_SIZE- 1)/ MEM_BLOCK_SIZE;
}

void TCB::operator delete(void *p){
    MemoryAllocator::free(p);
}
void* TCB::operator new(size_t n) {
    return MemoryAllocator::alloc(bytesToBlocks(n));
}

TCB::TCB(Body b, void* a, void* stack_space) :
    body(b),
    arg(a),
    // stack_space pokazuje na KRAJ prostora; pocetak (za oslobadjanje) je
    // DEFAULT_STACK_SIZE bajtova unazad - toliko C API uvek alocira
    stackBegin(stack_space
        ? (uint64*)((char*)stack_space - DEFAULT_STACK_SIZE) : nullptr),

    context({ b ? (uint64) &threadWrapper : 0, (uint64) stack_space }),
    finished(false),
    next(nullptr)
{
    if (b) Scheduler::put(this);
}

TCB::~TCB() {
if(stackBegin) MemoryAllocator::free(stackBegin);
}

TCB* TCB::createThread(Body body, void* arg, void* stack_space) {
    return new TCB(body, arg, stack_space);
}

void TCB::dispatch() {
    timeSliceCounter = 0;
    TCB* old = running;
    if (!old->isFinished()) Scheduler::put(old);  // zavrsene se ne vracaju u listu
    running = Scheduler::get();
    contextSwitch(&old->context, &running->context);

}

void TCB::threadWrapper() {
    Riscv::popSppSpie();           // iskoci iz trap
    running->body(running->arg);
    thread_exit();
}


void TCB::onTimerTick() {
    if (++timeSliceCounter >= DEFAULT_TIME_SLICE)
        dispatch();
}