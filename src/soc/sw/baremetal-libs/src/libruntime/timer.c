#include <or1k-support.h>

#include "timer.h"


struct optimsoc_list_t *_optimsoc_timers;

void _optimsoc_timer_tick()
{
    if (_optimsoc_timers != NULL) {

        optimsoc_list_iterator_t iter;

        struct _optimsoc_timer_t *timer =
            optimsoc_list_first_element(_optimsoc_timers, &iter);
        while (timer) {
            timer->ticks++;

            if (timer->ticks == timer->goal) {

                /* save addr and thread of current timer */
                optimsoc_thread_t resume = timer->thread;
                void *remove = timer;

                /* go to the next element before removing current */
                timer = optimsoc_list_next_element(_optimsoc_timers, &iter);

                optimsoc_list_remove(_optimsoc_timers, remove);
                optimsoc_thread_resume(resume);
            } else {
                timer = optimsoc_list_next_element(_optimsoc_timers, &iter);
            }
        }
    }
}

void optimsoc_timer_wait_ticks(uint32_t ticks)
{
    struct _optimsoc_timer_t timer;

    uint32_t restore = or1k_critical_begin();

    timer.ticks = 0;
    timer.goal = ticks;
    timer.thread = optimsoc_thread_current();

    if (_optimsoc_timers == NULL) {
        _optimsoc_timers = optimsoc_list_init(&timer);
    } else {
        optimsoc_list_add_tail(_optimsoc_timers, &timer);
    }

    optimsoc_thread_suspend(timer.thread);

    or1k_critical_end(restore);
}
