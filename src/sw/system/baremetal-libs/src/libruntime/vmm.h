/* Copyright (c) 2012-2015 by the author(s)
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

#include "include/optimsoc-runtime.h"

/**
 * \ingroup paging
 * @{
 */

/**
 * Page table entry
 *
 * A page table entry is a 32 bit value as defined by the architecture
 * specification.
 */
typedef uint32_t optimsoc_pte_t;

/**
 * Page table
 *
 * A page table is an array of page table entries.
 */
typedef optimsoc_pte_t* optimsoc_page_table_t;

/**
 * Initialize virtual memory subsystem
 *
 * Set the exception handlers for the memory management units.
 */
void _optimsoc_vmm_init(void);

/**
 * Allocate page table
 *
 * Allocate a page table in memory and initialize all entries to 0.
 *
 * @return New allocated table
 */
optimsoc_page_table_t _optimsoc_vmm_create_page_table();

/**
 * Lookup virtual address in page directory
 *
 * This is the lookup function that does the two level lookup for a virtual
 * address and returns the page table entry. Call optimsoc_vmm_virt2phys if
 * you need the physical address.
 *
 * @param directory Directory to search
 * @param vaddr Virtual address to lookup
 * @return Page table entry for virtual address
 */
optimsoc_pte_t _optimsoc_vmm_lookup(optimsoc_page_dir_t directory,
                                    uint32_t vaddr);

/**
 * Set the DTLB entry
 *
 * @param vaddr Virtual address to set
 * @param pte Corresponding page table entry
 */
void _optimsoc_set_dtlb(uint32_t vaddr, optimsoc_pte_t pte);

/**
 * Set the ITLB entry
 *
 * @param vaddr Virtual address to set
 * @param pte Corresponding page table entry
 */
void _optimsoc_set_itlb(uint32_t vaddr, optimsoc_pte_t pte);


/*! DTLB miss handler */
void _optimsoc_dtlb_miss(void);
/*! ITLB miss handler */
void _optimsoc_itlb_miss(void);

/*! DMMU fault handler */
void _optimsoc_dpage_fault(void);
/*! IMMU fault handler */
void _optimsoc_ipage_fault(void);

/**
 * @}
 */

#endif
