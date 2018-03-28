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
 * This file is inspired by the syscall handling in the musl-libc port
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

static inline uint32_t syscall(uint32_t id, uint32_t p0, uint32_t p1,
        uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5)
{
    register uint32_t r11 __asm__("r11") = id;
    register uint32_t r3 __asm__("r3") = p0;
    register uint32_t r4 __asm__("r4") = p1;
    register uint32_t r5 __asm__("r5") = p2;
    register uint32_t r6 __asm__("r6") = p3;
    register uint32_t r7 __asm__("r7") = p4;
    register uint32_t r8 __asm__("r8") = p5;

    __asm__ __volatile__ ("l.sys 0" : "=r"(r11) : "r"(r11), "r"(r3), "r"(r4),
            "r"(r5), "r"(r6), "r"(r7), "r"(r8) : "memory");

    return r11;
}

#endif
