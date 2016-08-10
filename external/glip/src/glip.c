/* Copyright (c) 2014 by the author(s)
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
 * ============================================================================
 *
 * libglip is a flexible FIFO-based communication library between an FPGA and
 * a PC.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <libglip.h>
#include "glip-protected.h"
#include "glip-private.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Default logging function: log to STDERR
 *
 * @see glip_log()
 */
static void log_stderr(struct glip_ctx *ctx, int priority, const char *file,
                       int line, const char *fn, const char *format,
                       va_list args)
{
    fprintf(stderr, "glip: %s: ", fn);
    vfprintf(stderr, format, args);
}

/**
 * Get the log priority as integer for a priority name
 *
 * @param priority the priority name
 * @return the priority as integer
 */
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

static const struct glip_version glip_version_internal = {GLIP_VERSION_MAJOR,
        GLIP_VERSION_MINOR, GLIP_VERSION_MICRO, GLIP_VERSION_SUFFIX};

/**
 * Get the version of the GLIP library
 *
 * @return the library version information
 *
 * @ingroup utilities
 */
API_EXPORT
const struct glip_version * glip_get_version(void)
{
    return &glip_version_internal;
}

/**
 * Create a new GLIP context (constructor)
 *
 * Call this function to create a new glip library context for the backend
 * with ID @p backend_id. The resulting context is allocated by this function;
 * you need to free those resources with glip_free().
 *
 * You can specify the options to be used by the backend by passing those to
 * @p backend_options, and passing the number of entries in the
 * @p backend_options array to @p num_backend_options.
 *
 * @param[out] ctx          the newly created library context, or NULL if
 *                          something went wrong
 * @param[in]  backend_name the name of the used backend
 * @param[in]  backend_options options (key/value pairs) for the library and the
 *                          backend
 * @param[in]  num_backend_options  number of entries in the @p options array
 * @return     0 if the call was successful
 * @return     -GLIP_EUNKNOWNBACKEND the specified @p backend_name is not
 *             available
 * @return     any other value indicates an error
 *
 * @ingroup library-init-deinit
 */
API_EXPORT
int glip_new(struct glip_ctx **ctx, char* backend_name,
             struct glip_option backend_options[],
             size_t num_backend_options)
{
    struct glip_ctx *c = calloc(1, sizeof(struct glip_ctx));
    if (!c) {
        return -ENOMEM;
    }

    /* should not be necessary due to calloc(), but be safe */
    c->connected = false;

    /*
     * Setup the logging infrastructure
     */
    c->log_fn = log_stderr;
    c->log_priority = LOG_ERR;

    /* environment overwrites config */
    const char *env;
    env = getenv("GLIP_LOG");
    if (env != NULL) {
        c->log_priority = log_priority(env);
    }
    dbg(c, "log_priority=%d\n", c->log_priority);

    /*
     * Determine the used backend
     */
    unsigned int num_backends = sizeof(glip_backends) / sizeof(struct glip_backend);

    int backend_id = -1;
    for (unsigned int i = 0; i < num_backends; i++) {
        if ((strcmp(glip_backends[i].name, backend_name) == 0)) {
            backend_id = i;
            break;
        }
    }

    if (backend_id == -1) {
        err(c, "Unknown backend: %s\n", backend_name);
        free(c);
        return -GLIP_EUNKNOWNBACKEND;
    }

    if (glip_backends[backend_id].new == NULL) {
        err(c, "Backend not enabled at compile time: %s\n", backend_name);
        free(c);
        return -GLIP_EUNKNOWNBACKEND;
    }

    /*
     * Initialize the backend. This also sets all vtable pointers for the
     * backend functions.
     */
    c->backend_options = backend_options;
    c->num_backend_options = num_backend_options;
    glip_backends[backend_id].new(c);

    *ctx = c;

    return 0;
}

/**
 * Free all library resources (destructor)
 *
 * The @p ctx object is invalid after calling this function and may not be
 * accessed any more.
 *
 * @param  ctx the library context
 * @return 0 if the call was successful, or an error code if something went
 *         wrong
 *
 * @ingroup library-init-deinit
 */
API_EXPORT
int glip_free(struct glip_ctx *ctx)
{
    free(ctx);
    return 0;
}

/**
 * Set a caller context pointer
 *
 * In some cases GLIP executes callback functions. These functions always
 * provide the GLIP context object of type struct glip_ctx. To make it possible
 * to associate the GLIP context with the right context of the calling
 * application register the context or <code>this</code> (in C++) pointer with
 * GLIP and retrieve it inside the callback using glip_get_caller_ctx().
 *
 * GLIP does not use this pointer in any way, you're free to set it to whatever
 * your application needs.
 *
 * @param ctx        the library context
 * @param caller_ctx the caller context pointer
 *
 * @see glip_get_caller_ctx()
 * @see glip_set_log_fn() for a code example using this functionality
 *
 * @ingroup utilities
 */
API_EXPORT
void glip_set_caller_ctx(struct glip_ctx *ctx, void *caller_ctx)
{
    ctx->caller_ctx = caller_ctx;
}

/**
 * Get the caller context pointer
 *
 * @param ctx the library context
 * @return the caller context pointer
 *
 * @see glip_set_caller_ctx()
 *
 * @ingroup utilities
 */
API_EXPORT
void* glip_get_caller_ctx(struct glip_ctx *ctx)
{
    return ctx->caller_ctx;
}

/**
 * Get the width of the FIFO on the logic side in bytes
 *
 * Depending on the used backend and possibly the logic configuration different
 * FIFO widths on the logic side are possible. Data is always transferred in
 * chunks of at least one FIFO width, so make sure to always transfer at least
 * as many bytes as the FIFO is wide.
 *
 * @param ctx the library context
 * @return the width of the FIFO on the target side in bytes
 *
 * @ingroup utilities
 */
API_EXPORT
unsigned int glip_get_fifo_width(struct glip_ctx *ctx)
{
    return ctx->backend_functions.get_fifo_width(ctx);
}

/**
 * Get the number of supported channels
 *
 * Depending on the backend (and possibly the target device) a different number
 * of channels might be supported.
 *
 * @param ctx the library contexxt
 * @return the number of supported channels
 *
 * @ingroup utilities
 */
API_EXPORT
unsigned int glip_get_channel_count(struct glip_ctx *ctx)
{
    return ctx->backend_functions.get_channel_count(ctx);
}

/**
 * Get a list of the names of all backends
 *
 * The @p name array is allocated for you, you need to free the list after using
 * it. You do *not* need to free any of the character strings, they are static.
 *
 * @code{.cpp}
 * const char** name;
 * size_t count;
 *
 * glip_get_backends(&name, &count);
 * for (size_t i = 0; i < count; i++) {
 *     printf("backend %zu: %s\n", i, name[i]);
 * }
 *
 * free(name);
 * @endcode
 *
 * @param[out] name  array of backend names
 * @param[out] count number of backends (entries in the @p name array)
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @ingroup backend
 */
API_EXPORT
int glip_get_backends(const char*** name, size_t* count)
{
    size_t num_backends = sizeof(glip_backends) / sizeof(struct glip_backend);
    const char** backend_names;
    backend_names = calloc(num_backends, sizeof(char*));

    for (size_t i = 0; i < num_backends; i++) {
        backend_names[i] = glip_backends[i].name;
    }

    *name = backend_names;
    *count = num_backends;

    return 0;
}

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
 * @param ctx      the library context
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
void glip_log(struct glip_ctx *ctx,
              int priority, const char *file, int line, const char *fn,
              const char *format, ...)
{
    va_list args;

    va_start(args, format);
    ctx->log_fn(ctx, priority, file, line, fn, format, args);
    va_end(args);
}

/**
 * Set logging function
 *
 * The built-in logging writes to STDERR. It can be overridden by a custom
 * function to log messages into the user's logging functionality.
 *
 * In many cases you want the log message to be associated with a context or
 * object of your application, i.e. the object that uses GLIP. In this
 * case, set the context or <code>this</code> pointer with glip_set_caller_ctx()
 * and retrieve it inside your @p log_fn.
 *
 * An example in C++ could look like this:
 * @code{.cpp}
 * static void MyClass::glipLogCallback(struct glip_ctx *gctx,
 *                                      int priority, const char *file,
 *                                      int line, const char *fn,
 *                                      const char *format, va_list args)
 * {
 *   MyClass *myclassptr = static_cast<MyClass*>(glip_get_caller_ctx(gctx));
 *   myclassptr->doLogging(format, args);
 * }
 *
 * MyClass::MyClass()
 * {
 *   // ...
 *   glip_set_caller_ctx(gctx, this);
 *   glip_set_log_fn(&MyClass::glipLogCallback);
 *   // ...
 * }
 *
 * MyClass::doLogging(const char* format, va_list args)
 * {
 *    printf("this = %p", this);
 *    vprintf(format, args);
 * }
 * @endcode
 *
 *
 * @param ctx    the library context
 * @param log_fn the used logging function
 *
 * @ingroup log
 */
API_EXPORT
void glip_set_log_fn(struct glip_ctx *ctx, glip_log_fn log_fn)
{
    ctx->log_fn = log_fn;
}

/**
 * Get the logging priority
 *
 * The logging priority is the lowest message type that is reported.
 *
 * @param ctx the library context
 * @return the log priority
 *
 * @see glip_set_log_priority()
 *
 * @ingroup log
 */
API_EXPORT
int glip_get_log_priority(struct glip_ctx *ctx)
{
    return ctx->log_priority;
}

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
 * @param ctx       the library context
 * @param priority  new priority value
 *
 * @see glip_get_log_priority()
 *
 * @ingroup log
 */
API_EXPORT
void glip_set_log_priority(struct glip_ctx *ctx, int priority)
{
    ctx->log_priority = priority;
}

/**
 * Open a connection to the target
 *
 * @param  ctx          library context
 * @param  num_channels the number of channels to open to the target
 * @return 0 if the call was successful, or an error code if something went
 *         wrong
 *
 * @ingroup connection-handling
 */
API_EXPORT
int glip_open(struct glip_ctx *ctx, unsigned int num_channels)
{
    int rv;

    if (!ctx) {
        return -GLIP_ENOINIT;
    }

    unsigned int nchan = ctx->backend_functions.get_channel_count(ctx);
    if (nchan < num_channels) {
        err(ctx, "The backend only supports up to %u channels and channel "
            "multiplexing is not supported at the moment!\n", nchan);
        return -GLIP_EFEATURE;
    }

    rv = ctx->backend_functions.open(ctx, num_channels);
    if (rv != 0) {
        err(ctx, "Cannot open backend\n");
        return -1;
    }

    ctx->connected = true;

    return 0;
}

/**
 * Close the connection to the target
 *
 * @param  ctx library context
 * @return 0 if the call was successful, or an error code if something went
 *         wrong
 *
 * @ingroup connection-handling
 */
API_EXPORT
int glip_close(struct glip_ctx* ctx)
{
    /*
     * We cannot be sure of the connection state if the backend close() call
     * fails, so just always assume we're disconnected after calling the
     * backend.
     */
    ctx->connected = false;

    return ctx->backend_functions.close(ctx);
}

/**
 * Are we connected to a target device?
 *
 * @param ctx library context
 * @return true if a connection is established, false otherwise
 */
API_EXPORT
bool glip_is_connected(struct glip_ctx *ctx)
{
    return ctx->connected;
}

/**
 * Send reset signal to the user logic
 *
 * This function enables the ctrl_logic_rst output signal of the GLIP hardware
 * module. It is up to the hardware designer to use this signal for any purpose.
 * Usually, this means only the attached logic is reset, not the the
 * communication with the host PC, even though the signal can be used to create
 * such functionality.
 *
 * @param ctx  the library context
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @ingroup communication
 */
API_EXPORT
int glip_logic_reset(struct glip_ctx *ctx)
{
    if (!ctx->connected) {
        err(ctx, "No connection; you need to call glip_open() first!\n");
        return -1;
    }
    return ctx->backend_functions.logic_reset(ctx);
}

/**
 * Read from the target device FIFO on a given channel
 *
 * Requests to read @p size bytes of data from channel @p channel into the
 * variable @p data. The number of bytes actually read from the target is
 * stored in the @p size_read output argument.
 *
 * This function returns 0 if the call was successful; a nonzero return value
 * indicates an error.
 *
 * Note: You need to allocate sufficient space to read @p size bytes into
 * @p data before calling this function.
 *
 * @param[in]  ctx        the library context
 * @param[in]  channel    the channel to read from
 * @param[in]  size       the number of bytes to read
 * @param[out] data       the data read from the target (allocated by the user)
 * @param[out] size_read  the number of bytes actually read from the target.
 *                        Only those bytes may be considered valid inside
 *                        @p data!
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @ingroup communication
 */
API_EXPORT
int glip_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
              uint8_t *data, size_t *size_read)
{
    if (!ctx->connected) {
        err(ctx, "No connection; you need to call glip_open() first!\n");
        return -1;
    }
    if (size == 0) {
        *size_read = 0;
        return 0;
    }
    return ctx->backend_functions.read(ctx, channel, size, data, size_read);
}

/**
 * Blocking read from the target device FIFO on a given channel
 *
 * This function is similar to glip_read(), but instead of returning whatever
 * amount of data is available and returning immediately, it blocks until either
 * the requested amount of data is available or the timeout is hit.
 *
 * @param[in]  ctx        the library context
 * @param[in]  channel    the channel to read from
 * @param[in]  size       the number of bytes to read
 * @param[out] data       the data read from the target (allocated by the user)
 * @param[out] size_read  the number of bytes actually read from the target.
 *                        Only those bytes may be considered valid inside
 *                        @p data!
 * @param[in]  timeout    the timeout in milliseconds (ms) after which this
 *                        function gives up reading, i.e. the maximum blocking
 *                        time
 * @return 0 if the call was successful and @p size bytes have been read
 * @return -ETIMEDOUT if the call timed out (some data might still have been
 *         read, see @p size_read)
 * @return any other value indicates an error
 *
 * Note: You need to allocate sufficient space to read @p size bytes into
 * @p data before calling this function.
 *
 * @see glip_read()
 *
 * @ingroup communication
 */
API_EXPORT
int glip_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                uint8_t *data, size_t *size_read, unsigned int timeout)
{
    if (!ctx->connected) {
        err(ctx, "No connection; you need to call glip_open() first!\n");
        return -1;
    }
    if (size == 0) {
        *size_read = 0;
        return 0;
    }
    return ctx->backend_functions.read_b(ctx, channel, size, data, size_read,
                                         timeout);
}

/**
 * Write data to the target FIFO on a given channel
 *
 * Request the transfer of @p size bytes of @p data to the target FIFO of
 * channel @p channel. The actual number of written bytes is returned as
 * @p size_written. Always check if all data was transferred, and repeat the
 * transfer if this is not the case.
 *
 * This function returns 0 if the call was successful; a nonzero return value
 * indicates an error.
 *
 * This function behaves like you would expect from a FIFO with no latency
 * guarantees. This function is non-blocking. There are no write
 * acknowledgements; the data is scheduled to be transferred "soon", whereas
 * the definition of "soon" depends on the used backend. The backend is free
 * to do with the data whatever it wants, as long as the following guarantees
 * are obeyed:
 *
 * - @p size_written bytes of @p data will eventually reach the target device
 * - the data ordering is preserved; the first byte written will be the first
 *   byte read from the FIFO
 *
 * All data transfer is **big endian**. This means for a 2 byte wide FIFO (check
 * the FIFO width with glip_get_fifo_width()) the first byte written will be the
 * MSB in the FIFO, the second byte written will be the LSB.
 *
 * @param[in]  ctx          the library context
 * @param[in]  channel      the channel to write to
 * @param[in]  size         the number of bytes to write (length of data)
 * @param[in]  data         the data to write
 * @param[out] size_written the number of bytes actually written; repeat the
 *                          transfer for the remaining data if
 * @return     0 if the call was successful
 * @return     any other value indicates an error
 *
 * @see glip_write_b()
 *
 * @ingroup communication
 */
API_EXPORT
int glip_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
               uint8_t *data, size_t *size_written)
{
    if (!ctx->connected) {
        err(ctx, "No connection; you need to call glip_open() first!\n");
        return -1;
    }
    if (size == 0) {
        *size_written = 0;
        return 0;
    }
    return ctx->backend_functions.write(ctx, channel, size, data, size_written);
}

/**
 * Blocking write to the target FIFO on a given channel
 *
 * This function is the blocking version of glip_write(). The function returns
 * not before either all data is written (i.e. @p size bytes) or @p timeout
 * milliseconds passed.
 *
 *
 * @param[in]  ctx          the library context
 * @param[in]  channel      the channel to write to
 * @param[in]  size         the number of bytes to write (length of data)
 * @param[in]  data         the data to write
 * @param[out] size_written the number of bytes actually written; repeat the
 *                          transfer for the remaining data if
 * @param[in]  timeout      the timeout in milliseconds (ms) after which this
 *                          function gives up writing, i.e. the maximum blocking
 *                          time
 * @return 0 if the call was successful and @p size bytes have been written
 * @return -ETIMEDOUT if the call timed out (some data might still have been
 *         written, see @p size_written)
 * @return any other value indicates an error
 *
 * @see glip_write()
 *
 * @ingroup communication
 */
API_EXPORT
int glip_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_written, unsigned int timeout)
{
    if (!ctx->connected) {
        err(ctx, "No connection; you need to call glip_open() first!\n");
        return -1;
    }
    if (size == 0) {
        *size_written = 0;
        return 0;
    }
    return ctx->backend_functions.write_b(ctx, channel, size, data,
                                          size_written, timeout);
}

/**
 * Get the value of an option as 32 bit wide unsigned integer
 *
 * @param[in]  ctx         the library context
 * @param[in]  option_name the name of the option (the option key)
 * @param[out] out         the option value as unsigned integer
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @see glip_option_get_uint16()
 */
int glip_option_get_uint32(struct glip_ctx* ctx, const char* option_name,
                           uint32_t *out)
{
    unsigned long int val;
    int found = 0;

    for (unsigned int i = 0; i < ctx->num_backend_options; i++) {
        if (strcmp(ctx->backend_options[i].name, option_name) == 0) {
            char* endptr;
            char* option_value;

            option_value = ctx->backend_options[i].value;
            errno = 0;
            val = strtoul(option_value, &endptr, 0);
            if ((errno == ERANGE && val == ULONG_MAX) ||
                (errno != 0 && val == 0)) {

                err(ctx, "Unable to get number from option value "
                    "for key %s\n", option_name);
                return -1;
            }

            if (endptr == option_value) {
                info(ctx, "No digits found in value when parsing "
                     "option key %s; setting value to 0\n", option_name);
                val = 0;
            }

            if (*endptr != '\0') {
                dbg(ctx, "Unused characters found in option value for "
                    "key %s\n", option_name);
            }

            found = 1;
            break;
        }
    }

    if (!found) {
        info(ctx, "Option with key '%s' not found.\n", option_name);
        return -1;
    }

    /*
     * Depending on the size of "unsigned long" we need to case from
     * uint64_t to uint32_t (notably 64 Bit Linux).
     */
    if (val >= UINT32_MAX) {
        dbg(ctx, "Possibly truncating out-of-bounds value down "
            "to 32 bit.\n");
    }
    *out = (uint32_t)val;

    return 0;
}

/**
 * Get a option value as 16 bit unsigned integer
 *
 * The value will be truncated to UINT16_MAX (0xFFFF) if the value is out of
 * bounds.
 *
 * @param[in]  ctx         the library context
 * @param[in]  option_name the name of the option (the option key)
 * @param[out] out         the option value as unsigned integer
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @see glip_option_get_uint32()
 */
int glip_option_get_uint16(struct glip_ctx* ctx, const char* option_name,
                           uint16_t *out)
{
    uint32_t tmp;
    int rv = glip_option_get_uint32(ctx, option_name, &tmp);
    if (rv != 0) {
        return rv;
    }
    if (tmp > UINT16_MAX) {
        info(ctx, "Truncating out-of-bounds value down to 16 bit.\n");
        tmp = UINT16_MAX;
    }
    *out = (uint16_t)tmp;
    return 0;
}

/**
 * Get a option value as 8 bit unsigned integer
 *
 * The value will be truncated to UINT8_MAX (0xFF) if the value is out of
 * bounds.
 *
 * @param[in]  ctx         the library context
 * @param[in]  option_name the name of the option (the option key)
 * @param[out] out         the option value as unsigned integer
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @see glip_option_get_uint32()
 */
int glip_option_get_uint8(struct glip_ctx* ctx, const char* option_name,
                          uint8_t *out)
{
    uint32_t tmp;
    int rv = glip_option_get_uint32(ctx, option_name, &tmp);
    if (rv != 0) {
        return rv;
    }
    if (tmp > UINT8_MAX) {
        info(ctx, "Truncating out-of-bounds value down to 8 bit.\n");
        tmp = UINT8_MAX;
    }
    *out = (uint8_t)tmp;
    return 0;
}

/**
 * Get the value for an option with a given key/name
 *
 * The resulting string is not cloned; make sure to duplicate the string before
 * modifying it!
 *
 * @param[in]  ctx         the library context
 * @param[in]  option_name the name of the option (the option key)
 * @param[out] out         the option value
 * @return     0 if the call was successful, or an error code if something went
 *             wrong
 *
 * @see glip_option_get_uint32()
 */
int glip_option_get_char(struct glip_ctx* ctx, const char* option_name,
                         const char** out)
{
    int found = 0;

    for (unsigned int i = 0; i < ctx->num_backend_options; i++) {
        if (strcmp(ctx->backend_options[i].name, option_name) == 0) {
            *out = ctx->backend_options[i].value;
            found = 1;
            break;
        }
    }

    if (!found) {
        info(ctx, "Option with key '%s' not found.\n", option_name);
        return -1;
    }

    return 0;
}
