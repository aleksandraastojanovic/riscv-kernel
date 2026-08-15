#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/TCB.h"

extern "C" void supervisorTrap();

static void print(const char* s) {
    while (*s) __putc(*s++);
}

static void printInt(int n) {
    if (n >= 10) printInt(n / 10);
    __putc('0' + n % 10);
}

// tela probnih niti: rade po malo posla pa dobrovoljno prepuste procesor
static void workerA() {
    for (int i = 0; i < 3; i++) {
        print("A: i="); printInt(i); print("\n");
        TCB::yield();
    }
    print("A: kraj\n");
}

static void workerB() {
    for (int i = 10; i < 13; i++) {
        print("B: i="); printInt(i); print("\n");
        TCB::yield();
    }
    print("B: kraj\n");
}

int main() {
    print("OS1 kernel: start\n");

    // registruj prekidnu rutinu (kapiju) PRE prvog sistemskog poziva
    Riscv::w_stvec((uint64) &supervisorTrap);

    // --- regresija: zadatak 1 (mem_alloc/mem_free kroz sistemske pozive) ---
    void* a = mem_alloc(100);
    void* b = mem_alloc(65);
    print(a && b ? "mem_alloc: OK\n" : "mem_alloc: FAIL\n");
    print(mem_free(a) == 0 && mem_free(b) == 0 ? "mem_free: OK\n"
                                               : "mem_free: FAIL\n");

    // --- zadatak 2, koraci 5-6: niti se stvarno smenjuju! ---
    TCB::running = TCB::createThread(nullptr);  // omotac za main: vec se
    // izvrsava i ima svoj stek
    TCB* ta = TCB::createThread(workerA);
    TCB* tb = TCB::createThread(workerB);

    while (!(ta->isFinished() && tb->isFinished())) {
        TCB::yield();
    }
    delete ta;
    delete tb;
    print("niti zavrsene, main nastavlja\n");

    print("OS1 kernel: kraj\n");
    for (;;) { /* kernel se ne "vraca" nikuda */ }
}
