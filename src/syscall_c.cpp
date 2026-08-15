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

int thread_create(thread_t* handle, void (*start_routine)(void*), void* arg) {
    if (!handle || !start_routine) return -1;

    // C API alocira stek  ABI dobija
    // pokazivac na njegovu poslednju lokaciju [po postavci]
    void* stack = mem_alloc(DEFAULT_STACK_SIZE);
    if (!stack) return -2;
    void* stack_space = (char*) stack + DEFAULT_STACK_SIZE;

    register uint64 a0 __asm__("a0") = 0x11;
    register uint64 a1 __asm__("a1") = (uint64) handle;
    register uint64 a2 __asm__("a2") = (uint64) start_routine;
    register uint64 a3 __asm__("a3") = (uint64) arg;
    register uint64 a4 __asm__("a4") = (uint64) stack_space;
    __asm__ volatile("ecall"
                     : "+r"(a0)
                     : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
                     : "memory");

    return (int) a0;
}

int thread_exit() {
    register uint64 a0 __asm__("a0") = 0x12;
    __asm__ volatile("ecall" : "+r"(a0) : : "memory");
    return (int) a0;
}

void thread_dispatch() {
    register uint64 a0 __asm__("a0") = 0x13;
    __asm__ volatile("ecall" : "+r"(a0) : : "memory");
}

char getc() {
    register uint64 a0 __asm__("a0") = 0x41;
    __asm__ volatile("ecall" : "+r"(a0) : : "memory");
    return (char) a0;
}

void putc(char chr) {
    register uint64 a0 __asm__("a0") = 0x42;
    register uint64 a1 __asm__("a1") = (uint64) chr;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
}


int sem_open(sem_t* handle, unsigned init) {
    register uint64 a0 __asm__("a0") = 0x21;
    register uint64 a1 __asm__("a1") = (uint64) handle;
    register uint64 a2 __asm__("a2") = (uint64) init;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2) : "memory");
    return (int) a0;
}

int sem_close(sem_t handle) {
    register uint64 a0 __asm__("a0") = 0x22;
    register uint64 a1 __asm__("a1") = (uint64) handle;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return (int) a0;
}

int sem_wait(sem_t id) {
    register uint64 a0 __asm__("a0") = 0x23;
    register uint64 a1 __asm__("a1") = (uint64) id;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return (int) a0;
}

int sem_signal(sem_t id) {
    register uint64 a0 __asm__("a0") = 0x24;
    register uint64 a1 __asm__("a1") = (uint64) id;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return (int) a0;
}

int time_sleep(time_t t) {
    register uint64 a0 __asm__("a0") = 0x31;
    register uint64 a1 __asm__("a1") = (uint64) t;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1) : "memory");
    return (int) a0;
}
