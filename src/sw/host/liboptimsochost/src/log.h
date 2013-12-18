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
