/* Copyright (c) 2016 by the author(s)
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
 * ============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#include "cli.h"

#include <assert.h>

int write_configreg(struct osd_context *ctx) {
    int success = 1;
    printf("CONFREG TEST\n");

    osd_reg_write16(ctx, 5, 0x200, 0x0001);		// System On/Off

    osd_reg_write16(ctx, 5, 0x201, 0x2540);		// First PC Event LSB
    osd_reg_write16(ctx, 5, 0x202, 0x0000);		// First PC Event MSB
    osd_reg_write16(ctx, 5, 0x203, 0x8001);		// First PC Event valid and Event ID 1

    osd_reg_write16(ctx, 5, 0x204, 0x0000);		// Second PC Event LSB
    osd_reg_write16(ctx, 5, 0x205, 0x0000);		// Second PC Event MSB
    osd_reg_write16(ctx, 5, 0x206, 0x8002);		// Second PC Event valid and Event ID 2

    osd_reg_write16(ctx, 5, 0x207, 0x0000);		// Third PC Event LSB
    osd_reg_write16(ctx, 5, 0x208, 0x0000);		// Third PC Event MSB
    osd_reg_write16(ctx, 5, 0x209, 0x8003);		// Third PC Event valid and Event ID 3

    osd_reg_write16(ctx, 5, 0x20a, 0x2540);		// First Function Return Event LSB
    osd_reg_write16(ctx, 5, 0x20b, 0x0000);		// First Function Return Event MSB
    osd_reg_write16(ctx, 5, 0x20c, 0x8004);		// First Function Return Event valid and Event ID 4

    osd_reg_write16(ctx, 5, 0x20d, 0x0000);		// Second Function Return Event LSB
    osd_reg_write16(ctx, 5, 0x20e, 0x0000);		// Second Function Return Event MSB
    osd_reg_write16(ctx, 5, 0x20f, 0x80c5);		// Second Function Return Event valid and Event ID 5
 
    osd_reg_write16(ctx, 5, 0x210, 0x0000);		// First Memaddr Monitor Event LSB
    osd_reg_write16(ctx, 5, 0x211, 0x0000);		// First Memaddr Monitor Event MSB
    osd_reg_write16(ctx, 5, 0x212, 0x8006);		// First Memaddr Monitor Event valid and Event ID 6

    osd_reg_write16(ctx, 5, 0x213, 0x03f8);		// Configuration GPR selection vector LSB		// R3, R4, R5, R6, R7, R8, R9 (arg values 1-6 and function return address)
    osd_reg_write16(ctx, 5, 0x214, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x215, 0x80c1);		// Configuration First Event valid and Event ID 1	// stack values 1, 2, 3 (arg values 7-9)

    osd_reg_write16(ctx, 5, 0x216, 0x0000);		// Configuration GPR selection vector LSB
    osd_reg_write16(ctx, 5, 0x217, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x218, 0x8002);		// Configuration Second Event valid and Event ID 2

    osd_reg_write16(ctx, 5, 0x219, 0x0000);		// Configuration GPR selection vector LSB
    osd_reg_write16(ctx, 5, 0x21a, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x21b, 0x8003);		// Configuration Third Event valid and Event ID 3

    osd_reg_write16(ctx, 5, 0x21c, 0x0800);		// Configuration GPR selection vector LSB		// R11 (return value)
    osd_reg_write16(ctx, 5, 0x21d, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x21e, 0x8004);		// Configuration Fourth Event valid and Event ID 4

    osd_reg_write16(ctx, 5, 0x21f, 0x0000);		// Configuration GPR selection vector LSB
    osd_reg_write16(ctx, 5, 0x220, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x221, 0x8005);		// Configuration Fifth Event valid and Event ID 5

    osd_reg_write16(ctx, 5, 0x222, 0x0000);		// Configuration GPR selection vector LSB
    osd_reg_write16(ctx, 5, 0x223, 0x0000);		// Configuration GPR selection vector MSB
    osd_reg_write16(ctx, 5, 0x224, 0x8006);		// Configuration Fifth Event valid and Event ID 6

    return success;
}