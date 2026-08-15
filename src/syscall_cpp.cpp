#include "../h/syscall_cpp.hpp"

// globalni new/delete obmotavaju sistemske pozive
// svaki new u korisnickom kodu od sada ide kroz ecall
void* operator new (size_t n) { return mem_alloc(n); }
void  operator delete (void* p) { mem_free(p); }


Thread::Thread(void (*body)(void*), void* arg)
    : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread()
    : myHandle(nullptr), body(nullptr), arg(nullptr) {}

Thread::~Thread() {}

void Thread::runWrapper(void* thread) {
    ((Thread*) thread)->run();
}

int Thread::start() {
    if (body) return thread_create(&myHandle, body, arg);
    else      return thread_create(&myHandle, &runWrapper, this);
}

void Thread::dispatch() { thread_dispatch(); }

int Thread::sleep(time_t t) { return time_sleep(t); }


Semaphore::Semaphore(unsigned init) : myHandle(nullptr) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() { sem_close(myHandle); }

int Semaphore::wait() { return sem_wait(myHandle); }

int Semaphore::signal() { return sem_signal(myHandle); }

// --- PeriodicThread ---

void PeriodicThread::periodicWrapper(void* thread) {
    PeriodicThread* pt = (PeriodicThread*) thread;
    while (pt->period != 0) {
        pt->periodicActivation();
        Thread::sleep(pt->period);
    }
}

PeriodicThread::PeriodicThread(time_t period)
    : Thread(&periodicWrapper, this), period(period) {}

void PeriodicThread::terminate() { period = 0; }


char Console::getc() { return ::getc(); }

void Console::putc(char chr) { ::putc(chr); }
