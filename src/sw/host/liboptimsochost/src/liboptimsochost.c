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
 *    Philipp Wagner, mail@philipp-wagner.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>

#include <optimsochost/liboptimsochost.h>
#include "liboptimsochost-private.h"

#include "log.h"
#include "backends.h"
#include "backend_dbgnoc.h"
#include "backend_simtcp.h"

/**
 * Maximum number of bytes sent as one DMA write when initializing memory
 */
const int MEM_INIT_MAX_BYTES = 512;

/**
 * Address of the TCM module in the Debug NoC
 * Keep this in sync with dbg_config.vh
 */
const int DBG_NOC_ADDR_TCM = 0x01;

/**
 * Opaque object representing the library context.
 */
struct optimsoc_ctx {
    /** logging context */
    struct optimsoc_log_ctx *log_ctx;

    /** Selected backend */
    struct optimsoc_backend_ctx *backend_ctx;

    /** backend calls definition */
    struct optimsoc_backend_interface backend_call;
};

/**
 * \mainpage liboptimsochost API reference
 *
 * \section intro Introduction
 * TBD: introduction, features
 *
 * \section gettingstarted Getting Started
 * TBD: getting started
 *
 * \section logging Logging
 * TBD: description of the logging framework
 */


/**
 * \defgroup lib Basic Library Functionality
 *
 * Before using liboptimsochost, you need to initialize the library. After you are
 * done with liboptimsochost, you also need to deinitialize it to free all
 * associated resources. This page describes the functions required for these
 * tasks.
 */

/**
 * \defgroup log Logging
 * liboptimsochost contains a flexible logging mechanism, which provides useful
 * to the developer during development as well as for the end-user.
 */

/**
 * \defgroup highlevel High-Level API
 */

/**
 * \defgroup lowlevel Low-Level Debug NoC API
 */


/**
 * Create a new library context with a given backend.
 *
 * \param ctx library context
 * \param backend the backend which should be used
 *
 * \ingroup lib
 */
OPTIMSOC_EXPORT
int optimsoc_new(struct optimsoc_ctx **ctx, optimsoc_backend_id backend,
                 int num_options, struct optimsoc_backend_option options[])
{
    struct optimsoc_ctx *c;
    int rv;

    c = calloc(1, sizeof(struct optimsoc_ctx));
    if (!c) {
        return -ENOMEM;
    }

    rv = optimsoc_log_new(&(c->log_ctx));
    if (rv < 0) {
        return rv;
    }

    switch (backend) {
    case OPTIMSOC_BACKEND_DBGNOC:
        rv = ob_dbgnoc_new(&(c->backend_ctx), &(c->backend_call),
                           c->log_ctx, num_options, options);
        break;
    case OPTIMSOC_BACKEND_SIMTCP:
        rv = ob_simtcp_new(&(c->backend_ctx), &(c->backend_call),
                           c->log_ctx, num_options, options);
        break;
    default:
        err(c->log_ctx, "Unknown backend selected.\n");
        return -1;
    }

    if (rv < 0) {
        err(c->log_ctx, "Unable to initialize backend\n");
        return rv;
    }

    info(c->log_ctx, "ctx %p created\n", c);
    *ctx = c;
    return 0;
}

/**
 * Set logging function
 *
 * The built-in logging writes to STDERR. It can be overridden by a custom
 * function, to plug log messages into the user's logging functionality.
 *
 * \param ctx    the library context
 * \param log_fn the used logging function
 * \ingroup log
 */
OPTIMSOC_EXPORT
void optimsoc_set_log_fn(struct optimsoc_ctx *ctx, optimsoc_log_fn log_fn)
{
    optimsoc_log_set_log_fn(ctx->log_ctx, log_fn);
}

/**
 * Get the logging priority
 *
 * The logging priority is the lowest message type that is reported.
 *
 * \param ctx the library context
 * \ingroup log
 */
OPTIMSOC_EXPORT
int optimsoc_get_log_priority(struct optimsoc_ctx *ctx)
{
    return optimsoc_log_get_priority(ctx->log_ctx);
}

/**
 * Set the logging priority
 *
 * The logging priority is the lowest message type that is reported.
 *
 * \param ctx       the library context
 * \param priority  new priority value
 * \ingroup log
 */
OPTIMSOC_EXPORT
void optimsoc_set_log_priority(struct optimsoc_ctx *ctx,  int priority)
{
    optimsoc_log_set_priority(ctx->log_ctx, priority);
}

/**
 * Connect to the OpTiMSoC system via backend.
 *
 * Note: This resets the whole system after connection.
 *
 * \todo Reset only the communication infrastructure and not the whole system.
 * \ingroup lib
 */
OPTIMSOC_EXPORT
int optimsoc_connect(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.connect(ctx->backend_ctx);
}

/**
 * Disconnect from the OpTiMSoC system
 *
 * This method disconnects from all backend devices and frees the associated
 * resources. It blocks until all operations are finished.
 *
 * \param ctx the library context
 * \ingroup lib
 */
OPTIMSOC_EXPORT
int optimsoc_disconnect(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.disconnect(ctx->backend_ctx);
}

/**
 * Free all library resources
 *
 * Call this function after you disconnected from the target using
 * optimsoc_disconnect().
 *
 * The \p ctx object is invalid after calling this function and may not be
 * accessed any more.
 *
 * \param ctx the library context
 * \ingroup lib
 */
OPTIMSOC_EXPORT
int optimsoc_free(struct optimsoc_ctx *ctx)
{
    int rv;

    if (ctx->backend_call.connected(ctx->backend_ctx)) {
        err(ctx->log_ctx, "Call optimsoc_disconnect() first!\n");
        return -1;
    }

    rv = ctx->backend_call.free(ctx->backend_ctx);
    if (rv < 0) {
        return rv;
    }

    free(ctx);
    return 0;
}

/**
 * Get the version number of this library.
 *
 * The number format is not further specified, use this value only
 * for display (e.g. in the end-user application), but do not try to parse
 * it.
 *
 * \ingroup lib
 */
OPTIMSOC_EXPORT
char* optimsoc_get_version_string(void)
{
    return PACKAGE_VERSION;
}

/**
 * Start the CPUs OpTiMSoC system.
 *
 * The CPUs are being held in reset state after a system reset (e.g. after
 * calling optimsoc_connect()). Call this function to start the system.
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_cpu_start(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.cpu_start(ctx->backend_ctx);
}

/**
 * Run system discovery
 *
 * The system discovery process determines the system identifier and all
 * modules attached to the debug system. This includes e.g. the address of the
 * NCM module, which is used later to send lisnoc32 packets.
 *
 * Run this function after connecting to the system.
 *
 * \param ctx the library context
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_discover_system(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.discover_system(ctx->backend_ctx);
}


/**
 * Initialize a memory tile with data
 *
 * This function overwrites all existing memory contents with the new data,
 * starting from address 0x00.
 *
 * \param ctx         the library context
 * \param mem_tile_id ID of the memory tile to write the data to
 * \param data        the data to use for initialization
 * \param data_len    length of the data to write
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_mem_init(struct optimsoc_ctx *ctx, int mem_tile_id,
                      const uint8_t* data, int data_len)
{
    int res;

    if (data_len % 4 != 0) {
        /* see the implementation in lisnoc_dma_target.v */
        err(ctx->log_ctx, "DMA writes are only supported in 4-byte blocks, "
                          "tried to write %d bytes.\n", data_len);
        return -1;
    }

    /* stall CPUs while writing memory */
    res = optimsoc_cpu_stall(ctx, 1);
    if (res < 0) {
        err(ctx->log_ctx, "Unable to stall CPUs.\n");
        return -1;
    }

    int nr_of_writes = 1 + ((data_len - 1) / MEM_INIT_MAX_BYTES);
    int remaining_bytes = data_len;
    for (int i = 0; i < nr_of_writes; i++) {
        int len;
        if (i == nr_of_writes - 1) {
            len = remaining_bytes;
        } else {
            len = MEM_INIT_MAX_BYTES;
        }
        res = optimsoc_mem_write(ctx, mem_tile_id, i*MEM_INIT_MAX_BYTES,
                                 &data[i*MEM_INIT_MAX_BYTES],
                                 len);
        if (res < 0) {
            err(ctx->log_ctx,
                "Unable to complete DMA write %i. CPUs remain stalled!\n", i);
            return -1;
        }
        remaining_bytes -= len;
    }


    /* reset and un-stall CPUs */
    res = optimsoc_cpu_reset(ctx);
    if (res < 0) {
        err(ctx->log_ctx, "Unable to reset CPUs.\n");
        return -1;
    }
    res = optimsoc_cpu_stall(ctx, 0);
    if (res < 0) {
        err(ctx->log_ctx, "Unable to un-stall CPUs.\n");
        return -1;
    }

    return 0;
}

/**
 * Write data into a memory tile
 *
 * The DMA interface supports only writes in 4-byte steps.
 *
 * The data is written with DMA requests over the LISNoC, which is accessed
 * through the Debug NoC. This requires the debug system to contain a NCM
 * module.
 *
 * The CPUs are stalled while the memory write is happening, and reset after
 * the memory is successfully written.
 *
 * \param ctx           library context
 * \param mem_tile_id   memory tile id to write the data to
 * \param base_address  base address of the write
 * \param data          data to write
 * \param data_len      number of bytes to write
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_mem_write(struct optimsoc_ctx *ctx, int mem_tile_id,
                       int base_address, const uint8_t* data, int data_len)
{
    return ctx->backend_call.mem_write(ctx->backend_ctx, mem_tile_id,
                                       base_address, data, data_len);
}

/**
 * Reset the whole system
 *
 * This is done using a special control message, that triggers custom code
 * in the ztex firmware, that in turn triggers the a reset pin of the FPGA.
 * All send/receive FIFOs of the FX2 chip are cleared as well.
 *
 * \param ctx library context
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_reset(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.reset(ctx->backend_ctx);
}

/**
 * Get the System ID of the system
 *
 * \see optimsoc_discover_system()
 *
 * \return a negative value indicates an error, a value >= 0 is the system id
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_get_sysid(struct optimsoc_ctx *ctx)
{
    struct optimsoc_sysinfo *sysinfo = NULL;
    int rv = ctx->backend_call.get_sysinfo(ctx->backend_ctx, &sysinfo);
    if (rv < 0) {
        return rv;
    }
    if (sysinfo == NULL) {
        err(ctx->log_ctx, "Run optimsoc_system_discover() first!\n");
        return -1;
    }
    return sysinfo->sysid;
}

/**
 * Get all modules in the debug system
 *
 * \see optimsoc_discover_system()
 *
 * \return 0 if successful, a value < 0 indicates an error
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_get_modules(struct optimsoc_ctx *ctx,
                         struct optimsoc_dbg_module **modules,
                         int *module_count)
{
    struct optimsoc_sysinfo *sysinfo;
    int rv = ctx->backend_call.get_sysinfo(ctx->backend_ctx, &sysinfo);
    if (rv < 0) {
        return rv;
    }
    if (sysinfo == NULL) {
        err(ctx->log_ctx, "Run optimsoc_system_discover() first!\n");
        return -1;
    }

    *modules = sysinfo->dbg_modules;
    *module_count = sysinfo->dbg_module_count;
    return 0;
}

/**
 * Get the name for a module with a given type identifier
 *
 * This name is useful only for displaying it, as it might change without
 * notice. The resulting name is not localized.
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
char* optimsoc_get_module_name(int module_type)
{
    switch (module_type) {
    case MODULE_TYPE_CTM:
        return "CTM";
    case MODULE_TYPE_ITM:
        return "ITM";
    case MODULE_TYPE_NRM:
        return "NRM";
    case MODULE_TYPE_NCM:
        return "NCM";
    case MODULE_TYPE_STM:
        return "STM";
    default:
        return "unknown";
    }
}

/**
 * Stall/unstall all CPUs in the system
 *
 * \param ctx library context
 * \param do_stall 1 = stall; 0 = unstall
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_cpu_stall(struct optimsoc_ctx *ctx, int do_stall)
{
    return ctx->backend_call.cpu_stall(ctx->backend_ctx, do_stall);
}

/**
 * Reset all CPUs in the system
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_cpu_reset(struct optimsoc_ctx *ctx)
{
    return ctx->backend_call.cpu_reset(ctx->backend_ctx);
}

/**
 * Register a callback function to receive instruction traces from the ITM
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_itm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_itm_cb cb)
{
    return ctx->backend_call.itm_register_callback(ctx->backend_ctx, cb);
}

/**
 * Register a callback function to receive router monitoring data from NRM
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_nrm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_nrm_cb cb)
{
    return ctx->backend_call.nrm_register_callback(ctx->backend_ctx, cb);
}

/**
 * Register a callback function to receive trace messages from the STM
 *
 * \ingroup highlevel
 */
OPTIMSOC_EXPORT
int optimsoc_stm_register_callback(struct optimsoc_ctx *ctx,
                                   optimsoc_stm_cb cb)
{
    return ctx->backend_call.stm_register_callback(ctx->backend_ctx, cb);
}

/**
 * Set the sample interval for all NRM modules
 *
 * \param ctx library context
 * \param sample_interval sample interval in clock cycles, set 0 to disable NRMs
 */
OPTIMSOC_EXPORT
int optimsoc_nrm_set_sample_interval(struct optimsoc_ctx *ctx,
                                     int sample_interval)
{
    return ctx->backend_call.nrm_set_sample_interval(ctx->backend_ctx,
                                                     sample_interval);
}

OPTIMSOC_EXPORT
int optimsoc_read_clkstats(struct optimsoc_ctx *ctx, uint32_t *sys_clk,
                           uint32_t *sys_clk_halted)
{
    return ctx->backend_call.read_clkstats(ctx->backend_ctx, sys_clk,
                                           sys_clk_halted);
}
