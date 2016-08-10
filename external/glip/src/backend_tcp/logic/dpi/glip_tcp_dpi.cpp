/* Copyright (c) 2016 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "GlipTcp.h"

/*
 * Those C functions work with an object that is a class instance of
 * GlipTcp. The DPI functions just wrap class function calls.
 */

extern "C" {

/**
 * Create a GlipTcp instance
 *
 * @param tcp_port TCP port to open
 * @param width Data width in bit, must be a multiple of eight and <= 64
 * @return Object handle, or 0 in case of error
 */
void* glip_tcp_create(int tcp_port, int width) {
    if(((width % 8) != 0) || (width > 64)) {
        return 0;
    }

    GlipTcp *obj = &GlipTcp::instance();
    obj->init(tcp_port, width);
    return (void*) obj;
}

/**
 * Reset glip tcp
 *
 * @param obj Object handle
 * @return Always returns 0
 */
int glip_tcp_reset(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    return inst->reset();
}

/**
 * Check if a GLIP instance is connected
 *
 * @param obj Object handle
 * @return 1 if connected, 0 if not
 */
int glip_tcp_connected(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    return inst->connected() ? 1 : 0;
}

/**
 * Iterate to next cycle
 *
 * This function is used to advance to the next cycle. It is the
 * combinational part of the interface. It is called on the negative
 * clock edge. It returns a bit mask of available operations:
 *  - bit 0 if a control message can be received
 *  - bit 1 if an incoming data item is available
 *  - bit 2 if an outgoing data item can be send
 *
 *  @param obj Object handle
 *  @return Bit mask with available operations
 */
uint32_t glip_tcp_next_cycle(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    return inst->next_cycle();
}

/**
 * This function is used to pop the current control message
 *
 * It is called at the negative clock edge to set the output registers
 * of the interface.
 *
 * @param obj Object handle
 * @return 0x1 if logic reset is requested, 0 otherwise
 */
uint32_t glip_tcp_control_msg(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    return inst->control_msg();
}

/**
 * This function is used to get the current incoming data
 *
 * It is called at the negative clock edge to set the incoming data
 * item for this cycle. It does not pop the data item, which requires
 * a call to glip_tcp_read_ack.
 *
 * @param obj Object handle
 * @return Current incoming data item
 */
uint64_t glip_tcp_read(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    return inst->readData();
}

/**
 * Acknowledge read
 *
 * This function is called on a positive clock edge.
 *
 * @param obj Object handle
 */
void glip_tcp_read_ack(void* obj) {
    GlipTcp *inst = (GlipTcp*) obj;
    inst->readAck();
}

/**
 * Write a data item in this cycle
 *
 * This function is called on a positive clock edge.
 *
 * @param obj Object handle
 * @param data Data item to write
 */
void glip_tcp_write(void* obj, uint64_t data) {
    GlipTcp *inst = (GlipTcp*) obj;
    inst->writeData(data);
}

}
