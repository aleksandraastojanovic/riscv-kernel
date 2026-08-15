#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/TCB.h"
#include "../h/Scheduler.h"

// Test korak 3: mem_alloc/mem_free kao PRAVI sistemski pozivi (kroz C API).
// main vise ne dira MemoryAllocator direktno - ide iskljucivo kroz slojeve,
// kao sto ce raditi i javni testovi.

extern "C" void supervisorTrap();

static void print(const char* s) {
    while (*s) __putc(*s++);
}

// telo za probne niti - nikad se ne izvrsava u ovom koraku
static void dummyBody() {}

int main() {
    print("OS1 kernel: start\n");

    // registruj prekidnu rutinu (kapiju) PRE prvog sistemskog poziva
    Riscv::w_stvec((uint64) &supervisorTrap);

    // alokacije u BAJTOVIMA - C API zaokruzuje na blokove
    void* a = mem_alloc(100);   // 100 B -> 2 bloka (128 B)
    void* b = mem_alloc(65);    //  65 B -> 2 bloka (ne 1!)
    print(a ? "mem_alloc a: OK\n" : "mem_alloc a: FAIL\n");
    print(b ? "mem_alloc b: OK\n" : "mem_alloc b: FAIL\n");

    // upis u dobijeni prostor (ako adrese nisu dobre, ovde puca)
    if (a) { char* p = (char*)a; for (int i = 0; i < 100; i++) p[i] = 'x'; }

    int f1 = mem_free(a);
    print(f1 == 0 ? "mem_free a: OK\n" : "mem_free a: FAIL\n");

    void* c = mem_alloc(100);
    print(c == a ? "realloc na isto mesto: OK\n" : "realloc: (proveriti)\n");

    int f2 = mem_free(nullptr);
    print(f2 < 0 ? "mem_free(nullptr) = greska: OK\n" : "mem_free(nullptr): FAIL\n");

    // --- Zadatak 2, korak 4: TCB kostur + Scheduler FIFO test ---
    // (niti se jos NE izvrsavaju - promena konteksta je sledeci korak;
    //  testiramo samo da red radi kako treba)
    TCB* t1 = TCB::createThread(dummyBody);   // konstruktor ih sam stavlja u red
    TCB* t2 = TCB::createThread(dummyBody);
    TCB* t3 = TCB::createThread(dummyBody);

    bool fifo = (Scheduler::get() == t1) &&
                (Scheduler::get() == t2) &&
                (Scheduler::get() == t3);
    print(fifo ? "Scheduler FIFO redosled: OK\n" : "Scheduler FIFO redosled: FAIL\n");
    print(Scheduler::get() == nullptr ? "Scheduler prazan: OK\n"
                                      : "Scheduler prazan: FAIL\n");

    delete t1; delete t2; delete t3;          // vraca TCB + stekove alokatoru

    print("OS1 kernel: kraj\n");
    for (;;) { /* kernel se ne "vraca" nikuda */ }
}
