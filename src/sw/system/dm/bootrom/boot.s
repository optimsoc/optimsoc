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

#define DMA_BASE 0xe0001000
#define MEMORY_TILE 2

        /* Initialize memory */
        l.movhi r1,0xe020
        l.ori   r1,r1,0x1000

        /* Setup first page DMA */
        l.addi  r2,r0,0x0
        l.sw    0(r1),r2                /* local address */
        l.addi  r3,r0,0x800
        l.sw    4(r1),r3                /* size in words */
        l.addi  r4,r0,1
        l.sw    8(r1),r4                /* remote tile */
        l.sw    12(r1),r2               /* remote address */
        l.addi  r5,r0,1
        l.sw    16(r1),r5               /* R2L */
        l.sw    20(r1),r5               /* go! */

        /* Wait for first page DMA */
loop1:  l.lwz   r7,20(r1)
        l.sfeqi r7,1
        l.bnf   loop1
        l.nop

        /* Reset entry point */
        l.addi  r1,r0,0x100
        l.jr    r1
        l.nop
