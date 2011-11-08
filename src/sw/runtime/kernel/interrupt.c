/*
 * interrupt.c
 *
 *  Created on: Aug 11, 2011
 *      Author: wallento
 */

#include "dma.h"
#include ARCH_INCL(utils.h)
#include ARCH_INCL(spr-defs.h)

void (*interrupt_handlers[]) (void ) = {
		dma_interrupt_handler
};


void interrupt() {
	unsigned long picsr = mfspr(SPR_PICSR);
	unsigned long i = 0;

	mtspr(SPR_PICSR, 0);

	while(i < 32) {
		if((picsr & (0x01L << i)) && (interrupt_handlers[i] != 0)) {
			(*interrupt_handlers[i])();
			mtspr(SPR_PICSR, mfspr(SPR_PICSR) & ~(0x00000001L << i));
		}
		i++;
	}
}
