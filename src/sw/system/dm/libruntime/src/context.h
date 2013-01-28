#ifndef CONTEXT_H_
#define CONTEXT_H_

#define NUMCORES 1

typedef struct arch_thread_ctx_t {
	unsigned int	pc;
	unsigned int	regs[31];
	unsigned int	sr;
} arch_thread_ctx_t;

#endif /* CONTEXT_H_ */
