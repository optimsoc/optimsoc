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
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
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
int ob_simtcp_mem_write(struct optimsoc_backend_ctx *ctx,
                        unsigned int mem_tile_id, unsigned int base_address,
                        const uint8_t* data, unsigned int data_len);
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
int ob_simtcp_itm_refresh_config(struct optimsoc_backend_ctx *ctx,
                                 struct optimsoc_dbg_module *dbg_module);

#endif /* _BACKEND_SIMTCP_H_ */
