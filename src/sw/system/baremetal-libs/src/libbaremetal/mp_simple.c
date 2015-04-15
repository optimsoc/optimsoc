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
unsigned int* optimsoc_mp_simple_buffer;

// List of handlers for the classes
void (*cls_handlers[OPTIMSOC_CLASS_NUM])(unsigned int*,int);

void optimsoc_mp_simple_init(void);
void optimsoc_mp_simple_inth(void* arg);

volatile uint8_t *_optimsoc_mp_simple_domains_ready;

void optimsoc_mp_simple_init(void) {
    // Register interrupt
    or1k_interrupt_handler_add(3, &optimsoc_mp_simple_inth, 0);
    or1k_interrupt_enable(3);

    // Reset class handler
    for (int i=0;i<OPTIMSOC_CLASS_NUM;i++) {
        cls_handlers[i] = 0;
    }

    _optimsoc_mp_simple_domains_ready = calloc(optimsoc_get_numct(), 1);

    // Allocate buffer
    optimsoc_mp_simple_buffer = malloc(optimsoc_noc_maxpacketsize()*4);
}

void optimsoc_mp_simple_enable(void) {
    REG32(OPTIMSOC_MPSIMPLE_ENABLE) = 1;
}

int optimsoc_mp_simple_ctready(uint32_t rank) {
    if (_optimsoc_mp_simple_domains_ready[rank]) {
        return 1;
    }

    uint32_t tile = optimsoc_get_ranktile(rank);
    uint32_t req = tile << OPTIMSOC_DEST_LSB;
    req = SET(req, OPTIMSOC_CLASS_NUM-1, OPTIMSOC_CLASS_MSB,
              OPTIMSOC_CLASS_LSB);
    req = SET(req, optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);

    REG32(OPTIMSOC_MPSIMPLE_SEND) = 1;
    REG32(OPTIMSOC_MPSIMPLE_SEND) = req;

    return 0;
}

void optimsoc_mp_simple_addhandler(unsigned int class,
                                   void (*hnd)(unsigned int*,int)) {
    cls_handlers[class] = hnd;
}

void optimsoc_mp_simple_inth(void* arg) {

    (void) arg;

    while (1) {
        // Store message in buffer
        // Get size
        int size = REG32(OPTIMSOC_MPSIMPLE_RECV);

        if (size==0) {
            // There are no further messages in the buffer
            break;
        } else if (optimsoc_noc_maxpacketsize()<size) {
            // Abort and drop if message cannot be stored
            //            printf("FATAL: not sufficent buffer space. Drop packet\n");
            for (int i=0;i<size;i++) {
                REG32(OPTIMSOC_MPSIMPLE_RECV);
            }
        } else {
            for (int i=0;i<size;i++) {
                optimsoc_mp_simple_buffer[i] = REG32(OPTIMSOC_MPSIMPLE_RECV);
            }
        }

        uint32_t header = optimsoc_mp_simple_buffer[0];
        // Extract class
        uint32_t class = EXTRACT(header, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);

        if (class == OPTIMSOC_CLASS_NUM-1) {
            uint32_t ready = (header & 0x2) >> 1;
            if (ready) {
                uint32_t tile, domain;
                tile = EXTRACT(header, OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
                domain = optimsoc_get_tilerank(tile);
                _optimsoc_mp_simple_domains_ready[domain] = 1;
            }
        }

        // Call respective class handler
        if (cls_handlers[class] == 0) {
            // No handler registered, packet gets lost
            //printf("Packet of unknown class (%d) received. Drop.\n",class);
            continue;
        }


        uint32_t src = (optimsoc_mp_simple_buffer[0]>>OPTIMSOC_SRC_LSB) & 0x1f;
        trace_mp_simple_recv(src, class, size);

        cls_handlers[class](optimsoc_mp_simple_buffer,size);

        trace_mp_simple_recv_finished(src);
    }
}

void optimsoc_mp_simple_send(unsigned int size, uint32_t *buf) {
    trace_mp_simple_send(buf[0]>>OPTIMSOC_DEST_LSB, size, buf);

    uint32_t restore = or1k_critical_begin();
    REG32(OPTIMSOC_MPSIMPLE_SEND) = size;
    for (int i=0;i<size;i++) {
        REG32(OPTIMSOC_MPSIMPLE_SEND) = buf[i];
    }

    or1k_critical_end(restore);

    trace_mp_simple_send_finished(buf[0] >> OPTIMSOC_DEST_LSB);
}

