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
	optimsoc_thread_t thread = _optimsoc_scheduler_current();

    return thread->page_dir;
}

void optimsoc_thread_set_pagedir(optimsoc_thread_t thread, optimsoc_page_dir_t dir) {
	thread->page_dir = dir;
}

optimsoc_page_dir_t optimsoc_thread_get_pagedir(optimsoc_thread_t thread) {
	return thread->page_dir;
}


void thread_attr_init(struct optimsoc_thread_attr *attr) {
    attr->args = NULL;
    attr->flags = THREAD_FLAG_NO_FLAGS | THREAD_FLAG_KERNEL;
    attr->force_id = 0;
    attr->identifier = NULL;
}

volatile uint32_t thread_next_id;

/*
 * Create a new thread with a given starting function.
 */
int optimsoc_thread_create(optimsoc_thread_t *thread,
		void (*start)(void*), struct optimsoc_thread_attr *attr) {

	optimsoc_thread_t t;

    if (attr == NULL) {
        attr = malloc(sizeof(struct optimsoc_thread_attr));
        assert(attr != NULL);
        thread_attr_init(attr);
    }

    t = malloc(sizeof(struct optimsoc_thread));
    assert(t != NULL);
    t->attributes = attr;

    _optimsoc_context_create(t, start, t->attributes->args);

    if (t->attributes->flags & THREAD_FLAG_FORCEID) {
        t->id = t->attributes->force_id;
    } else {
        t->id = thread_next_id++;
    }

    if (t->attributes->identifier == NULL) {
        t->name = malloc(64);
        assert(t->name != NULL);
        snprintf(t->name, 64, "thread %lu", t->id);
        runtime_trace_createthread(t->name, t->id, thread, start);
    } else {
        t->name = strndup(t->attributes->identifier, 64);
        runtime_trace_createthread(t->attributes->identifier, t->id, thread, start);
    }

    t->joinlist = optimsoc_list_init(0);

    if(attr->flags & THREAD_FLAG_CREATE_SUSPENDED) {
		_optimsoc_scheduler_add(t, wait_q);
		t->state = THREAD_SUSPENDED;
    } else {
    	_optimsoc_scheduler_add(t,ready_q);
    }
    optimsoc_list_add_tail(all_threads,(void*)t);

    *thread = t;

    return t->id;
}

optimsoc_thread_t optimsoc_thread_current() {
	return _optimsoc_scheduler_current();
}

void optimsoc_thread_exit() {
    optimsoc_thread_t thread = optimsoc_thread_current();

    optimsoc_thread_t t;
    t = (optimsoc_thread_t) optimsoc_list_remove_head(thread->joinlist);
    while (t) {
        optimsoc_thread_resume(t);
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
    if (thread->attributes->flags & THREAD_FLAG_KERNEL) {
        thread->stack = malloc(4096);
        assert(thread->stack != NULL);
        thread->ctx->regs[0] = (unsigned int) thread->stack + 4092; /* Stack pointer */
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
