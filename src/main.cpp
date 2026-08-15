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

// JEDNA funkcija za SVE radnike - svako dobija svoje podatke kroz arg!
struct WorkerArgs {
    const char* name;
    int from, to;
    volatile bool* done;
};

static void worker(void* p) {
    WorkerArgs* w = (WorkerArgs*) p;
    for (int i = w->from; i < w->to; i++) {
        print(w->name); print(": i="); printInt(i); print("\n");
        thread_dispatch();
    }
    print(w->name); print(": kraj\n");
    *w->done = true;
}

int main() {
    print("OS1 kernel: start\n");

    // inicijalizacija jezgra: kapija + omotac za vec-izvrsavajuci main
    Riscv::w_stvec((uint64) &supervisorTrap);
    TCB::running = TCB::createThread(nullptr, nullptr, nullptr);

    // regresija zadatka 1
    void* m = mem_alloc(100);
    print(m && mem_free(m) == 0 ? "mem_alloc/mem_free: OK\n"
                                : "mem_alloc/mem_free: FAIL\n");

    // --- niti kroz PRAVE sistemske pozive, sa argumentima ---
    volatile bool doneA = false, doneB = false;
    WorkerArgs wa = { "A", 0, 3, &doneA };
    WorkerArgs wb = { "B", 10, 13, &doneB };

    thread_t ta = nullptr, tb = nullptr;
    int s1 = thread_create(&ta, worker, &wa);
    int s2 = thread_create(&tb, worker, &wb);
    print(s1 == 0 && s2 == 0 ? "thread_create: OK\n" : "thread_create: FAIL\n");

    while (!(doneA && doneB)) {
        thread_dispatch();
    }
    print("niti zavrsene kroz sistemske pozive!\n");

    print("OS1 kernel: kraj\n");
    for (;;) { /* kernel se ne "vraca" nikuda */ }
}
