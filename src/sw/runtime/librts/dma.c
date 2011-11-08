#include "dma.h"
#include "malloc.h"
#include "thread.h"

#include "services.h"
#include "stdio.h"

#define DMA_TRANSFER 0
#define DMA_POLL     1
#define DMA_WAIT     2

#ifndef EVENT_POLL_KERNEL
#ifndef EVENT_POLL_USER
#ifndef EVENT_IRQ
#ifndef EVENT_DQM
#error No event scheme set
#endif
#endif
#endif
#endif

struct dma_state {
	unsigned short initialized;
	unsigned int svc_id;
} dma_state = {0,0};

inline void dma_check_initialized() {
	if (__builtin_expect(dma_state.initialized,1)==0) {
		dma_state.svc_id = svc_identify("dma");
	}
}

#ifdef EVENT_POLL_KERNEL
void dma_wait(dma_handler* dma) {
	while ((unsigned int) svc_req1(dma_state.svc_id,DMA_POLL,dma->address) == 1) {
		thread_yield();
	}
}
#endif

#ifdef EVENT_POLL_USER
void dma_wait(dma_handler* dma) {
	while (*((volatile unsigned int*) dma->address) == 1) {
		thread_yield();
	}
}
#endif

#ifdef EVENT_IRQ
void dma_wait(dma_handler* dma) {
	svc_req1(dma_state.svc_id,DMA_WAIT,(void*)dma->address);
}
#endif

dma_handler *dma_transfer(void* local,void* remote,unsigned int size) {
	dma_check_initialized();
	dma_handler *dma = malloc(sizeof(dma_handler));
	dma_check_initialized();
	dma->address = (void*) svc_req3(dma_state.svc_id,DMA_TRANSFER,local,remote,(void*)size);
	return dma;
}
