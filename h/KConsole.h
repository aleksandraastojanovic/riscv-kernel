#ifndef PROJECT_BASE_V1_1_KCONSOLE_H
#define PROJECT_BASE_V1_1_KCONSOLE_H

#include "../lib/hw.h"
class SCB;

// Jezgrova konzola: baferisan ulaz/izlaz preko kontrolera serijske veze.
// (prefiks K: kernel-strana, za razliku od C++ API klase Console)
class KConsole {
public:
    static void init();              // baferi, semafori, izlazna nit (zove main)
    static void handleInterrupt();   // spoljasnji prekid: prijem znakova (handler)
    static void kputc(char c);       // jezgro syscall-a 0x42 (handler)
    static char kgetc();             // jezgro syscall-a 0x41 (handler)
    static bool isOutEmpty() { return outHead == outTail; }  // za flush pred gasenje

private:
    static void outputBody(void*);   // telo izlazne SISTEMSKE niti jezgra

    static const int N = 1024;
    static char inBuf[N];
    static volatile int inHead, inTail;
    static int inCount;              // eksplicitno: pun bafer -> odbacivanje

    static char outBuf[N];
    static volatile int outHead, outTail;   // pun kad (tail+1)%N == head

    static SCB* inputItems;    // znakovi koji cekaju u ulaznom baferu
    static SCB* outputItems;   // znakovi koji cekaju u izlaznom baferu
    static SCB* outputSpace;   // slobodna mesta u izlaznom baferu
};

#endif