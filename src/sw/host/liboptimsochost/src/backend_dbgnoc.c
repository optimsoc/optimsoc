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

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "backend_dbgnoc.h"
#include "backend_dbgnoc_usb.h"
#include "backend_dbgnoc_tcp.h"

const int NUM_SAMPLES_PER_TRANSFER = 256;

/**
 * Maximum count of flits per packet for the NCM.
 *
 * When sending data over the NCM to the lisnoc32, a lisnoc32 packet can contain
 * at most this number of 32 bit flits.
 *
 * Must be the same as FIFO_DEPTH_16to32 in ncm.v.
 * XXX: Read this value from a NCM register.
 */
const int NCM_MAX_LISNOC32_FLITS_PER_PKG = 16;

/**
 * Maximum number of flits per packet in the Debug NoC
 * Keep this in sync with MAX_DBGNOC_TO_USB_PACKET_LENGTH in dbg_config.vh!
 */
const int LISNOC16_MAX_FLITS_PER_PKG = 32;

/**
 * Timeout in seconds to wait for an acknowledge after DMA writes
 */
const int MEM_WRITE_ACK_TIMEOUT = 60;

/**
 * Timeout in seconds for register reads
 */
const int REGISTER_READ_TIMEOUT = 60;
/**
 * Expect the answer for a register read request with in this number of
 * packets, otherwise the read is considered to have failed.
 */
const int REGISTER_READ_MAXPKGS = 100000;

/**
 * Mask to filter out the CLASS part of a lisnoc16 header flit
 */
const unsigned int FLIT16_HEADER_CLASS_MASK = 0x0700;
/**
 * Position of the CLASS part in a lisnoc16 header flit
 */
const unsigned int FLIT16_HEADER_CLASS_POS = 8;

/**
 * Mask to filter out the CLASS part of a lisnoc32 header flit
 */
const unsigned int FLIT32_HEADER_CLASS_MASK = 0x07000000;
/**
 * Position of the CLASS part in a lisnoc32 header flit
 */
const unsigned int FLIT32_HEADER_CLASS_POS = 24;
/**
 * Debug NoC CLASS: Register read request
 * Keep this in sync with dbg_config.vh
 */
const unsigned int DBG_NOC_CLASS_REG_READ_REQ = 0x0;
/**
 * Debug NoC CLASS: Register read response
 * Keep this in sync with dbg_config.vh
 */
const unsigned int DBG_NOC_CLASS_REG_READ_RESP = 0x1;
/**
 * Debug NoC CLASS: Register write request
 * Keep this in sync with dbg_config.vh
 */
const unsigned int DBG_NOC_CLASS_REG_WRITE_REQ = 0x2;
/**
 * Debug NoC CLASS: encapsulated lisnoc32 packets (from/to NCM)
 * Keep this in sync with dbg_config.vh
 */
const unsigned int DBG_NOC_CLASS_NCM = 0x7;
/**
 * Debug NoC CLASS: instruction trace data
 */
const unsigned int DBG_NOC_CLASS_ITM = 0x4;
/**
 * Debug NoC CLASS: software trace data
 */
const unsigned int DBG_NOC_CLASS_STM = 0x3;
/**
 * Debug NoC CLASS: NoC router monitoring data
 */
const unsigned int DBG_NOC_CLASS_NRM = 0x5;
/**
 * NoC CLASS: DMA transfer
 */
const unsigned int NOC_CLASS_DMA = 0x2;
/**
 * NoC CLASS: control message
 */
const unsigned int NOC_CLASS_CONTROLMSG = 0x7;

typedef enum {
    OPTIMSOC_DBGNOC_USB,
    OPTIMSOC_DBGNOC_TCP
} ob_dbgnoc_connection_id;

struct ob_dbg_noc_connection_ctx;

struct optimsoc_backend_ctx {
    struct optimsoc_log_ctx *log_ctx;

    /** ITM callback */
    optimsoc_itm_cb itm_cb;
    /** NRM callback */
    optimsoc_nrm_cb nrm_cb;
    /** STM callback */
    optimsoc_stm_cb stm_cb;

    /** address of the NCM module in the Debug NoC (lisnoc16) */
    int ncm_dbgnoc_addr;
    /** address of the NCM module in the LISNoC (lisnoc32) */
    int ncm_lisnoc_addr;
    /** receive thread */
    pthread_t receive_thread;
    /** receive thread attributes */
    pthread_attr_t receive_thread_attr;

    /** received lisnoc16 packet */
    struct lisnoc16_packet rcv_lisnoc16_pkg;
    /** should a consumer be informed about new rcv_lisnoc16_pkg? */
    int signal_rcv_lisnoc16_pkg;
    /** semaphore: a new rcv_lisnoc16_pkg is available */
    sem_t sem_rcv_lisnoc16_pkg_available;
    /** semaphore: the processing of the rcv_lisnoc16_pkg is done */
    sem_t sem_rcv_lisnoc16_pkg_processed;

    /** received lisnoc32 packet */
    struct lisnoc32_packet rcv_lisnoc32_pkg;

    /** optimsoc_discover_system() has been run */
    int system_discovery_done;
    /** system information */
    struct optimsoc_sysinfo *sysinfo;

    /** Connection context */
    struct ob_dbgnoc_connection_ctx *conn_ctx;

    struct ob_dbgnoc_connection_interface *conn_calls;
};

int ob_dbgnoc_new(struct optimsoc_backend_ctx **ctx,
                  struct optimsoc_backend_interface *calls,
                  struct optimsoc_log_ctx *log_ctx,
                  int num_options,
                  struct optimsoc_backend_option options[]) {

    int i, rv;
    struct optimsoc_backend_ctx *c;
    ob_dbgnoc_connection_id connid = OPTIMSOC_DBGNOC_USB; /* USB is default */

    /* extract the connection type from the options */
    for (i = 0; i < num_options; i++) {
        if (strcmp(options[i].name, "conn") == 0) {
            // Connection type is set in options
            if (strcmp(options[i].value, "usb") == 0) {
                connid = OPTIMSOC_DBGNOC_USB;
            } else if (strcmp(options[i].value, "tcp") == 0) {
                connid = OPTIMSOC_DBGNOC_TCP;
            } else {
                info(log_ctx, "Unknown connection option %s, using the "
                     "USB connection (conn=usb) instead.\n", options[i].value);
            }
        }
    }

    c = calloc(1, sizeof(struct optimsoc_backend_ctx));
    if (!c) {
        return -ENOMEM;
    }
    *ctx = c;

    c->log_ctx = log_ctx;
    c->sysinfo = NULL;

    /* set the common calls */
    calls->cpu_stall = &ob_dbgnoc_cpu_stall;
    calls->cpu_reset = &ob_dbgnoc_cpu_reset;
    calls->itm_register_callback = &ob_dbgnoc_itm_register_callback;
    calls->nrm_register_callback = &ob_dbgnoc_nrm_register_callback;
    calls->stm_register_callback = &ob_dbgnoc_stm_register_callback;
    calls->nrm_set_sample_interval = &ob_dbgnoc_nrm_set_sample_interval;
    calls->read_clkstats = &ob_dbgnoc_read_clkstats;
    calls->free = &ob_dbgnoc_free;
    calls->connect = &ob_dbgnoc_connect;
    calls->disconnect = &ob_dbgnoc_disconnect;
    calls->connected = &ob_dbgnoc_connected;
    calls->reset = &ob_dbgnoc_reset;
    calls->discover_system = &ob_dbgnoc_discover_system;
    calls->get_sysinfo = &ob_dbgnoc_get_sysinfo;
    calls->cpu_start = &ob_dbgnoc_cpu_start;
    calls->mem_write = &ob_dbgnoc_mem_write;
    calls->itm_refresh_config = &ob_dbgnoc_itm_refresh_config;

    c->conn_calls = calloc(1, sizeof(struct ob_dbgnoc_connection_interface));

    /* initialize the connection context */
    switch (connid) {
    case OPTIMSOC_DBGNOC_USB:
        rv = ob_dbgnoc_usb_new(&c->conn_ctx, c->conn_calls, log_ctx,
                               num_options, options);
        break;
    case OPTIMSOC_DBGNOC_TCP:
        rv = ob_dbgnoc_tcp_new(&c->conn_ctx, c->conn_calls, log_ctx,
                               num_options, options);
        break;
    default:
        rv = -1;
        break;
    }

    return rv;
}

int ob_dbgnoc_free(struct optimsoc_backend_ctx *ctx)
{
    ctx->conn_calls->free(ctx->conn_ctx);

    if (ctx->rcv_lisnoc16_pkg.flit_data) {
        free(ctx->rcv_lisnoc16_pkg.flit_data);
        ctx->rcv_lisnoc16_pkg.flit_data = NULL;
    }

    if (ctx->rcv_lisnoc32_pkg.flit_data) {
        free(ctx->rcv_lisnoc32_pkg.flit_data);
        ctx->rcv_lisnoc32_pkg.flit_data = NULL;
    }

    optimsoc_sysinfo_free(ctx->sysinfo);
    ctx->sysinfo = NULL;

    return 0;
}

int ob_dbgnoc_connect(struct optimsoc_backend_ctx *ctx)
{
    int rv;

    rv = ctx->conn_calls->connect(ctx->conn_ctx);
    if (rv < 0) {
        return rv;
    }

    /* Initialize semaphores used to communicate between the receiving thread
     * and the processing functions when receiving a Debug NoC packet. */
    ctx->signal_rcv_lisnoc16_pkg = 0;
    rv = sem_init(&ctx->sem_rcv_lisnoc16_pkg_available, 0, 0);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to create sem_rcv_lisnoc16_pkg_available");
        return -1;
    }
    rv = sem_init(&ctx->sem_rcv_lisnoc16_pkg_processed, 0, 0);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to create sem_rcv_lisnoc16_pkg_available");
        return -1;
    }

    /* start receiving thread */
    pthread_attr_init(&ctx->receive_thread_attr);
    pthread_attr_setdetachstate(&ctx->receive_thread_attr,
                                PTHREAD_CREATE_DETACHED);

    rv = pthread_create(&ctx->receive_thread, &ctx->receive_thread_attr,
                        receive_thread, (void*)ctx);
    if (rv) {
        err(ctx->log_ctx, "Unable to create receiving thread: %d\n", rv);
        return -1;
    }

    return 0;
}

int ob_dbgnoc_disconnect(struct optimsoc_backend_ctx *ctx)
{
    void *status;

    /* clean-up receiving thread */
    pthread_cancel(ctx->receive_thread);
    pthread_join(ctx->receive_thread, &status);
    pthread_attr_destroy(&ctx->receive_thread_attr);

    sem_destroy(&ctx->sem_rcv_lisnoc16_pkg_available);
    sem_destroy(&ctx->sem_rcv_lisnoc16_pkg_processed);
    /* XXX: This cannot be non-zero at this point, but better safe than sorry! */
    ctx->signal_rcv_lisnoc16_pkg = 0;

    int rv = ctx->conn_calls->disconnect(ctx->conn_ctx);
    if (rv < 0) {
        return rv;
    }

    return 0;
}

int ob_dbgnoc_connected(struct optimsoc_backend_ctx *ctx)
{
    return ctx->conn_calls->connected(ctx->conn_ctx);
}

int ob_dbgnoc_reset(struct optimsoc_backend_ctx *ctx)
{
    ctx->conn_calls->reset(ctx->conn_ctx);
    return 0;
}

/**
 * Start the CPUs OpTiMSoC system.
 *
 * The CPUs are being held in reset state after a system reset (e.g. after
 * calling optimsoc_connect()). Call this function to start the system.
 */
int ob_dbgnoc_cpu_start(struct optimsoc_backend_ctx *ctx)
{
    uint16_t data;
    data = (1 << 4); /* bit 4: start CPUs (RU) */
    return register_write(ctx, DBG_NOC_ADDR_TCM, 3, 1, &data);
}

int ob_dbgnoc_discover_system(struct optimsoc_backend_ctx *ctx)
{
    uint16_t data[4];
    int res;

    res = register_read(ctx, DBG_NOC_ADDR_TCM, 0, 3, data);
    if (res < 0) {
        err(ctx->log_ctx, "Register read to get system information from TCM failed.\n");
        return -1;
    }

    if (data[0] != 0) {
        err(ctx->log_ctx, "System/TCM version %d not supported.\n", data[0]);
        return -1;
    }

    struct optimsoc_sysinfo *sysinfo;
    sysinfo = calloc(1, sizeof(struct optimsoc_sysinfo));
    if (!sysinfo) {
        return -ENOMEM;
    }

    sysinfo->sysid = data[1];
    sysinfo->dbg_module_count = data[2];
    dbg(ctx->log_ctx, "Discovered a system with ID 0x%04x and %d debug modules\n",
        sysinfo->sysid, sysinfo->dbg_module_count);

    sysinfo->dbg_modules = calloc(sysinfo->dbg_module_count,
                                  sizeof(struct optimsoc_dbg_module));

    for (int addr = DBG_NOC_ADDR_TCM+1;
         addr <= DBG_NOC_ADDR_TCM + sysinfo->dbg_module_count; addr++) {
        res = register_read(ctx, addr, 0, 1, data);
        if (res < 0) {
            err(ctx->log_ctx, "Unable to get module information from module "
                              "at address 0x%x.\n", addr);
            return -1;
        }

        int module_type, module_version;
        module_type = (data[0] >> 8) & 0xFF;
        module_version = data[0] & 0xFF;

        sysinfo->dbg_modules[addr-DBG_NOC_ADDR_TCM-1].module_type = module_type;
        sysinfo->dbg_modules[addr-DBG_NOC_ADDR_TCM-1].module_version = module_version;
        sysinfo->dbg_modules[addr-DBG_NOC_ADDR_TCM-1].dbgnoc_addr = addr;

        dbg(ctx->log_ctx, "Found module of type 0x%x and version 0x%x at "
                          "address 0x%x\n",
            module_type, module_version, addr);

        if (module_type == OPTIMSOC_MODULE_TYPE_NCM) {
            ctx->ncm_dbgnoc_addr = addr;

            res = register_read(ctx, addr, 1, 1, data);
            if (res < 0) {
                return -1;
            }

            ctx->ncm_lisnoc_addr = data[0];
            dbg(ctx->log_ctx, "Got lisnoc32 address for NCM: %d, dbgnoc addr "
                              "is %d\n",
                ctx->ncm_lisnoc_addr, ctx->ncm_dbgnoc_addr);
        }
    }

    /*
     * Allocate memory for the ITM and MAM configurations.
     *
     * We use the address in the Debug NoC as index for faster lookups.
     * dbg_module_count contains the number of debug modules *in addition* to
     * the TCM, so the highest address in the Debug NoC is
     * |DBG_NOC_ADDR_TCM + dbg_module_count|. We also need to allocate space for
     * address 0 (the external, i.e. USB or TCP, interface), thus the +1 below.
     */
    sysinfo->itm_config = calloc(sysinfo->dbg_module_count + DBG_NOC_ADDR_TCM + 1,
                                 sizeof(struct optimsoc_itm_config*));
    sysinfo->mam_config = calloc(sysinfo->dbg_module_count + DBG_NOC_ADDR_TCM + 1,
                                 sizeof(struct optimsoc_mam_config*));

    optimsoc_sysinfo_free(ctx->sysinfo);
    ctx->sysinfo = sysinfo;

    /* retrieve additional configuration for the individual modules */
    if (ob_dbgnoc_mam_get_config(ctx) < 0) {
        info(ctx->log_ctx, "Unable to retrieve MAM configuration.\n");
    }

    return 0;
}

int ob_dbgnoc_get_sysinfo(struct optimsoc_backend_ctx *ctx,
                          struct optimsoc_sysinfo **sysinfo)
{
    *sysinfo = ctx->sysinfo;
    return 0;
}

int ob_dbgnoc_mem_write(struct optimsoc_backend_ctx *ctx,
                        unsigned int memory_id, unsigned int base_address,
                        const uint8_t* data, unsigned int data_len)
{
    int rv = 0;
    int data_send_idx = 0;
    int remaining_bytes = data_len;

    dbg(ctx->log_ctx, "Attempting to send %d bytes of data to address 0x%x of "
                      "memory %d.\n", data_len, base_address, memory_id);

    /* get Debug NoC address for Memory ID */
    int module_addr = -1;
    for (int i = 0; i < ctx->sysinfo->dbg_module_count; i++) {
        if (ctx->sysinfo->dbg_modules[i].module_type != OPTIMSOC_MODULE_TYPE_MAM) {
            continue;
        }
        if (ctx->sysinfo->mam_config[ctx->sysinfo->dbg_modules[i].dbgnoc_addr]->memory_id == memory_id) {
            module_addr = ctx->sysinfo->dbg_modules[i].dbgnoc_addr;
        }
    }
    if (module_addr == -1) {
        err(ctx->log_ctx, "Unable to find a MAM module for the memory with ID %d.\n", memory_id);
        return -1;
    }

    dbg(ctx->log_ctx, "Writing memory %d through MAM at Debug NoC address %d", memory_id, module_addr);

    /* calculate the number of required packets and allocate memory for it */
    int bytes_per_pkg = (LISNOC16_MAX_FLITS_PER_PKG - 3 * 2);
    bytes_per_pkg -= bytes_per_pkg % 4; /* ceil to full words */
    dbg(ctx->log_ctx, "Transferring %d bytes per packet\n", bytes_per_pkg);

    /* ceil(data_len/bytes_per_pkg) with integers */
    int number_of_pkgs = 1 + ((data_len - 1) / bytes_per_pkg);
    struct lisnoc16_packet* packets;
    packets = calloc(number_of_pkgs, sizeof(struct lisnoc16_packet));
    if (!packets) {
        rv = -ENOMEM;
        goto free_return;
    }

    for (int i = 0; i < number_of_pkgs; i++) {
        int flits_in_pkg;
        if (remaining_bytes < bytes_per_pkg) {
            /* 3 flits = header, address MSB and LSB */
            flits_in_pkg = 3 + (remaining_bytes / 2);
        } else {
            flits_in_pkg = 3 + (bytes_per_pkg / 2);
        }
        packets[i].len = flits_in_pkg;

        uint16_t* flit_data = calloc(flits_in_pkg, sizeof(uint16_t));
        if (!flit_data) {
            rv = -ENOMEM;
            goto free_return;
        }
        packets[i].flit_data = flit_data;

        /*
         * MAM header flit
         *
         * ----------------------------------------
         * | DEST[15:11] | CLASS[10:8] | TYPE [7:0]
         * ----------------------------------------
         *
         * DEST      = module_addr
         * CLASS     = 0b111 (HACK! we "reuse" this class as it does not matter
         *             much when sending the data to a specific endpoint, it's
         *             only important when receiving data from different
         *             sources)
         * TYPE      = 0x00: write
         */
        int next_write_addr = base_address + data_send_idx;
        flit_data[0] = ((module_addr & 0x1F) << 11) |
                       (0x7 << 8) |
                       (0x00 << 0);

        /* address (MSB) */
        flit_data[1] = next_write_addr >> 16;
        /* address (LSB) */
        flit_data[2] = next_write_addr & 0xFFFF;

        /* data flits */
        for (int j = 0; j < flits_in_pkg - 3; j++) {
            flit_data[j+3] = data[data_send_idx] << 8 |
                             data[data_send_idx+1];
            data_send_idx += 2;
            remaining_bytes -= 2;
        }
    }

    if (lisnoc16_send_packets(ctx, packets, number_of_pkgs) < 0) {
        err(ctx->log_ctx, "Memory write operation failed.\n");
        rv = -1;
        goto free_return;
    }

free_return:
    if (packets) {
        for (int i=0; i<number_of_pkgs; i++) {
            if (packets[i].flit_data) {
                free(packets[i].flit_data);
            }
        }
        free(packets);
    }
    return rv;
}

int ob_dbgnoc_cpu_stall(struct optimsoc_backend_ctx *ctx, int do_stall)
{
    uint16_t data = 0;

    if (do_stall) {
        data = (1 << 0); /* bit 0: stall */
    } else {
        data = (1 << 1); /* bit 1: unstall */
    }

    return register_write(ctx, DBG_NOC_ADDR_TCM, 3, 1, &data);
}

int ob_dbgnoc_cpu_reset(struct optimsoc_backend_ctx *ctx)
{
    uint16_t data = 0;
    data = (1 << 2); /* bit 2: reset */

    return register_write(ctx, DBG_NOC_ADDR_TCM, 3, 1, &data);
}

/**
 * Thread: receive data
 */
void* receive_thread(void* ctx_void)
{
    struct optimsoc_backend_ctx *ctx = ctx_void;
    int rv;
    uint16_t buffer_rx[NUM_SAMPLES_PER_TRANSFER * 2];

    int read_pos = NUM_SAMPLES_PER_TRANSFER;
    int process_pkgs;


    ctx->rcv_lisnoc16_pkg.len = 0;
    ctx->rcv_lisnoc16_pkg.flit_data = calloc(NUM_SAMPLES_PER_TRANSFER,
                                             sizeof(uint16_t));

    ctx->rcv_lisnoc32_pkg.len = 0;
    ctx->rcv_lisnoc32_pkg.flit_data = calloc(NCM_MAX_LISNOC32_FLITS_PER_PKG,
                                             sizeof(uint32_t));

    while (1) {
        /* clear buffer */
        for (int i=NUM_SAMPLES_PER_TRANSFER; i<2*NUM_SAMPLES_PER_TRANSFER; i++) {
            buffer_rx[i] = 0;
        }

        rv = ctx->conn_calls->read_fn(ctx->conn_ctx,
                                      buffer_rx + NUM_SAMPLES_PER_TRANSFER,
                                      NUM_SAMPLES_PER_TRANSFER);

        if (rv < 0) {
            /* probably a timeout, i.e. no data to read. try again. */
            continue;
        }

#ifdef DEBUG_DUMP_DATA
        fprintf(stderr, "Received %d data bytes from backend:\n  ", rv);
        for (int i=NUM_SAMPLES_PER_TRANSFER; i<2*NUM_SAMPLES_PER_TRANSFER; i++) {
            fprintf(stderr, "%04x  ", buffer_rx[i]);
            if ((i-NUM_SAMPLES_PER_TRANSFER) % 12 == 11) fprintf(stderr, "\n  ");
        }
        fprintf(stderr, "\n");
#endif

        /* create lisnoc16 packets from data stream */
        int packet_len = 0;
        process_pkgs = 1;
        do {
            packet_len = buffer_rx[read_pos];

            if (read_pos + packet_len > NUM_SAMPLES_PER_TRANSFER * 2 - 1) {
                if (read_pos < NUM_SAMPLES_PER_TRANSFER) {
                    err(ctx->log_ctx, "Received packet is longer than one "
                            "transfer, i.e. %d flits. This is currently not "
                            "supported.\n",
                        NUM_SAMPLES_PER_TRANSFER);
                    /* XXX: What do we want to do here? */
                    return 0;
                }

                /*
                 * We need to read a new block before continuing with this
                 * packet. Copy this block to the first half of the buffer and
                 * adjust the position pointers. The new block will be written
                 * to the second half of the buffer.
                 */
                for (int copy_pos=0; copy_pos<NUM_SAMPLES_PER_TRANSFER;
                     copy_pos++) {
                    buffer_rx[copy_pos] = buffer_rx[copy_pos + NUM_SAMPLES_PER_TRANSFER];
                }
                read_pos -= NUM_SAMPLES_PER_TRANSFER;

                process_pkgs = 0;
                continue;
            }

            if (packet_len < 1) {
                /*
                 * this flit is just padding-data to fill up a burst
                 */
            } else {

#ifdef DEBUG_DUMP_DATA
                fprintf(stderr, "Received lisnoc16 packet with %d flits:\n  ",
                        packet_len);
                for (int j=read_pos+1; j<read_pos+1+packet_len; j++) {
                    fprintf(stderr, "%04x  ", buffer_rx[j]);
                    if (j-read_pos-1 % 12 == 11) fprintf(stderr, "\n  ");
                }
                fprintf(stderr, "\n");
#endif

                /* check for a lisnoc32 packet */
                uint8_t class = (buffer_rx[read_pos+1] &
                                 FLIT16_HEADER_CLASS_MASK) >> 8;
                if (class == DBG_NOC_CLASS_NCM) {
                    /* encapsulated lisnoc32 packet */
                    dbg(ctx->log_ctx, "Processing lisnoc32 packet.\n");
                    if (packet_len % 2 != 1) {
                        err(ctx->log_ctx, "Invalid number of flits in "
                                          "encapsulated lisnoc32 packet: %d\n",
                            packet_len);
                    } else {
                        int packet32_len = (packet_len - 1) / 2;
                        if (packet32_len > NCM_MAX_LISNOC32_FLITS_PER_PKG) {
                            err(ctx->log_ctx, "lisnoc32 packet with %d more "
                                              "than %d flits received. This is "
                                              "not supported.\n",
                                packet32_len,
                                NCM_MAX_LISNOC32_FLITS_PER_PKG);
                            return 0;
                        }
                        ctx->rcv_lisnoc32_pkg.len = packet32_len;

                        int flit_pos = 0;
                        for (int j=read_pos+2; j<read_pos+1+packet_len; j+=2) {
                            ctx->rcv_lisnoc32_pkg.flit_data[flit_pos++] = (buffer_rx[j] << 16) |
                                                                          buffer_rx[j+1];
                        }
                    }

                    unsigned int headerflit = ctx->rcv_lisnoc32_pkg.flit_data[0];
                    unsigned int class32 = (headerflit >> 24) & 0x7;

                    if (class32 == NOC_CLASS_CONTROLMSG) {
                        unsigned int source = (headerflit >> 19) & 0x1f;
                        unsigned int message = headerflit & 0xff;
                        printf("Received control message from %d: 0x%02x\n",
                               source, message);
                    }
                } else {
                    /* regular lisnoc16 packet */
                    dbg(ctx->log_ctx, "Processing lisnoc16 packet.\n");

                    if (packet_len > NUM_SAMPLES_PER_TRANSFER) {
                        err(ctx->log_ctx, "Debug NoC packet with %d more than "
                            "%d flits received. This is not supported.\n",
                            packet_len,
                            NUM_SAMPLES_PER_TRANSFER);
                        return 0;
                    }
                    ctx->rcv_lisnoc16_pkg.len = packet_len;

                    for (int j=read_pos+1; j<read_pos+1+packet_len; j++) {
                        ctx->rcv_lisnoc16_pkg.flit_data[j-read_pos-1] = buffer_rx[j];
                    }

                    /* filter out ITM packets */
                    if (ctx->itm_cb && ctx->rcv_lisnoc16_pkg.len == 6 &&
                        class == DBG_NOC_CLASS_ITM) {

                        int core = ctx->rcv_lisnoc16_pkg.flit_data[0] & 0x00FF;
                        uint32_t ts = ctx->rcv_lisnoc16_pkg.flit_data[1] << 16 |
                                      ctx->rcv_lisnoc16_pkg.flit_data[2];
                        uint32_t pc = ctx->rcv_lisnoc16_pkg.flit_data[3] << 16 |
                                      ctx->rcv_lisnoc16_pkg.flit_data[4];
                        int count = ctx->rcv_lisnoc16_pkg.flit_data[5];
                        ctx->itm_cb(core, ts, pc, count);
                    }

                    /* filter out NRM packets */
                    if (ctx->nrm_cb && ctx->rcv_lisnoc16_pkg.len >= 4 &&
                        class == DBG_NOC_CLASS_NRM) {
                        int router = ctx->rcv_lisnoc16_pkg.flit_data[0] & 0x00FF;
                        uint32_t ts = ctx->rcv_lisnoc16_pkg.flit_data[1] << 16 |
                                      ctx->rcv_lisnoc16_pkg.flit_data[2];
                        /* each flit contains the flit_count of two links */
                        int monitored_links = (ctx->rcv_lisnoc16_pkg.len-3)*2;
                        uint8_t *link_flit_count;
                        link_flit_count = calloc(monitored_links, sizeof(uint8_t));
                        /* XXX: add OOM check */
                        int j = 0;
                        for (int k = 3; k < ctx->rcv_lisnoc16_pkg.len; k++) {
                            link_flit_count[j++] = (ctx->rcv_lisnoc16_pkg.flit_data[k] >> 8) & 0xFF;
                            link_flit_count[j++] = ctx->rcv_lisnoc16_pkg.flit_data[k] & 0xFF;
                        }
                        ctx->nrm_cb(router, ts, link_flit_count, monitored_links);
                        free(link_flit_count);
                    }

                    if (ctx->stm_cb && ctx->rcv_lisnoc16_pkg.len == 6 &&
                        class == DBG_NOC_CLASS_STM) {

                        uint8_t core_id = ctx->rcv_lisnoc16_pkg.flit_data[0] & 0x00FF;
                        uint32_t timestamp = ctx->rcv_lisnoc16_pkg.flit_data[1] << 16 | ctx->rcv_lisnoc16_pkg.flit_data[2];
                        uint32_t value = ctx->rcv_lisnoc16_pkg.flit_data[3] << 16 | ctx->rcv_lisnoc16_pkg.flit_data[4];
                        uint16_t id = ctx->rcv_lisnoc16_pkg.flit_data[5];

                        ctx->stm_cb(core_id, timestamp, id, value);
                    }

                    /* allow the consumer thread to process the packet as well */
                    if (ctx->signal_rcv_lisnoc16_pkg) {
                        sem_post(&ctx->sem_rcv_lisnoc16_pkg_available);
                        sem_wait(&ctx->sem_rcv_lisnoc16_pkg_processed);
                    }
                }
            }

            read_pos += packet_len + 1; /* jump to next packet */
        } while (process_pkgs);
    }

    free(ctx->rcv_lisnoc16_pkg.flit_data);
    free(ctx->rcv_lisnoc32_pkg.flit_data);

    pthread_exit(NULL);
    return 0;
}

int ob_dbgnoc_itm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_itm_cb cb)
{
    ctx->itm_cb = cb;
    return 0;
}

int ob_dbgnoc_nrm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_nrm_cb cb)
{
    ctx->nrm_cb = cb;
    return 0;
}

int ob_dbgnoc_stm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_stm_cb cb)
{
    ctx->stm_cb = cb;
    return 0;
}


/**
 * Read a register from a module in the debug system (blocking).
 *
 * The caller needs to allocate sufficient memory for burst_len entries in
 * \p data.
 *
 * \param ctx          backend context
 * \param module_addr  module address in the Debug NoC to read from
 * \param reg_addr     register to read
 * \param burst_len    number of 16-bit words to read (4 at most)
 * \param[out] data    read data
 *
 * \return 0 on success, a negative error code otherwise
 */
int register_read(struct optimsoc_backend_ctx *ctx, int module_addr,
                  int reg_addr, int burst_len, uint16_t *data)
{
    uint16_t flit_data;

    if (burst_len > 4) {
        err(ctx->log_ctx, "Only bursts up to 4 words are possible.\n");
        return -1;
    }

    /*
     * Notify the receiving thread that we want to be informed if a new packet
     * arrives.
     */
    ctx->signal_rcv_lisnoc16_pkg = 1;

    struct lisnoc16_packet packets[1];
    packets[0].flit_data = &flit_data;
    packets[0].len = 1;

    /*
     * A register read is a single-flit packet:
     *
     * --------------------------------------------------------------
     * | DEST[15:11] | CLASS[10:8] | REG_ADDR[7:2] | BURST_LEN[1:0] |
     * --------------------------------------------------------------
     *
     * DEST      = module_addr
     * CLASS     = 0b000 (register read)
     * REG_ADDR  = reg_addr
     * BURST_LEN = burst_len - 1 (BURST_LEN = 0 means "read 1 word")
     */
    flit_data = 0x0000;
    flit_data = ((module_addr & 0x1F) << 11) |
                ((reg_addr & 0x3F) << 2) |
                ((burst_len - 1) & 0x03);
    if (lisnoc16_send_packets(ctx, packets, 1) != 0) {
        err(ctx->log_ctx, "Unable to send register read request.\n");
        return -1;
    }

    /* process response */
    for (int i = 0; i < REGISTER_READ_MAXPKGS; i++) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += REGISTER_READ_TIMEOUT;

        int rv = sem_timedwait(&ctx->sem_rcv_lisnoc16_pkg_available, &ts);
        if (rv < 0 && errno == ETIMEDOUT) {
            err(ctx->log_ctx, "Response for register read not received within "
                "%d seconds. Timing out.\n", REGISTER_READ_TIMEOUT);

            ctx->signal_rcv_lisnoc16_pkg = 0;
            return -ETIMEDOUT;
        }

        uint8_t class = (ctx->rcv_lisnoc16_pkg.flit_data[0] &
                         FLIT16_HEADER_CLASS_MASK) >> 8;
        if (ctx->rcv_lisnoc16_pkg.len == burst_len + 1 &&
            class == DBG_NOC_CLASS_REG_READ_RESP) {

            info(ctx->log_ctx, "Received register read response.\n");
            for (int flit=1; flit<=burst_len; flit++) {
                data[flit-1] = ctx->rcv_lisnoc16_pkg.flit_data[flit];
            }
#ifdef DEBUG_DUMP_DATA
            fprintf(stderr, "Received register read response:\n  ");
            for (int flit=1; flit<=burst_len; flit++) {
                fprintf(stderr, "%04x  ", ctx->rcv_lisnoc16_pkg.flit_data[flit]);
            }
            fprintf(stderr, "\n");
#endif

            /* received read response, it's all good */
            ctx->signal_rcv_lisnoc16_pkg = 0;
            sem_post(&ctx->sem_rcv_lisnoc16_pkg_processed);
            return 0;
        }

        if (i == REGISTER_READ_MAXPKGS - 1) {
            /*
             * This is the last try, don't get informed for new packets.
             * This needs happen while the thread is waiting for the semaphore
             * to be posted!
             */
            ctx->signal_rcv_lisnoc16_pkg = 0;
        }
        sem_post(&ctx->sem_rcv_lisnoc16_pkg_processed);
    }

    err(ctx->log_ctx, "Did not receive the register read response within "
        "%d packets. Giving up.\n", REGISTER_READ_MAXPKGS);
    return -1;
}

int register_write(struct optimsoc_backend_ctx *ctx, int module_addr,
                   int reg_addr, int burst_len, const uint16_t *data)
{
    uint16_t* flit_data;
    struct lisnoc16_packet packets[1];

    flit_data = calloc(burst_len + 1, sizeof(uint16_t));
    if (!flit_data) {
        return -ENOMEM;
    }

    packets[0].flit_data = flit_data;
    packets[0].len = burst_len + 1;

    /*
     * A register write is a multi-flit packet, starting with this header:
     *
     * -----------------------------------------------------------
     * | DEST[15:11] | CLASS[10:8] | REG_ADDR[7:2] | UNUSED[1:0] |
     * -----------------------------------------------------------
     *
     * DEST      = module_addr
     * CLASS     = 0b010 (register write request)
     * REG_ADDR  = reg_addr
     */
    flit_data[0] = ((module_addr & 0x1F) << 11) |
                   (DBG_NOC_CLASS_REG_WRITE_REQ << 8) |
                   ((reg_addr & 0x3F) << 2);

    /*
     * data to write (writes start at reg_addr and increment by one with every
     * following flit
     */
    for (int i=0; i<burst_len; i++) {
        flit_data[i+1] = data[i];
    }

    if (lisnoc16_send_packets(ctx, packets, 1) < 0) {
        free(flit_data);
        return -1;
    }

    free(flit_data);
    return 0;
}


/**
 * Send packets to the Debug NoC (lisnoc16)
 */
int lisnoc16_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc16_packet packets[], int length)
{
    int length_transfer;
    int length_transfer_rounded;
    uint16_t* data_transfer;
    int rv;


#ifdef DEBUG_DUMP_DATA
    fprintf(stderr, "Sending %d lisnoc16 packets:\n", length);
    for (int i=0; i<length; i++) {
        fprintf(stderr, "packet %d, %d flits:\n  ", i, packets[i].len);
        for (int j=0; j<packets[i].len; j++) {
            fprintf(stderr, "%04x  ", packets[i].flit_data[j]);
            if (j % 12 == 11) fprintf(stderr, "\n  ");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    length_transfer = 0;
    for (int i=0; i<length; i++) {
        /* the additional word is used for the packet length */
        length_transfer += packets[i].len + 1;
    }

    /*
     * we can only transfer in blocks of NUM_SAMPLES_PER_TRANSFER bytes
     */
    length_transfer_rounded = ((length_transfer /
                                NUM_SAMPLES_PER_TRANSFER)
                               + 1) * NUM_SAMPLES_PER_TRANSFER;

    data_transfer = calloc(length_transfer_rounded, sizeof(uint16_t));
    if (!data_transfer) {
        rv = -ENOMEM;
        goto free_return;
    }

    /* copy flits into output buffer */
    int pos = 0;
    for (int i=0; i<length; i++) {
        data_transfer[pos++] = packets[i].len;

        for (int j=0; j<packets[i].len; j++) {
            data_transfer[pos++] = packets[i].flit_data[j];
        }
    }

    dbg(ctx->log_ctx, "Transferring %d samples over backend for %d payload "
                      "words.\n", length_transfer_rounded, length_transfer);


#ifdef DEBUG_DUMP_DATA
    fprintf(stderr, "Sending %d samples:\n  ", length_transfer_rounded);
    for (int i=0; i<length_transfer_rounded; i++) {
        fprintf(stderr, "%04x  ", data_transfer[i]);
        if (i % 12 == 11) fprintf(stderr, "\n  ");
    }
    fprintf(stderr, "\n");
#endif

    for (int i = 0; i < length_transfer_rounded / NUM_SAMPLES_PER_TRANSFER; i++) {
        int ret = ctx->conn_calls->write_fn(ctx->conn_ctx,
                                            &data_transfer[i*NUM_SAMPLES_PER_TRANSFER],
                                            NUM_SAMPLES_PER_TRANSFER);

        if (ret < 0) {
            err(ctx->log_ctx, "Transfer failed.\n");
            rv = -1;
            goto free_return;
        }
    }

    rv = 0;

free_return:
    if (data_transfer) {
        free(data_transfer);
    }
    return rv;
}

/**
 * Send packets to the LISNoC (lisnoc32)
 *
 * These packets are wrapped inside lisnoc16 packets and sent over the Debug
 * NoC.
 * A lisnoc32 packet can consist of at most NCM_MAX_LISNOC32_FLITS_PER_PKG
 * flits.
 */
int lisnoc32_send_packets(struct optimsoc_backend_ctx *ctx,
                          struct lisnoc32_packet packets[], int length)
{
    struct lisnoc16_packet* lisnoc16_packets;
    int lisnoc16_length;
    int rv;

    if (ctx->ncm_dbgnoc_addr == -1) {
        err(ctx->log_ctx, "Address of NCM module in the Debug NoC is not set. "
                          "Run optimsoc_discover_system() first and ensure "
                          "that the target system contains an NCM module.\n");
        return -1;
    }

    /*
     * 1 lisnoc32 packet results in 1 lisnoc16 packet
     */
    lisnoc16_length = length;
    lisnoc16_packets = calloc(lisnoc16_length, sizeof(struct lisnoc16_packet));
    if (!lisnoc16_packets) {
        rv = -ENOMEM;
        goto free_return;
    }

    dbg(ctx->log_ctx, "Sending %d packets over the LISNoC.\n", length);

#ifdef DEBUG_DUMP_DATA
    fprintf(stderr, "Sending %d lisnoc32 packets:\n", length);
    for (int i=0; i<length; i++) {
        fprintf(stderr, "packet %d, %d flits:\n  ", i, packets[i].len);
        for (int j=0; j<packets[i].len; j++) {
            fprintf(stderr, "%08x  ", packets[i].flit_data[j]);
            if (j % 7 == 6) fprintf(stderr, "\n  ");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    for (int i=0; i<length; i++) {

        if (packets[i].len > NCM_MAX_LISNOC32_FLITS_PER_PKG) {
            err(ctx->log_ctx, "A LISNoC packet can consist of at most %d "
                              "lisnoc32 flits, %d found.\n",
                NCM_MAX_LISNOC32_FLITS_PER_PKG, packets[i].len);
            return -1;
        }

        /*
         * The lisnoc16 packet consists of
         * a) one header flit
         * b) all flits of the lisnoc32 packet, split into two lisnoc16 flits
         */
        int packet16_flit_count = packets[i].len * 2 + 1;

        uint16_t* packet16;
        packet16 = calloc(packet16_flit_count, sizeof(uint16_t));
        if (!packet16) {
            rv = -ENOMEM;
            goto free_return;
        }

        int pos = 0;
        /*
         * Header flit of lisnoc16 packet
         * ------------------------------------------------
         * | DEST[15:11] | CLASS[10:8] | HEADER_DATA[7:0] |
         * ------------------------------------------------
         *
         * DEST = ctx->ncm_dbgnoc_addr
         * CLASS = DBG_NOC_CLASS_NCM
         * HEADER_DATA = 0x00
         */
        packet16[pos++] = (ctx->ncm_dbgnoc_addr & 0x1F) << 11 |
                          (DBG_NOC_CLASS_NCM << 8);

        /*
         * Take all flits from the lisnoc32 packet and split it in two
         * lisnoc16 flits.
         */
        for (int j=0; j<packets[i].len; j++) {
            packet16[pos++] = (packets[i].flit_data[j] >> 16) & 0x0000FFFF;
            packet16[pos++] = packets[i].flit_data[j] & 0x0000FFFF;
        }

        lisnoc16_packets[i].len = packet16_flit_count;
        lisnoc16_packets[i].flit_data = packet16;
    }

    lisnoc16_send_packets(ctx, lisnoc16_packets, lisnoc16_length);

    rv = 0;

free_return:
    if (lisnoc16_packets) {
        for (int i=0; i<lisnoc16_length; i++) {
            if (lisnoc16_packets[i].flit_data) {
                free(lisnoc16_packets[i].flit_data);
            }
        }
        free(lisnoc16_packets);
    }

    return rv;
}

/**
 * Set the sample interval for all NRM modules
 *
 * \param ctx library context
 * \param sample_interval sample interval in clock cycles, set 0 to disable NRMs
 */
int ob_dbgnoc_nrm_set_sample_interval(struct optimsoc_backend_ctx *ctx,
                                      int sample_interval)
{
    assert(ctx->sysinfo);

    if (sample_interval > 255) {
        info(ctx->log_ctx, "Overflows in NRM might happen with a sample "
                           "interval > 255!\n");
    } else if (sample_interval > 65536) {
        err(ctx->log_ctx, "Sample interval is only a 16-bit value!\n");
        return -1;
    }

    for (int i=0; i<ctx->sysinfo->dbg_module_count; i++) {
        if (ctx->sysinfo->dbg_modules[i].module_type == OPTIMSOC_MODULE_TYPE_NRM) {
            register_write(ctx, ctx->sysinfo->dbg_modules[i].dbgnoc_addr,
                           2, 1, (uint16_t*)&sample_interval);
        }
    }
    return 0;
}

/**
 * Read the clock statistics from the system
 */
int ob_dbgnoc_read_clkstats(struct optimsoc_backend_ctx *ctx, uint32_t *sys_clk,
                            uint32_t *sys_clk_halted)
{
    uint16_t data[4];
    data[0] = 0;
    data[0] = (1 << 3); /* bit 3: sample clock stat counters */

    int rv = register_write(ctx, DBG_NOC_ADDR_TCM, 3, 1, data);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to sample system clock counters\n");
        return -1;
    }

    rv = register_read(ctx, DBG_NOC_ADDR_TCM, 4, 4, data);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to read clock statistics from TCM.\n");
        return -1;
    }

    *sys_clk_halted = (data[0] << 16) | data[1];
    *sys_clk = (data[2] << 16) | data[3];

    return 0;
}

/**
 * Refresh the configuration of a single Instruction Trace Module (ITM)
 */
int ob_dbgnoc_itm_refresh_config(struct optimsoc_backend_ctx *ctx,
                                 struct optimsoc_dbg_module *dbg_module)
{
    uint16_t data_out[1];
    int dbgnoc_addr = dbg_module->dbgnoc_addr;
    int rv = register_read(ctx, dbgnoc_addr, 0x01, 1, data_out);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to read ITM configuration at Debug NoC "
            "address %d.\n", dbgnoc_addr);
        return -1;
    }

    if (ctx->sysinfo->itm_config[dbgnoc_addr] == NULL) {
        ctx->sysinfo->itm_config[dbgnoc_addr] = calloc(1, sizeof(struct optimsoc_itm_config));
    }

    ctx->sysinfo->itm_config[dbgnoc_addr]->core_id = data_out[0];
    return 0;
}


/**
 * Retrieve the configuration of all Memory Access Modules (MAM)
 */
int ob_dbgnoc_mam_get_config(struct optimsoc_backend_ctx *ctx)
{
    for (int i = 0; i < ctx->sysinfo->dbg_module_count; i++) {
        if (ctx->sysinfo->dbg_modules[i].module_type != OPTIMSOC_MODULE_TYPE_MAM) {
            continue;
        }

        uint16_t data_out[1];
        int dbgnoc_addr = ctx->sysinfo->dbg_modules[i].dbgnoc_addr;
        int rv = register_read(ctx, dbgnoc_addr, 0x01, 1, data_out);
        if (rv < 0) {
            err(ctx->log_ctx, "Unable to read MAM configuration at Debug NoC "
                "address %d.\n", dbgnoc_addr);
            return -1;
        }

        if (ctx->sysinfo->mam_config[dbgnoc_addr] == NULL) {
            ctx->sysinfo->mam_config[dbgnoc_addr] = calloc(1, sizeof(struct optimsoc_mam_config));
        }

        ctx->sysinfo->mam_config[dbgnoc_addr]->memory_id = data_out[0];
    }

    return 0;
}
