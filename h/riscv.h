//
// Created by os on 8/14/26.
//

#ifndef PROJECT_BASE_V1_1_RISCV_H
#define PROJECT_BASE_V1_1_RISCV_H


#include "../lib/hw.h"

class Riscv{
    //pomocne funkcije za citanje i pisanje csr registara
public:
    static const uint64 SCAUSE_ECALL_USER       = 0x0000000000000008UL;
    static const uint64 SCAUSE_ECALL_SUPERVISOR = 0x0000000000000009UL;

    static uint64 r_scause(){
        uint64 v;
        __asm__ volatile("csrr %0, scause" : "=r"(v));
        return v;
    }
    static uint64 r_sepc(){
        uint64 v;
        __asm__ volatile("csrr %0, sepc" : "=r"(v));
        return v;
    }
    static void w_sepc(uint64 v){
        __asm__ volatile("csrw sepc, %0" : : "r"(v));
    }
    static void w_stvec(uint64 v){
        __asm__ volatile("csrw stvec, %0" : : "r"(v));
    }

    static uint64 r_sstatus(){
        uint64 v;
        __asm__ volatile("csrr %0, sstatus" : "=r"(v));
        return v;
    }
    static void w_sstatus(uint64 v){

        __asm__ volatile("csrw sstatus, %0" : : "r"(v));

    }

    static void popSppSpie();
};
#endif //PROJECT_BASE_V1_1_RISCV_H
