/*
 * thread.c
 *
 *  Created on: Nov 20, 2010
 *      Author: wallento
 */

#include "thread.h"

#include <stdlib.h>

#include "scheduler.h"
#include "malloc.h"

//#include ARCH_INCL(syscalls.h)
#include ARCH_INCL(spr-defs.h) // TODO: remove later

thread_id_t thread_next_id = 0;
thread_t 	*thread_table[256];

void thread_yield() {
	scheduler_suspend(&exception_ctx[coreid()]);
}

void thread_suspend() {
//	arch_syscall_suspend();
}

void thread_resume(unsigned int id) {
	scheduler_resume(id);
}

void thread_handle(void *(*start_routine)(void*), void *arg) {
	void *value = start_routine(arg);
	thread_exit(value);
}

void thread_exit(void* value) {
	// Drop value for the moment
//	thread_id_t id = thread_self();
//	thread_t *thread = thread_table[id];

	// Clean up

	scheduler_exit();
}

thread_id_t thread_self() {
	return scheduler_threadid();
}

int thread_create(thread_t *thread, const thread_attr_t *attr,
    void *(*start_routine)(void*), void *arg) {

	thread->id = thread_next_id++;

	for ( unsigned int i = 0; i < 31; i++ ) {
		thread->ctx.regs[i] = 0;
	}

	thread->stack = malloc(1024);
	thread->ctx.regs[0] = (unsigned int) thread->stack + 1020; // Stack pointer
	thread->ctx.regs[2] = (unsigned int) start_routine;
	thread->ctx.regs[3] = (unsigned int) arg;
	thread->ctx.pc      = (unsigned int) &thread_handle;
	thread->ctx.sr      = 0x801a;   // seems a good initial value
									// - user mode
									// - tick timer enabled
									// - ic/dc enabled
									// - mmu disabled for now

	thread_table[thread->id] = thread;

	scheduler_add(thread);

	return 0;
}

void process_terminate(arch_thread_ctx_t *syscall_ctx) {
	// Stop all threads/ensure they are stopped

	// Free resources

	// Delete data structures
}

void syscall_thread_yield(arch_thread_ctx_t *ctx) {
	scheduler_timer(ctx);
}
