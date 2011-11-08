/*
 * cas.c
 *
 *  Created on: Dec 2, 2010
 *      Author: wallento
 */

#include "spr-defs.h"
#include "utils.h"

unsigned int cas(unsigned int address,unsigned int compare,unsigned int value) {
/*	unsigned int old;

	do {
		old = (unsigned int) arch_load_linked((void*)address);

		if (old == compare) {
			if ( (unsigned int) arch_store_conditional((void*)address,(void*)value) == 0 ) {
				return value;
			}
		} else {
			arch_store_conditional((void*)address,(void*)old);
			return old;
		}
	} while (1);*/


// disable interrupts
	unsigned int result;
	mtspr(SPR_SR,mfspr(SPR_SR) & ~SPR_SR_DCE);
	REG32(0x7ffffffc) = address;
	REG32(0x7ffffffc) = compare;
	REG32(0x7ffffffc) = value;
	result = REG32(0x7ffffffc);
	mtspr(SPR_SR,mfspr(SPR_SR) | SPR_SR_DCE);
	return result;
// enable interrupts
}
