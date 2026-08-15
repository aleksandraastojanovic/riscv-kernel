#include "../h/syscall_c.h"

//implemtacija c api sloja registri ecall


void* mem_alloc(size_t size){
    if ( size == 0) return nullptr;
    size_t blocks = (size + MEM_BLOCK_SIZE -1)/MEM_BLOCK_SIZE;
    register uint64 a0 __asm__("a0") = 0x01; // memalloc
    register uint64 a1 __asm__("a1") = blocks; // abi prima blokove
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return (void*) a0;
}
int mem_free(void*ptr) {
    register uint64 a0 __asm__("a0") = 0x02; // memfree
    register uint64 a1 __asm__("a1") =(uint64) ptr;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return ( int) a0;
}