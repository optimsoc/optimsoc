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

#include <or1k-sprs.h>

.section .text

.global ctx_replace;
.type   ctx_replace,function;
.align;

ctx_replace:
    l.lwz   r1,0x04(r0)
    l.mtspr r0,r1,OR1K_SPR_SYS_EPCR_BASE;
    l.lwz   r1,0x84(r0)
    l.mtspr r0,r1,OR1K_SPR_SYS_ESR_BASE;
    l.lwz   r1,0x08(r0)
    l.lwz   r2,0x0c(r0)
    l.lwz   r3,0x10(r0)
    l.lwz   r4,0x14(r0)
    l.lwz   r5,0x18(r0)
    l.lwz   r6,0x1c(r0)
    l.lwz   r7,0x20(r0)
    l.lwz   r8,0x24(r0)
    l.lwz   r9,0x28(r0)
    l.lwz   r10,0x2c(r0)
    l.lwz   r11,0x30(r0)
    l.lwz   r12,0x34(r0)
    l.lwz   r13,0x38(r0)
    l.lwz   r14,0x3c(r0)
    l.lwz   r15,0x40(r0)
    l.lwz   r16,0x44(r0)
    l.lwz   r17,0x48(r0)
    l.lwz   r18,0x4c(r0)
    l.lwz   r19,0x50(r0)
    l.lwz   r20,0x54(r0)
    l.lwz   r21,0x58(r0)
    l.lwz   r22,0x5c(r0)
    l.lwz   r23,0x60(r0)
    l.lwz   r24,0x64(r0)
    l.lwz   r25,0x68(r0)
    l.lwz   r26,0x6c(r0)
    l.lwz   r27,0x70(r0)
    l.lwz   r28,0x74(r0)
    l.lwz   r29,0x78(r0)
    l.lwz   r30,0x7c(r0)
    l.lwz   r31,0x80(r0)
    l.rfe
    l.nop 0x24

.fend_ctx_replace:
.size  ctx_replace,.fend_ctx_replace-ctx_replace;
