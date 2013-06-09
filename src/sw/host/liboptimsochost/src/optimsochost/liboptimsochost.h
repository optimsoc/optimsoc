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
 * liboptimsochost is a host-side library for talking with the OpTiMSoC system.
 *
 * For connecting with the system, the USB interface of the Debug NoC is used.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

#ifndef _LIBOPTIMSOC_H_
#define _LIBOPTIMSOC_H_

#include <stdarg.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Opaque context object
 *
 * This object contains all state information. Create and initialize a new
 * object with optimsoc_new() and delete it with optimsoc_free().
 */
struct optimsoc_ctx;

/**
 * A description of a single debug module
 */
struct optimsoc_dbg_module {
    /** the address in the Debug NoC */
    int dbgnoc_addr;
    /** the module type */
    int module_type;
    /** the module version */
    int module_version;
};

typedef enum {
    OPTIMSOC_BACKEND_DBGNOC,
    OPTIMSOC_BACKEND_SIMTCP
} optimsoc_backend_id;

/**
 * Opaque logging context
 */
struct optimsoc_log_ctx;

/**
 * Logging function template
 *
 * Implement a function with this signature and pass it to optimsoc_set_log_fn()
 * if you want to implement custom logging.
 */
typedef void (*optimsoc_log_fn)(struct optimsoc_log_ctx *ctx,
                                int priority, const char *file,
                                int line, const char *fn,
                                const char *format, va_list args);

typedef void (*optimsoc_itm_cb)(int core_id,
                                uint32_t timestamp,
                                uint32_t pc,
                                int count);

typedef void (*optimsoc_nrm_cb)(int router_id,
                                uint32_t timestamp,
                                uint8_t *link_flit_count,
                                int monitored_links);

typedef void (*optimsoc_stm_cb)(uint32_t core_id,
                                uint32_t timestamp,
                                uint16_t id,
                                uint32_t value);

struct optimsoc_backend_option {
	char* name;
	char* value;
};

int optimsoc_new(struct optimsoc_ctx **ctx,
		         optimsoc_backend_id backend,
		         int num_options,
		         struct optimsoc_backend_option options[]);

int optimsoc_connect(struct optimsoc_ctx *ctx);
int optimsoc_disconnect(struct optimsoc_ctx *ctx);
int optimsoc_free(struct optimsoc_ctx *ctx);

void optimsoc_set_log_fn(struct optimsoc_ctx *ctx, optimsoc_log_fn log_fn);
int optimsoc_get_log_priority(struct optimsoc_ctx *ctx);
void optimsoc_set_log_priority(struct optimsoc_ctx *ctx, int priority);

int optimsoc_mem_read(struct optimsoc_ctx *ctx, int tile_id,
                      int base_address, char** data);

int optimsoc_mem_write(struct optimsoc_ctx *ctx, int mem_tile_id,
                       int base_address, const uint8_t* data, int data_len);
int optimsoc_mem_init(struct optimsoc_ctx *ctx, int mem_tile_id,
                      const uint8_t* data, int data_len);

char* optimsoc_get_version_string(void);

int optimsoc_discover_system(struct optimsoc_ctx *ctx);

int optimsoc_reset(struct optimsoc_ctx *ctx);

int optimsoc_get_sysid(struct optimsoc_ctx *ctx);

int optimsoc_get_modules(struct optimsoc_ctx *ctx,
                         struct optimsoc_dbg_module **modules,
                         int *module_count);

char* optimsoc_get_module_name(int module_type);

int optimsoc_cpu_stall(struct optimsoc_ctx *ctx, int do_stall);
int optimsoc_cpu_reset(struct optimsoc_ctx *ctx);
int optimsoc_cpu_start(struct optimsoc_ctx *ctx);

int optimsoc_itm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_itm_cb cb);
int optimsoc_nrm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_nrm_cb cb);
int optimsoc_stm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_stm_cb cb);
int optimsoc_nrm_set_sample_interval(struct optimsoc_ctx *ctx,
                                     int sample_interval);
int optimsoc_read_clkstats(struct optimsoc_ctx *ctx, uint32_t *sys_clk,
                           uint32_t *sys_clk_halted);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
