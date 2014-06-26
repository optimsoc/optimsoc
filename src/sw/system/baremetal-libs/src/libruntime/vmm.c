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

#include <vmm.h>
#include <list.h>
#include <spr-defs.h>
#include <optimsoc-baremetal.h>
#include <assert.h>

#include "trace.h"
#include "task.h"

#include <stdio.h>

#define ITLB_PR_MASK    (SPR_ITLBTR_SXE | SPR_ITLBTR_UXE)
#define DTLB_PR_MASK    (SPR_DTLBTR_URE | SPR_DTLBTR_UWE | SPR_DTLBTR_SRE | SPR_DTLBTR_SWE)


void arch_set_itlb(void* vaddr, void* paddr) {
    unsigned int flags = ITLB_PR_NOLIMIT;
    unsigned int mr = ((unsigned int)vaddr & SPR_ITLBMR_VPN) | SPR_ITLBMR_V;
    unsigned int tr = ((unsigned int)paddr & SPR_ITLBTR_PPN) | (flags & ITLB_PR_MASK) | SPR_ITLBTR_A | SPR_ITLBTR_D;
    unsigned int set = ((((unsigned int)vaddr) >> PAGE_BITS) & 63);
    or1k_mtspr (SPR_ITLBMR_BASE(0)+set, mr);
    or1k_mtspr (SPR_ITLBTR_BASE(0)+set, tr);
}

void arch_set_dtlb(void *vaddr,void *paddr) {
    unsigned int flags = DTLB_PR_NOLIMIT;
    unsigned int mr = ((unsigned int)vaddr & SPR_DTLBMR_VPN) | SPR_ITLBMR_V;
    unsigned int tr = ((unsigned int)paddr & SPR_DTLBTR_PPN) | (flags & DTLB_PR_MASK) | SPR_DTLBTR_A | SPR_DTLBTR_D;
    unsigned int set = ((((unsigned int)vaddr) >> PAGE_BITS) & 63);
    or1k_mtspr (SPR_DTLBMR_BASE(0)+set, mr);
    or1k_mtspr (SPR_DTLBTR_BASE(0)+set, tr);
}

void vmm_init() {
    page_pool = list_init(NULL);

    /* Get page pool size */
    void *start_page = (void*) ((PHYSMEM_START + PAGESIZE) & ~PAGEMASK);
    void *end_page   = (void*) (PHYSMEM_END   & ~PAGEMASK);

    /* Setup page pool */
    for(void* page = start_page; page <= end_page; page += PAGESIZE) {
        list_add_tail(page_pool, page);
    }

    or1k_exception_handler_add(0x9, dtlb_miss);
    or1k_exception_handler_add(0xA, itlb_miss);
}

/* Initialize the page table for a new thread */
void vmm_init_thread(thread_t thread) {
    /* Initialize empty page table */
/*    thread->page_table = list_init(NULL);

    void *start_page = (void*) ((unsigned)thread->paddr_start & ~PAGEMASK);
    void *end_page   = (void*) ((unsigned)thread->paddr_end   & ~PAGEMASK);

    for(void* page = start_page; page <= end_page; page += PAGESIZE) {

        struct page_table_entry_t* entry = malloc(sizeof(struct page_table_entry_t));

        entry->vaddr_base = (void*) (page - start_page);
        entry->paddr_base = page;

        list_add_tail(thread->page_table, (void*)entry);
    }*/
}

void dtlb_miss() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    void *vaddr = (void*) or1k_mfspr(SPR_EEAR_BASE);

    runtime_trace_dtlb_miss(vaddr);

    /* Look up virtual address in the page table */
    struct page_table_entry_t* entry;
    entry = find_page_entry(core_ctx->active_thread->task->page_table, vaddr);

    if(entry != NULL) {

        /* Write address back to DTLB */
        arch_set_dtlb(entry->vaddr_base, entry->paddr_base);

    } else {

        void *page = vmm_alloc_page();
        assert(page != NULL);

        entry = malloc(sizeof(struct page_table_entry_t));

        entry->vaddr_base = PAGE_BASE(vaddr);
        entry->paddr_base = page;

        printf("Allocated new data page %p and mapped to %p\n", entry->paddr_base, entry->vaddr_base);

        list_add_tail(core_ctx->active_thread->task->page_table, (void*)entry);
        runtime_trace_dtlb_allocate_page(page);

        arch_set_dtlb(entry->vaddr_base, entry->paddr_base);

    }

}

void itlb_miss() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    void *vaddr = (void*) or1k_mfspr(SPR_EEAR_BASE);

    runtime_trace_itlb_miss(vaddr);

    /* Look up virtual address in the page table */
    struct page_table_entry_t* entry;
    entry = find_page_entry(core_ctx->active_thread->task->page_table, vaddr);
    assert(entry != NULL);
    /* Write address back to ITLB */
    arch_set_itlb(entry->vaddr_base, entry->paddr_base);
}

/* Find the page table entry to the given vaddr */
page_table_entry_t* find_page_entry(struct list_t* page_table, void* vaddr)
{
    struct list_entry_t* entry = page_table->head;

    while(entry != NULL) {

        if(((struct page_table_entry_t*)entry->data)->vaddr_base == PAGE_BASE(vaddr)) {
            return (struct page_table_entry_t*)entry->data;
        }

        entry = entry->next;
    }

    return NULL; /* Address not in page table */
}

void* vmm_virt2phys(thread_t thread, void* vaddr, size_t size){

    struct page_table_entry_t* entry = find_page_entry(thread->task->page_table, PAGE_BASE(vaddr));

    if(entry==NULL){
        return NULL;
    }

    return (entry->paddr_base + (vaddr - PAGE_BASE(vaddr)));
    //TODO page border check paddr+size
}

void vmm_page_table_free(list_t *page_table){

    struct page_table_entry_t* entry;

    entry = (page_table_entry_t*)list_remove_head(page_table);
    while(entry){
	/* Do not add pages from the memory image to the page pool */
	if((unsigned int)entry->paddr_base >= PHYSMEM_START){
	    list_add_tail(page_pool, entry->paddr_base);
	}
	free(entry);
	entry = (page_table_entry_t*)list_remove_head(page_table);
    }
}
void *vmm_alloc_page() {
    void *page = list_remove_head(page_pool);
    assert(page != NULL);

    printf("Page pool has %d pages left\n", list_length(page_pool));

    return page;
}
