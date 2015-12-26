/* Copyright (c) 2015 by the author(s)
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
 * =================================================================
 *
 * Syscall handling.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <assert.h>

#include <optimsoc-baremetal.h>
#include "include/optimsoc-runtime.h"
#include "syscalls.h"

/*! Pointer to the array of structs which contains one syscall per core. */
struct optimsoc_syscall *_optimsoc_syscall_data;

optimsoc_syscall_handler_fptr _optimsoc_syscall_handler;

void _optimsoc_syscalls_init() {
    // Set the exception handler for syscall exceptions
    or1k_exception_handler_add(0xc, _optimsoc_syscall_entry);

    void* c = calloc(or1k_numcores(), sizeof(struct optimsoc_syscall));
    assert(c);
    _optimsoc_syscall_data = (struct optimsoc_syscall*) c;
}

void optimsoc_syscall_handler_set(optimsoc_syscall_handler_fptr handler) {
    _optimsoc_syscall_handler = handler;
}
