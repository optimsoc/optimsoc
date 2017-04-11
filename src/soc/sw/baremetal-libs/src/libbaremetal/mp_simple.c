/* Copyright (c) 2012-2013 by the author(s)
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
 * Driver for the simple message passing hardware.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <or1k-support.h>

#include "include/optimsoc-baremetal.h"

#include <stdlib.h>

#define BASE       (OPTIMSOC_NA_BASE + 0x100000)
#define REG_NUMEP  BASE
#define EP_BASE    BASE + 0x2000
#define EP_OFFSET  0x2000
#define REG_SEND   0x0
#define REG_RECV   0x0
#define REG_ENABLE 0x4

#define SEND(ep) REG32(EP_BASE + ep*EP_OFFSET+REG_SEND)
#define RECV(ep) REG32(EP_BASE + ep*EP_OFFSET+REG_RECV)
#define ENABLE(ep) REG32(EP_BASE + ep*EP_OFFSET+REG_ENABLE)

//#define TRACE_ENABLE

#define OPTIMSOC_TRACE_MPSIMPLE_SEND          0x100
#define OPTIMSOC_TRACE_MPSIMPLE_SEND_FINISHED 0x101
#define OPTIMSOC_TRACE_MPSIMPLE_RECV          0x102
#define OPTIMSOC_TRACE_MPSIMPLE_RECV_FINISHED 0x103

static inline void trace_mp_simple_send(uint32_t dest, uint32_t size, void *buf) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_SEND, dest);
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_SEND, size);
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_SEND, buf);
#endif
}
static inline void trace_mp_simple_send_finished(uint32_t dest) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_SEND_FINISHED, dest);
#endif
}

static inline void trace_mp_simple_recv(uint32_t src, uint32_t class, uint32_t size) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_RECV, src);
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_RECV, class);
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_RECV, size);
#endif
}

static inline void trace_mp_simple_recv_finished(uint32_t src) {
#ifdef TRACE_ENABLE
    OPTIMSOC_TRACE(OPTIMSOC_TRACE_MPSIMPLE_RECV_FINISHED, src);
#endif
}

#define EXTRACT(x,msb,lsb) ((x>>lsb) & ~(~0 << (msb-lsb+1)))
#define SET(x,v,msb,lsb) (((~0 << ((msb)+1) | ~(~0 << (lsb)))&x) | \
        (((v) & ~(~0<<((msb)-(lsb)+1))) << (lsb)))

// Local buffer for the simple message passing
static uint32_t* _buffer;

// List of handlers for the classes
void (*cls_handlers[OPTIMSOC_CLASS_NUM])(uint32_t*,size_t);

static void _irq_handler(void* arg);

static volatile uint32_t *_domains_ready;

static uint16_t _num_endpoints;

void optimsoc_mp_simple_init(void) {
    // Register interrupt
    or1k_interrupt_handler_add(3, &_irq_handler, 0);
    or1k_interrupt_enable(3);

    // Reset class handler
    for (int i=0;i<OPTIMSOC_CLASS_NUM;i++) {
        cls_handlers[i] = 0;
    }

    _num_endpoints = REG32(REG_NUMEP);
    _domains_ready = calloc(optimsoc_get_numct(), sizeof(uint32_t));

    // Allocate buffer
    _buffer = malloc(optimsoc_noc_maxpacketsize()*sizeof(uint32_t));
}

uint16_t optimsoc_mp_simple_num_endpoints() {
    return _num_endpoints;
}

void optimsoc_mp_simple_enable(uint16_t endpoint) {
    ENABLE(endpoint) = 1;
}

int optimsoc_mp_simple_ctready(uint32_t rank, uint16_t endpoint) {
    uint32_t ready = _domains_ready[rank];
    if ((ready >> endpoint) & 0x1) {
        return 1;
    }

    uint32_t tile = optimsoc_get_ranktile(rank);
    uint32_t req = tile << OPTIMSOC_DEST_LSB;
    req = SET(req, OPTIMSOC_CLASS_NUM-1, OPTIMSOC_CLASS_MSB,
              OPTIMSOC_CLASS_LSB);
    req = SET(req, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    req = SET(req, endpoint & 0xff, 9, 2);

    SEND(endpoint) = 1;
    SEND(endpoint) = req;

    return 0;
}

void optimsoc_mp_simple_addhandler(uint8_t class,
                                   void (*hnd)(uint32_t*,size_t)) {
    cls_handlers[class] = hnd;
}

void _irq_handler(void* arg) {

    (void) arg;

    while (1) {
        uint16_t empty = 0;
        for (uint16_t ep = 0; ep < _num_endpoints; ep++) {
            // Store message in buffer
            // Get size
            size_t size = RECV(ep);

            if (size==0) {
                // There are no further messages in the buffer
                empty++;
                continue;
            } else if (optimsoc_noc_maxpacketsize()<size) {
                // Abort and drop if message cannot be stored
                //            printf("FATAL: not sufficent buffer space. Drop packet\n");
                for (int i=0;i<size;i++) {
                    RECV(ep);
                }
            } else {
                for (int i=0;i<size;i++) {
                    _buffer[i] = RECV(ep);
                }
            }

            uint32_t header = _buffer[0];
            // Extract class
            uint8_t class = EXTRACT(header, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);

            if (class == OPTIMSOC_CLASS_NUM-1) {
                uint32_t ready = (header & 0x2) >> 1;
                if (ready) {
                    uint32_t tile, domain;
                    uint8_t endpoint;
                    tile = EXTRACT(header, OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
                    domain = optimsoc_get_tilerank(tile);
                    endpoint = EXTRACT(header, 9, 2);
                    _domains_ready[domain] |= 1 << endpoint;
                }
            }

            // Call respective class handler
            if (cls_handlers[class] == 0) {
                // No handler registered, packet gets lost
                //printf("Packet of unknown class (%d) received. Drop.\n",class);
                continue;
            }

            uint32_t src = (_buffer[0]>>OPTIMSOC_SRC_LSB) & 0x1f;
            trace_mp_simple_recv(src, class, size);

            cls_handlers[class](_buffer,size);

            trace_mp_simple_recv_finished(src);
        }
        if (empty == _num_endpoints)
            break;
    }
}

void optimsoc_mp_simple_send(uint16_t endpoint, size_t size, uint32_t *buf) {
    trace_mp_simple_send(buf[0]>>OPTIMSOC_DEST_LSB, size, buf);

    uint32_t restore = or1k_critical_begin();

    SEND(endpoint) = size;
    for (int i=0;i<size;i++) {
        SEND(endpoint) = buf[i];
    }

    or1k_critical_end(restore);

    trace_mp_simple_send_finished(buf[0] >> OPTIMSOC_DEST_LSB);
}

