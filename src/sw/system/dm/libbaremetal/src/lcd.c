#include <mp_simple.h>
#include <sysconfig.h>
#include <optimsoc.h>
#include <or1k-support.h>

int lcd_init() {
    unsigned int buffer = 0;
    set_bits(&buffer,optimsoc_uarttile,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
    set_bits(&buffer,0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
    set_bits(&buffer,optimsoc_get_tileid(),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
    set_bits(&buffer,1,13,13);
    set_bits(&buffer,(unsigned int) ' ',7,0);

    for (int r=0;r<=1;r++) {
        set_bits(&buffer,r,12,12);
        for (int c=0;c<16;c++) {
            set_bits(&buffer,c,11,8);
            optimsoc_mp_simple_send(1,&buffer);
        }
    }

    return 0;
}

int lcd_set(unsigned int row,unsigned int col,char c) {
    unsigned int buffer = 0;
    if (optimsoc_has_uart && optimsoc_uart_lcd_enable) {
        set_bits(&buffer,optimsoc_uarttile,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
        set_bits(&buffer,0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
        set_bits(&buffer,optimsoc_get_tileid(),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

        set_bits(&buffer,1,13,13);
        set_bits(&buffer,row,12,12);
        set_bits(&buffer,col,11,8);
        set_bits(&buffer,(unsigned int) c,7,0);

        optimsoc_mp_simple_send(1,&buffer);
    }

    return 0;
}
