/* Copyright (c) 2012-2014 by the author(s)
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
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef _BACKEND_DBGNOC_H_
#define _BACKEND_DBGNOC_H_

#include <inttypes.h>
#include <libglip.h>
#include <pthread.h>

#include <optimsochost/liboptimsochost.h>
#include "log.h"
#include "backends.h"


extern const int DBG_NOC_ADDR_TCM; /* from liboptimsochost.c */

/**
 * A single Debug NoC packet
 *
 * A packet can consist of at most 2^16 flits.
 */
struct lisnoc16_packet {
    /** number of flits in this packets */
    uint16_t len;
    /**
     * flits, without flit header (the first two bits), but including the
     * header flit
     */
    uint16_t* flit_data;
};

/**
 * A single LISNoC packet
 *
 * These packets are sent over the NCM to the LISNoC.
 *
 * A packet can consist of at most 2^16-1 lisnoc16 flits, i.e. 2^15-1
 * lisnoc32 flits.
 */
struct lisnoc32_packet {
    /** number of flits in this packet */
    uint16_t len;
    /**
     * flits, without flit header (the first two bits), but including the
     * header flit
     */
    uint32_t* flit_data;
};

struct ob_dbgnoc_connection_ctx;

struct ob_dbgnoc_connection_interface {
    int (*read_fn)(struct ob_dbgnoc_connection_ctx*, uint16_t* /*buffer*/,
                   int /*len*/);
    int (*write_fn)(struct ob_dbgnoc_connection_ctx*, uint16_t* /*buffer*/,
                    int /*len*/);
    int (*free)(struct ob_dbgnoc_connection_ctx*);
    int (*connect)(struct ob_dbgnoc_connection_ctx*);
    int (*disconnect)(struct ob_dbgnoc_connection_ctx*);
    int (*connected)(struct ob_dbgnoc_connection_ctx*);
    int (*reset)(struct ob_dbgnoc_connection_ctx*);
};

int ob_dbgnoc_new(struct optimsoc_backend_ctx **ctx,
                  struct optimsoc_backend_interface *calls,
                  struct optimsoc_log_ctx *log_ctx,
                  int num_options,
                  struct optimsoc_backend_option options[]);

int ob_dbgnoc_free(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_connect(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_disconnect(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_connected(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_reset(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_cpu_start(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_discover_system(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_get_sysinfo(struct optimsoc_backend_ctx *ctx,
                          struct optimsoc_sysinfo **sysinfo);
int ob_dbgnoc_mem_write(struct optimsoc_backend_ctx *ctx,
                        unsigned int mem_tile_id, unsigned int base_address,
                        const uint8_t* data, unsigned int data_len);
int ob_dbgnoc_cpu_stall(struct optimsoc_backend_ctx *ctx, int do_stall);
int ob_dbgnoc_cpu_reset(struct optimsoc_backend_ctx *ctx);
int ob_dbgnoc_itm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_itm_cb cb);
int ob_dbgnoc_nrm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_nrm_cb cb);
int ob_dbgnoc_stm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_stm_cb cb);
int ob_dbgnoc_nrm_set_sample_interval(struct optimsoc_backend_ctx *ctx,
                                      int sample_interval);
int ob_dbgnoc_read_clkstats(struct optimsoc_backend_ctx *ctx,
                            uint32_t *sys_clk,
                            uint32_t *sys_clk_halted);
int ob_dbgnoc_itm_refresh_config(struct optimsoc_backend_ctx *ctx,
                                 struct optimsoc_dbg_module *dbg_module);
int ob_dbgnoc_stm_refresh_config(struct optimsoc_backend_ctx *ctx,
                                 struct optimsoc_dbg_module *dbg_module);
int ob_dbgnoc_mam_get_config(struct optimsoc_backend_ctx *ctx);

/* private functions */
int lisnoc16_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc16_packet packets[], int length);
int lisnoc32_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc32_packet packets[], int length);
void* receive_thread(void* ctx_void);
int register_read(struct optimsoc_backend_ctx *ctx, int module_addr,
                  int reg_addr, int burst_len, uint16_t *data);
int register_write(struct optimsoc_backend_ctx *ctx, int module_addr,
                   int reg_addr, int burst_len, const uint16_t *data);
void ob_dbgnoc_glip_log(struct glip_ctx *gctx, int priority,
                        const char *file, int line, const char *fn,
                        const char *format, va_list args);
#endif /* _BACKEND_DBGNOC_H_ */
