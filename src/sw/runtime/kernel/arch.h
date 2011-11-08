/*
 * arch.h
 *
 *  Created on: Nov 27, 2010
 *      Author: wallento
 */

#ifndef ARCH_H_
#define ARCH_H_

#define ARCH optimsoc

#define ARCH_INCL(file) <arch/ARCH/file>

#include ARCH_INCL(board.h)

#endif /* ARCH_H_ */
