#include <optimsoc.h>
#include <stdlib.h>

#define START 1*1024*1024
#define END   128*1024*1024

void main() {
    unsigned int x = 0xdeadbeef;

    OPTIMSOC_TRACE(0x1500,0);

    for (unsigned int a = START; a < END; a = a+4) {
        REG32(a) = x + a;
        if (a % 0x10000 == 0) {
            OPTIMSOC_TRACE(0x1501,a);
        }
    }

    OPTIMSOC_TRACE(0x1500,1);

    unsigned int errors = 0;

    for (unsigned int a = START; a < END; a = a+4) {
        unsigned int y = REG32(a);
        if (y != x + a) {
            errors++;
            OPTIMSOC_TRACE(0x1502,a);
            OPTIMSOC_TRACE(0x1503,x+a);
            OPTIMSOC_TRACE(0x1504,y);
        }

        if (a % 0x10000 == 0) {
            OPTIMSOC_TRACE(0x1501,a);
        }
    }

    OPTIMSOC_TRACE(0x1505,errors);
}
