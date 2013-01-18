/*
 * macros.h
 *
 *  Created on: Nov 21, 2010
 *      Author: wallento
 */

#ifndef MACROS_H_
#define MACROS_H_

#define CLEAR_GPR(gpr)		\
		l.or    gpr, r0, r0

#define ENTRY(symbol)		\
		.global symbol ;	\
		symbol:

#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
		.global symbol ;               \
		l.movhi gpr, hi(symbol) ;      \
		l.ori   gpr, gpr, lo(symbol)

#endif /* MACROS_H_ */
