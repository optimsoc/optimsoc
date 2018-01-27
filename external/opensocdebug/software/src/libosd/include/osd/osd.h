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

#ifndef OSD_OSD_H
#define OSD_OSD_H

/**
 * Open SoC Debug Library (common parts)
 *
 * Data structures and functionality used across the layers of Open SoC Debug.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-errorhandling Error handling
 * @ingroup libosd
 *
 * @{
 */

/** Standard return type */
typedef int osd_result;

/** Return code: The operation was successful */
#define OSD_OK 0
/** Return code: Generic (unknown) failure */
#define OSD_ERROR_FAILURE -1
/** Return code: debug system returned a failure */
#define OSD_ERROR_DEVICE_ERROR -2
/** Return code: received invalid or malformed data from device */
#define OSD_ERROR_DEVICE_INVALID_DATA -3
/** Return code: failed to communicate with device */
#define OSD_ERROR_COM -4
/** Return code: operation timed out */
#define OSD_ERROR_TIMEDOUT -5
/** Return code: not connected to the device */
#define OSD_ERROR_NOT_CONNECTED -6
/** Return code: this is a partial result, not all requested data was obtained */
#define OSD_ERROR_PARTIAL_RESULT -7
/** Return code: operation aborted */
#define OSD_ERROR_ABORTED -8
/** Return code: connection failed */
#define OSD_ERROR_CONNECTION_FAILED -9
/** Return code: Out of memory */
#define OSD_ERROR_OOM -11
/** Return code: file operation failed */
#define OSD_ERROR_FILE -12
/** Return code: memory verification failed */
#define OSD_ERROR_MEM_VERIFY_FAILED -13
/** Return code: unexpected module type */
#define OSD_ERROR_WRONG_MODULE -14

/**
 * Return true if |rv| is an error code
 */
#define OSD_FAILED(rv) ((rv) < 0)
/**
 * Return true if |rv| is a successful return code
 */
#define OSD_SUCCEEDED(rv) ((rv) >= 0)

/**@}*/ /* end of doxygen group libosd-errorhandling */


/**
 * @defgroup libosd-log Log
 * @ingroup libosd
 *
 * @{
 */

/**
 * Opaque logging context
 */
struct osd_log_ctx;

/**
 * Logging function template
 *
 * Implement a function with this signature and pass it to set_log_fn()
 * if you want to implement custom logging.
 */
typedef void (*osd_log_fn)(struct osd_log_ctx *ctx, int priority,
                           const char *file, int line, const char *fn,
                           const char *format, va_list args);

/**
 * Create a new instance of osd_log_ctx
 *
 * You may use the resulting log context on multiple threads.
 *
 * @param ctx the logging context
 * @param log_priority filter: only log messages greater or equal the given
 *                     priority. Use on the LOG_* constants in stdlog.h
 *                     Set to 0 to use the default logging priority.
 * @param log_fn logging callback. Set to NULL to disable logging output.
 *
 * @see osd_log_set_priority()
 * @see osd_log_set_fn()
 */
osd_result osd_log_new(struct osd_log_ctx **ctx, int log_priority,
                       osd_log_fn log_fn);

/**
 * Free and NULL an osd_log_ctx object
 *
 * @param ctx_p    the log context
 */
void osd_log_free(struct osd_log_ctx **ctx_p);

/**
 * Set logging function
 *
 * The built-in logging writes to STDERR. It can be overridden by a custom
 * function to log messages into the user's logging functionality.
 *
 * In many cases you want the log message to be associated with a context or
 * object of your application, i.e. the object that uses OSD. In this
 * case, set the context or <code>this</code> pointer with
 * osd_log_set_caller_ctx() and retrieve it inside your @p log_fn.
 *
 * An example in C++ could look like this:
 *
 * @rst
 *
 * .. code-block:: c
 *
 *   static void MyClass::osdLogCallback(struct osd_log_ctx *gctx,
 *                                       int priority, const char *file,
 *                                       int line, const char *fn,
 *                                       const char *format, va_list args)
 *   {
 *     MyClass *myclassptr = static_cast<MyClass*>(osd_get_caller_ctx(gctx));
 *     myclassptr->doLogging(format, args);
 *   }
 *
 *   MyClass::MyClass()
 *   {
 *     // ...
 *     osd_log_set_caller_ctx(gctx, this);
 *     osd_log_set_fn(&MyClass::osdLogCallback);
 *     // ...
 *   }
 *
 *   MyClass::doLogging(const char*   format, va_list args)
 *   {
 *      printf("this = %p", this);
 *      vprintf(format, args);
 *   }
 * @endrst
 *
 *
 * @param ctx    the log context
 * @param log_fn the used logging function
 */
void osd_log_set_fn(struct osd_log_ctx *ctx, osd_log_fn log_fn);

/**
 * Get the logging priority
 *
 * The logging priority is the lowest message type that is reported.
 *
 * @param ctx the log context
 * @return the log priority
 *
 * @see osd_log_set_priority()
 */
int osd_log_get_priority(struct osd_log_ctx *ctx);

/**
 * Set the logging priority
 *
 * The logging priority is the lowest message type that is reported.
 *
 * Allowed values for @p priority are <code>LOG_DEBUG</code>,
 * <code>LOG_INFO</code> and <code>LOG_ERR</code> as defined in
 * <code>syslog.h</code>.
 *
 * For example setting @p priority will to <code>LOG_INFO</code> will result in
 * all error and info messages to be shown, and all debug messages to be
 * discarded.
 *
 * @param ctx       the log context
 * @param priority  new priority value
 *
 * @see osd_log_get_priority()
 */
void osd_log_set_priority(struct osd_log_ctx *ctx, int priority);

/**
 * Set a caller context pointer
 *
 * This library does not use this pointer in any way, you're free to set it to
 * whatever your application needs.
 *
 * @param ctx        the log context
 * @param caller_ctx the caller context pointer
 *
 * @see osd_log_get_caller_ctx()
 * @see osd_log_set_fn() for a code example using this functionality
 */
void osd_log_set_caller_ctx(struct osd_log_ctx *ctx, void *caller_ctx);

/**
 * Get the caller context pointer
 *
 * @param ctx the log context
 * @return the caller context pointer
 *
 * @see osd_log_set_caller_ctx()
 */
void *osd_log_get_caller_ctx(struct osd_log_ctx *ctx);

/**@}*/ /* end of doxygen group libosd-log */

/**
 * API version
 */
struct osd_version {
    /** major version */
    const uint16_t major;

    /** minor version */
    const uint16_t minor;

    /** micro version */
    const uint16_t micro;

    /**
     * suffix string, e.g. for release candidates ("-rc4") and development
     * versions ("-dev")
     */
    const char *suffix;
};

const struct osd_version *osd_version_get(void);

/**
 * Number of bits in the address used to describe the subnet
 */
#define OSD_DIADDR_SUBNET_BITS 6
#define OSD_DIADDR_LOCAL_BITS (16 - OSD_DIADDR_SUBNET_BITS)
#define OSD_DIADDR_SUBNET_MAX ((1 << OSD_DIADDR_SUBNET_BITS) - 1)
#define OSD_DIADDR_LOCAL_MAX ((1 << OSD_DIADDR_LOCAL_BITS) - 1)

unsigned int osd_diaddr_subnet(unsigned int diaddr);
unsigned int osd_diaddr_localaddr(unsigned int diaddr);
unsigned int osd_diaddr_build(unsigned int subnet, unsigned int local_diaddr);


// bit handling macros
#define BITS_PER_LONG  (sizeof(long) * 8)
#define BIT(nr)        (1UL << (nr))
#define BIT_MASK(nr)   (1UL << ((nr) % BITS_PER_LONG))

#ifdef __cplusplus
}
#endif

#endif  // OSD_OSD_H
