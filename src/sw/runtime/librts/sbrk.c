/*
 * sbrk.c
 *
 *  Created on: Nov 25, 2010
 *      Author: wallento
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include "arch.h"
#include ARCH_INCL(syscalls.h)

extern caddr_t _end;

/* just in case, most boards have at least some memory */
#ifndef RAMSIZE
#  define RAMSIZE             (caddr_t)0x40000
#endif

#define PAGESIZE 0x2000
#define PAGEBITS 13
#define PAGEMASK 0x1fff

void *sbrk(ptrdiff_t size) {
	static caddr_t heap_ptr = 0;
	caddr_t        base;

	if (heap_ptr == 0) {
		heap_ptr = (caddr_t)&_end;
	}

	if ((RAMSIZE - heap_ptr) >= 0) {
		base = heap_ptr;
		unsigned int current_heap_pageno = ((unsigned int) heap_ptr) >> PAGEBITS;
		unsigned int final_heap_pageno   = ((unsigned int) heap_ptr + size ) >> PAGEBITS;
		unsigned int needed_pages = final_heap_pageno - current_heap_pageno;
		if ( needed_pages > 0 ) {
			void *next_heap_page = (void*) ((current_heap_pageno + 1) << PAGEBITS);
			arch_syscall_alloc_pages(next_heap_page,needed_pages);
		}
		heap_ptr += size;
		return (base);
	} else {
		errno = ENOMEM;
		return ((caddr_t)-1);
	}
}

