/*
 * This file is part of liboptimsochost.
 *
 * liboptimsochost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsochost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with liboptimsochost. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner <philipp.wagner@tum.de>
 *    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef _BACKEND_DBGNOC_H_
#define _BACKEND_DBGNOC_H_

#include <inttypes.h>
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
                               int mem_tile_id, int base_address,
                               const uint8_t* data, int data_len);
int ob_dbgnoc_cpu_stall(struct optimsoc_backend_ctx *ctx, int do_stall);
int ob_dbgnoc_cpu_reset(struct optimsoc_backend_ctx *ctx);

int lisnoc16_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc16_packet packets[], int length);
int lisnoc32_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc32_packet packets[], int length);
void* receive_thread(void* ctx_void);
int register_read(struct optimsoc_backend_ctx *ctx, int module_addr,
                  int reg_addr, int burst_len, uint16_t *data);
int register_write(struct optimsoc_backend_ctx *ctx, int module_addr,
                   int reg_addr, int burst_len, const uint16_t *data);
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

#endif /* _BACKEND_DBGNOC_H_ */
