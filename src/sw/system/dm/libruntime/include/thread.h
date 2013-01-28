#ifndef THREAD_H_
#define THREAD_H_

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

#endif /* THREAD_H_ */
