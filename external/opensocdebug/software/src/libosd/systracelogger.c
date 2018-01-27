/* Copyright 2017-2018 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <osd/cl_stm.h>
#include <osd/module.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/systracelogger.h>
#include "osd-private.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

struct event_stats {
    unsigned int overflowed_events;
    unsigned int sysprint_events;
    unsigned int trace_events;
};

/**
 * System Trace Logger context
 */
struct osd_systracelogger_ctx {
    struct osd_hostmod_ctx *hostmod_ctx;
    struct osd_log_ctx *log_ctx;
    uint16_t stm_di_addr;
    struct osd_stm_desc stm_desc;
    struct osd_stm_event_handler stm_event_handler;
    FILE *fp_sysprint;
    FILE *fp_event;
    struct osd_cl_stm_print_buf sysprint_buf;
    struct event_stats stats;
};

static void stm_event_handler(void *ctx_void,
                              const struct osd_stm_desc *stm_desc,
                              const struct osd_stm_event *event)
{
    int rv;
    osd_result osd_rv;
    struct osd_systracelogger_ctx *ctx = ctx_void;

    // update stats
    if (event->overflow) {
        ctx->stats.overflowed_events += event->overflow;
    } else {
        ctx->stats.sysprint_events += osd_cl_stm_is_print_event(event);
        ctx->stats.trace_events += 1;
    }

    if (event->overflow) {
        if (ctx->fp_event) {
            rv = fprintf(ctx->fp_event, "Overflow, missed %u events\n",
                         event->overflow);
            if (rv < 0) {
                err(ctx->log_ctx, "Unable to write STM event to log file.");
            }
        }

        // XXX: handle overflow in sysprint (e.g. by newline and explicit flush)

        return;
    }

    if (ctx->fp_event) {
        rv = fprintf(ctx->fp_event, "%08x %04x %016lx\n", event->timestamp,
                     event->id, event->value);
        if (rv < 0) {
            err(ctx->log_ctx, "Unable to write STM event to log file.");
        }
    }

    if (ctx->fp_sysprint && osd_cl_stm_is_print_event(event)) {
        bool should_flush = false;
        osd_rv = osd_cl_stm_add_to_print_buf(event, &ctx->sysprint_buf,
                                             &should_flush);
        if (OSD_FAILED(osd_rv)) {
            return;
        }
        if (should_flush) {
            size_t b_wr = fwrite(ctx->sysprint_buf.buf, 1,
                                 ctx->sysprint_buf.len_str, ctx->fp_sysprint);
            if (b_wr != ctx->sysprint_buf.len_str) {
                err(ctx->log_ctx, "Unable to write %zu bytes to file.",
                    ctx->sysprint_buf.len_str);
                // not much more error handling we can do here
            }
            free(ctx->sysprint_buf.buf);
            ctx->sysprint_buf.buf = NULL;
            ctx->sysprint_buf.len_buf = 0;
            ctx->sysprint_buf.len_str = 0;
        }
    }
}

API_EXPORT
osd_result osd_systracelogger_new(struct osd_systracelogger_ctx **ctx,
                                  struct osd_log_ctx *log_ctx,
                                  const char *host_controller_address,
                                  uint16_t stm_di_addr)
{
    osd_result rv;

    struct osd_systracelogger_ctx *c =
        calloc(1, sizeof(struct osd_systracelogger_ctx));
    assert(c);

    c->log_ctx = log_ctx;
    c->stm_di_addr = stm_di_addr;
    c->stm_event_handler.cb_fn = stm_event_handler;
    c->stm_event_handler.cb_arg = (void *)c;
    c->stats.overflowed_events = 0;
    c->stats.trace_events = 0;
    c->stats.sysprint_events = 0;

    struct osd_hostmod_ctx *hostmod_ctx;
    rv =
        osd_hostmod_new(&hostmod_ctx, log_ctx, host_controller_address,
                        osd_cl_stm_handle_event, (void *)&c->stm_event_handler);
    assert(OSD_SUCCEEDED(rv));
    c->hostmod_ctx = hostmod_ctx;

    *ctx = c;

    return OSD_OK;
}

API_EXPORT
osd_result osd_systracelogger_connect(struct osd_systracelogger_ctx *ctx)
{
    return osd_hostmod_connect(ctx->hostmod_ctx);
}

API_EXPORT
osd_result osd_systracelogger_disconnect(struct osd_systracelogger_ctx *ctx)
{
    return osd_hostmod_disconnect(ctx->hostmod_ctx);
}

API_EXPORT
bool osd_systracelogger_is_connected(struct osd_systracelogger_ctx *ctx)
{
    return osd_hostmod_is_connected(ctx->hostmod_ctx);
}

API_EXPORT
void osd_systracelogger_free(struct osd_systracelogger_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_systracelogger_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    // Flush remaining sysprint data to file
    if (ctx->sysprint_buf.len_str > 0) {
        size_t b_wr = fwrite(ctx->sysprint_buf.buf, 1,
                             ctx->sysprint_buf.len_str, ctx->fp_sysprint);
        if (b_wr != ctx->sysprint_buf.len_str) {
            err(ctx->log_ctx, "Unable to write %zu bytes to file.",
                ctx->sysprint_buf.len_str);
            // not much more error handling we can do here
        }
        free(ctx->sysprint_buf.buf);
    }

    info(ctx->log_ctx, "Systracelogger statistics: %u overflowed packets, "
         "%u trace events, %u sysprint events", ctx->stats.overflowed_events,
         ctx->stats.trace_events, ctx->stats.sysprint_events);

    osd_hostmod_free(&ctx->hostmod_ctx);

    free(ctx);
    *ctx_p = NULL;
}

API_EXPORT
osd_result osd_systracelogger_start(struct osd_systracelogger_ctx *ctx)
{
    osd_result rv;

    rv =
        osd_cl_stm_get_desc(ctx->hostmod_ctx, ctx->stm_di_addr, &ctx->stm_desc);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    ctx->stm_event_handler.stm_desc = &ctx->stm_desc;

    rv = osd_hostmod_mod_set_event_dest(ctx->hostmod_ctx, ctx->stm_di_addr, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    rv = osd_hostmod_mod_set_event_active(ctx->hostmod_ctx, ctx->stm_di_addr,
                                          true, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    return OSD_OK;
}

API_EXPORT
osd_result osd_systracelogger_stop(struct osd_systracelogger_ctx *ctx)
{
    osd_result rv;
    rv = osd_hostmod_mod_set_event_active(ctx->hostmod_ctx, ctx->stm_di_addr,
                                          false, 0);
    if (rv == OSD_ERROR_TIMEDOUT) {
        rv = OSD_OK;
    }
    return rv;
}

API_EXPORT
osd_result osd_systracelogger_set_sysprint_log(
    struct osd_systracelogger_ctx *ctx, FILE *fp)
{
    ctx->fp_sysprint = fp;
    return OSD_OK;
}

API_EXPORT
osd_result osd_systracelogger_set_event_log(struct osd_systracelogger_ctx *ctx,
                                            FILE *fp)
{
    ctx->fp_event = fp;
    return OSD_OK;
}
