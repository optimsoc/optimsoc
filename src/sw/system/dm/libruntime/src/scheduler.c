#include <exceptions.h>
#include <or1k-support.h>

#include <runtimeconfig.h>

#include "list.h"
#include "thread.h"
#include "scheduler.h"
#include "runtime.h"
#include "report.h"

struct list_t* all_threads;
struct list_t* ready_q;
struct list_t* wait_q;

thread_t idle_thread;
thread_t init_thread;
extern void init();

thread_t active_thread;

extern const unsigned int runtime_config_terminate_on_empty_readyq;

void idle_thread_func() {
	while (1) { }
}

void scheduler_tick() {
    runtime_report_schedulertick();
	/* save context */
	memcpy(active_thread->ctx, exception_ctx, sizeof(struct arch_thread_ctx_t));

	/* put active thread into the queue */
	scheduler_add(active_thread, ready_q);

	/* schedule next thread */
	schedule();
}

void scheduler_yieldcurrent() {
	/* put active thread into the queue */
	scheduler_add(active_thread, ready_q);

	runtime_report_scheduler_yield();
	yield_switchctx(active_thread->ctx);
}

void scheduler_suspendcurrent() {
	/* put active thread into the queue */
	scheduler_add(active_thread, wait_q);

	active_thread->state = THREAD_SUSPENDED;

	runtime_report_scheduler_suspend();
	yield_switchctx(active_thread->ctx);
}

void scheduler_init() {
	_exceptions_add_handler(5,&scheduler_tick);

	or1k_timer_init(optimsoc_ticks);

	wait_q = list_init(0);
	ready_q= list_init(0);
	all_threads = list_init(0);

	thread_create(&init_thread,&init,0);
	thread_create(&idle_thread,&idle_thread_func,0);
	list_remove(ready_q,(void*)idle_thread);
	list_remove(all_threads,(void*)idle_thread);
}

void scheduler_add(thread_t t, struct list_t* q) {
	if(q == ready_q) {
		t->state = THREAD_RUNNABLE;
	}

	list_add_tail(q, (void*)t);
	runtime_report_scheduleradd_queue((unsigned int) q);
	runtime_report_scheduleradd_thread((unsigned int) t);
}

void scheduler_start() {
	schedule();
	// We got to nirvana
	ctx_replace();
}

void context_set(arch_thread_ctx_t *ctx) {
	memcpy(exception_ctx, ctx, sizeof(struct arch_thread_ctx_t));
}

void schedule() {
	/* get the next thread from the ready_q */
	struct thread_t* t = (struct thread_t*)list_remove_head(ready_q);

	/* In case we don't have a thread in the ready_q: schedule idle thread */
	if(!t) {
		t = idle_thread;
	}

	/* set active */
	active_thread = t;

	runtime_report_scheduler_select((unsigned int)t);

	/* switch the context */
	context_set(active_thread->ctx);

	/* activate timer */
    or1k_timer_reset();
    or1k_timer_enable();
}


/*
 * Is called when a thread has ended to free memory and schedule next thread
 */
void scheduler_thread_exit() {
	/* free thread memory */
	thread_destroy(active_thread);

	list_remove(all_threads,(void*) active_thread);

	if (!all_threads->head) {
		if (runtime_config_terminate_on_empty_readyq) {
			exit(0);
		}
	}

	/* reset active_thread */
	active_thread = NULL;

	schedule();

	ctx_replace();
}

int scheduler_thread_exists(thread_t thread) {
	return (list_contains(all_threads,(void*) thread));
}
