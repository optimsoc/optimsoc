#include <optimsoc.h>

volatile unsigned int _optimsoc_timer_wait_signal;

extern uint32_t _optimsoc_system_clock;

void _optimsoc_timer_handler() {
    or1k_timer_disable();
    _optimsoc_timer_wait_signal = 1;
}

void optimsoc_timer_init() {
    or1k_exception_handler_add(5,&_optimsoc_timer_handler);
    _optimsoc_timer_wait_signal = 0;
}

void optimsoc_timer_wait(uint32_t microseconds) {
    uint32_t value = 1000000/microseconds;
    or1k_timer_init(value);
    or1k_timer_reset();
    or1k_timer_enable();
    while (_optimsoc_timer_wait_signal == 0) {}
    _optimsoc_timer_wait_signal = 0;
}
