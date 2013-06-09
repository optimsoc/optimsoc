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
    int (*mem_write)(struct optimsoc_backend_ctx*, int /*mem_tile_id*/,
                     int /*base_address*/, const uint8_t* /*data*/,
                     int /*data_len*/);
    int (*itm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_itm_cb);
    int (*nrm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_nrm_cb);
    int (*stm_register_callback)(struct optimsoc_backend_ctx*, optimsoc_stm_cb);
    int (*nrm_set_sample_interval)(struct optimsoc_backend_ctx*,
                                   int /*sample_interval*/);
    int (*read_clkstats)(struct optimsoc_backend_ctx*, uint32_t* /*sys_clk*/,
                         uint32_t* /*sys_clk_halted*/);
};

#endif /* _BACKENDS_H_ */
