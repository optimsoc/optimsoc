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

#include "include/optimsoc-runtime.h"
#include "runtime.h"
#include "thread.h"
#include "trace.h"
#include "scheduler.h"
#include "vmm.h"
#include "optimsoc-baremetal.h"

#include <errno.h>
#include <assert.h>
#include <stdio.h>

optimsoc_page_dir_t _optimsoc_thread_get_pagedir_current() {
    // Get the currently executed thread
    optimsoc_thread_t thread = _optimsoc_scheduler_get_current();
    assert(thread);

    return thread->page_dir;
}

void optimsoc_thread_set_pagedir(optimsoc_thread_t thread,
                                 optimsoc_page_dir_t dir) {
    // Verify input
    assert(thread);

    thread->page_dir = dir;
}

optimsoc_page_dir_t optimsoc_thread_get_pagedir(optimsoc_thread_t thread) {
    // Verify input
    assert(thread);

    return thread->page_dir;
}


void optimsoc_thread_attr_init(struct optimsoc_thread_attr *attr) {
    // Verify input
    assert(attr);

    attr->args = NULL;
    attr->flags = 0;
    attr->force_id = 0;
    attr->identifier = NULL;
}

volatile uint32_t _optimsoc_thread_next_id;

int optimsoc_thread_create(optimsoc_thread_t *thread,
                           void (*start)(void*),
                           struct optimsoc_thread_attr *attr) {
    // Verify input
    assert(thread);

    optimsoc_thread_t t;

    if (attr == NULL) {
        // If no attributes are given, create default attributes
        attr = malloc(sizeof(struct optimsoc_thread_attr));
        assert(attr);
        optimsoc_thread_attr_init(attr);
    }

    // Allocate a new thread control block
    t = malloc(sizeof(struct optimsoc_thread));
    assert(t);

    // Set thread attributes
    t->attributes = attr;

    // Generate a context for the thread
    _optimsoc_context_create(t, start, t->attributes->args);

    // Check if the thread identifier is forced
    if (t->attributes->flags & OPTIMSOC_THREAD_FLAG_FORCEID) {
        // Set forced identifier
        t->id = t->attributes->force_id;
    } else {
        uint32_t id;
        // Assign next thread id and increment next thread id (thread-safe)
        do {
            id = _optimsoc_thread_next_id;
            t->id = id;
            // Try to write new value of thread_next_id. If it was changed
            // meanwhile, we retry the whole operation.
        } while (or1k_sync_cas((void*) &_optimsoc_thread_next_id, id, id+1) != id);
    }

    // Check if a thread identifier name is given
    if (t->attributes->identifier) {
        // Allocate memory for the string
        t->name = malloc(65);
        assert(t->name);

        // Set "thread <id>" as standard thread name
        snprintf(t->name, 64, "thread %lu", t->id);
    } else {
        // Otherwise copy string
        t->name = strndup(t->attributes->identifier, 64);
    }

    // Trace creation of thread
    runtime_trace_createthread(t->name, t->id, thread, start);

    // Initialize list of threads that wait for a join on exit of this thread
    t->joinlist = optimsoc_list_init(0);

    // Set initial state of thread
    if(attr->flags & OPTIMSOC_THREAD_FLAG_CREATE_SUSPENDED) {
        // Add to wait queue for suspended threads
        _optimsoc_scheduler_add(t, wait_q);
        // Set suspended state
        t->state = THREAD_SUSPENDED;
    } else {
        // Add to ready queue for active threads
        _optimsoc_scheduler_add(t,ready_q);
        // Set runnable state
        t->state = THREAD_RUNNABLE;
    }

    // Add to list of all threads
    optimsoc_list_add_tail(all_threads,(void*)t);

    // Assign to users pointer
    *thread = t;

    return 0;
}

optimsoc_thread_t optimsoc_thread_current() {
    optimsoc_thread_t t = _optimsoc_scheduler_get_current();
    assert(t);
    return t;
}

void optimsoc_thread_exit() {
    // Get current thread
    optimsoc_thread_t thread = optimsoc_thread_current();
    assert(thread);

    // Iterate list and resume all threads that have been waiting for this
    // to join
    optimsoc_thread_t t; // list iterator

    // Pop first waiting thread
    t = (optimsoc_thread_t) optimsoc_list_remove_head(thread->joinlist);
    while (t) {
        // Resume thread
        // TODO: optimsoc_thread_resume(t);
        // Pop next thread
        t = (optimsoc_thread_t) optimsoc_list_remove_head(thread->joinlist);
    }

    optimsoc_list_remove(all_threads,(void*) thread);

    _optimsoc_schedule();
}


void _optimsoc_kthread_handle(void (*f)(void*),void *arg) {
    f(arg);
    // TODO: Exit
    while(1) {}
}

/*
 * Create a context for a new thread.
 */
int _optimsoc_context_create(optimsoc_thread_t thread,
                             void (*start_routine)(void*), void *arg)
{
    /* Create context and initialize to 0 */
    thread->ctx = calloc(sizeof(struct arch_thread_ctx_t), 1);

    assert(thread->ctx != NULL);

    /* Set stack, arguments, starting routine and thread_handler */
    if (thread->attributes->flags & OPTIMSOC_THREAD_FLAG_KERNEL) {
        thread->stack = malloc(4096);
        assert(thread->stack != NULL);
        thread->ctx->regs[0] = (unsigned int) thread->stack + 4092;
        thread->ctx->regs[2] = (unsigned int) start_routine;
        thread->ctx->regs[3] = (unsigned int) arg;
        thread->ctx->pc      = (unsigned int) &_optimsoc_kthread_handle;
        thread->ctx->sr      = 0x8017;
        /* - supervisor mode
         * - tick timer enabled
         * - irq enabled
         * - ic enabled
         * - dc disabled
         * - mmu disabled */
    } else {
        /* Set stack, arguments, starting routine and thread_handler */
        thread->stack = NULL;
        thread->ctx->regs[0] = 0xfffffffc;
        /* For the moment only one parameter is supported. */
        thread->ctx->regs[2] = (unsigned int) arg;
        thread->ctx->pc = 0x0;  /* entry point of external app*/
        thread->ctx->sr = 0x8077;
        /* - supervisor mode
         * - tick timer enabled
         * - irq enabled
         * - ic enabled
         * - dc disabled
         * - immu/dmmu enabled */
    }

    return 0;
}
