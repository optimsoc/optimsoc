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

#include "optimsoc-runtime-internal.h"
#include <vmm.h>
#include <list.h>
#include <or1k-sprs.h>
#include <optimsoc-baremetal.h>
#include <assert.h>

#include "trace.h"

#include <stdio.h>
#include <malloc.h>

#define ITLB_PR_MASK    (SPR_ITLBTR_SXE | SPR_ITLBTR_UXE)
#define DTLB_PR_MASK    (SPR_DTLBTR_URE | SPR_DTLBTR_UWE | SPR_DTLBTR_SRE | SPR_DTLBTR_SWE)

#define OR1K_PAGES_L1_BITS      8
#define OR1K_PAGES_L1_MSB      31
#define OR1K_PAGES_L1_LSB      24
#define OR1K_PAGES_L2_BITS     11
#define OR1K_PAGES_L2_MSB      23
#define OR1K_PAGES_L2_LSB      13
#define OR1K_PAGES_OFFSET_BITS 13
#define OR1K_PAGES_OFFSET_MSB  12
#define OR1K_PAGES_OFFSET_LSB   0

#define OR1K_PTE_PPN_BITS      22
#define OR1K_PTE_PPN_MSB       31
#define OR1K_PTE_PPN_LSB       10
#define OR1K_PTE_LAST_BITS      1
#define OR1K_PTE_LAST_MSB       9
#define OR1K_PTE_LAST_LSB       9
#define OR1K_PTE_PPI_BITS       3
#define OR1K_PTE_PPI_MSB        8
#define OR1K_PTE_PPI_LSB        6
#define OR1K_PTE_DIRTY_BITS     1
#define OR1K_PTE_DIRTY_MSB      5
#define OR1K_PTE_DIRTY_LSB      5
#define OR1K_PTE_ACCESSED_BITS  1
#define OR1K_PTE_ACCESSED_MSB   4
#define OR1K_PTE_ACCESSED_LSB   4
#define OR1K_PTE_WOM_BITS       1
#define OR1K_PTE_WOM_MSB        3
#define OR1K_PTE_WOM_LSB        3
#define OR1K_PTE_WBC_BITS       1
#define OR1K_PTE_WBC_MSB        2
#define OR1K_PTE_WBC_LSB        2
#define OR1K_PTE_CI_BITS        1
#define OR1K_PTE_CI_MSB         1
#define OR1K_PTE_CI_LSB         1
#define OR1K_PTE_CC_BITS        1
#define OR1K_PTE_CC_MSB         0
#define OR1K_PTE_CC_LSB         0

#define OR1K_ADDR_L1_INDEX_GET(addr) (addr >> OR1K_PAGES_L1_LSB)
#define OR1K_ADDR_L2_INDEX_GET(addr) ((addr >> OR1K_PAGES_L2_LSB) & 0x7ff)
#define OR1K_ADDR_OFFSET(addr) (addr & 0x1fff)

#define OR1K_PADDR_L1(pte,vaddr) (pte & 0xff000000) | (vaddr & 0x00ffffff)
#define OR1K_PADDR_L2(pte,vaddr) (pte & 0xffffe000) | (vaddr & 0x00001fff)
#define OR1K_PTABLE(pte) (pte & 0xffffe000)

#define OR1K_PTE_PPN_GET(x) (x >> OR1K_PTE_PPN_LSB)
#define OR1K_PTE_PPN_TRUNC_GET(x) (x >> OR1K_PAGES_L1_LSB)
#define OR1K_PTE_PPI_GET(x) ((x >> OR1K_PTE_PPI_LSB) & 0x7)
#define OR1K_PTE_LAST (1 << OR1K_PTE_LAST_LSB)
#define OR1K_PTE_DIRTY (1 << OR1K_PTE_DIRTY_LSB)
#define OR1K_PTE_ACCESSED (1 << OR1K_PTE_ACCESSED_LSB)


optimsoc_page_dir_t optimsoc_vmm_create_page_dir() {
	return (optimsoc_page_dir_t) memalign(0x800, 0x800);
}

optimsoc_page_table_t optimsoc_vmm_create_page_table() {
	return memalign(0x2000, 0x2000);
}

void optimsoc_vmm_add_page_table(optimsoc_page_dir_t directory,
		uint32_t index, optimsoc_page_table_t table) {
	optimsoc_pte_t pte = (uint32_t) table & 0xffffe000; // no futher bits needed
	directory[index] = pte;
}

uint32_t optimsoc_vmm_virt2phys(optimsoc_page_dir_t directory,
		uint32_t vaddr, uint32_t *paddr) {

	optimsoc_pte_t pte = directory[OR1K_ADDR_L2_INDEX_GET(vaddr)];

	if (OR1K_PTE_PPI_GET(pte) == 0) {
		// invalid
		return 0;
	}

	if (pte & OR1K_PTE_LAST) {
		// Huge page
		*paddr = OR1K_PADDR_L1(pte, vaddr);
		return 1;
	}

	optimsoc_page_table_t table = (optimsoc_page_table_t) OR1K_PTABLE(pte);
	pte = table[OR1K_ADDR_L2_INDEX_GET(vaddr)];

	if (OR1K_PTE_PPI_GET(pte) == 0) {
		// invalid
		return 0;
	}

	*paddr = OR1K_PADDR_L2(pte, vaddr);
	return 1;
}

uint32_t optimsoc_vmm_phys2virt(optimsoc_page_dir_t directory,
		uint32_t paddr, uint32_t *vaddr) {

	// First we iterate the directory
	for (int dirindex = 0; dirindex < 256; dirindex++) {
		// Extract each entry
		optimsoc_pte_t pte = directory[dirindex];

		// If the entry is invalid, try next one
		if (OR1K_PTE_PPI_GET(pte) == 0) {
			continue;
		}

		// If this is a huge page
		if (pte & OR1K_PTE_LAST) {
			// If the truncated PPN matches the MSBs of the physical address
			// this is a match
			if (OR1K_PTE_PPN_TRUNC_GET(paddr) == OR1K_PTE_PPN_TRUNC_GET(pte)) {
				// The vaddr is (index, lower MSBs of paddr)
				*vaddr = (dirindex << OR1K_PAGES_L1_LSB) | (paddr & 0x00ffffff);
				return 1;
			}
		}

		// No huge page, instead search (L2) page table
		optimsoc_page_table_t table = (optimsoc_page_table_t) OR1K_PTABLE(pte);

		// Iterate the page table
		for (int tableindex = 0; tableindex < 2048; tableindex++) {
			// Extract each entry
			pte = table[tableindex];

			// Try next one if invalid
			if (OR1K_PTE_PPI_GET(pte) == 0) {
				continue;
			}

			// If the PPN matches the MSBs of the physical address this is a match
			if (OR1K_PTE_PPN_GET(paddr) == OR1K_PTE_PPN_GET(pte)) {
				// The vaddr is (L1 index, L2 index, offset)
				*vaddr = (dirindex << OR1K_PAGES_L1_LSB) | \
						(tableindex << OR1K_PAGES_L2_LSB) | OR1K_ADDR_OFFSET(paddr);
				return 1;
			}
		}
	}

	return 0;
}

//void arch_set_itlb(void* vaddr, void* paddr) {
//    // Extract the index (shift by 13 and mask)
//    uint32_t index = OR1K_SPR_IMMU_ITLBW_MR_VPN_GET((uint32_t) vaddr) & 0x3f;
//
//    // Extract VPN (properly shift and mask)
//    uint32_t vpn = OR1K_SPR_IMMU_ITLBW_MR_VPN_GET((uint32_t) vaddr);
//
//    // Set page match register
//    //  - valid
//    //  - level 2 (8kB)
//    //  - VPN from vaddr
//    uint32_t mr = OR1K_SPR_IMMU_ITLBW_MR_V_SET(0, 1);
//    mr = OR1K_SPR_IMMU_ITLBW_MR_VPN_SET(mr, vpn);
//
//    or1k_mtspr (OR1K_SPR_IMMU_ITLBW_MR_ADDR(0, index), mr);
//
//    // Extract PPN (properly shift and mask)
//    uint32_t ppn = OR1K_SPR_IMMU_ITLBW_TR_PPN_GET((uint32_t) paddr);
//
//    // Set page translation register
//    //  - allow all accesses
//    uint32_t tr = OR1K_SPR_IMMU_ITLBW_TR_UXE_SET(0, 1);
//    tr = OR1K_SPR_IMMU_ITLBW_TR_SXE_SET(tr, 1);
//    tr = OR1K_SPR_IMMU_ITLBW_TR_PPN_SET(tr, ppn);
//
//    or1k_mtspr (OR1K_SPR_IMMU_ITLBW_TR_ADDR(0, index), tr);
//}
//
//void arch_set_dtlb(void *vaddr, void *paddr) {
//    // Extract the index (shift by 13 and mask)
//    uint32_t index = OR1K_SPR_DMMU_DTLBW_MR_VPN_GET((uint32_t) vaddr) & 0x3f;
//
//    // Extract VPN (properly shift and mask)
//    uint32_t vpn = OR1K_SPR_DMMU_DTLBW_MR_VPN_GET((uint32_t) vaddr);
//
//    // Set page match register
//    //  - valid
//    //  - level 2 (8kB)
//    //  - VPN from vaddr
//    uint32_t mr = OR1K_SPR_DMMU_DTLBW_MR_V_SET(0, 1);
//    mr = OR1K_SPR_DMMU_DTLBW_MR_VPN_SET(mr, vpn);
//
//    or1k_mtspr (OR1K_SPR_DMMU_DTLBW_MR_ADDR(0, index), mr);
//
//    // Extract PPN (properly shift and mask)
//    uint32_t ppn = OR1K_SPR_DMMU_DTLBW_TR_PPN_GET((uint32_t) paddr);
//
//    // Set page translation register
//    //  - allow all accesses
//    uint32_t tr = OR1K_SPR_DMMU_DTLBW_TR_URE_SET(0, 1);
//    tr = OR1K_SPR_DMMU_DTLBW_TR_UWE_SET(tr, 1);
//    tr = OR1K_SPR_DMMU_DTLBW_TR_SRE_SET(tr, 1);
//    tr = OR1K_SPR_DMMU_DTLBW_TR_SWE_SET(tr, 1);
//    tr = OR1K_SPR_DMMU_DTLBW_TR_PPN_SET(tr, ppn);
//
//    or1k_mtspr (OR1K_SPR_DMMU_DTLBW_TR_ADDR(0, index), tr);
//}
//
//void vmm_init() {
//    or1k_exception_handler_add(0x9, dtlb_miss);
//    or1k_exception_handler_add(0xA, itlb_miss);
//}
//
//void dtlb_miss() {
//    struct optimsoc_scheduler_core *core_ctx;
//    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];
//
//    void *vaddr = (void*) or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));
//
//    runtime_trace_dtlb_miss(vaddr);
//
//    /* Look up virtual address in the page table */
//    struct page_table_entry_t* entry;
//    entry = find_page_entry(core_ctx->active_thread->task->page_table, vaddr);
//
//    if(entry != NULL) {
//
//        /* Write address back to DTLB */
//        arch_set_dtlb(entry->vaddr_base, entry->paddr_base);
//
//    } else {
//
//        void *page = vmm_alloc_page();
//        assert(page != NULL);
//
//        entry = malloc(sizeof(struct page_table_entry_t));
//
//        entry->vaddr_base = PAGE_BASE(vaddr);
//        entry->paddr_base = page;
//
//        printf("Allocated new data page %p and mapped to %p\n", entry->paddr_base, entry->vaddr_base);
//
//        optimsoc_list_add_tail(core_ctx->active_thread->task->page_table, (void*)entry);
//        runtime_trace_dtlb_allocate_page(page);
//
//        arch_set_dtlb(entry->vaddr_base, entry->paddr_base);
//
//    }
//
//}
//
//void itlb_miss() {
//    struct optimsoc_scheduler_core *core_ctx;
//    core_ctx = &optimsoc_scheduler_core[optimsoc_get_relcoreid()];
//
//    void *vaddr = (void*) or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));
//
//    runtime_trace_itlb_miss(vaddr);
//
//    /* Look up virtual address in the page table */
//    struct page_table_entry_t* entry;
//    entry = find_page_entry(core_ctx->active_thread->task->page_table, vaddr);
//    assert(entry != NULL);
//    /* Write address back to ITLB */
//    arch_set_itlb(entry->vaddr_base, entry->paddr_base);
//}
//
//
