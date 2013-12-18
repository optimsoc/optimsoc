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

#ifndef _BACKENDS_H_
#define _BACKENDS_H_

#include "liboptimsochost-private.h"

/**
 * Opaque backend context. Define this struct inside the backend C file.
 */
struct optimsoc_backend_ctx;

/**
 * Interface of a liboptimsochost backend.
 *
 * A backend needs to implement these functions, which are then called from
 * the public liboptimsochost functions. The backend context can be individual for
 * each backend, and needs to be defined inside the backend C file.
 */
struct optimsoc_backend_interface {
    int (*free)(struct optimsoc_backend_ctx*);
    int (*connect)(struct optimsoc_backend_ctx*);
    int (*disconnect)(struct optimsoc_backend_ctx*);
    int (*connected)(struct optimsoc_backend_ctx*);
    int (*discover_system)(struct optimsoc_backend_ctx*);
    int (*get_sysinfo)(struct optimsoc_backend_ctx*, struct optimsoc_sysinfo**);
    int (*reset)(struct optimsoc_backend_ctx*);
    int (*cpu_start)(struct optimsoc_backend_ctx*);
    int (*cpu_stall)(struct optimsoc_backend_ctx*, int /*do_stall*/);
    int (*cpu_reset)(struct optimsoc_backend_ctx*);
    int (*mem_write)(struct optimsoc_backend_ctx*, unsigned int /*memory_id*/,
                     unsigned int /*base_address*/, const uint8_t* /*data*/,
                     unsigned int /*data_len*/);
    int (*itm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_itm_cb);
    int (*nrm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_nrm_cb);
    int (*stm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_stm_cb);
    int (*nrm_set_sample_interval)(struct optimsoc_backend_ctx*,
                                   int /*sample_interval*/);
    int (*read_clkstats)(struct optimsoc_backend_ctx*, uint32_t* /*sys_clk*/,
                         uint32_t* /*sys_clk_halted*/);
    int (*itm_refresh_config)(struct optimsoc_backend_ctx*,
                              struct optimsoc_dbg_module* /*dbg_module*/);
};

#endif /* _BACKENDS_H_ */
