/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =================================================================
 *
 * DMA driver. Work-in-progress.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <or1k-support.h>
#include <stdlib.h>

#include "include/optimsoc-baremetal.h"

#define DMA_SLOTS 4
// This must be changed to 2000 when conf part is removed from dma
#define DMA_BASE 0xe0200000

#define DMA_SLOT_FREE 0
#define DMA_SLOT_ALLOC 1
#define DMA_SLOT_WAIT 2


uint8_t _optimsoc_dma_initialized = 0;

struct dma_slot {
    unsigned int flag;
    unsigned int id;
};

struct dma_slot *dma_slots[DMA_SLOTS];

struct dma_slot *dma_alloc_slot() {
    int i;
    for (i=0;i<DMA_SLOTS;i++) {
        if (dma_slots[i]->flag == DMA_SLOT_FREE) {
            dma_slots[i]->flag = DMA_SLOT_ALLOC;
            return dma_slots[i];
        }
    }
    return 0;
}

void dma_free_slot(struct dma_slot *s) {
    s->flag = DMA_SLOT_FREE;
}

void dma_alloc_blocking(dma_transfer_handle_t *id) {
    // not supported in baremetal
}


unsigned int dma_poll(dma_transfer_handle_t id) {
    struct dma_slot *slot = dma_slots[id];
    unsigned int p = REG32(DMA_BASE + 0x20 * slot->id + 0x14);
    return p;
}

void dma_init(void) {
    unsigned int s;
    if (DMA_SLOTS>0) {
        for (s=0;s<DMA_SLOTS;s++) {
            dma_slots[s] = (struct dma_slot*) malloc(sizeof(struct dma_slot));
            dma_slots[s]->id = s;
            dma_slots[s]->flag = DMA_SLOT_FREE;
        }
    }

    _optimsoc_dma_initialized = 1;
}

dma_success_t dma_alloc(dma_transfer_handle_t *id) {
    if (_optimsoc_dma_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    struct dma_slot *slot = dma_alloc_slot();
    if (!slot) {
        return DMA_ERR_NOSLOT;
    } else {
        *id = slot->id;
        return DMA_SUCCESS;
    }
}

dma_success_t dma_free(dma_transfer_handle_t id) {

    if (_optimsoc_dma_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    assert(id < DMA_SLOTS);

    struct dma_slot *slot = dma_slots[id];

    if(slot->flag == DMA_SLOT_ALLOC) {
        slot->flag = DMA_SLOT_FREE;
        return DMA_SUCCESS;
    } else {
        return DMA_ERR_NOTALLOCATED;
    }
}

dma_success_t dma_transfer(void* local, uint32_t remote_tile, void* remote,
                           size_t size, dma_direction_t dir,
                           dma_transfer_handle_t id) {
    if (_optimsoc_dma_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    struct dma_slot *slot = dma_slots[id];
    REG32(DMA_BASE + 0x20 * slot->id) = (uint32_t) local;
    REG32(DMA_BASE + 0x20 * slot->id + 0x4) = size;
    REG32(DMA_BASE + 0x20 * slot->id + 0x8) = remote_tile;
    REG32(DMA_BASE + 0x20 * slot->id + 0xc) = (uint32_t) remote;
    REG32(DMA_BASE + 0x20 * slot->id + 0x10) = dir;
    REG32(DMA_BASE + 0x20 * slot->id + 0x14) = 1; // go

#ifdef DMA_IRQ
    dma_slots[id]->flag = DMA_SLOT_WAIT;
#endif

    return DMA_SUCCESS;
}

dma_success_t dma_wait(dma_transfer_handle_t id) {
    if (_optimsoc_dma_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    while (dma_poll(id)==0) { __asm__ volatile("l.nop"); }

    return DMA_SUCCESS;
}
