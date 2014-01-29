/* Copyright (c) 2013 by the author(s)
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
 * Driver for the compare and swap functionality.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <spr-defs.h>
#include <or1k-support.h>
#include <stdint.h>

extern uint32_t optimsoc_critical_begin(void);
extern void optimsoc_critical_end(uint32_t);

/* TODO distributed memory - missing cas unit */

uint32_t cas(void* address, uint32_t compare, uint32_t value) {
    uint32_t restore, ack;

    restore = optimsoc_critical_begin();

    unsigned int result;

    REG32(0xe030000C) = (uint32_t) address;
    REG32(0xe0300010) = compare;
    REG32(0xe0300014) = value;

    do {
	ack = REG32(0xe0300008);
    } while(ack == 0x00);
    result = REG32(0xe0300018);

    optimsoc_critical_end(restore);

    return result;
}
