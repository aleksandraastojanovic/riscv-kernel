#include "../h/riscv.h"
#include "../h/MemoryAllocator.h"

static const uint64 SYS_MEM_ALLOC = 0x01;
static const uint64 SYS_MEM_FREE  = 0x02;

extern "C" void handleSupervisorTrap(uint64* frame){
    uint64 scause =Riscv::r_scause();
    if(scause == Riscv::SCAUSE_ECALL_SUPERVISOR || scause == Riscv::SCAUSE_ECALL_USER){
        Riscv::w_sepc(Riscv::r_sepc() + 4); // povrataknakonecall
        switch (frame[10]) {
            case SYS_MEM_ALLOC:
                frame[10] =(uint64) MemoryAllocator::alloc(frame[11]);
                break;
            case SYS_MEM_FREE:

                frame[10] = (uint64)(long) MemoryAllocator::free((void*) frame[11]);
                break;
            default:

                break;
        }

    }
    else{

    }
}