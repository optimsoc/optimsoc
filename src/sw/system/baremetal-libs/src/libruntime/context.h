#ifndef __CONTEXT_H__
#define __CONTEXT_H__

typedef struct arch_thread_ctx_t {
    unsigned int    pc;
    unsigned int    regs[31];
    unsigned int    sr;
} arch_thread_ctx_t;

#endif
