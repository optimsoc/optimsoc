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
 *    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *    Philipp Wagner <philipp.wagner@tum.de>
 */

#ifndef _BACKEND_SIMTCP_H_
#define _BACKEND_SIMTCP_H_

#include <inttypes.h>

#include "log.h"
#include "backends.h"

void *ob_simtcp_receive_thread(void* ctx_void);

int ob_simtcp_new(struct optimsoc_backend_ctx **ctx,
                  struct optimsoc_backend_interface *calls,
                  struct optimsoc_log_ctx *log_ctx,
                  int num_options,
                  struct optimsoc_backend_option options[]);
int ob_simtcp_free(struct optimsoc_backend_ctx *ctx);
int ob_simtcp_connect(struct optimsoc_backend_ctx *ctx);
int ob_simtcp_disconnect(struct optimsoc_backend_ctx *ctx);
int ob_simtcp_connected(struct optimsoc_backend_ctx*);
int ob_simtcp_cpu_start(struct optimsoc_backend_ctx *ctx);
int ob_simtcp_discover_system(struct optimsoc_backend_ctx *ctx);
int ob_simtcp_get_sysinfo(struct optimsoc_backend_ctx *ctx,
                          struct optimsoc_sysinfo **sysinfo);
int ob_simtcp_mem_write(struct optimsoc_backend_ctx *ctx, int mem_tile_id,
                        int base_address, const uint8_t* data, int data_len);
int ob_simtcp_reset(struct optimsoc_backend_ctx*);
int ob_simtcp_cpu_stall(struct optimsoc_backend_ctx*, int do_stall);
int ob_simtcp_cpu_reset(struct optimsoc_backend_ctx*);
int ob_simtcp_itm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_itm_cb cb);
int ob_simtcp_nrm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_nrm_cb cb);
int ob_simtcp_stm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_stm_cb cb);
int ob_simtcp_nrm_set_sample_interval(struct optimsoc_backend_ctx *ctx,
                                      int sample_interval);
int ob_simtcp_read_clkstats(struct optimsoc_backend_ctx *ctx,
                            uint32_t *sys_clk, uint32_t *sys_clk_halted);

#endif /* _BACKEND_SIMTCP_H_ */
