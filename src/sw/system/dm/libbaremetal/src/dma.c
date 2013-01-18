/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * DMA driver. Work-in-progress.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <malloc.h>
#include "utils.h"

#include "dma.h"

#define DMA_SLOTS 4
// This must be changed to 2000 when conf part is removed from dma
#define DMA_BASE 0xe0001000

#define DMA_SLOT_FREE 0
#define DMA_SLOT_ALLOC 1
#define DMA_SLOT_WAIT 2


struct dma_slot {
  unsigned int flag;
  unsigned int id;
};

struct dma_slot *dma_slots[DMA_SLOTS];

void dma_init() {
  unsigned int s;
  if (DMA_SLOTS>0) {
      for (s=0;s<DMA_SLOTS;s++) {
          dma_slots[s] = (struct dma_slot*) malloc(sizeof(struct dma_slot));
          dma_slots[s]->id = s;
          dma_slots[s]->flag = DMA_SLOT_FREE;
      }
  }
#ifndef DMA_IRQ
  arch_disable_irq_dma();
#else
  arch_enable_irq_dma();
#endif
}

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

unsigned int dma_alloc(dma_transfer_handle_t *id) {
  struct dma_slot *slot = dma_alloc_slot();
  if (!slot) {
    return DMA_ERR_NOSLOT;
  } else {
    *id = slot->id;
    return DMA_SUCCESS;
  }
}

void dma_alloc_blocking(dma_transfer_handle_t *id) {
  // not supported in baremetal
}

unsigned int dma_transfer(void* local,unsigned int remote_tile, void* remote, unsigned int size, dma_direction_t dir, dma_transfer_handle_t id) {
  struct dma_slot *slot = dma_slots[id];
  *((volatile void**) ((unsigned int) 0xe0001000 + 0x20 * slot->id)) = local;
  *((volatile unsigned int*) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0x4)) = size;
  *((volatile unsigned int*) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0x8)) = remote_tile;
  *((volatile void**) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0xc)) = remote;
  *((volatile unsigned int*) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0x10)) = dir;
  *((volatile unsigned int*) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0x14)) = 1;

#ifdef DMA_IRQ
  dma_slots[id]->flag = DMA_SLOT_WAIT;
#endif

  return DMA_SUCCESS;
}

unsigned int dma_poll(dma_transfer_handle_t id) {
  struct dma_slot *slot = dma_slots[id];
  unsigned int p = *((volatile unsigned int*) ((unsigned int) 0xe0001000 + 0x20 * slot->id + 0x14));
  return p;
}

void dma_wait(dma_transfer_handle_t id) {
  while (dma_poll(id)==0) { __asm__ volatile("l.nop"); }
}
