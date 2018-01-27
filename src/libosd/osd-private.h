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

#ifndef OSD_OSD_PRIVATE_H
#define OSD_OSD_PRIVATE_H

#include <osd/osd.h>
#include <pthread.h>
#include <czmq.h>

/*
 * Mark functions to be exported from the library as part of the API
 *
 * We set compiler options to mark all functions as hidden by default, causing
 * them to be private to the library. If a function is part of the API, you
 * must explicitly mark it with this macro.
 */
#define API_EXPORT __attribute__ ((visibility("default")))



/**
 * "Null" log handler: discard log messages
 */
static inline void __attribute__((always_inline, format(printf, 2, 3)))
osd_log_null(struct osd_log_ctx *ctx, const char *format, ...) {}

/**
 * Conditional logging: call osd_log only if priority is higher than
 * osd_log_get_priority()
 */
#define osd_log_cond(ctx, prio, arg...) \
  do { \
    if (ctx && osd_log_get_priority(ctx) >= prio) \
      osd_log(ctx, prio, __FILE__, __LINE__, __FUNCTION__, ## arg); \
  } while (0)

#ifdef LOGGING
#  ifdef DEBUG
#    define dbg(ctx, arg...) osd_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg(ctx, arg...) osd_log_null(ctx, ## arg)
#  endif
#  define info(ctx, arg...) osd_log_cond(ctx, LOG_INFO, ## arg)
#  define err(ctx, arg...) osd_log_cond(ctx, LOG_ERR, ## arg)
#else
#  define dbg(ctx, arg...) osd_log_null(ctx, ## arg)
#  define info(ctx, arg...) osd_log_null(ctx, ## arg)
#  define err(ctx, arg...) osd_log_null(ctx, ## arg)
#endif

/**
 * Log a message (taking a va_list argument)
 *
 * @see osd_log()
 */
void osd_vlog(struct osd_log_ctx *ctx, int priority, const char *file, int line,
              const char *fn, const char *format, va_list args);

/**
 * Log a message
 *
 * This calls the registered logging function to output (or possibly discard)
 * the log message.
 *
 * Don't use this function directly, use the dbg(), info() and err() macros
 * instead, which fill in all details for you (e.g. file name, line number,
 * etc.).
 *
 * Each call to osd_log() creates a self-contained log record. This has two
 * implications:
 * - Do not use repeated calls to osd_log() like printf(). Instead, assemble
 *   the full message first, and then call osd_log() to create a log entry.
 * - Do not add a newline character at the end of the message.
 *
 * This function is thread safe.
 *
 * @param ctx      the log context
 * @param priority the priority of the log message
 * @param file     the file the log message originates from (use __FILE__)
 * @param line     the line number the message originates from
 * @param fn       the C function the message originates from
 * @param format   the format string of the message (as used in printf() and
 *                 friends)
 *
 * @see dbg()
 * @see info()
 * @see err()
 */
void osd_log(struct osd_log_ctx *ctx,
             int priority, const char *file, int line, const char *fn,
             const char *format, ...)
             __attribute__((format(printf, 6, 7)));


static inline uint32_t __iter_div_u64_rem(uint64_t dividend, uint32_t divisor,
                                          uint64_t *remainder)
{
    uint32_t ret = 0;

    while (dividend >= divisor) {
        /* The following asm() prevents the compiler from
           optimising this loop into a modulo operation. */
        asm("" : "+rm"(dividend));
        dividend -= divisor;
        ret++;
    }
    *remainder = dividend;
    return ret;
}

#define NSEC_PER_SEC 1000000000L
static inline void timespec_add_ns(struct timespec *a, uint64_t ns)
{
    a->tv_sec += __iter_div_u64_rem(a->tv_nsec + ns, NSEC_PER_SEC, &ns);
    a->tv_nsec = ns;
}

/**
 * Timeout (in ms) when receiving data from a ZeroMQ socket. This can be either
 * data from the host controller, or data from the I/O thread.
 */
#define ZMQ_RCV_TIMEOUT (1*1000) // 1 s

/**
 * zframe_dup() taking a const argument
 */
static inline zframe_t* zframe_dup_c(const zframe_t *self)
{
    return zframe_dup((zframe_t*)self);
}

/**
 * zframe_eq() taking a const arguments
 */
static inline bool zframe_eq_c(const zframe_t *self, const zframe_t *other)
{
    return zframe_eq((zframe_t*)self, (zframe_t*)other);
}

/**
 * Maximum length of DI packets in words
 *
 * XXX: This should not be a constant but read from the SCM in the target
 * subnet.
 */
#define OSD_MAX_PKG_LEN_WORDS 8

/**
 * Ceiling integer devision
 */
#define INT_DIV_CEIL(x, y) 1 + (((x) - 1) / (y))

#endif // OSD_OSD_PRIVATE_H
