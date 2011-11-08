/*
 * vmm.h
 *
 *  Created on: Nov 21, 2010
 *      Author: wallento
 */

#ifndef VMM_H_
#define VMM_H_

#include <stdlib.h>
#include "arch.h"

#define DEBUG_VMM 0

#define ADDR_TO_TABLEINDEX(addr) (((unsigned int) addr & PAGETABLEENTRY_MASK) >> PAGEBITS)
#define PAGE_BASE(addr) ((void*) ((unsigned int) addr & ~PAGEMASK))

typedef unsigned int page_table_entry_t;

typedef page_table_entry_t *page_table_t;

typedef struct page_directory_entry_t { // Same layout as list_elem_t
	struct page_directory_entry_t *next;
	struct page_directory_entry_t *prev;
	void         *vaddr_base;
	page_table_t table;
} page_directory_entry_t;

typedef struct page_directory_t { // Same layout as list_t
	page_directory_entry_t *head;
	page_directory_entry_t *tail;
} page_directory_t;

void vmm_initialize();
void vmm_reserve_page(void *p);

void *vmm_alloc_page();
void vmm_free_page(void* page);

size_t vmm_available_pages();

void pagetable_addmapping(page_directory_t* dir,void *vaddr,void *phyaddr);
int pagetable_virt2phys(page_directory_t *dir,void *vaddr,void **paddr);

page_directory_t *pagedirectory_create();
page_table_t pagedirectory_findtable(page_directory_t *dir,void *vaddr);

void itlb_miss_handler(void *vaddr,unsigned int pc);
void dtlb_miss_handler(void *vaddr,unsigned int pc);

void vmm_page_alloc_handler(void *vbase,unsigned int n);

#endif /* VMM_H_ */
