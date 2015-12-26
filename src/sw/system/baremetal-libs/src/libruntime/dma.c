#include "include/optimsoc-runtime.h"
#include "optimsoc-baremetal.h"

void optimsoc_dma_transfer(void *local, uint32_t remote_tile, void *remote,
                           size_t size, dma_direction_t dir)
{
    dma_transfer_handle_t dma_handle;

    /* allocate transfer handle */
    while(dma_alloc(&dma_handle) != DMA_SUCCESS) {
        optimsoc_thread_yield(optimsoc_thread_current());
    }

    assert(size % 4 == 0);
    dma_transfer(local, remote_tile, remote, size/4, dir, dma_handle);

    dma_wait(dma_handle);

    dma_free(dma_handle);
}
