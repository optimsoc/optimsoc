/* Copyright (c) 2012-2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#include <optimsoc-baremetal.h>
#include <or1k-support.h>
#include <context.h>

#include "runtime.h"
#include "config.h"
#include "list.h"
#include "thread.h"
#include "timer.h"
#include "scheduler.h"
#include "trace.h"

#include <stdio.h>
#include <assert.h>

// Shared structures
struct optimsoc_list_t* all_threads;
struct optimsoc_list_t* ready_q;
struct optimsoc_list_t* wait_q;

// This is the entry point
optimsoc_thread_t init_thread;

// Core specifics
struct _optimsoc_scheduler_core {
    optimsoc_thread_t idle_thread;
    optimsoc_thread_t active_thread;
};

struct _optimsoc_scheduler_core* _optimsoc_scheduler_core;

extern void init();

void _optimsoc_idle_thread_func() {
    while (1) { }
}

optimsoc_thread_t _optimsoc_scheduler_get_current(void) {
    struct _optimsoc_scheduler_core *core_ctx;
    core_ctx = &_optimsoc_scheduler_core[or1k_coreid()];
    assert (core_ctx && core_ctx->active_thread);

    return core_ctx->active_thread;
}

void _optimsoc_scheduler_tick() {
    struct _optimsoc_scheduler_core *core_ctx;
    core_ctx = &_optimsoc_scheduler_core[or1k_coreid()];

    _optimsoc_timer_tick();

    /* save context */
    _optimsoc_context_save(core_ctx->active_thread->ctx);

    /* put active thread into the queue */
    if (core_ctx->active_thread != core_ctx->idle_thread) {
        // only if this is not the idle thread of course..
        _optimsoc_scheduler_add(core_ctx->active_thread, ready_q);
    }

    /* schedule next thread */
    _optimsoc_schedule();

    /* activate timer */
    or1k_timer_reset();
}

void _optimsoc_scheduler_init() {

    wait_q = optimsoc_list_init(0);
    ready_q= optimsoc_list_init(0);
    all_threads = optimsoc_list_init(0);

    struct optimsoc_thread_attr *attr_init;
    attr_init = malloc(sizeof(struct optimsoc_thread_attr));

    optimsoc_thread_attr_init(attr_init);
    attr_init->flags |= OPTIMSOC_THREAD_FLAG_KERNEL;
    attr_init->identifier = "init";

    optimsoc_thread_create(&init_thread, &init, attr_init);

    _optimsoc_scheduler_core = calloc(or1k_numcores(),
                                      sizeof(struct _optimsoc_scheduler_core));

    for (int c = 0; c < or1k_numcores(); c++) {
        struct optimsoc_thread_attr *attr_idle;
        attr_idle = malloc(sizeof(struct optimsoc_thread_attr));
        optimsoc_thread_attr_init(attr_idle);
        attr_idle->flags |= OPTIMSOC_THREAD_FLAG_KERNEL;
        attr_idle->identifier = "idle";
        optimsoc_thread_create(&(_optimsoc_scheduler_core[c].idle_thread),
                               &_optimsoc_idle_thread_func, attr_idle);
        optimsoc_list_remove(ready_q,
                             (void*) _optimsoc_scheduler_core[c].idle_thread);
        optimsoc_list_remove(all_threads,
                             (void*) _optimsoc_scheduler_core[c].idle_thread);
    }
}

void _optimsoc_scheduler_add(optimsoc_thread_t t, struct optimsoc_list_t* q) {
    if(q == ready_q) {
        t->state = THREAD_RUNNABLE;
    }

    optimsoc_list_add_tail(q, (void*)t);
}

void _optimsoc_scheduler_start() {

    /* init timer */
    or1k_timer_init(runtime_config_get_numticks());
    /* timer set handle must be after timer init */
    or1k_timer_set_handler(&_optimsoc_scheduler_tick);

    _optimsoc_schedule();

    /* activate timer */
    or1k_timer_reset();
    or1k_timer_enable();

    _optimsoc_thread_ctx_t *ctx;
    ctx = _optimsoc_scheduler_core[or1k_coreid()].active_thread->ctx;
    _optimsoc_context_replace(ctx);
}

void _optimsoc_schedule() {

    /* get the next thread from the ready_q */
    optimsoc_thread_t t;
    t = (optimsoc_thread_t) optimsoc_list_remove_head(ready_q);

    /* In case we don't have a thread in the ready_q: schedule idle thread */
    if(!t) {
        t = _optimsoc_scheduler_core[or1k_coreid()].idle_thread;
    }

    assert(t);

    /* set active */
    _optimsoc_scheduler_core[or1k_coreid()].active_thread = t;

    runtime_trace_schedule(t->id);

    /* switch the context */
    _optimsoc_thread_ctx_t *ctx;
    ctx = _optimsoc_scheduler_core[or1k_coreid()].active_thread->ctx;
    _optimsoc_context_restore(ctx);

    // TODO: Clear TLB
}
