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

void *poll_thread(void *arg);

ssize_t write_b(int fildes, const void *buf, size_t nbyte);

int start_openocd(struct glip_ctx *ctx);
int close_openocd(struct glip_ctx *ctx);

int discover_targetconf(struct glip_ctx *ctx);

int jtag_drscan(struct glip_ctx *ctx, const uint8_t* req, uint8_t* resp,
                const size_t size);
int jtag_drscan_read(struct glip_ctx *ctx, uint8_t* resp, const size_t size);
int jtag_drscan_write(struct glip_ctx *ctx, const uint8_t* req,
                      const size_t size);
int jtag_set_ir(struct glip_ctx *ctx);

int hexstring_to_binarray_le(char* in, size_t in_size, uint8_t* out);
uint8_t hexstring_to_byte(char* in);

wsize_t calc_payload_len(struct glip_ctx *ctx,
                         wsize_t wanted_words,
                         wsize_t *wr_words, wsize_t *rd_words);
int build_jtag_req(struct glip_ctx *ctx, wsize_t wr_words, wsize_t rd_words,
                   uint8_t* req, size_t req_size);
void parse_jtag_resp(struct glip_ctx *ctx, uint8_t *resp, size_t resp_size,
                     wsize_t *acked_words, wsize_t *read_words);

wsize_t to_words(struct glip_ctx *ctx, size_t bytes);
size_t to_bytes(struct glip_ctx *ctx, wsize_t words);
void copy_to_word(struct glip_ctx *ctx, uint32_t value, uint8_t *dest);
void copy_from_word(struct glip_ctx *ctx, uint8_t *from, uint32_t *out);

size_t grow_buf(size_t current_size);

struct cmdbuf* cmdbuf_new(const size_t buf_size);
void cmdbuf_append(struct cmdbuf *buf, const char* data, const size_t data_size);
#endif /* __BACKEND_JTAG_H__ */
