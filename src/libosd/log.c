/* Copyright 2017 The Open SoC Debug Project
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

#include <osd/osd.h>
#include "osd-private.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

/**
 * Default log priority if not set otherwise
 */
#define LOG_PRIORITY_DEFAULT LOG_ERR

/**
 * Logging context
 */
struct osd_log_ctx {
    /** logging function */
    osd_log_fn log_fn;
    /** logging priority */
    int log_priority;
    /** caller context */
    void *caller_ctx;
    /** log mutex */
    pthread_mutex_t lock;
};

void osd_vlog(struct osd_log_ctx *ctx, int priority, const char *file, int line,
              const char *fn, const char *format, va_list args)
{
    if (!ctx->log_fn) {
        return;
    }

    // make thread doing the logging uncancellable while holding the lock to
    // avoid deadlocks if a thread is cancelled while creating a log entry
    int old_cancelstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate);

    pthread_mutex_lock(&ctx->lock);
    ctx->log_fn(ctx, priority, file, line, fn, format, args);
    pthread_mutex_unlock(&ctx->lock);

    pthread_setcancelstate(old_cancelstate, &old_cancelstate);
}

void osd_log(struct osd_log_ctx *ctx, int priority, const char *file, int line,
             const char *fn, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    osd_vlog(ctx, priority, file, line, fn, format, args);
    va_end(args);
}

API_EXPORT
osd_result osd_log_new(struct osd_log_ctx **ctx, const int log_priority,
                       osd_log_fn log_fn)
{
    struct osd_log_ctx *c;

    c = calloc(1, sizeof(struct osd_log_ctx));
    assert(c);

    c->log_fn = log_fn;

    if (!log_priority) {
        c->log_priority = LOG_PRIORITY_DEFAULT;
    } else {
        c->log_priority = log_priority;
    }

    pthread_mutex_init(&c->lock, NULL);

    *ctx = c;
    return OSD_OK;
}

API_EXPORT
void osd_log_free(struct osd_log_ctx **ctx_p)
{
    assert(ctx_p);

    struct osd_log_ctx *ctx;
    ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    pthread_mutex_destroy(&ctx->lock);
    free(ctx);

    *ctx_p = NULL;
}

API_EXPORT
void osd_log_set_fn(struct osd_log_ctx *ctx, osd_log_fn log_fn)
{
    ctx->log_fn = log_fn;
}

API_EXPORT
int osd_log_get_priority(struct osd_log_ctx *ctx) { return ctx->log_priority; }

API_EXPORT
void osd_log_set_priority(struct osd_log_ctx *ctx, int priority)
{
    ctx->log_priority = priority;
}

API_EXPORT
void osd_log_set_caller_ctx(struct osd_log_ctx *ctx, void *caller_ctx)
{
    ctx->caller_ctx = caller_ctx;
}

API_EXPORT
void *osd_log_get_caller_ctx(struct osd_log_ctx *ctx)
{
    return ctx->caller_ctx;
}
