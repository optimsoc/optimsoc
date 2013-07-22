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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>

#include "backend_simtcp.h"

/**
 * Opaque object anonymous to liboptimsochost and defined here.
 * Stores the backend context.
 */
struct optimsoc_backend_ctx {
    struct optimsoc_log_ctx *log_ctx;

    pthread_t       receive_thread;
    pthread_mutex_t ctrl_msg_mutex;
    pthread_cond_t  ctrl_msg_cond;
    unsigned int    ctrl_msg_paylen;
    unsigned char   *ctrl_msg_data;

    char *hostname;
    int port;
    int socketfd;

    struct optimsoc_sysinfo *sysinfo;

    /** ITM callback */
    optimsoc_itm_cb itm_cb;
    /** NRM callback */
    optimsoc_nrm_cb nrm_cb;
    /** STM callback */
    optimsoc_stm_cb stm_cb;
};

struct message {
    unsigned char len;
    unsigned char type;
    unsigned char *payload;
};

#define MSGTYPE_SYSDISCOVER  0
#define MSGTYPE_SYSENUMERATE 1
#define MSGTYPE_SYSSTART     2
#define MSGTYPE_SYSRESET     3
#define MSGTYPE_CPUSTALL     4
#define MSGTYPE_MODSEND      5
#define MSGTYPE_MODREAD      6
#define MSGTYPE_TRACE        7

#define DBGTYPE_ITM 2
#define DBGTYPE_STM 5

void *ob_simtcp_receive_thread(void* ctx_void)
{
    struct optimsoc_backend_ctx *ctx = (struct optimsoc_backend_ctx*) ctx_void;
    int rv;

    assert(ctx->socketfd >= 0);

    while (1) {
        struct message *msg = malloc(sizeof(struct message));
        assert(msg != 0);

        rv = read(ctx->socketfd, msg, 2);

        if (rv <= 0) {
            err(ctx->log_ctx, "Connection closed or error with connection");
            return 0;
        }

        assert(rv == 2);

        msg->payload = malloc(msg->len - 2);
        rv = read(ctx->socketfd, msg->payload, msg->len - 2);

        assert(rv == msg->len - 2);

        uint8_t trtype;
        uint16_t traddr;

        switch (msg->type) {
        case MSGTYPE_SYSDISCOVER:
        case MSGTYPE_SYSENUMERATE:
        case MSGTYPE_SYSSTART:
            pthread_mutex_lock(&ctx->ctrl_msg_mutex);
            ctx->ctrl_msg_data = msg->payload;
            ctx->ctrl_msg_paylen = msg->len - 2;
            pthread_cond_signal(&ctx->ctrl_msg_cond);
            pthread_mutex_unlock(&ctx->ctrl_msg_mutex);
            break;
        case MSGTYPE_TRACE:
            trtype = (uint8_t) msg->payload[0];
            traddr = *((uint16_t*) &msg->payload[2]);
            if (trtype == DBGTYPE_STM && ctx->stm_cb) {
                uint32_t coreid, timestamp, value;
                uint16_t id;
                assert(msg->len - 6 == 14);
                memcpy(&coreid, &msg->payload[4], 4);
                memcpy(&timestamp, &msg->payload[8], 4);
                memcpy(&id, &msg->payload[12], 2);
                memcpy(&value, &msg->payload[14], 4);
                ctx->stm_cb(coreid, timestamp, id, value);
            }
            break;
        default:
            err(ctx->log_ctx, "Unknown packet (type=%02x). Drop it.\n",
                msg->type);
            break;
        }
    }
    return 0;
}

int ob_simtcp_new(struct optimsoc_backend_ctx **ctx,
                  struct optimsoc_backend_interface *calls,
                  struct optimsoc_log_ctx *log_ctx, int num_options,
                  struct optimsoc_backend_option options[])
{
    struct optimsoc_backend_ctx *c;

    c = calloc(1, sizeof(struct optimsoc_backend_ctx));
    if (!c) {
        return -ENOMEM;
    }

    c->log_ctx = log_ctx;

    calls->free = &ob_simtcp_free;
    calls->connect = &ob_simtcp_connect;
    calls->disconnect = &ob_simtcp_disconnect;
    calls->connected = &ob_simtcp_connected;
    calls->discover_system = &ob_simtcp_discover_system;
    calls->get_sysinfo = &ob_simtcp_get_sysinfo;
    calls->reset = &ob_simtcp_reset;
    calls->cpu_start = &ob_simtcp_cpu_start;
    calls->cpu_stall = &ob_simtcp_cpu_stall;
    calls->cpu_reset = &ob_simtcp_cpu_reset;
    calls->mem_write = &ob_simtcp_mem_write;
    calls->itm_register_callback = &ob_simtcp_itm_register_callback;
    calls->nrm_register_callback = &ob_simtcp_nrm_register_callback;
    calls->stm_register_callback = &ob_simtcp_stm_register_callback;
    calls->nrm_set_sample_interval = &ob_simtcp_nrm_set_sample_interval;
    calls->read_clkstats = &ob_simtcp_read_clkstats;
    calls->itm_refresh_config = &ob_simtcp_itm_refresh_config;

    *ctx = c;

    c->hostname = "localhost";
    c->port = 22000;

    // Search options for non-default options
    int i;
    for (i = 0; i < num_options; i++) {
        if (strcmp(options[i].name, "host") == 0) {
            c->hostname = strdup(options[i].value);
        } else if (strcmp(options[i].name, "port") == 0) {
            c->port = atoi(options[i].value);
        }
    }

    c->socketfd = -1;

    pthread_mutex_init(&c->ctrl_msg_mutex, NULL);
    pthread_cond_init(&c->ctrl_msg_cond, NULL);

    return 0;
}

int ob_simtcp_free(struct optimsoc_backend_ctx *ctx)
{
    free(ctx);
    ctx = 0;
    return 0;
}

int ob_simtcp_connect(struct optimsoc_backend_ctx *ctx)
{
    int rv;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    ctx->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->socketfd < 0) {
        err(ctx->log_ctx, "Error opening data socket\n");
        return -1;
    }

    server = gethostbyname(ctx->hostname);
    if (server == NULL) {
        err(ctx->log_ctx, "Name lookup for host %s failed.\n", ctx->hostname);
        return -1;
    }

    memset((void*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(ctx->port);

    /* connect to system */
    rv = connect(ctx->socketfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr));
    if (rv < 0) {
        err(ctx->log_ctx, "Cannot connect to system\n");
        return -1;
    }

    pthread_create(&ctx->receive_thread, NULL, ob_simtcp_receive_thread,
                   (void*) ctx);

    return 0;
}

int ob_simtcp_disconnect(struct optimsoc_backend_ctx *ctx)
{
    pthread_cancel(ctx->receive_thread);
    int rv = close(ctx->socketfd);
    ctx->socketfd = -1;
    if (rv != 0) {
        err(ctx->log_ctx, "Error when closing socket.\n");
    }
    return rv;
}

int ob_simtcp_connected(struct optimsoc_backend_ctx *ctx)
{
    return ctx->socketfd != -1;
}

int ob_simtcp_cpu_start(struct optimsoc_backend_ctx *ctx)
{
    int rv;
    struct timespec ts;
    uint8_t buf[2];
    // Lock access to the control message
    pthread_mutex_lock(&ctx->ctrl_msg_mutex);

    // Send start request
    buf[0] = 2;
    buf[1] = MSGTYPE_SYSSTART;
    rv = write(ctx->socketfd, buf, 2);
    assert(rv == 2);

    // Wait for response
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;
    rv = pthread_cond_timedwait(&ctx->ctrl_msg_cond, &ctx->ctrl_msg_mutex, &ts);
    assert(rv == 0);

    if (ctx->ctrl_msg_paylen == 0)
        rv = 0;
    else
        rv = -1;

    // Unlock access to the control message
    pthread_mutex_unlock(&ctx->ctrl_msg_mutex);

    return rv;
}

int ob_simtcp_discover_system(struct optimsoc_backend_ctx *ctx)
{
    int rv;
    struct timespec ts;
    char buf[2];

    // Lock access to the control message
    pthread_mutex_lock(&ctx->ctrl_msg_mutex);

    // Send system discovery request
    buf[0] = 2;
    buf[1] = MSGTYPE_SYSDISCOVER;
    rv = write(ctx->socketfd, buf, 2);
    assert(rv == 2);

    // Wait for system discovery response
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;
    rv = pthread_cond_timedwait(&ctx->ctrl_msg_cond, &ctx->ctrl_msg_mutex, &ts);
    if (rv != 0) {
        if (rv == ETIMEDOUT) {
            err(ctx->log_ctx, "Timed out while waiting for system discovery "
                "response.\n");
        } else {
            err(ctx->log_ctx, "Error while waiting for system discovery "
                "response.\n");
        }
        return -1;
    }

    // Parse info from payload
    struct optimsoc_sysinfo *sysinfo;
    sysinfo = malloc(sizeof(struct optimsoc_sysinfo));
    assert(sysinfo != 0);
    sysinfo->sysid = ((uint8_t) ctx->ctrl_msg_data[4] << 8) |
                     ((uint8_t) ctx->ctrl_msg_data[5]);
    sysinfo->dbg_module_count = ctx->ctrl_msg_data[2] << 8 |
                                ctx->ctrl_msg_data[3];
    sysinfo->dbg_modules = calloc(sysinfo->dbg_module_count,
                                  sizeof(struct optimsoc_dbg_module));
    ctx->sysinfo = sysinfo;

    // Free payload
    free(ctx->ctrl_msg_data);

    // Send system enumeration
    buf[0] = 2;
    buf[1] = MSGTYPE_SYSENUMERATE;

    rv = write(ctx->socketfd, buf, 2);
    assert(rv == 2);

    // Wait for system discovery response
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;
    rv = pthread_cond_timedwait(&ctx->ctrl_msg_cond, &ctx->ctrl_msg_mutex, &ts);
    assert(rv == 0);

    // Parse info from payload
    assert(ctx->ctrl_msg_paylen == (ctx->sysinfo->dbg_module_count * 6));

    uint16_t *payload_short = (uint16_t*) ctx->ctrl_msg_data;
    for (int i = 0; i < ctx->sysinfo->dbg_module_count; i++) {
        ctx->sysinfo->dbg_modules[i].dbgnoc_addr = payload_short[i * 3 + 0];
        ctx->sysinfo->dbg_modules[i].module_type = payload_short[i * 3 + 1];
        ctx->sysinfo->dbg_modules[i].module_version = payload_short[i * 3 + 2];
    }

    // Unlock access to control message
    pthread_mutex_unlock(&ctx->ctrl_msg_mutex);

    return 0;
}

int ob_simtcp_get_sysinfo(struct optimsoc_backend_ctx *ctx,
                          struct optimsoc_sysinfo **sysinfo)
{
    *sysinfo = ctx->sysinfo;
    return 0;
}

int ob_simtcp_mem_write(struct optimsoc_backend_ctx *ctx, int mem_tile_id,
                        int base_address, const uint8_t* data, int data_len)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_reset(struct optimsoc_backend_ctx *ctx)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_cpu_stall(struct optimsoc_backend_ctx *ctx, int do_stall)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_cpu_reset(struct optimsoc_backend_ctx *ctx)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_itm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_itm_cb cb)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_nrm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_nrm_cb cb)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_stm_register_callback(struct optimsoc_backend_ctx *ctx,
                                    optimsoc_stm_cb cb)
{
    ctx->stm_cb = cb;
    return -1;
}

int ob_simtcp_nrm_set_sample_interval(struct optimsoc_backend_ctx *ctx,
                                      int sample_interval)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_read_clkstats(struct optimsoc_backend_ctx *ctx, uint32_t *sys_clk,
                            uint32_t *sys_clk_halted)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}

int ob_simtcp_itm_refresh_config(struct optimsoc_backend_ctx *ctx,
                                 struct optimsoc_dbg_module *dbg_module)
{
    err(ctx->log_ctx, "Not implemented!\n");
    return -1;
}
