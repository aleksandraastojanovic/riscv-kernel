#include "../h/syscall_c.h"
#include "printing.hpp"
#include "Semaphore_test.h"

static sem_t semA, semB;
static volatile int pingPongDone = 0;

static void threadA(void*){
    for(int i = 0; i< 5; i++){
        sem_wait(semA);
        printString("A");
        sem_signal(semB);
    }
}

static void threadB(void*){
    for(int i = 0; i< 5; i++){
        sem_wait(semB);
        printString("B");
        sem_signal(semA);
    }
    pingPongDone =1;
}

static sem_t semClosing;
static volatile int closeTestDone = 0;

static void waiterOnClosed(void*){
    int ret = sem_wait(semClosing);
    if (ret < 0) printString("OK: wait vratio gresku posle sem_close\n");
    else         printString("GRESKA: wait vratio 0 iako je semafor zatvoren!\n");
    closeTestDone = 1;
}

static sem_t semN;
static volatile int waitNDone = 0;

static void waiterN(void*){
    int ret = sem_wait_n(semN,5);
    if (ret == 0) printString("OK: wait_n prosao kad se nakupilo 5\n");
    else          printString("GRESKA: wait_n vratio gresku\n");
    waitNDone = 1;
}


void Semaphore_test() {
    thread_t t1, t2, t3, t4;

    // DEO 1
    printString("-- ping-pong (ocekivano ABABABABAB):\n");
    sem_open(&semA, 1);
    sem_open(&semB, 0);
    thread_create(&t1, threadA, nullptr);
    thread_create(&t2, threadB, nullptr);
    while (!pingPongDone) thread_dispatch();   // glavna nit ustupa procesor dok se ne zavrsi
    printString("\n");
    sem_close(semA);
    sem_close(semB);

    // DEO 2
    printString("-- sem_close deblokira cekaca:\n");
    sem_open(&semClosing, 0);
    thread_create(&t3, waiterOnClosed, nullptr);
    for (int i = 0; i < 5; i++) thread_dispatch();  // pusti cekaca da se stvarno zablokira
    sem_close(semClosing);                          // ovo mora da ga probudi sa greskom
    while (!closeTestDone) thread_dispatch();

    // DEO 3
    printString("-- sem_wait_n / sem_signal_n:\n");
    sem_open(&semN, 2);                       // krece sa 2, trazi se 5
    thread_create(&t4, waiterN, nullptr);
    for (int i = 0; i < 5; i++) thread_dispatch();  // cekac se blokira (2 < 5)
    sem_signal_n(semN, 3);                    // sad ima 2+3=5 -> treba da se probudi
    while (!waitNDone) thread_dispatch();
    sem_close(semN);

    printString("Semaphore_test zavrsen\n");
}
