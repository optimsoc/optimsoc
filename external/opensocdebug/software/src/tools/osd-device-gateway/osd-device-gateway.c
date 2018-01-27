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

/**
 * Open SoC Debug Device Gateway
 */

#define CLI_TOOL_PROGNAME "osd-device-gateway"
#define CLI_TOOL_SHORTDESC "Open SoC Debug device gateway"

#include <osd/gateway_glip.h>
#include "../cli-util.h"

/**
 * Default GLIP backend to be used when connecting to a device
 */
#define GLIP_DEFAULT_BACKEND "tcp"

/**
 * Subnet address of the device. Currently static and must be 0.
 */
#define DEVICE_SUBNET_ADDRESS 0

/**
 * GLIP library context
 */
struct glip_ctx *glip_ctx;

// command line arguments
struct arg_str *a_glip_backend;
struct arg_str *a_glip_backend_options;
struct arg_str *a_hostctrl_ep;

osd_result setup(void)
{
    a_hostctrl_ep = arg_str0("e", "hostctrl", "<URL>",
                             "ZeroMQ endpoint of the host controller "
                             "(default: " DEFAULT_HOSTCTRL_EP ")");
    a_hostctrl_ep->sval[0] = DEFAULT_HOSTCTRL_EP;
    osd_tool_add_arg(a_hostctrl_ep);

    a_glip_backend =
        arg_str0("b", "glip-backend", "<name>", "GLIP backend name");
    a_glip_backend->sval[0] = GLIP_DEFAULT_BACKEND;
    osd_tool_add_arg(a_glip_backend);

    a_glip_backend_options =
        arg_str0("o", "glip-backend-options",
                 "<option1=value1,option2=value2,...>", "GLIP backend options");
    osd_tool_add_arg(a_glip_backend_options);

    return OSD_OK;
}

int run(void)
{
    osd_result rv;
    int exitcode;

    zsys_init();

    // Logging setup
    struct osd_log_ctx *osd_log_ctx;
    rv = osd_log_new(&osd_log_ctx, cfg.log_level, &osd_log_handler);
    assert(OSD_SUCCEEDED(rv));

    // GLIP options
    struct glip_option *glip_backend_options;
    size_t glip_backend_options_len;
    rv = glip_parse_option_string(a_glip_backend_options->sval[0],
                                  &glip_backend_options,
                                  &glip_backend_options_len);
    if (rv != 0) {
        fatal("Unable to parse GLIP backend options.");
        exitcode = 1;
        goto free_return;
    }

    struct osd_gateway_glip_ctx *gateway_glip_ctx;
    rv = osd_gateway_glip_new(&gateway_glip_ctx, osd_log_ctx,
                              a_hostctrl_ep->sval[0], DEVICE_SUBNET_ADDRESS,
                              a_glip_backend->sval[0], glip_backend_options,
                              glip_backend_options_len);
    if (OSD_FAILED(rv)) {
        fatal("Unable to create gateway.");
        exitcode = 1;
        goto free_return;
    }
    assert(gateway_glip_ctx);

    rv = osd_gateway_glip_connect(gateway_glip_ctx);
    if (OSD_FAILED(rv)) {
        fatal("Unable to connect to host controller and to device.");
        exitcode = 1;
        goto free_return;
    }

    while (!zsys_interrupted) {
        pause();
    }
    info("Shutdown signal received, cleaning up.");

    rv = osd_gateway_glip_disconnect(gateway_glip_ctx);
    if (OSD_FAILED(rv)) {
        err("Unable to cleanly shut down gateway. (%d)", rv);
    }

    exitcode = 0;
free_return:
    osd_gateway_glip_free(&gateway_glip_ctx);
    osd_log_free(&osd_log_ctx);
    return exitcode;
}
