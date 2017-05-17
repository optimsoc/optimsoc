/* Copyright (c) 2014-2015 by the author(s)
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
 * ============================================================================
 *
 * libglip is a flexible FIFO-based communication library between an FPGA and
 * a PC.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */

#ifndef __BACKEND_JTAG_H__
#define __BACKEND_JTAG_H__

#include <libglip.h>

#include <unistd.h>


/**
 * Type of "number of words", similar to size_t, which gives the
 * "number of bytes"
 */
typedef uint32_t wsize_t;

struct cmdbuf {
    size_t buf_size;
    size_t pos;
    char buf[];
};

int gb_jtag_new(struct glip_ctx* ctx);

int gb_jtag_open(struct glip_ctx *ctx, unsigned int num_channels);
int gb_jtag_close(struct glip_ctx *ctx);

int gb_jtag_logic_reset(struct glip_ctx *ctx);

int gb_jtag_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_read);
int gb_jtag_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_read,
                   unsigned int timeout);
int gb_jtag_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                  uint8_t *data, size_t *size_written);
int gb_jtag_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                    uint8_t *data, size_t *size_written,
                    unsigned int timeout);

unsigned int gb_jtag_get_channel_count(struct glip_ctx *ctx);
unsigned int gb_jtag_get_fifo_width(struct glip_ctx *ctx);
#endif /* __BACKEND_JTAG_H__ */
