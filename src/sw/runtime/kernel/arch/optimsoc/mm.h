/*
 * mm.h
 *
 *  Created on: Nov 27, 2010
 *      Author: wallento
 */

#ifndef MM_H_
#define MM_H_

void arch_set_itlb(void *vaddr,void *paddr);
void arch_set_dtlb(void *vaddr,void *paddr);

void arch_itlb_invalidate();
void arch_dtlb_invalidate();

#endif /* MM_H_ */
