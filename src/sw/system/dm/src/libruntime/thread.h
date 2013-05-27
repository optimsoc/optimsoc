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
