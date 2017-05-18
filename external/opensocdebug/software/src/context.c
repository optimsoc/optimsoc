/* Copyright (c) 2016 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#include "osd-private.h"
#include <libglip.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>

static
int osd_new_standalone(struct osd_context_standalone **ctx,
                       struct osd_mode_functions *fnc,
                       size_t num_mode_options,
                       struct osd_mode_option *options);

static
int osd_new_daemon(struct osd_context_daemon **ctx,
                   struct osd_mode_functions *fnc,
                   size_t num_mode_options,
                   struct osd_mode_option *options);

OSD_EXPORT
int osd_new(struct osd_context **ctx, enum osd_mode mode,
            size_t num_mode_options, struct osd_mode_option *options) {
    if ((mode != OSD_MODE_STANDALONE) &&
            (mode != OSD_MODE_DAEMON)) {
        return OSD_E_GENERIC;
    }

    struct osd_context *c = calloc(1, sizeof(struct osd_context));
    *ctx = c;

    // Activate for low level debugging
    //c->debug_packets = 1;

    if (mode == OSD_MODE_STANDALONE) {
        return osd_new_standalone(&c->ctx.standalone, &c->functions,
                                  num_mode_options, options);
    } else {
        return osd_new_daemon(&c->ctx.daemon, &c->functions,
                              num_mode_options, options);
    }

    return OSD_E_GENERIC;
}

static
int osd_new_standalone(struct osd_context_standalone **ctx,
                       struct osd_mode_functions *fnc,
                       size_t num_mode_options,
                       struct osd_mode_option *options) {
    struct osd_context_standalone *c = malloc(sizeof(struct osd_context_standalone));

    *ctx = c;

    struct glip_option *glip_options = calloc(num_mode_options, sizeof(struct glip_option));
    char *backend_name = 0;
    size_t num_glip_options = 0;

    for (size_t i = 0; i < num_mode_options; i++) {
        if (strcmp(options[i].name, "backend") == 0) {
            backend_name = options[i].value;
        } else if (strcmp(options[i].name, "backend_option") == 0) {
            char *name, *value;
            name = strtok(options[i].value, "=");
            value = strtok(NULL, "");
            glip_options[num_glip_options].name = name;
            glip_options[num_glip_options].value = value;
            num_glip_options++;
        }
    }

    fnc->connect = osd_connect_standalone;
    fnc->send = osd_send_packet_standalone;
    fnc->claim = claim_standalone;

    return glip_new(&c->glip_ctx, backend_name, glip_options, num_glip_options, NULL);
}

static
int osd_new_daemon(struct osd_context_daemon **ctx,
                   struct osd_mode_functions *fnc,
                   size_t num_mode_options,
                   struct osd_mode_option *options) {

    struct osd_context_daemon *c = malloc(sizeof(struct osd_context_daemon));

    *ctx = c;

    c->host = strdup("localhost");
    c->port = 7450;
    fnc->connect = osd_connect_daemon;
    fnc->send = osd_send_packet_daemon;
    fnc->claim = claim_daemon;

    return 0;
}
