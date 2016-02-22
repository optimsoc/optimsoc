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

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "log.h"

/**
 * Logging context
 */
struct optimsoc_log_ctx {
    /** logging function */
    optimsoc_log_fn log_fn;
    /** logging priority */
    int log_priority;
};

void optimsoc_log(struct optimsoc_log_ctx *ctx,
                  int priority, const char *file, int line, const char *fn,
                  const char *format, ...)
{
    va_list args;

    va_start(args, format);
    ctx->log_fn(ctx, priority, file, line, fn, format, args);
    va_end(args);
}

void optimsoc_vlog(struct optimsoc_log_ctx *ctx,
                   int priority, const char *file, int line, const char *fn,
                   const char *format, va_list args)
{
    ctx->log_fn(ctx, priority, file, line, fn, format, args);
}

/**
 * Default logging function: log to STDERR
 */
static void log_stderr(struct optimsoc_log_ctx *ctx,
                       int priority, const char *file, int line, const char *fn,
                       const char *format, va_list args)
{
    fprintf(stderr, "liboptimsochost: %s: ", fn);
    vfprintf(stderr, format, args);
}

static int log_priority(const char *priority)
{
    char *endptr;
    int prio;

    prio = strtol(priority, &endptr, 10);
    if (endptr[0] == '\0' || isspace(endptr[0]))
        return prio;
    if (strncmp(priority, "err", 3) == 0)
        return LOG_ERR;
    if (strncmp(priority, "info", 4) == 0)
        return LOG_INFO;
    if (strncmp(priority, "debug", 5) == 0)
        return LOG_DEBUG;
    return 0;
}


int optimsoc_log_new(struct optimsoc_log_ctx **ctx)
{
    const char *env;
    struct optimsoc_log_ctx *c;

    c = calloc(1, sizeof(struct optimsoc_log_ctx));
    if (!c)
        return -ENOMEM;

    c->log_fn = log_stderr;
    c->log_priority = LOG_ERR;

    /* environment overwrites config */
    env = getenv("OPTIMSOC_LOG");
    if (env != NULL) {
        c->log_priority = log_priority(env);
    }

    *ctx = c;
    dbg(c, "log_priority=%d\n", c->log_priority);

    return 0;
}

void optimsoc_log_set_log_fn(struct optimsoc_log_ctx *ctx,
                             optimsoc_log_fn log_fn)
{
    ctx->log_fn = log_fn;
    info(ctx, "custom logging function %p registered\n", log_fn);
}

int optimsoc_log_get_priority(struct optimsoc_log_ctx *ctx)
{
    return ctx->log_priority;
}

void optimsoc_log_set_priority(struct optimsoc_log_ctx *ctx, int priority)
{
    ctx->log_priority = priority;
}
