#pragma once

#include "../lib/hw.h"


class Riscv {
public:
    // razlozi ulaska u prekidnu rutinu
    static const uint64 SCAUSE_ECALL_USER       = 0x0000000000000008UL;
    static const uint64 SCAUSE_ECALL_SUPERVISOR = 0x0000000000000009UL;
    static const uint64 SCAUSE_SOFTWARE_TIMER   = 0x8000000000000001UL;
    static const uint64 SCAUSE_EXTERNAL_CONSOLE = 0x8000000000000009UL;

    static const uint64 SCAUSE_ILLEGAL_INSTRUCTION = 2;  // privilegovana/nepostojeca instrukcija
    static const uint64 SCAUSE_LOAD_FAULT          = 5;  // nedozvoljena adresa citanja
    static const uint64 SCAUSE_STORE_FAULT         = 7;  // nedozvoljena adresa upisa

    // biti u sstatus / sip / sie registrima
    static const uint64 SSTATUS_SIE = 1UL << 1;  // globalni prekidac prekida
    static const uint64 SIP_SSIP    = 1UL << 1;  // softverski prekid "na cekanju"
    static const uint64 SIE_SSIE    = 1UL << 1;  // dozvola softverskih (tajmer)
    static const uint64 SIE_STIE    = 1UL << 5;  // dozvola tajmerskih
    static const uint64 SIE_SEIE    = 1UL << 9;  // dozvola spoljasnjih (konzola)

    static const uint64 SSTATUS_SPP  = 1UL << 8;  // rezim povratka za sret (0=U, 1=S)
    static const uint64 SSTATUS_SPIE = 1UL << 5;  // vrednost SIE posle sret

    static uint64 r_scause() {
        uint64 v;
        __asm__ volatile("csrr %0, scause" : "=r"(v));
        return v;
    }

    static uint64 r_sepc() {
        uint64 v;
        __asm__ volatile("csrr %0, sepc" : "=r"(v));
        return v;
    }

    static void w_sepc(uint64 v) {
        __asm__ volatile("csrw sepc, %0" : : "r"(v));
    }

    static void w_stvec(uint64 v) {
        __asm__ volatile("csrw stvec, %0" : : "r"(v));
    }

    static uint64 r_sstatus() {
        uint64 v;
        __asm__ volatile("csrr %0, sstatus" : "=r"(v));
        return v;
    }

    static void w_sstatus(uint64 v) {
        __asm__ volatile("csrw sstatus, %0" : : "r"(v));
    }

    static uint64 r_sip() {
        uint64 v;
        __asm__ volatile("csrr %0, sip" : "=r"(v));
        return v;
    }

    static void w_sip(uint64 v) {
        __asm__ volatile("csrw sip, %0" : : "r"(v));
    }

    static uint64 r_sie() {
        uint64 v;
        __asm__ volatile("csrr %0, sie" : "=r"(v));
        return v;
    }

    static void w_sie(uint64 v) {
        __asm__ volatile("csrw sie, %0" : : "r"(v));
    }


    static void popSppSpie();
};
