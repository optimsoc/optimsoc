/*
 * mm.c
 *
 *  Created on: Nov 27, 2010
 *      Author: wallento
 */

#include "mm.h"

#include "utils.h"
#include "spr-defs.h"
#include "board.h"

void arch_set_itlb(void *vaddr,void *paddr) {
	unsigned int flags = ITLB_PR_NOLIMIT;
	unsigned int mr = ((unsigned int)vaddr & SPR_ITLBMR_VPN) | SPR_ITLBMR_V;
	unsigned int tr = ((unsigned int)paddr & SPR_ITLBTR_PPN) | (flags & ITLB_PR_MASK) | SPR_ITLBTR_A | SPR_ITLBTR_D;
	unsigned int set = ((((unsigned int)vaddr) >> PAGE_BITS) & 63);
	mtspr (SPR_ITLBMR_BASE(0)+set, mr);
	mtspr (SPR_ITLBTR_BASE(0)+set, tr);
}

void arch_set_dtlb(void *vaddr,void *paddr) {
	unsigned int flags = ITLB_PR_NOLIMIT;
	unsigned int mr = ((unsigned int)vaddr & SPR_DTLBMR_VPN) | SPR_ITLBMR_V;
	unsigned int tr = ((unsigned int)paddr & SPR_DTLBTR_PPN) | (flags & DTLB_PR_MASK) | SPR_DTLBTR_A | SPR_DTLBTR_D;
	unsigned int set = ((((unsigned int)vaddr) >> PAGE_BITS) & 63);
	mtspr (SPR_DTLBMR_BASE(0)+set, mr);
	mtspr (SPR_DTLBTR_BASE(0)+set, tr);
}

void arch_itlb_invalidate() {
	for ( unsigned int i = 0; i < 64; ++i ) {
		mtspr(SPR_ITLBMR_BASE(0)+i,0);
	}
}

void arch_dtlb_invalidate() {
	for ( unsigned int i = 0; i < 64; ++i ) {
		mtspr(SPR_DTLBMR_BASE(0)+i,0);
	}
}
