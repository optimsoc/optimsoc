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
 * ============================================================================
 *
 * liboptimsochost is a host-side library for talking with the OpTiMSoC system.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef _LIBOPTIMSOCHOST_H_
#define _LIBOPTIMSOCHOST_H_

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

/*
 * Module types
 */
typedef enum {
    OPTIMSOC_MODULE_TYPE_CTM = 0x01,
    OPTIMSOC_MODULE_TYPE_ITM = 0x02,
    OPTIMSOC_MODULE_TYPE_NRM = 0x03,
    OPTIMSOC_MODULE_TYPE_NCM = 0x04,
    OPTIMSOC_MODULE_TYPE_STM = 0x05,
    OPTIMSOC_MODULE_TYPE_MAM = 0x07
} optimsoc_module_type;

typedef enum {
    OPTIMSOC_BACKEND_DBGNOC,
    OPTIMSOC_BACKEND_SIMTCP
} optimsoc_backend_id;

/**
 * A description of a single debug module
 */
struct optimsoc_dbg_module {
    /** the address in the Debug NoC */
    int dbgnoc_addr;
    /** the module type */
    optimsoc_module_type module_type;
    /** the module version */
    int module_version;
};

/**
 * Configuration of a single ITM module
 */
struct optimsoc_itm_config {
    /** ID of the associated core */
    unsigned int core_id;
};

struct optimsoc_stm_config {
    /** ID of the associated core */
    unsigned int core_id;
};

/**
 * Configuration of a single MAM module
 */
struct optimsoc_mam_config {
    /** ID of the associated memory */
    unsigned int memory_id;
};

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

typedef void (*optimsoc_itm_cb)(unsigned int core_id,
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

int optimsoc_new(struct optimsoc_ctx **ctx, optimsoc_backend_id backend,
                 int num_options, struct optimsoc_backend_option options[]);

int optimsoc_connect(struct optimsoc_ctx *ctx);
int optimsoc_disconnect(struct optimsoc_ctx *ctx);
int optimsoc_free(struct optimsoc_ctx *ctx);

void optimsoc_set_log_fn(struct optimsoc_ctx *ctx, optimsoc_log_fn log_fn);
int optimsoc_get_log_priority(struct optimsoc_ctx *ctx);
void optimsoc_set_log_priority(struct optimsoc_ctx *ctx, int priority);

int optimsoc_mem_read(struct optimsoc_ctx *ctx, int tile_id,
                      int base_address, char** data);

int optimsoc_mem_write(struct optimsoc_ctx *ctx, unsigned int memory_id,
                       unsigned int base_address, const uint8_t* data,
                       unsigned int data_len);
int optimsoc_mem_init(struct optimsoc_ctx *ctx, unsigned int* memory_ids,
                      unsigned int memory_count, const uint8_t* data,
                      int data_len);

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
int optimsoc_itm_get_config(struct optimsoc_ctx *ctx,
                            struct optimsoc_dbg_module *dbg_module,
                            struct optimsoc_itm_config **itm_config);
int optimsoc_stm_get_config(struct optimsoc_ctx *ctx,
                            struct optimsoc_dbg_module *dbg_module,
                            struct optimsoc_stm_config **stm_config);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
