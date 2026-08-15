#include "../h/riscv.h"
#include "../h/MemoryAllocator.h"
#include "../h/TCB.h"

static const uint64 SYS_MEM_ALLOC = 0x01;
static const uint64 SYS_MEM_FREE  = 0x02;
static const uint64 SYS_THREAD_CREATE = 0x11;
static const uint64 SYS_THREAD_EXIT = 0x12;
static const uint64 SYS_THREAD_DISPATCH = 0x13;

void Riscv::popSppSpie() {
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

extern "C" void handleSupervisorTrap(uint64* frame){
    uint64 scause =Riscv::r_scause();
    if(scause == Riscv::SCAUSE_ECALL_SUPERVISOR || scause == Riscv::SCAUSE_ECALL_USER){
        uint64 sepc = Riscv::r_sepc() + 4; // povrataknakonecall
        uint64 sstatus = Riscv::r_sstatus();

        switch (frame[10]) {
            case SYS_MEM_ALLOC:
                frame[10] =(uint64) MemoryAllocator::alloc(frame[11]);
                break;
            case SYS_MEM_FREE:

                frame[10] = (uint64)(long) MemoryAllocator::free((void*) frame[11]);
                break;
            case SYS_THREAD_CREATE: {
                TCB* t = TCB::createThread((TCB::Body) frame[12],
                    (void*) frame[13],
                    (void*) frame[14]);
                if (t&& frame[11]) {
                    *(uint64*) frame[11] =(uint64) t;
                    frame[10] = 0;

                }else {
                    frame[10] = (uint64)(long) -1;
                }
                break;
            }
            case SYS_THREAD_EXIT: {
                TCB::running->setFinished(true);
                TCB::dispatch();
                break;
            }
            case SYS_THREAD_DISPATCH: {
                TCB::dispatch();
                break;
            }
            default:

                break;
        }
        Riscv::w_sstatus(sstatus);
        Riscv::w_sepc(sepc);
    }
    else{

    }
}