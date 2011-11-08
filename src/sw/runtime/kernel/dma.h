#ifndef __DMA_H__
#define __DMA_H__

#include "thread.h"

typedef unsigned int dma_transfer_id;

void svc_dma_handler(unsigned int req,arch_thread_ctx_t *ctx);
void dma_interrupt_handler();

void dma_init();

#endif
