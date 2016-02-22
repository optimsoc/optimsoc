#include "context.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
    printf("context %p:\n", ctx);
    printf("  r0:  0x%08x r1:  0x%08x r2:  0x%08x r3:  0x%08x\n", ctx->regs[0],
           ctx->regs[1], ctx->regs[2], ctx->regs[3]);
    printf("  r4:  0x%08x r5:  0x%08x r6:  0x%08x r7:  0x%08x\n", ctx->regs[4],
           ctx->regs[5], ctx->regs[6], ctx->regs[7]);
    printf("  r8:  0x%08x r9:  0x%08x r10: 0x%08x r11: 0x%08x\n", ctx->regs[8],
           ctx->regs[9], ctx->regs[10], ctx->regs[11]);
    printf("  r12: 0x%08x r13: 0x%08x r14: 0x%08x r15: 0x%08x\n", ctx->regs[12],
           ctx->regs[13], ctx->regs[14], ctx->regs[15]);
    printf("  r16: 0x%08x r17: 0x%08x r18: 0x%08x r19: 0x%08x\n", ctx->regs[16],
           ctx->regs[17], ctx->regs[18], ctx->regs[19]);
    printf("  r20: 0x%08x r21: 0x%08x r22: 0x%08x r23: 0x%08x\n", ctx->regs[20],
           ctx->regs[21], ctx->regs[22], ctx->regs[23]);
    printf("  r24: 0x%08x r25: 0x%08x r26: 0x%08x r27: 0x%08x\n", ctx->regs[24],
           ctx->regs[25], ctx->regs[26], ctx->regs[27]);
    printf("  r28: 0x%08x r29: 0x%08x r30: 0x%08x r31: 0x%08x\n", ctx->regs[28],
           ctx->regs[29], ctx->regs[30], ctx->regs[31]);
    printf("  PC: 0x%08x SR: 0x%08x\n", ctx->pc, ctx->sr);
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
