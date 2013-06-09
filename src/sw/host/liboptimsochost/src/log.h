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

#ifndef _LOG_H_
#define _LOG_H_

#include <syslog.h>

#include <optimsochost/liboptimsochost.h>

static inline void __attribute__((always_inline, format(printf, 2, 3)))
optimsoc_log_null(struct optimsoc_log_ctx *ctx, const char *format, ...) {}

#define optimsoc_log_cond(ctx, prio, arg...) \
  do { \
    if (optimsoc_log_get_priority(ctx) >= prio) \
      optimsoc_log(ctx, prio, __FILE__, __LINE__, __FUNCTION__, ## arg); \
  } while (0)

#ifdef LOGGING
#  ifdef DEBUG
#    define dbg(ctx, arg...) optimsoc_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg(ctx, arg...) optimsoc_log_null(ctx, ## arg)
#  endif
#  define info(ctx, arg...) optimsoc_log_cond(ctx, LOG_INFO, ## arg)
#  define err(ctx, arg...) optimsoc_log_cond(ctx, LOG_ERR, ## arg)
#else
#  define dbg(ctx, arg...) optimsoc_log_null(ctx, ## arg)
#  define info(ctx, arg...) optimsoc_log_null(ctx, ## arg)
#  define err(ctx, arg...) optimsoc_log_null(ctx, ## arg)
#endif

int optimsoc_log_new(struct optimsoc_log_ctx **ctx);

void optimsoc_log(struct optimsoc_log_ctx *ctx,
                  int priority, const char *file, int line, const char *fn,
                  const char *format, ...)
                  __attribute__((format(printf, 6, 7)));

void optimsoc_log_set_log_fn(struct optimsoc_log_ctx *ctx,
                             optimsoc_log_fn log_fn);
int optimsoc_log_get_priority(struct optimsoc_log_ctx *ctx);
void optimsoc_log_set_priority(struct optimsoc_log_ctx *ctx, int priority);

#endif /* _LOG_H_ */
