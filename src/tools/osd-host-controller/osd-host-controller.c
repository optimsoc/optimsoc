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
 * Open SoC Debug host controller
 */

#define CLI_TOOL_PROGNAME "osd-host-controller"
#define CLI_TOOL_SHORTDESC "Open SoC Debug host controller"

#include <osd/hostctrl.h>
#include <osd/packet.h>
#include "../cli-util.h"

#include <unistd.h>

// command line arguments
struct arg_str *a_bind_ep;

osd_result setup(void)
{
    a_bind_ep = arg_str0("b", "bind-address", "<URL>",
                         "ZeroMQ endpoint address to bind to "
                         "(default: " DEFAULT_HOSTCTRL_BIND_EP ")");
    a_bind_ep->sval[0] = DEFAULT_HOSTCTRL_BIND_EP;
    osd_tool_add_arg(a_bind_ep);

    return OSD_OK;
}

int run(void)
{
    osd_result rv;
    int exitcode;

    zsys_init();

    struct osd_log_ctx *osd_log_ctx;
    rv = osd_log_new(&osd_log_ctx, cfg.log_level, &osd_log_handler);
    assert(OSD_SUCCEEDED(rv));

    struct osd_hostctrl_ctx *hostctrl_ctx;
    rv = osd_hostctrl_new(&hostctrl_ctx, osd_log_ctx, a_bind_ep->sval[0]);
    if (OSD_FAILED(rv)) {
        fatal("Unable to initialize host controller (%d)", rv);
        exitcode = 1;
        goto free_return;
    }

    rv = osd_hostctrl_start(hostctrl_ctx);
    if (OSD_FAILED(rv)) {
        fatal("Unable to start host controller (%d)", rv);
        exitcode = 1;
        goto free_return;
    }

    info("Host controller up and running, listening at %s for connections",
         a_bind_ep->sval[0]);
    while (!zsys_interrupted) {
        pause();
    }
    info("Shutdown signal received, cleaning up.");

    rv = osd_hostctrl_stop(hostctrl_ctx);
    if (OSD_FAILED(rv)) {
        fatal("Unable to stop host controller (%d)", rv);
        exitcode = 1;
        goto free_return;
    }

    exitcode = 0;
free_return:
    osd_hostctrl_free(&hostctrl_ctx);
    osd_log_free(&osd_log_ctx);
    return exitcode;
}
