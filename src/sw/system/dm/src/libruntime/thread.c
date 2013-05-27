
#include "runtime.h"
#include "thread.h"
#include "report.h"
#include "scheduler.h"

/*
 * Create a new thread with a given starting function.
 */
int thread_create(thread_t *thread,void (*func)(void*), void* args)
{
	thread_t t;

	runtime_report_createthread_function((unsigned int) func);
	t = malloc(sizeof(struct thread_t));

	runtime_report_createthread_tid((unsigned int) t);

	arch_context_create(t, func, args);

	t->joinlist = list_init(0);

	scheduler_add(t,ready_q);
	list_add_tail(all_threads,(void*)t);

	*thread = t;

	return 0;
}

void thread_yield() {
	scheduler_yieldcurrent();
}

void thread_suspend() {
	scheduler_suspendcurrent();
}

void thread_resume(thread_t thread) {
	if (list_remove(wait_q,(void*)thread)) {
		runtime_report_scheduler_resume((unsigned int)thread);
		list_add_tail(ready_q,(void*) thread);
	} else {
		// Nothing to resume
	}
}

thread_t thread_self() {
	return active_thread;
}

void thread_join(thread_t thread) {
	if (scheduler_thread_exists(thread)) {
		// Add to waiting queue
		list_add_tail(thread->joinlist,(void*)active_thread);
		runtime_report_thread_joinwait((unsigned int)thread);

		// Suspend
		thread_suspend();
	}
}

/*
 * Destory a given thread. TODO use list-functions to destroy thread
 */
void thread_destroy(thread_t thread)
{
    /* Free memory */
	free(thread->ctx);
	free(thread->stack);
	free(thread);
}

void thread_handle(void (*f)(void*),void *arg) {
	runtime_report_thread_started((unsigned int)f);
	f(arg);
	runtime_report_thread_finished((unsigned int)f);
	thread_exit();
}

void thread_exit() {
	runtime_report_thread_exit((unsigned int)active_thread);

	thread_t t = (thread_t) list_remove_head(active_thread->joinlist);
	while (t) {
		runtime_report_thread_joinsignal((unsigned int)t);
		thread_resume(t);
		t = list_remove_head(active_thread->joinlist);
	}

	scheduler_thread_exit();
}

/*
 * Create a context for a new thread.
 */
int arch_context_create(thread_t thread, void (*start_routine)(void*), void *arg)
{
    /* Set thread id */
	thread->id = thread_next_id++;

    /* Create context and initialize to 0 */
	thread->ctx = malloc(sizeof(struct arch_thread_ctx_t));
	for (unsigned int i = 0; i < 31; i++) {
		thread->ctx->regs[i] = 0;
	}

    /* Set stack, arguments, starting routine and thread_handler */
	thread->stack = malloc(1024);
	thread->ctx->regs[0] = (unsigned int) thread->stack + 1020; /* Stack pointer */
	thread->ctx->regs[2] = (unsigned int) start_routine;
	thread->ctx->regs[3] = (unsigned int) arg;
	thread->ctx->pc      = (unsigned int) &thread_handle;
	thread->ctx->sr      = 0x8017;  /* - supervisor mode
									 * - tick timer enabled
									 * - irq enabled
									 * - ic enabled
									 * - dc disabled
									 * - mmu disabled for now */

	return 0;
}
