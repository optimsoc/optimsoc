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

#include <or1k-sprs.h>
#include <optimsoc-baremetal.h>

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <inttypes.h>

#include "include/optimsoc-runtime.h"
#include "vmm.h"
#include "list.h"
#include "thread.h"

#include "trace.h"

// A virtual address generally consists of a virtual page number (VPN) and
// an offset. The virtual memory subsystem translates this virtual address
// to a physical address, that consists of a physical page number and the
// same offset. The offset is the address inside the page, while the VPN
// needs to have a matching PPN.
//
//  +---------+--------+    +---------+--------+
//  |   VPN   | Offset | -> |   PPN   | Offset |
//  +---------+--------+    +---------+--------+
//
// In OpenRISC 1000 the pages are 8kB, leading to a 13 bit offset. The page
// number is therefore 19 bit wide, leading to a maximum of 524288 pages.

/*! Number of bits of page number in address */
#define OR1K_ADDR_PN_BITS          19
/*! MSB of page number in address */
#define OR1K_ADDR_PN_MSB           31
/*! LSB of page number in address */
#define OR1K_ADDR_PN_LSB           13

/*! Get page number in address */
#define OR1K_ADDR_PN_GET(addr) ((addr) >> OR1K_ADDR_PN_LSB)
/*! Set page number in address */
#define OR1K_ADDR_PN_SET(addr,pn) ((addr) | (((pn) & 0x7ffff) << OR1K_ADDR_PN_LSB))

/*! The number of bits for the offset */
#define OR1K_ADDR_OFFSET_BITS       13
/*! The MSB of the offset */
#define OR1K_ADDR_OFFSET_MSB        12
/*! The LSB of the offset */
#define OR1K_ADDR_OFFSET_LSB        0

/*! Get the offset in address */
#define OR1K_ADDR_OFFSET_GET(addr) ((addr) & 0x1fff)
/*! Set the offset in address */
#define OR1K_ADDR_OFFSET_SET(addr,offset) ((addr) | ((offset) & 0x1fff))

// The VPN-to-PPN translation is generally done using a table (as it is fast
// and it allows to do it also in hardware). We would therefore need a table
// with 524288 entries and lookup the PPN of VPN v at index v of this table.
// Such a table would be 2 MB large and we will need such a table for each
// task/process. Only if an application would use the entire memory space
// this would be efficient. As the number of actual physical pages is typically
// limited, it is a common approach to implement a two-level lookup, with a
// so called page directory at first level, where we look up the base pointer
// of the page table of a certain subrange of the address space.
//
// The VPN hence consists of two indices used for lookup at the L1 page
// directory and the L2 page table:
//
//  +----------+----------+--------+
//  | L1 index | L2 index | Offset |
//  +----------+----------+--------+

/*! The number of bits for the L1 index */
#define OR1K_ADDR_L1_INDEX_BITS      8
/*! The MSB of the L1 index */
#define OR1K_ADDR_L1_INDEX_MSB      31
/*! The LSB of the L1 index */
#define OR1K_ADDR_L1_INDEX_LSB      24

/*! Get the L1 index from a virtual address */
#define OR1K_ADDR_L1_INDEX_GET(addr) (addr >> OR1K_ADDR_L1_INDEX_LSB)
/*! Set the L1 index in address */
#define OR1K_ADDR_L1_INDEX_SET(addr,idx) ((addr & 0x00ffffff) | \
        (idx << OR1K_ADDR_L1_INDEX_LSB))

/*! The number of bits for the L2 index */
#define OR1K_ADDR_L2_INDEX_BITS     11
/*! The MSB of the L2 index */
#define OR1K_ADDR_L2_INDEX_MSB      23
/*! The LSB of the L2 index */
#define OR1K_ADDR_L2_INDEX_LSB      13

/*! Get the L2 index from a virtual address */
#define OR1K_ADDR_L2_INDEX_GET(addr) ((addr >> OR1K_ADDR_L2_INDEX_LSB) & 0x7ff)
/*! Set the L2 index in a virtual address */
#define OR1K_ADDR_L2_INDEX_SET(addr,idx) ((addr & 0xff001ff) | \
        ((idx & 0x7ff) << OR1K_ADDR_L2_INDEX_LSB))

// Page table entries are found both in the page directory and the page tables.
// OpenRISC 1000 also supports huge tables with 16MB pages where the page
// number is identical to the bits of the L1 index. But we do not support such
// large pages.
//
// A page table entry is defined as:
//
//  +-----+----+---------+------+-----+-------+----------+-----+-----+----+----+
//  | PPN | .. | PRESENT | LAST | PPI | DIRTY | ACCESSED | WOM | WBM | CI | CC |
//  +-----+----+---------+------+-----+-------+----------+-----+-----+----+----+
//   22 13         10       9    8   6    5         4       3     2     1    0
//
// The majority of flags are currently not supported. Dirty and accessed are
// used later to determine pages to write back to main memory in swapping. The
// page protection index has a simple mapping defined in the current hardware
// implementations that we adopt here.

/*! Number of bits in physical page number */
#define OR1K_PTE_PPN_BITS      19
/*! MSB of physical page number */
#define OR1K_PTE_PPN_MSB       31
/*! LSB of physical page number */
#define OR1K_PTE_PPN_LSB       13
#define OR1K_PTE_PPN_GET(pte) ((uint32_t) pte >> OR1K_ADDR_L2_INDEX_LSB)
#define OR1K_PTE_PPN_SET(pte,ppn) (pte | ( ppn << OR1K_ADDR_L2_INDEX_LSB))

/*! Bit number of present bit */
#define OR1K_PTE_PRESENT_BIT   10
#define OR1K_PTE_PRESENT_GET(pte) ((pte >> OR1K_PTE_PRESENT_BIT) & 0x1)
#define OR1K_PTE_PRESENT_SET(pte,present) \
        ((pte & ~(1 << OR1K_PTE_PRESENT_BIT)) | \
        ((present & 0x1) << OR1K_PTE_PRESENT_BIT))

/*! Bit number of last bit (marks huge table in directory) */
#define OR1K_PTE_LAST_LSB       9
/* Extract the last bit from pte */
#define OR1K_PTE_LAST_GET(pte) ((pte >> OR1K_PTE_LAST_LSB) & 0x1)

/*! Page protection index, number of bits */
#define OR1K_PTE_PPI_BITS       3
/*! MSB of PPI field */
#define OR1K_PTE_PPI_MSB        8
/*! LSB of PPI field */
#define OR1K_PTE_PPI_LSB        6
/*! Get PPI field from pte */
#define OR1K_PTE_PPI_GET(pte) ((pte >> OR1K_PTE_PPI_LSB) & 0x7)
/*! Set PPI field in pte */
#define OR1K_PTE_PPI_SET(pte,ppi) (pte | ((ppi & 0x7) << OR1K_PTE_PPI_LSB))
/*! Allow user access to page */
#define OR1K_PTE_PPI_USER_BIT   0
/*! Allow write access to page */
#define OR1K_PTE_PPI_WRITE_BIT  1
/*! Allow execution from page */
#define OR1K_PTE_PPI_EXEC_BIT   2

/*! Dirty bit */
#define OR1K_PTE_DIRTY_BIT      5
/*! Accessed bit */
#define OR1K_PTE_ACCESSED_BIT   4
/*! Weakly-ordered memory bit */
#define OR1K_PTE_WOM_BIT        3
/*! Write-back cache bit */
#define OR1K_PTE_WBC_BIT        2
/*! Cache inhibit bit */
#define OR1K_PTE_CI_BIT         1
/*! Cache coherency bit */
#define OR1K_PTE_CC_BIT         0

// A pte in the page directory contains the base pointer of the page table
/*! Assemble page table pointer from pte */
#define OR1K_PTABLE(pte) (pte & 0xffffe000)

/*! Helper macro to verify directory address from user input. */
#define VERIFY_DIR_ADDR(dir) assert(dir && (((uint32_t) dir & 0x3ff) == 0));

optimsoc_page_dir_t optimsoc_vmm_create_page_dir() {
    optimsoc_page_dir_t dir;

    // Allocate memory that is aligned to borders of multiples of its size.
    // This is necessary as the hardware-based TLB reload does an OR of the
    // page dir base and the offset.
    dir = (optimsoc_page_dir_t) memalign(0x400, 0x400);
    assert(dir);

    // Zero the directory
    memset((void*) dir, 0, 0x400);

    return dir;
}

optimsoc_page_table_t _optimsoc_vmm_create_page_table() {
    optimsoc_page_table_t table;

    // Allocate memory that is aligned to borders of multiples of its size.
    // This is necessary as the hardware-based TLB reload does an OR of the
    // page table base and the offset.
    table = memalign(0x2000, 0x2000);
    assert(table);

    // Zero the table
    memset((void*) table, 0, 0x2000);

    return table;
}

int optimsoc_vmm_map(optimsoc_page_dir_t directory, uint32_t vaddr,
                     uint32_t paddr) {
    optimsoc_pte_t dirpte;
    optimsoc_page_table_t table;
    optimsoc_pte_t pte;

    // Verify input
    VERIFY_DIR_ADDR(directory);

    // Address of the table's pte from the directory base
    void *taddr = (void*) &directory[OR1K_ADDR_L1_INDEX_GET(vaddr)];

    // Load-linked the pointer, as we may change it
    dirpte = or1k_sync_ll(taddr);

    // Check if table is present
    if (OR1K_PTE_PRESENT_GET(dirpte) == 0) {
        // There is no page table, allocate and map one
        table = _optimsoc_vmm_create_page_table();
        assert(table);

        // We now try to set this as te new table by updating the pte. As other
        // cores may have set a table at the same index concurrently, we do
        // a store-conditional.
        while (1) {
            // Set table present
            dirpte = OR1K_PTE_PRESENT_SET(dirpte, 1);
            // Set the physical page number, that is the table base address
            dirpte = OR1K_PTE_PPN_SET(dirpte, OR1K_PTE_PPN_GET(table));

            // Try to store this pte
            if (or1k_sync_sc(taddr,dirpte) == 1) {
                // The operation was successful
                break;
            } else {
                // There has been a write access to this PTE

                // Load the pte again. It may have been changed after we loaded
                // it or there was another LL call in between.
                dirpte = or1k_sync_ll(taddr);

                // Check if someone else modified or the store failed for other
                // reasons.
                if (OR1K_PTE_PRESENT_GET(dirpte) == 1) {
                    // Another core added this table. Simply free our
                    // table and continue.
                    free(table);
                    break;
                }
                // Otherwise we simply retry, do the operation again
            }
        }
    } else {
        // There already is a page, load base
        table = (optimsoc_page_table_t) OR1K_PTABLE(dirpte);
    }

    // This is a huge table what we do not support for now
    assert(OR1K_PTE_LAST_GET(dirpte) == 0);

    // Load the address of the pte in the page table
    void *pteaddr = (void*) &table[OR1K_ADDR_L2_INDEX_GET(vaddr)];

    // Load this entry
    pte = or1k_sync_ll(pteaddr);

    if (OR1K_PTE_PRESENT_GET(pte) == 1) {
        // There already is a page mapped
        return 0;
    }

    // Try to set this entry
    while (1) {
        // Get PN from physical address
        uint32_t ppn = OR1K_PTE_PPN_GET(paddr);

        // Assemble pte
        pte = OR1K_PTE_PPN_SET(0, ppn);
        pte = OR1K_PTE_PRESENT_SET(pte, 1);

        // TODO: Allow to set page policies
        // Allow all page accesses
        uint32_t ppi = (1 << OR1K_PTE_PPI_USER_BIT) | \
                (1 << OR1K_PTE_PPI_WRITE_BIT) | (1 << OR1K_PTE_PPI_EXEC_BIT);

        pte = OR1K_PTE_PPI_SET(pte, ppi);

        // Try to write pte
        if (or1k_sync_sc(pteaddr, pte) == 1) {
            // Operation was successful
            break;
        } else {
            // Another write occured
            pte = or1k_sync_ll(pteaddr);

            // Check why this failed
            if (OR1K_PTE_PRESENT_GET(pte) == 1) {
                // Another core added the same address
                return 0;
            }
            // Otherwise just retry
        }
    }

    return 1;
}

int optimsoc_vmm_unmap(optimsoc_page_dir_t directory,
                       uint32_t vaddr) {
    optimsoc_pte_t dirpte;
    optimsoc_page_table_t table;
    optimsoc_pte_t pte;

    // Verify input
    VERIFY_DIR_ADDR(directory);

    // Unmap is not thread safe at the moment. If you need to call it
    // concurrently it is necessary to guarantee that the operations
    // do not collide.

    // Load table pte in directory
    dirpte = directory[OR1K_ADDR_L1_INDEX_GET(vaddr)];

    // Check if table was mapped
    if (OR1K_PTE_PRESENT_GET(dirpte) == 0) {
        // There is no page table, this address was not mapped
        return 0;
    }

    // Extract table pointer
    table = (optimsoc_page_table_t) OR1K_PTABLE(dirpte);

    // We do not support huge tables for now
    assert(OR1K_PTE_LAST_GET(dirpte) == 0);

    // Load pte from table
    pte = table[OR1K_ADDR_L2_INDEX_GET(vaddr)];

    // Check if page is mapped
    if (OR1K_PTE_PRESENT_GET(pte) == 0) {
        // The page was not mapped
        return 0;
    }

    // Reset page table entry
    table[OR1K_ADDR_L2_INDEX_GET(vaddr)] = 0;

    return 1;
}

optimsoc_pte_t _optimsoc_vmm_lookup(optimsoc_page_dir_t directory,
                                    uint32_t vaddr) {
    optimsoc_pte_t pte;

    // Verify input
    VERIFY_DIR_ADDR(directory);

    // Load table pte
    pte = directory[OR1K_ADDR_L1_INDEX_GET(vaddr)];

    // Check if the table is present
    if (OR1K_PTE_PRESENT_GET(pte) == 0) {
        // invalid
        return 0;
    }

    // We do not support huge tables for now
    assert(OR1K_PTE_LAST_GET(pte) == 0);

    // Set page table base pointer
    optimsoc_page_table_t table = (optimsoc_page_table_t) OR1K_PTABLE(pte);

    // Load pte from table
    pte = table[OR1K_ADDR_L2_INDEX_GET(vaddr)];

    // Check if page is really mapped
    if (OR1K_PTE_PRESENT_GET(pte) == 0) {
        // invalid
        return 0;
    }

    return pte;
}

int optimsoc_vmm_virt2phys(optimsoc_page_dir_t directory,
                           uint32_t vaddr, uint32_t *paddr) {
    optimsoc_pte_t pte;

    // Verify input
    VERIFY_DIR_ADDR(directory);

    // Lookup pte
    pte = _optimsoc_vmm_lookup(directory, vaddr);

    // Check if page is mapped
    if (!pte) {
        return 0;
    }

    // Huge tables are not supported
    assert(OR1K_PTE_LAST_GET(pte) == 0);

    // Assemble physical address
    uint32_t addr = OR1K_ADDR_PN_SET(0, OR1K_PTE_PPN_GET(pte));
    addr = OR1K_ADDR_OFFSET_SET(addr, OR1K_ADDR_OFFSET_GET(vaddr));

    *paddr = addr;

    return 1;
}

int optimsoc_vmm_phys2virt(optimsoc_page_dir_t directory,
                           uint32_t paddr, uint32_t *vaddr) {

    // Verify input
    VERIFY_DIR_ADDR(directory);

    // First we iterate the directory
    for (int dirindex = 0; dirindex < 256; dirindex++) {
        // Extract each entry
        optimsoc_pte_t pte = directory[dirindex];

        // If the entry is invalid, try next one
        if (OR1K_PTE_PPI_GET(pte) == 0) {
            continue;
        }

        // Huge pages are not supported
        assert(OR1K_PTE_LAST_GET(pte) == 0);

        // No huge page, instead search (L2) page table
        optimsoc_page_table_t table = (optimsoc_page_table_t) OR1K_PTABLE(pte);

        // Iterate the page table
        for (int tableindex = 0; tableindex < 2048; tableindex++) {
            // Extract each entry
            pte = table[tableindex];

            // Try next one if invalid
            if (OR1K_PTE_PRESENT_GET(pte) == 0) {
                continue;
            }

            // If PPN matches the MSBs of the physical address this is a match
            if (OR1K_PTE_PPN_GET(paddr) == OR1K_PTE_PPN_GET(pte)) {
                // The vaddr is (L1 index, L2 index, offset)
                uint32_t addr = OR1K_ADDR_L1_INDEX_SET(0, dirindex);
                addr = OR1K_ADDR_L2_INDEX_SET(addr, tableindex);
                addr = OR1K_ADDR_OFFSET_SET(addr, OR1K_ADDR_OFFSET_GET(paddr));
                *vaddr = addr;
                return 1;
            }
        }
    }

    return 0;
}

void _optimsoc_vmm_init(void) {
    // Register exception handlers
    or1k_exception_handler_add(0x9, _optimsoc_dtlb_miss);
    or1k_exception_handler_add(0xA, _optimsoc_itlb_miss);
    or1k_exception_handler_add(0x3, _optimsoc_dpage_fault);
    or1k_exception_handler_add(0x4, _optimsoc_ipage_fault);
}

void _optimsoc_set_itlb(uint32_t vaddr, optimsoc_pte_t pte) {
    // Extract the index
    uint32_t index = OR1K_ADDR_L2_INDEX_GET(vaddr) & 0x3f;

    // Extract PPN from pte
    uint32_t ppn = OR1K_PTE_PPN_GET(pte);

    // Assemble translation register
    // Set PPN
    uint32_t tr = OR1K_SPR_IMMU_ITLBW_TR_PPN_SET(0, ppn);

    // Set access rights
    if (OR1K_PTE_PPI_GET(pte) & (1 << OR1K_PTE_PPI_EXEC_BIT)) {
        tr = OR1K_SPR_IMMU_ITLBW_TR_SXE_SET(tr, 1);
        // Allow user access
        if (OR1K_PTE_PPI_GET(pte) & (1 << OR1K_PTE_PPI_USER_BIT)) {
            tr = OR1K_SPR_IMMU_ITLBW_TR_UXE_SET(tr, 1);
        }
    }

    // Write translate register
    or1k_mtspr (OR1K_SPR_IMMU_ITLBW_TR_ADDR(0, index), tr);

    // Set page match register
    //  - valid
    //  - level 2 (8kB)
    //  - VPN from vaddr
    uint32_t mr = OR1K_SPR_IMMU_ITLBW_MR_V_SET(0, 1);
    mr = OR1K_SPR_IMMU_ITLBW_MR_VPN_SET(mr, OR1K_ADDR_PN_GET(vaddr));

    // Write match register
    or1k_mtspr (OR1K_SPR_IMMU_ITLBW_MR_ADDR(0, index), mr);
}

void _optimsoc_set_dtlb(uint32_t vaddr, optimsoc_pte_t pte) {
    // Extract the index
    uint32_t index = OR1K_ADDR_L2_INDEX_GET(vaddr) & 0x3f;

    // Extract PPN from pte
    uint32_t ppn = OR1K_PTE_PPN_GET(pte);

    // Assemble translation register
    // Set ppn
    uint32_t tr = OR1K_SPR_DMMU_DTLBW_TR_PPN_SET(0, ppn);

    // Set read rights
    if (OR1K_PTE_PPI_GET(pte) & (1 << OR1K_PTE_PPI_USER_BIT)) {
        // Allow user access
        tr = OR1K_SPR_DMMU_DTLBW_TR_URE_SET(tr, 1);
    }
    tr = OR1K_SPR_DMMU_DTLBW_TR_SRE_SET(tr, 1);

    // Set write rights
    if (OR1K_PTE_PPI_GET(pte) & (1 << OR1K_PTE_PPI_WRITE_BIT)) {
        tr = OR1K_SPR_DMMU_DTLBW_TR_SWE_SET(tr, 1);
        if (OR1K_PTE_PPI_GET(pte) & (1 << OR1K_PTE_PPI_USER_BIT)) {
            // Allow user access
            tr = OR1K_SPR_DMMU_DTLBW_TR_UWE_SET(tr, 1);
        }
    }

    // Write translate register
    or1k_mtspr (OR1K_SPR_DMMU_DTLBW_TR_ADDR(0, index), tr);

    // Set page match register
    //  - valid
    //  - level 2 (8kB)
    //  - VPN from vaddr
    uint32_t mr = OR1K_SPR_DMMU_DTLBW_MR_V_SET(0, 1);
    mr = OR1K_SPR_DMMU_DTLBW_MR_VPN_SET(mr, OR1K_ADDR_PN_GET(vaddr));

    // Write match register
    or1k_mtspr (OR1K_SPR_DMMU_DTLBW_MR_ADDR(0, index), mr);
}

// Allocate the callback function pointers for the page fault handlers
optimsoc_pfault_handler_fptr _optimsoc_vmm_dfault_handler;
optimsoc_pfault_handler_fptr _optimsoc_vmm_ifault_handler;

void optimsoc_vmm_set_dfault_handler(optimsoc_pfault_handler_fptr handler) {
    _optimsoc_vmm_dfault_handler = handler;
}

void optimsoc_vmm_set_ifault_handler(optimsoc_pfault_handler_fptr handler) {
    _optimsoc_vmm_ifault_handler = handler;
}

void _optimsoc_dtlb_miss(void) {
    optimsoc_page_dir_t dir;

    // Get current thread's page directory
    dir = _optimsoc_thread_get_pagedir_current();
    VERIFY_DIR_ADDR(dir);

    // Load vaddr from EEAR
    uint32_t vaddr = or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));

    // Trace debug
    runtime_trace_dtlb_miss(vaddr);

    // Lookup page
    optimsoc_pte_t pte = _optimsoc_vmm_lookup(dir, vaddr);

    if (pte) {
        // Update TLB
        _optimsoc_set_dtlb(vaddr, pte);
    } else {
        // Raise page fault
        assert(_optimsoc_vmm_dfault_handler);
        _optimsoc_vmm_dfault_handler(vaddr);
    }
}

void _optimsoc_itlb_miss(void) {
    optimsoc_page_dir_t dir;

    // Get current thread's page directory
    dir = _optimsoc_thread_get_pagedir_current();
    VERIFY_DIR_ADDR(dir);

    // Load vaddr from EEAR
    uint32_t vaddr = or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));

    // Trace debug
    runtime_trace_itlb_miss(vaddr);

    // Lookup page
    optimsoc_pte_t pte = _optimsoc_vmm_lookup(dir, vaddr);

    if (pte) {
        // Update TLB
        _optimsoc_set_itlb(vaddr, pte);
    } else {
        // Raise page fault
        assert(_optimsoc_vmm_ifault_handler);
        _optimsoc_vmm_ifault_handler(vaddr);
    }
}


void _optimsoc_dpage_fault(void) {
    // Load address from EEAR
    uint32_t vaddr = or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));

    // Call handler
    assert(_optimsoc_vmm_dfault_handler);
    _optimsoc_vmm_dfault_handler(vaddr);
}

void _optimsoc_ipage_fault(void) {
    // Load address from EEAR
    uint32_t vaddr = or1k_mfspr(OR1K_SPR_SYS_EEAR_ADDR(0));

    // Call handler
    assert(_optimsoc_vmm_ifault_handler);
    _optimsoc_vmm_ifault_handler(vaddr);
}

void optimsoc_vmm_destroy_page_dir(optimsoc_page_dir_t dir) {
    // Verify input
    VERIFY_DIR_ADDR(dir);

    // First we iterate the directory
    for (int dirindex = 0; dirindex < 256; dirindex++) {
        // Extract each entry
        optimsoc_pte_t pte = dir[dirindex];

        // If the entry is invalid, try next one
        if (OR1K_PTE_PRESENT_GET(pte) == 0) {
            continue;
        }

        // Huge pages are not supported
        assert(OR1K_PTE_LAST_GET(pte) == 0);

        // page table exists, free
        optimsoc_page_table_t table = (optimsoc_page_table_t) OR1K_PTABLE(pte);
        assert(table);

        free(table);
    }

    free(dir);
}

optimsoc_page_dir_t optimsoc_vmm_dir_copy(uint32_t remote_tile,
                                          void *remote_addr,
                                          page_alloc_fptr page_alloc_fnc)
{
    optimsoc_page_dir_t local_dir;
    optimsoc_page_dir_t remote_dir;

    local_dir = optimsoc_vmm_create_page_dir();

    remote_dir = (optimsoc_page_dir_t) malloc(0x400);
    assert(remote_dir);

    uint32_t *remote_table = malloc(0x2000);
    assert(remote_table);

    /* copy remote page dir */
    optimsoc_dma_transfer(remote_dir, remote_tile, remote_addr,
                          0x400, REMOTE2LOCAL);

    for (uint32_t dir_index = 0; dir_index < 0x100; dir_index ++) {

        if (OR1K_PTE_PRESENT_GET(remote_dir[dir_index]) == 1) {

            uint32_t *local_table =
                _optimsoc_vmm_create_page_table();

            optimsoc_pte_t dirpte = 0;
            dirpte = OR1K_PTE_PRESENT_SET(dirpte, 1);
            dirpte = OR1K_PTE_PPN_SET(dirpte, OR1K_PTE_PPN_GET(local_table));
            local_dir[dir_index] = (uint32_t) dirpte;

            optimsoc_dma_transfer(remote_table, remote_tile,
                                  (void*) OR1K_PTABLE(remote_dir[dir_index]),
                                  0x2000,
                                  REMOTE2LOCAL);

            for (uint32_t table_index = 0;
                table_index < 0x800;
                table_index ++) {

                if (OR1K_PTE_PRESENT_GET(remote_table[table_index])) {
                    void *local_page = (void *)((uint32_t)(page_alloc_fnc)() << 13);

                    uint32_t ppn = OR1K_PTE_PPN_GET(local_page);
                    uint32_t pte = OR1K_PTE_PPN_SET(0, ppn);
                    pte = OR1K_PTE_PRESENT_SET(pte, 1);

                    uint32_t ppi = (1 << OR1K_PTE_PPI_USER_BIT)
                        | (1 << OR1K_PTE_PPI_WRITE_BIT)
                        | (1 << OR1K_PTE_PPI_EXEC_BIT);

                    pte = OR1K_PTE_PPI_SET(pte, ppi);

                    local_table[table_index] = pte;

                    void *remote_page = (void *) OR1K_ADDR_PN_SET(0, OR1K_PTE_PPN_GET(remote_table[table_index]));

                    printf("copy page %p (tile %"PRIx32") to local page %p\n",
                           remote_page, remote_tile, local_page);

                    optimsoc_dma_transfer(local_page,
                                          remote_tile,
                                          remote_page,
                                          0x2000,
                                          REMOTE2LOCAL);
                }
            }

        }
    }

    free(remote_table);
    free(remote_dir);
    return local_dir;
}
