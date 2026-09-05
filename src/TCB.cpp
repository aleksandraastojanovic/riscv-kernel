#include "../h/riscv.h"
#include  "../h/syscall_c.h"

#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/MemoryAllocator.h"


extern  "C" void contextSwitch(TCB::Context* old, TCB::Context* newContext);

TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;
TCB* TCB::idle = nullptr;
TCB* TCB::lastSwitchedOut = nullptr;

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
    stackSpace(stack_space),
    context({ b ? (uint64) &threadWrapper : 0, (uint64) stack_space }),
    finished(false),
    next(nullptr),
systemThread(system)
{
    if (b&& start) Scheduler::put(this);
}

TCB::~TCB() {
    // stack_space pokazuje na KRAJ prostora; pocetak (vrednost koju je vratio
    // mem_alloc) je DEFAULT_STACK_SIZE bajtova unazad - toliko C API uvek alocira
    if (stackSpace) MemoryAllocator::free((char*) stackSpace - DEFAULT_STACK_SIZE);
}

void TCB::reapLastSwitchedOut() {
    TCB* t = lastSwitchedOut;
    lastSwitchedOut = nullptr;          // svaki kandidat se razmatra tacno jednom
    if (!t || t == running) return;
    // brisu se samo ZAVRSENE KORISNICKE niti; idle, pocetna nit jezgra (bez tela)
    // i interne sistemske niti (izlazna nit konzole) se nikad ne brisu
    if (t->finished && !t->systemThread && t != idle && t->body) delete t;
}

TCB* TCB::createThread(Body body, void* arg, void* stack_space) {
    return new TCB(body, arg, stack_space);
}

void TCB::dispatch() {
    TCB* old = running;
    if (!old->isFinished() && old!= idle) Scheduler::put(old);  // zavrsene se ne vracaju u listu
    switchToNext(old);
}

void TCB::switchToNext(TCB* old) {
    timeSliceCounter = 0;          // nova tekuca nit uvek dobija pun odsecak
    running = pickNext();
    lastSwitchedOut = old;
    contextSwitch(&old->context, &running->context);
    // odavde se izvrsava nit koja je (ponovo) dobila procesor, na SVOM steku:
    // bezbedno je osloboditi nit koja ga je upravo izgubila ako se zavrsila
    reapLastSwitchedOut();
}

void TCB::threadWrapper() {
    reapLastSwitchedOut();         // nit pokrenuta prvi put: isti posao kao posle contextSwitch
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

TCB* TCB::createSystemThread(Body body, void* arg) {
    void* space = MemoryAllocator::alloc(DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE);
    if (!space) return nullptr;
    // start=true (odmah u Scheduler), system=true (telo u S rezimu!)
    return new TCB(body, arg, (char*) space + DEFAULT_STACK_SIZE, true, true);
}