#include "../h/riscv.h"
#include "../h/TCB.h"
#include "../h/syscall_c.h"

// userMain iz src/userMain.cpp (dosao uz javne testove)
void userMain();

extern "C" void supervisorTrap();

static sem_t userMainDone;

// telo KORISNICKE niti: ceo korisnicki program + javljanje mainu da je kraj
static void userMainWrapper(void*) {
    userMain();
    sem_signal(userMainDone);
}

int main() {
    // inicijalizacija jezgra: kapija za sistemske pozive + omotac za
    // vec-izvrsavajuci main (interna nit jezgra, ostaje u S rezimu)
    Riscv::w_stvec((uint64) &supervisorTrap);
    TCB::running = TCB::createThread(nullptr, nullptr, nullptr);
    TCB::initIdle();

    // dozvoli prijem prekida
    Riscv::w_sie(Riscv::r_sie() | Riscv::SIE_SSIE | Riscv::SIE_STIE | Riscv::SIE_SEIE);
    Riscv::w_sstatus(Riscv::r_sstatus() | Riscv::SSTATUS_SIE);

    // korisnicki program se pokrece kao POSEBNA KORISNICKA NIT (U rezim!),
    // po postavci: "treba da pokrene nit nad funkcijom userMain()"
    sem_open(&userMainDone, 0);
    thread_t userMainThread;
    thread_create(&userMainThread, &userMainWrapper, nullptr);

    sem_wait(userMainDone);   // main spava dok ceo korisnicki program ne zavrsi

    // regularan kraj programa: zaustavi emulator
    // (postavka: upis 32-bitne vrednosti 0x5555 na adresu 0x100000)
    *((volatile uint32*) 0x100000) = 0x5555;
    for (;;) { /* nedostizno - osiguranje */ }
}