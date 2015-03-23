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

#define GPR_BUF_OFFSET(x) (x << 2)

.section text
.global syscall_entry
.type syscall_entry, function

syscall_entry:
	// Load address of syscall data structures

	// Add core offset -> r3

	// Store id
	l.lwz	r4,GPR_BUF_OFFSET(3)(r1)
	l.sw	0(r3),r4
	// Store parameters
	l.lwz	r4,GPR_BUF_OFFSET(4)(r1)
	l.sw	4(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(5)(r1)
	l.sw	8(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(6)(r1)
	l.sw	12(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(7)(r1)
	l.sw	16(r3),r4
	l.lwz	r4,GPR_BUF_OFFSET(8)(r1)
	l.sw	20(r3),r4

	// Store struct pointer to callee-saved register
	l.or	r14,r3,r0
	// Store r9 to callee-saved register
	l.or	r16,r11,r0

	// Jump to gzll syscall handler
	// TODO: delay slot macro
	OR1K_DELAYED_NOP(l.jal gzll_syscall_handler)

	// Store return to register
	l.lwz	r11,24(r14)
	l.sw	GPR_BUF_OFFSET(11)(r1),r11

	// todo: delay slot macro
	OR1K_DELAYED_NOP(l.jalr r16)
