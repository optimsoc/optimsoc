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
