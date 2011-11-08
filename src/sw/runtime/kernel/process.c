/*
 * process.c
 *
 *  Created on: Nov 27, 2010
 *      Author: wallento
 */

#include "process.h"
#include "thread.h"
#include "scheduler.h"

#include "arch.h"
#include ARCH_INCL(printf.h)
#include "malloc.h"

unsigned int _next_id = 0;

process_t *process_create(void *bin_start,void *bin_end) {
  //	printf("Create process\n");

	process_t *proc = malloc(sizeof(process_t));
	proc->page_dir = pagedirectory_create();

	// Take the pages of the binary from pool
	for ( void* p = (void*) bin_start; p <= (void*) bin_end; p += PAGESIZE ) {
		pagetable_addmapping(proc->page_dir,(void*) ((unsigned int)p-(unsigned int)bin_start+PROCESS_ENTRY),p);
	}

	void *stackpage = vmm_alloc_page();
	pagetable_addmapping(proc->page_dir,(void*)PROCESS_STACK_PAGE,stackpage);

	thread_t *thread = malloc(sizeof(thread_t));
	thread->id = _next_id++;

	for ( unsigned int i = 0; i < 31; i++ ) { // This is arch-specific!
		thread->ctx.regs[i] = 0;
	}
	thread->ctx.regs[0] = (unsigned int) PROCESS_STACK_PAGE + 0x2000 - 4; // Stack pointer
	thread->ctx.pc      = (unsigned int) 0x2000;
	thread->ctx.sr      = 0x807a;

	thread->process = proc;

	proc->threads = malloc(sizeof(thread_list_t));
	proc->threads->next   = NULL;
	proc->threads->thread = thread;

	scheduler_add(thread);

	return proc;
}

void process_destroy(process_t* proc) {
	return;
	// Remove all threads from scheduler
	thread_list_t *thread_it = proc->threads;
	for (;thread_it;thread_it=thread_it->next) {
		scheduler_remove(thread_it->thread);
	}

	// Free pages
	// TODO

	// Clean up data structures
	// TODO
}
