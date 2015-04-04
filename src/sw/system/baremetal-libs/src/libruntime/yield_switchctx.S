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
 *   Stefan Rösch <stefan.roesch@tum.de>
 */

#include <or1k-sprs.h>
#include <or1k-asm.h>

#define CTX_REG(x) (x << 2)

#define CTX_PC_BYTE_OFFSET  0x7c
#define CTX_SR_BYTE_OFFSET  0x80

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

    l.sw CTX_REG(1)(r3),r1
    l.sw CTX_REG(2)(r3),r2
    l.sw CTX_REG(9)(r3),r9
    l.sw CTX_REG(10)(r3),r10
    l.sw CTX_REG(14)(r3),r14
    l.sw CTX_REG(16)(r3),r16
    l.sw CTX_REG(18)(r3),r18
    l.sw CTX_REG(20)(r3),r20
    l.sw CTX_REG(22)(r3),r22
    l.sw CTX_REG(24)(r3),r24
    l.sw CTX_REG(26)(r3),r26
    l.sw CTX_REG(28)(r3),r28
    l.sw CTX_REG(30)(r3),r30

    /* From now on we can do everything with the registers.. */

    /* Save status register */
    l.mfspr r4,r0,OR1K_SPR_SYS_SR_ADDR
    l.sw    CTX_SR_BYTE_OFFSET(r3),r4
    // disable interrupts from here on
    l.andi  r4,r4,0xFFFB
    l.mtspr r0,r4,OR1K_SPR_SYS_SR_ADDR

    /* Store a program counter that will bring us back down there
     * when scheduled again. The context switch mechanism will bring us
     * there then. */
    l.movhi r4,hi(yield_return)
    l.ori   r4,r4,lo(yield_return)
    l.sw    CTX_PC_BYTE_OFFSET(r3),r4

    /* Use the stack of the current thread
     * Next context switch will re-enable stack-pointer saved above
     */

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
