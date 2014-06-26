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
 */

#include <spr-defs.h>
#include <or1k-asm.h>

#define PC     0x0
#define GPR(x) (x<<2)
#define SR     0x80

.section .text

.global yield_switchctx;
.type   yield_switchctx,function;
.align;

yield_switchctx:
    /* This function is entered by a thread to switch to kernel
     * and yield the remaining time.  We need to store all registers
     * that are not saved elsewhere on the stack as the stack is
     * replaced. The return address in the link register is preserved
     * and finally the program counter set so that the context points
     * to the end of this function where it then return back to the
     * calling function. */

    /* Register 3 contains a pointer to our context */
    /* First we store all callee-saved registers in their position
     * in the context. */

    l.sw GPR(1)(r3),r1
    l.sw GPR(2)(r3),r2
    l.sw GPR(9)(r3),r9
    l.sw GPR(10)(r3),r10
    l.sw GPR(14)(r3),r14
    l.sw GPR(16)(r3),r16
    l.sw GPR(18)(r3),r18
    l.sw GPR(20)(r3),r20
    l.sw GPR(22)(r3),r22
    l.sw GPR(24)(r3),r24
    l.sw GPR(26)(r3),r26
    l.sw GPR(28)(r3),r28
    l.sw GPR(30)(r3),r30

    /* From now on we can do everything with the registers.. */

    /* Save status register */
    l.mfspr r4,r0,SPR_SR
    l.sw    SR(r3),r4
    // disable interrupts from here on
    l.andi  r4,r4,0xFFFB
    l.mtspr r0,r4,SPR_SR

    /* Store a program counter that will bring us back down there
     * when scheduled again. The context switch mechanism will bring us
     * there then. */
    l.movhi r4,hi(yield_return)
    l.ori   r4,r4,lo(yield_return)
    l.sw    PC(r3),r4

    /* Now we give the kernel his exception stack so that it works
     * similar as after a tick. */
    /* FIXME really necessary ? */
    .global exception_stack

    l.movhi r1, hi(exception_stack);
    l.ori   r1, r1, lo(exception_stack);
    l.lwz   r1, 0(r1)
    /* Generate the domain core id. */
    l.movhi r3, 0xe000
    l.lwz   r3, 0x10(r3)
    l.mfspr r4, r0, SPR_COREID;
    l.add   r4, r4, r3
    /* Multiply by pointer size (4) */
    l.slli  r4, r4, 2
    /* Add to base pointer */
    l.add   r1, r1, r4
    /* Load pointer to the stack from there. */
    l.lwz   r1, 0(r1)

    .global schedule
    l.jal   schedule
    l.nop
    l.j     ctx_replace
    l.nop

yield_return:

    /* And go back to the place we were called */
    l.jr r9
    l.nop

.fend_yield_switchctx:
.size  yield_switchctx,.fend_yield_switchctx-yield_switchctx;
