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
