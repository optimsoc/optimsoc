#ifndef __DMA_H__
#define __DMA_H__

typedef struct {
	volatile void* address;
} dma_handler;

dma_handler* dma_transfer(void* local,void* remote,unsigned int size);
void dma_wait(dma_handler* dma);

#endif
