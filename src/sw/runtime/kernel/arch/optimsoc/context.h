/*
 * context.h
 *
 *  Created on: Nov 20, 2010
 *      Author: wallento
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "board.h"

typedef struct arch_thread_ctx_t {
	unsigned int 	pc;
	unsigned int	regs[31];
	unsigned int	sr;
} arch_thread_ctx_t;

typedef struct arch_process_ctx_t {

} arch_process_ctx_t;

void arch_context_switch(arch_thread_ctx_t *old,arch_thread_ctx_t *new);
// Only supports 6 parameters at the moment!
unsigned int arch_context_extract_parameter(arch_thread_ctx_t *ctx,unsigned int n);
void arch_context_set_return(arch_thread_ctx_t *ctx,unsigned int val);

extern arch_thread_ctx_t exception_ctx[NUMCORES];

#endif /* CONTEXT_H_ */
