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
 *   Max Koenen <koenenwmn@googlemail.com>
 */

#include <optimsoc-runtime.h>
#include "gzll.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

extern void* _end;
extern void* _or1k_stack_bottom;

struct optimsoc_list_t *gzll_pagepool_local;

uint32_t gzll_swapping() {
	// TODO: Check if global memory is there
	return 0;
}

void gzll_paging_init() {
    optimsoc_vmm_set_dfault_handler(gzll_paging_dpage_fault);
    optimsoc_vmm_set_ifault_handler(gzll_paging_ipage_fault);

    // Local page table
    uint32_t start = (uint32_t) &_end;
    uint32_t end = (uint32_t) _or1k_stack_bottom;

    // We reserve four pages for the heap
    start += 4*8192;

    uint32_t start_page = start+8191 >> 13;
    uint32_t end_page = (end >> 13) - 1;

    gzll_pagepool_local = optimsoc_list_init(0);

    for (int p = start_page; p <= end_page; p++) {
        optimsoc_list_add_tail(gzll_pagepool_local, (void*) p);
    }
}

void gzll_paging_dpage_fault(uint32_t vaddr) {

}

void gzll_paging_ipage_fault(uint32_t vaddr) {

}
