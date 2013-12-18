/* Copyright (c) 2013 by the author(s)
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
 */

#ifndef THREAD_H
#define THREAD_H

#include "context.h"
#include <stdlib.h>
#include <string.h>
#include <list.h>

/* Thread identifier */
typedef unsigned int tid_t;

/*
 * Thread states.
 */
enum thread_state {
    THREAD_RUNNABLE   = 0,
    THREAD_SUSPENDED  = 1,
    THREAD_TERMINATED = 2
};

/*
 * Thread control block.
 */
struct thread_t {
	tid_t id;
	struct arch_thread_ctx_t *ctx;
	void *stack;

	enum thread_state state;
	unsigned int flags;

	/* Thread exit code */
	unsigned int exit_code;

	struct list_t *joinlist;
};

typedef struct thread_t *thread_t;

unsigned int thread_next_id;

int thread_create(thread_t* t,void (*func)(void*), void* args);
void thread_destroy(thread_t thread);
int arch_context_create(thread_t thread, void (*start_routine)(void*), void *arg);
void thread_yield();
void thread_suspend();
void thread_resume(thread_t t);
thread_t thread_self();
void thread_join(thread_t thread);
void thread_exit();

#endif
