#ifndef DMA_H_
#define DMA_H_

#define DMA_ERR_NOSLOT 1
#define DMA_SUCCESS    0

typedef unsigned int dma_transfer_handle_t;
typedef enum { LOCAL2REMOTE=0, REMOTE2LOCAL=1 } dma_direction_t;

void dma_init();

unsigned int dma_alloc(dma_transfer_handle_t *id);
void dma_alloc_blocking(dma_transfer_handle_t *id);

unsigned int dma_transfer(void* local,
                          unsigned int remote_tile,
                          void* remote,
                          unsigned int size,
                          dma_direction_t dir,
                          dma_transfer_handle_t id);

unsigned int dma_transfer_blocking(void* local,
                                   unsigned int remote_tile,
                                   void* remote,
                                   unsigned int size,
                                   dma_direction_t dir,
                                   dma_transfer_handle_t id);

void dma_wait(dma_transfer_handle_t);

#endif /* DMA_H_ */
