/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Driver for the simple message passing hardware.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include "int.h"
#include "utils.h"
#include <stdio.h>
#include <malloc.h>
#include <sysconfig.h>
#include <stdarg.h>

#include <mp_simple.h>
#include <optimsoc.h>

// Local buffer for the simple message passing
unsigned int* optimsoc_mp_simple_buffer;

// List of handlers for the classes
void (*cls_handlers[OPTIMSOC_CLASS_NUM])(unsigned int*,int);

void optimsoc_mp_simple_init() {
	// Register interrupt
	int_add(3,&optimsoc_mp_simple_inth,0);
	int_enable(3);

	// Reset class handler
	for (int i=0;i<OPTIMSOC_CLASS_NUM;i++) {
		cls_handlers[i] = 0;
	}

	// Allocate buffer
	optimsoc_mp_simple_buffer = malloc(optimsoc_noc_maxpacketsize*4);
}

void optimsoc_mp_simple_addhandler(unsigned int class,
		void (*hnd)(unsigned int*,int)) {
	cls_handlers[class] = hnd;
}

void optimsoc_mp_simple_inth(void* arg) {
	// Store message in buffer
	// Get size
	int size = REG32(OPTIMSOC_MPSIMPLE_RECV);

	// Abort and drop if message cannot be stored
	if (optimsoc_noc_maxpacketsize<size) {
		printf("FATAL: not sufficent buffer space. Drop packet\n");
		for (int i=0;i<size;i++) {
			REG32(OPTIMSOC_MPSIMPLE_RECV);
		}
	} else {
		for (int i=0;i<size;i++) {
			optimsoc_mp_simple_buffer[i] = REG32(OPTIMSOC_MPSIMPLE_RECV);
		}
	}

	// Extract class
	int class = (optimsoc_mp_simple_buffer[0] >> OPTIMSOC_CLASS_LSB) // Shift to position
    		& ((1<<(OPTIMSOC_CLASS_MSB-OPTIMSOC_CLASS_LSB+1))-1); // and mask other remain

	// Call respective class handler
	if (cls_handlers[class] == 0) {
		// No handler registered, packet gets lost
		//printf("Packet of unknown class (%d) received. Drop.\n",class);
		return;
	}

	cls_handlers[class](optimsoc_mp_simple_buffer,size);
}

void optimsoc_mp_simple_send(unsigned int size,unsigned int *buf) {
	REG32(OPTIMSOC_MPSIMPLE_SEND) = size;
	for (int i=0;i<size;i++) {
		REG32(OPTIMSOC_MPSIMPLE_SEND) = buf[i];
	}
}

void optimsoc_send_alive_message() {
	if (optimsoc_has_hostlink) {
		unsigned int buffer;
		buffer = (optimsoc_hostlink << OPTIMSOC_DEST_LSB) |
				(1 << OPTIMSOC_CLASS_LSB) |
				(optimsoc_get_tileid() << OPTIMSOC_SRC_LSB);
	}
}

void uart_printf(const char *fmt, ...) {
    if (!optimsoc_has_uart) {
        return;
    }

    char buffer[128];
    va_list ap;

    va_start(ap, fmt); /* Initialize the va_list */

    vsnprintf(buffer,128,fmt, ap); /* Call vprintf */

    va_end(ap); /* Cleanup the va_list */

    int size = strnlen(buffer,128);
    unsigned int msg = 0;
    set_bits(&msg,optimsoc_uarttile,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
    set_bits(&msg,0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
    set_bits(&msg,optimsoc_get_tileid(),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
    for (unsigned i=0;i<size;i++) {
        set_bits(&msg,(unsigned int) buffer[i],7,0);
        optimsoc_mp_simple_send(1,&msg);
    }
}

