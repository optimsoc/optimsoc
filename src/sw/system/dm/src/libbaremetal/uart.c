#include "include/optimsoc-baremetal.h"
#include <optimsoc-sysconfig.h>
#include <optimsoc.h>
#include <or1k-support.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void uart_printf(const char *fmt, ...) {
    if (!_optimsoc_has_uart) {
        return;
    }

    char buffer[128];
    va_list ap;

    va_start(ap, fmt); /* Initialize the va_list */

    vsnprintf(buffer,128,fmt, ap); /* Call vprintf */

    va_end(ap); /* Cleanup the va_list */

    int size = strnlen(buffer,128);
    uint32_t msg = 0;
    set_bits(&msg, _optimsoc_uarttile, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&msg, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&msg, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    for (unsigned i=0;i<size;i++) {
        set_bits(&msg, buffer[i],7,0);
        optimsoc_mp_simple_send(1,&msg);
    }
}

int lcd_init() {
    uint32_t buffer = 0;
    set_bits(&buffer, _optimsoc_uarttile, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&buffer, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&buffer, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    set_bits(&buffer, 1, 13, 13);
    set_bits(&buffer, ' ', 7, 0);

    for (int r=0;r<=1;r++) {
        set_bits(&buffer, r, 12, 12);
        for (int c=0;c<16;c++) {
            set_bits(&buffer, c, 11, 8);
            optimsoc_mp_simple_send(1,&buffer);
        }
    }

    return 0;
}

int lcd_set(unsigned int row,unsigned int col,char c) {
    uint32_t buffer = 0;
    if (_optimsoc_has_uart && _optimsoc_uart_lcd_enable) {
        set_bits(&buffer, _optimsoc_uarttile, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
        set_bits(&buffer, 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
        set_bits(&buffer, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);

        set_bits(&buffer, 1, 13, 13);
        set_bits(&buffer, row, 12, 12);
        set_bits(&buffer, col, 11, 8);
        set_bits(&buffer, (uint32_t) c, 7, 0);

        optimsoc_mp_simple_send(1,&buffer);
    }

    return 0;
}
