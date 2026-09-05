#include "../h/riscv.h"
#include "../h/MemoryAllocator.h"
#include "../h/TCB.h"
#include "../h/SCB.h"
#include "../h/KConsole.h"
#include "../h/SleepList.h"

static const uint64 SYS_MEM_ALLOC       = 0x01;
static const uint64 SYS_MEM_FREE        = 0x02;
static const uint64 SYS_THREAD_CREATE   = 0x11;
static const uint64 SYS_THREAD_EXIT     = 0x12;
static const uint64 SYS_THREAD_DISPATCH = 0x13;
static const uint64 SYS_SEM_OPEN        = 0x21;
static const uint64 SYS_SEM_CLOSE       = 0x22;
static const uint64 SYS_SEM_WAIT        = 0x23;
static const uint64 SYS_SEM_SIGNAL      = 0x24;
static const uint64 SYS_GETC            = 0x41;
static const uint64 SYS_PUTC            = 0x42;
static const uint64 SYS_TIME_SLEEP = 0x31;
static const uint64 SYS_SEM_WAIT_N   = 0x25;
static const uint64 SYS_SEM_SIGNAL_N = 0x26;


void Riscv::popSppSpie() {
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

/*
 * frame[i] = sacuvani registar x_i sa steka prekidne rutine:
 *   frame[10] = a0 (sifra poziva / povratna vrednost)
 *   frame[11..14] = a1..a4 (argumenti)
 */

// direktan (sinhron) upis u kontroler - SAMO za dijagnostiku izuzetaka:
// ne sme da zavisi od bafera/niti/rasporedjivaca kad je sistem u kvaru
static void kputcDirect(char c) {
    while (!(*(volatile char*) CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT)) {}
    *(volatile char*) CONSOLE_TX_DATA = c;
}

static void kprint(const char* s) {
    while (*s) kputcDirect(*s++);
}
extern "C" void handleSupervisorTrap(uint64* frame) {
    uint64 scause = Riscv::r_scause();

    uint64 sepc = Riscv::r_sepc();
    uint64 sstatus = Riscv::r_sstatus();
    if (scause == Riscv::SCAUSE_ECALL_SUPERVISOR ||
        scause == Riscv::SCAUSE_ECALL_USER) {
        sepc += 4;


        switch (frame[10]) {
            case SYS_MEM_ALLOC:
                frame[10] = (uint64) MemoryAllocator::alloc(frame[11]);
                break;
            case SYS_MEM_FREE:
                frame[10] = (uint64)(long) MemoryAllocator::free((void*) frame[11]);
                break;
            case SYS_THREAD_CREATE: {
                // a1=handle, a2=start_routine, a3=arg, a4=stack_space
                if (!frame[11] || !frame[12] || !frame[14]) {
                    frame[10] = (uint64)(long) -1;   // nit bez rucke/tela/steka
                    break;
                }
                TCB* t = TCB::createThread((TCB::Body) frame[12],
                                           (void*) frame[13],
                                           (void*) frame[14]);
                if (t) {
                    *(uint64*) frame[11] = (uint64) t;
                    frame[10] = 0;
                } else {
                    frame[10] = (uint64)(long) -1;
                }
                break;
            }
            case SYS_THREAD_EXIT:
                TCB::running->setFinished(true);
                TCB::dispatch();
                break;
            case SYS_THREAD_DISPATCH:
                TCB::dispatch();
                break;
            case SYS_SEM_OPEN: {
                if (!frame[11]) { frame[10] = (uint64)(long) -1; break; }
                SCB* s = new SCB((int) frame[12]);   // a2 = init vrednost
                if (s) {
                    *(uint64*) frame[11] = (uint64) s;   // upisi rucku
                    frame[10] = 0;
                } else {
                    frame[10] = (uint64)(long) -1;   // new vraca null -> nista nije alocirano
                }
                break;
            }
            case SYS_SEM_CLOSE: {
                SCB* s = (SCB*) frame[11];
                if (!s) { frame[10] = (uint64)(long) -1; break; }
                s->closeAll();       // probudi sve spavace sa greskom
                delete s;
                frame[10] = 0;
                break;
            }
            case SYS_SEM_WAIT: {
                SCB* s = (SCB*) frame[11];
                if (!s) { frame[10] = (uint64)(long) -1; break; }
                // block() unutra moze da nas uspava - nastavicemo se ovde
                // kad nas neko probudi (sepc/sstatus lokali to prezive!)
                frame[10] = (uint64)(long) s->wait();
                break;
            }
            case SYS_SEM_SIGNAL: {
                SCB* s = (SCB*) frame[11];
                if (!s) { frame[10] = (uint64)(long) -1; break; }
                frame[10] = (uint64)(long) s->signal();
                break;
            }
            case SYS_SEM_WAIT_N: {
                SCB* s = (SCB*) frame[11];
                if (!s) { frame[10] = (uint64)(long) -1; break; }
                frame[10] = (uint64)(long) s->wait_n((unsigned) frame[12]);
                break;
            }
            case SYS_SEM_SIGNAL_N: {
                SCB* s = (SCB*) frame[11];
                if (!s) { frame[10] = (uint64)(long) -1; break; }
                frame[10] = (uint64)(long) s->signal_n((unsigned) frame[12]);
                break;
            }
            case SYS_GETC:
                frame[10] = (uint64) KConsole::kgetc();
                break;
            case SYS_PUTC:
                KConsole::kputc((char) frame[11]);
                break;
            case SYS_TIME_SLEEP:
                frame[10] = 0;                 // rezultat upisujemo PRE spavanja -
                // frame je na nasem steku i saceka nas
                if (frame[11] > 0) SleepList::put(frame[11]);
                break;
            default:
                frame[10] = (uint64)(long) -1;
                break;
        }



    } else if (scause == Riscv::SCAUSE_SOFTWARE_TIMER) {

        Riscv::w_sip(Riscv::r_sip() & ~Riscv::SIP_SSIP);
        SleepList::tick();
        TCB::onTimerTick();
    } else if (scause == Riscv::SCAUSE_EXTERNAL_CONSOLE) {
            KConsole::handleInterrupt();

    }else if (scause == Riscv::SCAUSE_ILLEGAL_INSTRUCTION ||
               scause == Riscv::SCAUSE_LOAD_FAULT ||
               scause == Riscv::SCAUSE_STORE_FAULT) {
        // korisnicki kod pokusao nesto nedozvoljeno: prijavi i ugasi nit
        kprint("\nKERNEL: izuzetak (scause=");
        kputcDirect('0' + (char) scause);
        kprint("), nit se gasi\n");
        TCB::running->setFinished(true);
        TCB::dispatch();   // odavde se za ovu nit vise nikad ne vracamo
               }
    Riscv::w_sstatus(sstatus);
    Riscv::w_sepc(sepc);
}
