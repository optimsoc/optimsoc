/* Copyright (c) 2012-2013 by the author(s)
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
#include "scheduler.h"
#include "trace.h"

#include <stdio.h>
#include <assert.h>

// This is in the private data area (0x4-0x90)
extern arch_thread_ctx_t *exception_ctx;

// Shared structures
struct list_t* all_threads;
struct list_t* ready_q;
struct list_t* wait_q;

// This is the entry point
thread_t init_thread;

// Core specifics
struct optimsoc_scheduler_core* optimsoc_scheduler_core;

extern void init();

unsigned int runtime_config_terminate_on_empty_readyq = 0;

unsigned int load_history[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int load_current;

unsigned int *system_load = 0;
unsigned int system_load_update_counter;

int (*runtime_hook_needs_rebalance)(void);
void (*runtime_hook_rebalance)(void) = 0;

void idle_thread_func() {
    while (1) { }
}

void scheduler_tick() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];

    /* save context */
    memcpy(core_ctx->active_thread->ctx, exception_ctx,
           sizeof(struct arch_thread_ctx_t));

    /* put active thread into the queue */
    if (core_ctx->active_thread != core_ctx->idle_thread) {
        // only if this is not the idle thread of course..
        scheduler_add(core_ctx->active_thread, ready_q);
    }

    /* schedule next thread */
    schedule();
}

void scheduler_yieldcurrent() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];

    uint32_t restore = optimsoc_critical_begin();

    scheduler_add(core_ctx->active_thread, ready_q);

    runtime_trace_yield(core_ctx->active_thread->id);
    yield_switchctx(core_ctx->active_thread->ctx);

    optimsoc_critical_end(restore);
}

void scheduler_suspendcurrent() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];

    uint32_t restore = optimsoc_critical_begin();

    scheduler_add(core_ctx->active_thread, wait_q);

    core_ctx->active_thread->state = THREAD_SUSPENDED;

    runtime_trace_suspend(core_ctx->active_thread->id);
    yield_switchctx(core_ctx->active_thread->ctx);
    optimsoc_critical_end(restore);
}

void scheduler_handle_sysmsg(unsigned int* buffer, int size) {
    assert(size == 1);

    unsigned int source_tile = extract_bits(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

    system_load[source_tile] = extract_bits(buffer[0], 7, 0);
}

void scheduler_init() {
    or1k_exception_handler_add(5,&scheduler_tick);

    or1k_timer_init(runtime_config_get_numticks());

    wait_q = list_init(0);
    ready_q= list_init(0);
    all_threads = list_init(0);

    thread_attr_t *attr_init = malloc(sizeof(thread_attr_t));

    thread_attr_init(attr_init);

    attr_init->identifier = "init";

    thread_create(&init_thread, &init, attr_init);

    optimsoc_scheduler_core = calloc(optimsoc_get_relcoreid(),
                                     sizeof(optimsoc_scheduler_core));

    for (int c = 0; c < optimsoc_get_relcoreid(); c++) {
        thread_attr_t *attr_idle = malloc(sizeof(thread_attr_t));
        thread_attr_init(attr_idle);
        attr_idle->identifier = "idle";
        thread_create(&(optimsoc_scheduler_core[c].idle_thread), &idle_thread_func, attr_idle);
        list_remove(ready_q,(void*) optimsoc_scheduler_core[c].idle_thread);
        list_remove(all_threads,(void*) optimsoc_scheduler_core[c].idle_thread);
    }

    system_load = calloc(4, sizeof(unsigned int));
    system_load_update_counter = 0;
    load_current = 0;

    optimsoc_mp_simple_addhandler(7, &scheduler_handle_sysmsg);

    runtime_hook_needs_rebalance = 0;
}

void scheduler_add(thread_t t, struct list_t* q) {
    if(q == ready_q) {
	 t->state = THREAD_RUNNABLE;
    }

    list_add_tail(q, (void*)t);
}

void scheduler_start() {
    schedule();
    // We got to nirvana
    ctx_replace();
}

void context_set(arch_thread_ctx_t *ctx) {
    memcpy(exception_ctx, ctx, sizeof(struct arch_thread_ctx_t));
}

void scheduler_distribute_load() {
    system_load_update_counter++;

    if (system_load_update_counter == 1) {
        for (int t = 0; t < optimsoc_get_numct(); t++) {
            unsigned int id = optimsoc_get_ranktile(t);
            if (id != optimsoc_get_tileid()) {
                uint32_t buffer = 0;
                set_bits(&buffer, id, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
                set_bits(&buffer, 7, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
                set_bits(&buffer, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
                set_bits(&buffer, load_current, 7, 0);
                optimsoc_mp_simple_send(1, &buffer);
            }
        }
        system_load_update_counter = 0;
    }

}

int scheduler_needs_rebalance() {
    return 0;
    if (runtime_hook_needs_rebalance != 0) {
        return runtime_hook_needs_rebalance();
    }
    return ((load_current > 24) && (load_history[0] > 1));
}

void scheduler_rebalance() {
    return;
    if (runtime_hook_rebalance != 0) {
        runtime_hook_rebalance();
        return;
    }
    printf("Need to rebalance, select candidate tile:\n");
    int low_value = system_load[optimsoc_get_tileid()];
    int low_tile = optimsoc_get_tileid();

    for (int t = 0; t < 4; t++) {
        printf("%d: %d\n", t, system_load[t]);
        if (system_load[t] < low_value) {
            low_value = system_load[t];
            low_tile = t;
        }
    }
    if (low_tile == optimsoc_get_tileid()) {
        printf("No candidate found. Keep here.\n");
    } else {
        printf("Migrate to %d.\n", low_tile);
        thread_send((struct thread_t*)list_remove_head(ready_q), low_tile);
    }
}

void scheduler_update_load() {
    load_current -= load_history[15];

    // Determine load
    for (int l = 15; l > 0; l--) {
        load_history[l] = load_history[l-1];
    }

    load_history[0] = list_length(ready_q);

    load_current += load_history[0];

    runtime_trace_load(load_current);

    scheduler_distribute_load();

    system_load[optimsoc_get_tileid()] = load_current;

    if (scheduler_needs_rebalance() != 0) {
	scheduler_rebalance();
    }
}


void schedule() {
    scheduler_update_load();

    /* get the next thread from the ready_q */
    struct thread_t* t = (struct thread_t*)list_remove_head(ready_q);

    /* In case we don't have a thread in the ready_q: schedule idle thread */
    if(!t) {
        t = optimsoc_scheduler_core[optimsoc_get_relcoreid()].idle_thread;
    }

    /* set active */
    optimsoc_scheduler_core[optimsoc_get_relcoreid()].active_thread = t;

    runtime_trace_schedule(t->id);

    /* switch the context */
    context_set(optimsoc_scheduler_core[optimsoc_get_relcoreid()].active_thread->ctx);

    or1k_mmu_init();

    /* activate timer */
    or1k_timer_reset();
    or1k_timer_enable();
}


/*
 * Is called when a thread has ended to free memory and schedule next thread
 */
void scheduler_thread_exit() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];

    runtime_trace_thread_exit();

    /* free thread memory */
    thread_destroy(core_ctx->active_thread);

    list_remove(all_threads,(void*) core_ctx->active_thread);

    if (!all_threads->head) {
        if (runtime_config_terminate_on_empty_readyq) {
            exit(0);
        }
    }

    /* reset active_thread */
    core_ctx->active_thread = NULL;

    schedule();

    ctx_replace();
}

int scheduler_thread_exists(thread_t thread) {
    return (list_contains(all_threads,(void*) thread));
}
