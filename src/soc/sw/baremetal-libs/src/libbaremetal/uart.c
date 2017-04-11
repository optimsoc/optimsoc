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

#include "include/optimsoc-baremetal.h"
#include <or1k-support.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void uart_printf(const char *fmt, ...) {
    if (!optimsoc_has_uart()) {
        return;
    }

    char buffer[128];
    va_list ap;

    va_start(ap, fmt); /* Initialize the va_list */

    vsnprintf(buffer,128,fmt, ap); /* Call vprintf */

    va_end(ap); /* Cleanup the va_list */

    int size = strnlen(buffer,128);
    uint32_t msg = 0;
    set_bits(&msg, optimsoc_uarttile(), OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&msg, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&msg, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    for (unsigned i=0;i<size;i++) {
        set_bits(&msg, buffer[i],7,0);
        optimsoc_mp_simple_send(0,1,&msg);
    }
}

int lcd_init() {
    uint32_t buffer = 0;
    set_bits(&buffer, optimsoc_uarttile(), OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&buffer, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&buffer, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    set_bits(&buffer, 1, 13, 13);
    set_bits(&buffer, ' ', 7, 0);

    for (int r=0;r<=1;r++) {
        set_bits(&buffer, r, 12, 12);
        for (int c=0;c<16;c++) {
            set_bits(&buffer, c, 11, 8);
            optimsoc_mp_simple_send(0,1,&buffer);
        }
    }

    return 0;
}

int lcd_set(unsigned int row,unsigned int col,char c) {
    uint32_t buffer = 0;
    if (optimsoc_has_uart() && optimsoc_uart_lcd_enable()) {
        set_bits(&buffer, optimsoc_uarttile(), OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
        set_bits(&buffer, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
        set_bits(&buffer, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);

        set_bits(&buffer, 1, 13, 13);
        set_bits(&buffer, row, 12, 12);
        set_bits(&buffer, col, 11, 8);
        set_bits(&buffer, (uint32_t) c, 7, 0);

        optimsoc_mp_simple_send(0,1,&buffer);
    }

    return 0;
}
