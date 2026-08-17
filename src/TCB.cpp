#include "../h/riscv.h"
#include  "../h/syscall_c.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"


extern  "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);

TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;
TCB* TCB::idle = nullptr;

static inline  size_t bytesToBlocks(size_t bytes){
    return (bytes+ MEM_BLOCK_SIZE- 1)/ MEM_BLOCK_SIZE;
}

void TCB::operator delete(void *p){
    MemoryAllocator::free(p);
}
void* TCB::operator new(size_t n) {
    return MemoryAllocator::alloc(bytesToBlocks(n));
}

TCB::TCB(Body b, void* a, void* stack_space,bool start, bool system) :
    body(b),
    arg(a),
    // stack_space pokazuje na KRAJ prostora; pocetak (za oslobadjanje) je
    // DEFAULT_STACK_SIZE bajtova unazad - toliko C API uvek alocira
    stackBegin(stack_space
        ? (uint64*)((char*)stack_space - DEFAULT_STACK_SIZE) : nullptr),

    context({ b ? (uint64) &threadWrapper : 0, (uint64) stack_space }),
    finished(false),
    next(nullptr),
systemThread(system)
{
    if (b&& start) Scheduler::put(this);
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
    if (!old->isFinished() && old!= idle) Scheduler::put(old);  // zavrsene se ne vracaju u listu
    running = pickNext();
    contextSwitch(&old->context, &running->context);

}

void TCB::threadWrapper() {
    uint64 sstatus = Riscv::r_sstatus();
    if (running->systemThread) sstatus |= Riscv::SSTATUS_SPP;
    else sstatus &= ~Riscv::SSTATUS_SPP;
    sstatus |= Riscv::SSTATUS_SPIE;
    Riscv::w_sstatus(sstatus);
    Riscv::popSppSpie();           // iskoci iz trap
    running->body(running->arg);
    thread_exit();
}


void TCB::onTimerTick() {
    if (++timeSliceCounter >= DEFAULT_TIME_SLICE)
        dispatch();
}

TCB* TCB::pickNext() {
    TCB* next = Scheduler::get();
    return next? next:idle;
}

void TCB::idleBody(void*) {
    for (;;)thread_dispatch();
}


void TCB::initIdle() {
    // alloc prima BLOKOVE: 4096 B / 64 B = 64 bloka
    void* space = MemoryAllocator::alloc(DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE);
    // vrh steka = pocetak + velicina u BAJTOVIMA; false = ne ide u Scheduler
    idle = new TCB(&idleBody, nullptr, (char*)space + DEFAULT_STACK_SIZE, false,true);
}