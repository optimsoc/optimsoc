/*
 * runtime.h
 *
 *  Created on: Nov 19, 2012
 *      Author: gu45zin
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

#include <context.h>

extern arch_thread_ctx_t *exception_ctx;

extern void ctx_replace();
extern void yield_switchctx(struct arch_thread_ctx_t *ctx);

#endif /* RUNTIME_H_ */
