#include <thread.h>

void t0f(void *arg) {
    thread_suspend();
    for (int i=0;i<2;i++)
        thread_yield();
}

void init() {
    thread_t t0;
    thread_create(&t0,&t0f,0);
    for (int i=0;i<2;i++)
        thread_yield();
    thread_resume(t0);
    thread_join(t0);
}
