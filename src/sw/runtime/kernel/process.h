/*
 * process.h
 *
 *  Created on: Nov 27, 2010
 *      Author: wallento
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "vmm.h"

#define PROCESS_ENTRY      0x2000
#define PROCESS_STACK_PAGE 0x3e000

typedef struct thread_list_t {
	struct thread_list_t *next;
	struct thread_t      *thread;
} thread_list_t;

typedef struct process_t {
	thread_list_t    *threads;
	page_directory_t *page_dir;
} process_t;

process_t *process_create(void *bin_start,void *bin_end);
void process_destroy(process_t* proc);

#endif /* PROCESS_H_ */
