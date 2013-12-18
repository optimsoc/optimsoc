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

#ifndef OPTIMSOC_RUNTIME_H_
#define OPTIMSOC_RUNTIME_H_

/**
 * \defgroup libruntime Runtime library
 */

/**
 * \defgroup thread Thread Management
 * \ingroup libruntime
 * @{
 */

/**
 * Thread identifier.
 *
 * The internals are not exposed.
 */

typedef unsigned int thread_t;

/**
 * Create a new thread.
 *
 * The function allocates a thread data structure in the runtime system
 * and stores the pointer to it at the given address. The function
 * specified by start is called and the argument arg given to it.
 * The function is automatically added to the ready queue.
 */
int thread_create(thread_t *thread,void (*start)(void*),void* arg);

/**
 * Identify the thread currently running.
 */
thread_t thread_self();

/**
 * Yield current thread.
 *
 * The currently running thread yields its remaining quantum and is
 * scheduled to be executed later again (e.g., when waiting for I/O)
 */
void thread_yield();

/**
 * Suspend current thread.
 *
 * The currently running thread yields its remaining quantum and is not
 * executed again until woken up with thread_resume.
 */
void thread_suspend();

/**
 * Exit current thread's execution.
 */
void thread_exit();

/**
 * Resume a suspended thread.
 */
void thread_resume(thread_t thread);

/**
 * Wait for a thread until it exits.
 *
 * If the thread is still running, the function blocks and waits for
 * the finish of the thread. If it is already finished, the function
 * returns immediately.
 */
int thread_join(thread_t thread);

/**
 * @}
 */



#endif /* OPTIMSOC_RUNTIME_H_ */
