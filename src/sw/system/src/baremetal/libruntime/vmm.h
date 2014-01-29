/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#ifndef VMM_H
#define VMM_H

//#include "stdlib.h"
//#include "string.h"
#include "scheduler.h"
#include "thread.h"
//#include "arch.h"
//#include "services.h"
//#include ARCH_INCL(mm.h)

#define NUM_VIRT_PAGES 10
#define PAGE_BASE(addr) ((void*) ((unsigned int) addr & ~PAGEMASK))

extern char _apps_begin;
extern char _app_end;

/*optimsoc_rts/arch/openrisc/board.h*/
/*TODO verify and move to right position*/

#define PHYSMEM_START ((unsigned)&_app_end)
#define PHYSMEM_END   0x000fffff

#define VIRTMEM_START 0x00002000
#define VIRTMEM_END   0x000fffff // TODO: increase virtual memory

#define PAGESIZE 0x2000
#define PAGEBITS 13
#define PAGEMASK 0x1fff

#define PAGETABLE_ENTRIES 128
#define PAGETABLE_SIZE    0x100000
#define PAGETABLE_BITS    20
#define PAGETABLE_MASK    0x0fffff

#define PAGETABLEENTRY_BITS  7
#define PAGETABLEENTRY_MASK  0x000fe000

#define PAGETABLEENTRY_VPN   0xfffff000
#define PAGETABLEENTRY_VALID 0x00000001

#define STACK_SIZE 256 // in words
#define PAGE_SIZE  0x2000
#define PAGE_BITS  13
/*optimsoc_rts/arch/openrisc/board.h*/

/* TODO auslagern in page.h/c ? */
struct list_t* page_pool;
struct list_t* virt_page_pool;

typedef struct page_table_entry_t {
    void *vaddr_base;
    void *paddr_base;
    unsigned int flags;
} page_table_entry_t;

void vmm_init();
void vmm_init_thread(thread_t thread);
void dtlb_miss();
void itlb_miss();
page_table_entry_t* find_page_entry(struct list_t* page_table, void* vaddr);
void* vmm_virt2phys(thread_t thread, void* vaddr, size_t size);
void vmm_page_table_free(list_t *page_table);

void *vmm_alloc_page();

#endif
