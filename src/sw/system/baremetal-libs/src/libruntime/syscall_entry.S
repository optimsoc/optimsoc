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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <or1k-asm.h>
#include <or1k-sprs.h>

#define GPR_BUF_OFFSET(x) (x << 2)

.extern _optimsoc_syscall_data
.extern _optimsoc_syscall_handler

.section .text
.global _optimsoc_syscall_entry
.type _optimsoc_syscall_entry, function

_optimsoc_syscall_entry:
    // Load address of syscall data structures
    l.movhi r3,hi(_optimsoc_syscall_data)
    l.ori   r3,r3,lo(_optimsoc_syscall_data)
    // De-reference pointer
    l.lwz   r3,0(r3)
    // Add core offset
    l.mfspr r4,r0,OR1K_SPR_SYS_COREID_ADDR
    l.slli  r4,r4,5
    l.add   r3,r3,r4
    // Store id
    l.lwz   r4,GPR_BUF_OFFSET(11)(r1)
    l.sw    0(r3),r4
    // Store parameters
    l.lwz   r4,GPR_BUF_OFFSET(3)(r1)
    l.sw    8(r3),r4
    l.lwz   r4,GPR_BUF_OFFSET(4)(r1)
    l.sw    12(r3),r4
    l.lwz   r4,GPR_BUF_OFFSET(5)(r1)
    l.sw    16(r3),r4
    l.lwz   r4,GPR_BUF_OFFSET(6)(r1)
    l.sw    20(r3),r4
    l.lwz   r4,GPR_BUF_OFFSET(7)(r1)
    l.sw    24(r3),r4
    l.lwz   r4,GPR_BUF_OFFSET(8)(r1)
    l.sw    28(r3),r4

    // Store struct pointer to callee-saved register
    l.or    r14,r3,r0
    // Store r9 to callee-saved register
    l.or    r16,r9,r0

    // Load syscall handler address
    l.movhi r4,hi(_optimsoc_syscall_handler)
    l.ori   r4,r4,lo(_optimsoc_syscall_handler)

    // Load syscall handler
    l.lwz   r4,0(r4)

    // Check that syscall handler is set
    l.sfeqi r4,0
    OR1K_DELAYED_NOP(l.bf .no_entry_set)

    // Jump to registered syscall handler
    OR1K_DELAYED_NOP(l.jalr r4)

    // Store return to register
    l.lwz   r11,4(r14)
    l.sw    GPR_BUF_OFFSET(11)(r1),r11

    // Return to exception handler
    OR1K_DELAYED_NOP(l.jr r16)

.no_entry_set:
    OR1K_DELAYED_NOP(l.j exit)
