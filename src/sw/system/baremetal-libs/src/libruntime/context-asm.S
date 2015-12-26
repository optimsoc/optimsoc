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
 */

#include <or1k-sprs.h>
#include <or1k-asm.h>

#define CTX_REG(x) (x << 2)

#define CTX_PC     0x80
#define CTX_SR     0x84
#define CTX_SIZE   136

.section .text

.global _optimsoc_context_replace;
.type   _optimsoc_context_replace,function;

// r3 is the context to set
_optimsoc_context_replace:
    l.lwz   r1,CTX_PC(r3)
    l.mtspr r0,r1,OR1K_SPR_SYS_EPCR_BASE

    l.lwz   r1,CTX_SR(r3)
    l.mtspr r0,r1,OR1K_SPR_SYS_ESR_BASE

    l.lwz   r31,CTX_REG(31)(r3)
    l.lwz   r30,CTX_REG(30)(r3)
    l.lwz   r29,CTX_REG(29)(r3)
    l.lwz   r28,CTX_REG(28)(r3)
    l.lwz   r27,CTX_REG(27)(r3)
    l.lwz   r26,CTX_REG(26)(r3)
    l.lwz   r25,CTX_REG(25)(r3)
    l.lwz   r24,CTX_REG(24)(r3)
    l.lwz   r23,CTX_REG(23)(r3)
    l.lwz   r22,CTX_REG(22)(r3)
    l.lwz   r21,CTX_REG(21)(r3)
    l.lwz   r20,CTX_REG(20)(r3)
    l.lwz   r19,CTX_REG(19)(r3)
    l.lwz   r18,CTX_REG(18)(r3)
    l.lwz   r17,CTX_REG(17)(r3)
    l.lwz   r16,CTX_REG(16)(r3)
    l.lwz   r15,CTX_REG(15)(r3)
    l.lwz   r14,CTX_REG(14)(r3)
    l.lwz   r13,CTX_REG(13)(r3)
    l.lwz   r12,CTX_REG(12)(r3)
    l.lwz   r11,CTX_REG(11)(r3)
    l.lwz   r10,CTX_REG(10)(r3)
    l.lwz   r9,CTX_REG(9)(r3)
    l.lwz   r8,CTX_REG(8)(r3)
    l.lwz   r7,CTX_REG(7)(r3)
    l.lwz   r6,CTX_REG(6)(r3)
    l.lwz   r5,CTX_REG(5)(r3)
    l.lwz   r4,CTX_REG(4)(r3)
    l.lwz   r2,CTX_REG(2)(r3)
    l.lwz   r1,CTX_REG(1)(r3)
    l.lwz   r3,CTX_REG(3)(r3)

    l.nop 0x24
    l.rfe
    l.nop

.global _optimsoc_context_enter_exception;
.type   _optimsoc_context_enter_exception,function;

.extern _or1k_exception_stack_core;

// r3 is the context where to save to
_optimsoc_context_enter_exception:
    l.sw    CTX_REG(1)(r3),r1
    l.sw    CTX_REG(2)(r3),r2
    l.sw    CTX_REG(3)(r3),r3
    l.sw    CTX_REG(4)(r3),r4
    l.sw    CTX_REG(5)(r3),r5
    l.sw    CTX_REG(6)(r3),r6
    l.sw    CTX_REG(7)(r3),r7
    l.sw    CTX_REG(8)(r3),r8
    l.sw    CTX_REG(9)(r3),r9
    l.sw    CTX_REG(10)(r3),r10
    // This is important: store 0 as return value
    l.sw    CTX_REG(11)(r3),r0
    l.sw    CTX_REG(12)(r3),r12
    l.sw    CTX_REG(13)(r3),r13
    l.sw    CTX_REG(14)(r3),r14
    l.sw    CTX_REG(15)(r3),r15
    l.sw    CTX_REG(16)(r3),r16
    l.sw    CTX_REG(17)(r3),r17
    l.sw    CTX_REG(18)(r3),r18
    l.sw    CTX_REG(19)(r3),r19
    l.sw    CTX_REG(20)(r3),r20
    l.sw    CTX_REG(21)(r3),r21
    l.sw    CTX_REG(22)(r3),r22
    l.sw    CTX_REG(23)(r3),r23
    l.sw    CTX_REG(24)(r3),r24
    l.sw    CTX_REG(25)(r3),r25
    l.sw    CTX_REG(26)(r3),r26
    l.sw    CTX_REG(27)(r3),r27
    l.sw    CTX_REG(28)(r3),r28
    l.sw    CTX_REG(29)(r3),r29
    l.sw    CTX_REG(30)(r3),r30
    l.sw    CTX_REG(31)(r3),r31
    l.mfspr r4,r0,OR1K_SPR_SYS_SR_ADDR
    l.sw    CTX_SR(r3),r4
    l.movhi r4,hi(._optimsoc_context_enter_exception_wakeup)
    l.ori   r4,r4,lo(._optimsoc_context_enter_exception_wakeup)
    l.sw    CTX_PC(r3),r4

    l.movhi r1,hi(_or1k_exception_stack_core)
    l.ori   r1,r1,lo(_or1k_exception_stack_core)
    l.lwz   r1,0(r1)
    l.mfspr r3,r0,OR1K_SPR_SYS_COREID_ADDR
    l.slli  r3,r3,2
    l.add   r1,r1,r3
    l.lwz   r1,0(r1)
    l.addi  r1,r1,-CTX_SIZE
    l.ori   r11,r0,1
    l.jr    r9
    l.nop
._optimsoc_context_enter_exception_wakeup:
    l.jr    r9
    l.nop
