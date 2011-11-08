/*
 * scheduler.c
 *
 *  Created on: Nov 20, 2010
 *      Author: wallento
 */

#include "arch.h"

#include "scheduler.h"
#include ARCH_INCL(printf.h)
#include ARCH_INCL(utils.h) // TODO: Should later be remove
#include ARCH_INCL(mm.h)

#include <stdlib.h>
#include <string.h>

#include "thread.h"
#include "malloc.h"

#define DEBUG_PRINT(...) if ( DEBUG_SCHEDULER ) printf(__VA_ARGS__)
#define DEBUG_QUEUES if ( DEBUG_SCHEDULER ) _scheduler_queues_print();

typedef struct scheduler_q_entry {
	thread_t *thread;
	struct scheduler_q_entry *previous;
	struct scheduler_q_entry *next;
} scheduler_q_entry_t;

typedef struct scheduler_q_t {
	scheduler_q_entry_t	*head;
	scheduler_q_entry_t	*tail;
} scheduler_q_t;

scheduler_q_t ready_q;
scheduler_q_t wait_q;

scheduler_q_entry_t *_scheduler_active_thread[NUMCORES];
thread_t *active_thread[NUMCORES];

arch_thread_ctx_t scheduler_ctx[NUMCORES];
arch_thread_ctx_t kernel_ctx[NUMCORES];

mutex_t scheduler_mutex;

void _scheduler_queue_enqueue(scheduler_q_t *q,scheduler_q_entry_t *entry) {
	mutex_lock(&scheduler_mutex);
	entry->next = NULL;
	if ( q->tail == NULL ) { // empty list
		entry->previous = NULL;
		q->head = entry;
		q->tail = entry;
	} else {
		entry->previous = q->tail;
		q->tail->next = entry;
		q->tail = entry;
	}
	mutex_unlock(&scheduler_mutex);
}

scheduler_q_entry_t *_scheduler_queue_dequeue(scheduler_q_t *q) {
	mutex_lock(&scheduler_mutex);
	scheduler_q_entry_t *t = NULL;
	if ( ready_q.head ) {
		t = ready_q.head;
		ready_q.head = ready_q.head->next;
		if (ready_q.head)
			ready_q.head->previous = NULL;
		else
			ready_q.tail = NULL;
	}
	mutex_unlock(&scheduler_mutex);
	return t;
}

void _scheduler_queue_remove(scheduler_q_t *q, scheduler_q_entry_t * entry) {
	mutex_lock(&scheduler_mutex);
	if ( entry->previous == NULL ) { // first in queue
		q->head = entry->next;
	} else {
		entry->previous->next = entry->next;
	}
	if ( entry->next == NULL ) {
		q->tail = entry->previous;
	} else {
		entry->next->previous = entry->previous;
	}
	mutex_unlock(&scheduler_mutex);
}

void _scheduler_queues_print() {
	mutex_lock(&scheduler_mutex);
	scheduler_q_entry_t *it;
	printf("[scheduler] Ready Queue:");
	it = ready_q.head;
	while ( it ) {
		printf(" %u",it->thread->id);
		it = it->next;
	}
	printf("\n");
	printf("[scheduler] Wait Queue:");
	it = wait_q.head;
	while ( it ) {
		printf(" %u",it->thread->id);
		it = it->next;
	}
	printf("\n");
	mutex_unlock(&scheduler_mutex);
}

scheduler_q_entry_t *_scheduler_queue_find(scheduler_q_t *queue,thread_id_t thread_id) {
	mutex_lock(&scheduler_mutex);

	scheduler_q_entry_t *it = queue->head;
	for (;it;it=it->next) {
		if (it->thread->id == thread_id) {
			return it;
		}
	}
	return NULL;

	mutex_unlock(&scheduler_mutex);
}

void scheduler_init() {
	ready_q.head = NULL;
	ready_q.tail = NULL;
	wait_q.head = NULL;
	wait_q.tail = NULL;
}

void scheduler_add(thread_t *thread) {
	scheduler_q_entry_t *e = malloc(sizeof(scheduler_q_entry_t));
	e->thread = thread;

	_scheduler_queue_enqueue(&ready_q,e);
}

void scheduler_remove(thread_t *thread) {
	scheduler_q_entry_t *entry;
	entry = _scheduler_queue_find(&ready_q,thread->id);
	if (entry) {
		_scheduler_queue_remove(&ready_q,entry);
		free(entry);
		return;
	}
	entry = _scheduler_queue_find(&wait_q,thread->id);
	if (entry) {
		_scheduler_queue_remove(&ready_q,entry);
		free(entry);
		return;
	}
}

void scheduler_yield(arch_thread_ctx_t *ctx) {
	DEBUG_PRINT("[scheduler] yield %u\n",_scheduler_active_thread[coreid()]->thread->id);
	DEBUG_QUEUES
	memcpy(&_scheduler_active_thread[coreid()]->thread->ctx,ctx,sizeof(arch_thread_ctx_t));
	_scheduler_queue_enqueue(&ready_q,_scheduler_active_thread[coreid()]);
	DEBUG_QUEUES
	schedule();
}

void scheduler_suspend(arch_thread_ctx_t *ctx) {
	DEBUG_PRINT("[scheduler] suspend %u\n",_scheduler_active_thread[coreid()]->thread->id);
	DEBUG_QUEUES
	memcpy(&_scheduler_active_thread[coreid()]->thread->ctx,ctx,sizeof(arch_thread_ctx_t));
	_scheduler_queue_enqueue(&wait_q,_scheduler_active_thread[coreid()]);
	DEBUG_QUEUES
	schedule();
}

void scheduler_resume(unsigned int threadid) {
	DEBUG_PRINT("[scheduler] resume %u by %u\n",threadid,_scheduler_active_thread[coreid()]->thread->id);
	DEBUG_QUEUES
	// Find entry of thread
	scheduler_q_entry_t *thread = NULL;
	scheduler_q_entry_t *it = wait_q.head;
	while ( it && !thread ) {
		if ( it->thread->id == threadid ) {
			thread = it;
			_scheduler_queue_remove(&wait_q,thread);
		}
	}
	if (thread) _scheduler_queue_enqueue(&ready_q,thread);
	DEBUG_QUEUES
}

void schedule() {
	_scheduler_active_thread[coreid()] = _scheduler_queue_dequeue( &ready_q );

	while (!_scheduler_active_thread[coreid()]) {
		for (unsigned int c=0;c<100;++c) __asm__("l.nop");
		_scheduler_active_thread[coreid()] = _scheduler_queue_dequeue( &ready_q );
	}

	active_thread[coreid()] = _scheduler_active_thread[coreid()]->thread;

	arch_report_scheduled_thread(active_thread[coreid()]->id);
	//	printf("[scheduler %u] schedule %u\n",coreid(),_scheduler_active_thread[coreid()]->thread->id);
	DEBUG_QUEUES

	arch_itlb_invalidate();
	arch_dtlb_invalidate();
	arch_enable_timer();
	arch_context_switch(&kernel_ctx[coreid()],&_scheduler_active_thread[coreid()]->thread->ctx);
}

void scheduler_start() {
	schedule();
}

thread_id_t scheduler_threadid() {
	return _scheduler_active_thread[coreid()]->thread->id;
}

void scheduler_exit() {
	free(_scheduler_active_thread[coreid()]);
	if (!ready_q.head) or32exit(1);

	schedule();
}

void scheduler_timer(arch_thread_ctx_t *ctx) {
	DEBUG_PRINT("[scheduler] timer yields %u\n",_scheduler_active_thread[coreid()]->thread->id);
	DEBUG_QUEUES
	memcpy(&_scheduler_active_thread[coreid()]->thread->ctx,ctx,sizeof(arch_thread_ctx_t));
	_scheduler_queue_enqueue(&ready_q,_scheduler_active_thread[coreid()]);
	DEBUG_QUEUES
	schedule();
}

#define SVC_SCHEDULER_DESTROY_PROC 0

void svc_scheduler_handler(unsigned int req,arch_thread_ctx_t *ctx) {
	switch (req) {
		case SVC_SCHEDULER_DESTROY_PROC:
			process_destroy(active_thread[coreid()]->process);
			schedule();
			break;
		default:
			break;
	}
}
