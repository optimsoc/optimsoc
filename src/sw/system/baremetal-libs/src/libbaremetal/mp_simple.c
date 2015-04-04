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


// Local buffer for the simple message passing
unsigned int* optimsoc_mp_simple_buffer;

// List of handlers for the classes
void (*cls_handlers[OPTIMSOC_CLASS_NUM])(unsigned int*,int);

void optimsoc_mp_simple_init(void);
void optimsoc_mp_simple_inth(void* arg);

void optimsoc_mp_simple_init(void) {
    // Register interrupt
    or1k_interrupt_handler_add(3, &optimsoc_mp_simple_inth, 0);
    or1k_interrupt_enable(3);

    // Reset class handler
    for (int i=0;i<OPTIMSOC_CLASS_NUM;i++) {
        cls_handlers[i] = 0;
    }

    // Allocate buffer
    optimsoc_mp_simple_buffer = malloc(optimsoc_noc_maxpacketsize()*4);
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

        // Extract class
        int class = (optimsoc_mp_simple_buffer[0] >> OPTIMSOC_CLASS_LSB) // Shift to position
    		                        & ((1<<(OPTIMSOC_CLASS_MSB-OPTIMSOC_CLASS_LSB+1))-1); // and mask other remain

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

    uint32_t restore_timer = or1k_timer_disable();
    uint32_t restore_irq = or1k_interrupts_disable();
    REG32(OPTIMSOC_MPSIMPLE_SEND) = size;
    for (int i=0;i<size;i++) {
        REG32(OPTIMSOC_MPSIMPLE_SEND) = buf[i];
    }
    or1k_interrupts_restore(restore_irq);
    or1k_timer_restore(restore_timer);

    trace_mp_simple_send_finished(buf[0]>>OPTIMSOC_DEST_LSB);
}

