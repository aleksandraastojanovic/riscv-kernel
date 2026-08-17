#include "../h/KConsole.h"
#include "../h/SCB.h"
#include "../h/TCB.h"
#include "../h/syscall_c.h"

char KConsole::inBuf[KConsole::N];
volatile int KConsole::inHead = 0;
volatile int KConsole::inTail = 0;
int KConsole::inCount = 0;

char KConsole::outBuf[KConsole::N];
volatile int KConsole::outHead = 0;
volatile int KConsole::outTail = 0;

SCB* KConsole::inputItems  = nullptr;
SCB* KConsole::outputItems = nullptr;
SCB* KConsole::outputSpace = nullptr;

void KConsole::init() {
    // direktno new SCB (klasni new -> MemoryAllocator), NE sem_open syscall:
    // jezgro ne poziva sopstvene sistemske pozive
    inputItems  = new SCB(0);
    outputItems = new SCB(0);
    outputSpace = new SCB(N - 1);          // kruzni bafer: kapacitet N-1
    TCB::createSystemThread(&outputBody, nullptr);
}

// --- jezgra sistemskih poziva: izvrsavaju se U HANDLERU (prekidi maskirani) ---

void KConsole::kputc(char c) {
    outputSpace->wait();       // bafer pun -> pozivajuca nit se blokira
    outBuf[outTail] = c;
    outTail = (outTail + 1) % N;
    outputItems->signal();     // javi izlaznoj niti da ima posla
}

char KConsole::kgetc() {
    if (inputItems->wait() < 0) return (char) EOF;  // spava dok znak ne stigne
    char c = inBuf[inHead];
    inHead = (inHead + 1) % N;
    inCount--;
    return c;
}

// --- obrada prekida od konzole: prijemni smer (u handleru) ---

void KConsole::handleInterrupt() {
    int irq = plic_claim();                // ko je prekinuo?
    if (irq == CONSOLE_IRQ) {
        // povlaci znakove dok kontroler javlja da ih ima (prozivanje)
        while (*(volatile char*) CONSOLE_STATUS & CONSOLE_RX_STATUS_BIT) {
            char c = *(volatile char*) CONSOLE_RX_DATA;
            if (inCount < N) {
                inBuf[inTail] = c;
                inTail = (inTail + 1) % N;
                inCount++;
                inputItems->signal();      // probudi eventualnog getc cekaoca
            }
            // pun bafer: znak se odbacuje - hardver ne moze da se "blokira"
        }
    }
    if (irq) plic_complete(irq);           // potvrdi kontroleru prekida
}

// --- izlazna nit jezgra: obicna (sistemska) nit, sinhronizacija SYSCALLOVIMA ---

void KConsole::outputBody(void*) {
    for (;;) {
        sem_wait((sem_t) outputItems);     // spavaj dok se ne pojavi znak
        // prozivanje: cekaj spremnost kontrolera, ali ustupaj procesor
        while (!(*(volatile char*) CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT))
            thread_dispatch();
        *(volatile char*) CONSOLE_TX_DATA = outBuf[outHead];
        outHead = (outHead + 1) % N;       // pomeraj TEK POSLE upisa (flush logika)
        sem_signal((sem_t) outputSpace);   // oslobodjeno jedno mesto
    }
}