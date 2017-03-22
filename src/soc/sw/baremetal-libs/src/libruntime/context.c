#include "context.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <or1k-support.h>

_optimsoc_thread_ctx_t **_optimsoc_exception_ctx;

extern void **_or1k_exception_stack_core;
#define OR1K_EXCEPTION_FRAME 136

void _optimsoc_context_init(void) {
    _optimsoc_exception_ctx = calloc(or1k_numcores(), 4);

    for (int c = 0; c < or1k_numcores(); c++) {
        void *stack = _or1k_exception_stack_core[c];
        _optimsoc_exception_ctx[c] = stack - OR1K_EXCEPTION_FRAME;
    }
}

void _optimsoc_context_print(_optimsoc_thread_ctx_t *ctx) {
    printf("context %p:\n", (void *) ctx);
    printf("  r0:  0x%"PRIx32" r1:  0x%"PRIx32" r2:  0x%"PRIx32" r3:  0x%"PRIx32"\n",  ctx->regs[0],
            ctx->regs[1],  ctx->regs[2],  ctx->regs[3]);
    printf("  r4:  0x%"PRIx32" r5:  0x%"PRIx32" r6:  0x%"PRIx32" r7:  0x%"PRIx32"\n",  ctx->regs[4],
            ctx->regs[5],  ctx->regs[6],  ctx->regs[7]);
    printf("  r8:  0x%"PRIx32" r9:  0x%"PRIx32" r10: 0x%"PRIx32" r11: 0x%"PRIx32"\n",  ctx->regs[8],
            ctx->regs[9],  ctx->regs[10],  ctx->regs[11]);
    printf("  r12: 0x%"PRIx32" r13: 0x%"PRIx32" r14: 0x%"PRIx32" r15: 0x%"PRIx32"\n",  ctx->regs[12],
            ctx->regs[13],  ctx->regs[14],  ctx->regs[15]);
    printf("  r16: 0x%"PRIx32" r17: 0x%"PRIx32" r18: 0x%"PRIx32" r19: 0x%"PRIx32"\n",  ctx->regs[16],
            ctx->regs[17],  ctx->regs[18],  ctx->regs[19]);
    printf("  r20: 0x%"PRIx32" r21: 0x%"PRIx32" r22: 0x%"PRIx32" r23: 0x%"PRIx32"\n",  ctx->regs[20],
            ctx->regs[21],  ctx->regs[22],  ctx->regs[23]);
    printf("  r24: 0x%"PRIx32" r25: 0x%"PRIx32" r26: 0x%"PRIx32" r27: 0x%"PRIx32"\n",  ctx->regs[24],
            ctx->regs[25],  ctx->regs[26],  ctx->regs[27]);
    printf("  r28: 0x%"PRIx32" r29: 0x%"PRIx32" r30: 0x%"PRIx32" r31: 0x%"PRIx32"\n",  ctx->regs[28],
            ctx->regs[29],  ctx->regs[30],  ctx->regs[31]);
    printf("  PC: 0x%"PRIx32" SR: 0x%"PRIx32"\n",  ctx->pc,  ctx->sr);
}

void _optimsoc_context_copy(_optimsoc_thread_ctx_t *to,
                            _optimsoc_thread_ctx_t *from) {
    size_t size = sizeof(struct _optimsoc_thread_ctx_t);

    memcpy((void*) to, (void*) from, size);
}

void _optimsoc_context_restore(_optimsoc_thread_ctx_t *ctx) {
    _optimsoc_thread_ctx_t *c;
    c = _optimsoc_exception_ctx[or1k_coreid()];

    _optimsoc_context_copy(c, ctx);
}

void _optimsoc_context_save(_optimsoc_thread_ctx_t *ctx) {
    _optimsoc_thread_ctx_t *c;
    c = _optimsoc_exception_ctx[or1k_coreid()];

    _optimsoc_context_copy(ctx, c);
}
