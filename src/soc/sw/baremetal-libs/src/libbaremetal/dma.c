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

// This must be changed to 2000 when conf part is removed from dma
#define DMA_BASE 0xe0200000
#define DMA_SLOT_OFFSET 0x2000
#define DMA_REG_LADDR 0
#define DMA_REG_SIZE  1
#define DMA_REG_RTILE 2
#define DMA_REG_RADDR 3
#define DMA_REG_DIR   4
#define DMA_REG_CTRL  5

#define DMA_REG(slot,reg) REG32(DMA_BASE+slot*DMA_SLOT_OFFSET+((reg)<<2))

#define DMA_SLOT_FREE 0
#define DMA_SLOT_ALLOC 1
#define DMA_SLOT_WAIT 2

static uint8_t _initialized = 0;
static size_t _numslots;

struct dma_slot {
    unsigned int flag;
    unsigned int id;
};

struct dma_slot **dma_slots;

void optimsoc_dma_init(void) {
    _numslots = REG32(OPTIMSOC_NA_DMA_SLOTS);

    dma_slots = calloc(sizeof(struct dma_slot *), _numslots);

    for (size_t s = 0; s < _numslots; s++) {
        dma_slots[s] = (struct dma_slot*) malloc(sizeof(struct dma_slot));
        dma_slots[s]->id = s;
        dma_slots[s]->flag = DMA_SLOT_FREE;
    }

    _initialized = 1;
}

size_t optimsoc_dma_slots(void) {
    return _numslots;
}

static struct dma_slot *_alloc_slot() {
    for (size_t s = 0; s < _numslots; s++) {
        if (dma_slots[s]->flag == DMA_SLOT_FREE) {
            dma_slots[s]->flag = DMA_SLOT_ALLOC;
            return dma_slots[s];
        }
    }
    return 0;
}

static void _free_slot(struct dma_slot *s) {
    s->flag = DMA_SLOT_FREE;
}

unsigned int _poll(optimsoc_dma_handle_t id) {
    unsigned int p = DMA_REG(id,DMA_REG_CTRL);
    return p;
}

optimsoc_dma_success_t optimsoc_dma_alloc(optimsoc_dma_handle_t *id) {
    if (_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    struct dma_slot *slot = _alloc_slot();
    if (!slot) {
        return DMA_ERR_NOSLOT;
    } else {
        *id = slot->id;
        return DMA_SUCCESS;
    }
}

optimsoc_dma_success_t optimsoc_dma_free(optimsoc_dma_handle_t id) {
    if (_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    if (id >= _numslots) {
        return DMA_ERR_ILLEGALSLOT;
    }

    struct dma_slot *slot = dma_slots[id];

    if(slot->flag == DMA_SLOT_ALLOC) {
        _free_slot(slot);
        return DMA_SUCCESS;
    } else {
        return DMA_ERR_NOTALLOCATED;
    }
}

optimsoc_dma_success_t optimsoc_dma_transfer(optimsoc_dma_handle_t id,
                                             void* local,
                                             uint32_t remote_tile,
                                             void* remote,
                                             size_t size,
                                             optimsoc_dma_direction_t dir) {
    if (_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    if (id >= _numslots) {
        return DMA_ERR_ILLEGALSLOT;
    }

    DMA_REG(id,DMA_REG_LADDR) = (uint32_t) local;
    DMA_REG(id,DMA_REG_SIZE)  = size;
    DMA_REG(id,DMA_REG_RTILE) = remote_tile;
    DMA_REG(id,DMA_REG_RADDR) = (uint32_t) remote;
    DMA_REG(id,DMA_REG_DIR)   = dir;
    DMA_REG(id,DMA_REG_CTRL)  = 1;

    return DMA_SUCCESS;
}

optimsoc_dma_success_t optimsoc_dma_wait(optimsoc_dma_handle_t id) {
    if (_initialized==0) {
        return DMA_ERR_NOTINITIALIZED;
    }

    while (_poll(id)==0) { __asm__ volatile("l.nop"); }

    return DMA_SUCCESS;
}
