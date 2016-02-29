#ifndef TIMER_H
#define TIMER_H

#include "include/optimsoc-runtime.h"

struct _optimsoc_timer_t {
    uint32_t ticks;
    uint32_t goal;
    optimsoc_thread_t thread;
};

void _optimsoc_timer_tick();

#endif //TIMER_H
