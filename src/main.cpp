#include "../h/riscv.h"
#include "../h/TCB.h"

// userMain iz src/userMain.cpp (dosao uz javne testove)
void userMain();

extern "C" void supervisorTrap();

int main() {
    // inicijalizacija jezgra: kapija za sistemske pozive + omotac za
    // vec-izvrsavajuci main
    Riscv::w_stvec((uint64) &supervisorTrap);
    TCB::running = TCB::createThread(nullptr, nullptr, nullptr);

    // dozvoli prijem prekida: pojedinacne vrste (sie) + globalni
    // prekidac (sstatus.SIE) - bez ovoga znak sa tastature nikad ne stigne
    Riscv::w_sie(Riscv::r_sie() | Riscv::SIE_SSIE | Riscv::SIE_STIE | Riscv::SIE_SEIE);
    Riscv::w_sstatus(Riscv::r_sstatus() | Riscv::SSTATUS_SIE);

    // sve dalje radi korisnicki kod javnih testova
    userMain();

    for (;;) { /* kernel se ne "vraca" nikuda */ }
}
