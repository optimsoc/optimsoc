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
 * Open SoC Debug "target run" tool
 */

#define CLI_TOOL_PROGNAME "osd-target-run"
#define CLI_TOOL_SHORTDESC "Run software on an Open SoC Debug-enabled target"

#include <czmq.h>
#include <osd/coretracelogger.h>
#include <osd/gateway_glip.h>
#include <osd/hostctrl.h>
#include <osd/memaccess.h>
#include <osd/packet.h>
#include <osd/systracelogger.h>
#include <osd/terminal.h>
#include "../cli-util.h"

#include <unistd.h>

/**
 * Default GLIP backend to be used when connecting to a device
 */
#define GLIP_DEFAULT_BACKEND "tcp"

/**
 * Subnet address of the device. Currently static and must be 0.
 */
#define DEVICE_SUBNET_ADDRESS 0

/** ZeroMQ host controller endpoint */
#define HOSTCTRL_EP "inproc://osd-target-run"

// command line arguments
struct arg_str *a_glip_backend;
struct arg_str *a_glip_backend_options;
struct arg_str *a_hostctrl_ep;
struct arg_lit *a_coretrace;
struct arg_lit *a_systrace;
struct arg_lit *a_verify_memload;
struct arg_lit *a_terminal;
struct arg_file *a_elf_file;

// global objects
struct glip_ctx *glip_ctx;
struct osd_log_ctx *osd_log_ctx;
struct osd_hostctrl_ctx *hostctrl_ctx;
struct osd_gateway_glip_ctx *gateway_glip_ctx;
struct osd_terminal_ctx *terminal_ctx;

zlist_t *ctloggers;
zlist_t *stloggers;
zlist_t *open_files;

osd_result setup(void)
{
    a_elf_file =
        arg_file1("e", "elf-file", "file", "ELF file to run on the device");
    osd_tool_add_arg(a_elf_file);

    a_coretrace =
        arg_lit0(NULL, "coretrace", "create a core trace for all CPU cores");
    osd_tool_add_arg(a_coretrace);

    a_systrace =
        arg_lit0(NULL, "systrace", "create a system trace for all CPU cores");
    osd_tool_add_arg(a_systrace);

    a_verify_memload = arg_lit0(NULL, "verify-memload", "verify loaded memory");
    osd_tool_add_arg(a_verify_memload);

    a_terminal = arg_lit0(NULL, "terminal", "create pseudo-terminal device");
    osd_tool_add_arg(a_terminal);

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

static osd_result run_gateway_glip(void)
{
    osd_result rv;

    // GLIP options
    struct glip_option *glip_backend_options;
    size_t glip_backend_options_len;
    rv = glip_parse_option_string(a_glip_backend_options->sval[0],
                                  &glip_backend_options,
                                  &glip_backend_options_len);
    if (rv != 0) {
        fatal("Unable to parse GLIP backend options.");
        return OSD_ERROR_FAILURE;
    }

    rv = osd_gateway_glip_new(&gateway_glip_ctx, osd_log_ctx, HOSTCTRL_EP,
                              DEVICE_SUBNET_ADDRESS, a_glip_backend->sval[0],
                              glip_backend_options, glip_backend_options_len);
    if (OSD_FAILED(rv)) {
        fatal("Unable to create gateway.");
        return OSD_ERROR_FAILURE;
    }
    assert(gateway_glip_ctx);

    rv = osd_gateway_glip_connect(gateway_glip_ctx);
    if (OSD_FAILED(rv)) {
        fatal("Unable to connect to host controller and to device.");
        return OSD_ERROR_FAILURE;
    }

    return OSD_OK;
}

static osd_result run_hostctrl(void)
{
    osd_result rv;
    rv = osd_hostctrl_new(&hostctrl_ctx, osd_log_ctx, HOSTCTRL_EP);
    if (OSD_FAILED(rv)) {
        fatal("Unable to initialize host controller (%d)", rv);
        return OSD_ERROR_FAILURE;
    }

    rv = osd_hostctrl_start(hostctrl_ctx);
    if (OSD_FAILED(rv)) {
        fatal("Unable to start host controller (%d)", rv);
        return OSD_ERROR_FAILURE;
    }

    dbg("Host controller up and running, listening at %s for connections",
        HOSTCTRL_EP);

    return OSD_OK;
}

static osd_result run_systrace(uint16_t stm_di_addr)
{
    osd_result rv;
    osd_result retval;
    int irv;

    struct osd_systracelogger_ctx *systracelogger_ctx = NULL;
    rv = osd_systracelogger_new(&systracelogger_ctx, osd_log_ctx, HOSTCTRL_EP,
                                stm_di_addr);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    rv = osd_systracelogger_connect(systracelogger_ctx);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    // event output
    char systrace_log_filename_event[18] = {0};
    irv = snprintf(systrace_log_filename_event, 18, "systrace.%04d.log",
                   osd_diaddr_localaddr(stm_di_addr));
    assert(irv >= 0);

    FILE *fp = fopen(systrace_log_filename_event, "w");
    if (!fp) {
        err("Unable to open file %s: %s (%d)", systrace_log_filename_event,
            strerror(errno), errno);
        retval = OSD_ERROR_FILE;
        goto free_return;
    }
    rv = osd_systracelogger_set_event_log(systracelogger_ctx, fp);
    if (OSD_FAILED(rv)) {
        fclose(fp);
        retval = rv;
        goto free_return;
    }
    irv = zlist_append(open_files, fp);
    assert(irv == 0);
    info("Writing system trace event output to file %s",
         systrace_log_filename_event);

    char systrace_log_filename_sysprint[24] = {0};
    irv =
        snprintf(systrace_log_filename_sysprint, 24, "systrace.print.%04d.log",
                 osd_diaddr_localaddr(stm_di_addr));
    assert(irv >= 0);

    fp = fopen(systrace_log_filename_sysprint, "w");
    if (!fp) {
        err("Unable to open file %s: %s (%d)", systrace_log_filename_sysprint,
            strerror(errno), errno);
        retval = OSD_ERROR_FILE;
        goto free_return;
    }
    rv = osd_systracelogger_set_sysprint_log(systracelogger_ctx, fp);
    if (OSD_FAILED(rv)) {
        fclose(fp);
        retval = rv;
        goto free_return;
    }
    irv = zlist_append(open_files, fp);
    assert(irv == 0);
    info("Writing system trace print output to file %s",
         systrace_log_filename_sysprint);

    // start tracing
    rv = osd_systracelogger_start(systracelogger_ctx);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    fflush(stdout);
    irv = zlist_append(stloggers, systracelogger_ctx);
    assert(irv == 0);

    retval = OSD_OK;
free_return:
    if (OSD_FAILED(retval)) {
        if (systracelogger_ctx &&
            osd_systracelogger_is_connected(systracelogger_ctx)) {
            osd_systracelogger_disconnect(systracelogger_ctx);
        }
        osd_systracelogger_free(&systracelogger_ctx);
    }
    return retval;
}

static osd_result run_coretrace(uint16_t ctm_di_addr)
{
    osd_result rv;
    osd_result retval;
    int irv;

    struct osd_coretracelogger_ctx *coretracelogger_ctx = NULL;
    rv = osd_coretracelogger_new(&coretracelogger_ctx, osd_log_ctx, HOSTCTRL_EP,
                                 ctm_di_addr);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    rv = osd_coretracelogger_connect(coretracelogger_ctx);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    // ELF decoding
    rv = osd_coretracelogger_set_elf(coretracelogger_ctx,
                                     a_elf_file->filename[0]);
    if (OSD_FAILED(rv)) {
        err("Unable to use ELF file %s to decode control trace.",
            a_elf_file->filename[0]);
        // continue without ELF decoding
    }

    // trace output file
    char coretrace_log_filename[19] = {0};
    irv = snprintf(coretrace_log_filename, 19, "coretrace.%04d.log",
                   osd_diaddr_localaddr(ctm_di_addr));
    assert(irv >= 0);

    FILE *fp = fopen(coretrace_log_filename, "w");
    if (!fp) {
        err("Unable to open file %s: %s (%d)", coretrace_log_filename,
            strerror(errno), errno);
        retval = OSD_ERROR_FILE;
        goto free_return;
    }
    rv = osd_coretracelogger_set_log(coretracelogger_ctx, fp);
    if (OSD_FAILED(rv)) {
        fclose(fp);
        retval = rv;
        goto free_return;
    }
    irv = zlist_append(open_files, fp);
    assert(irv == 0);
    info("Writing core trace to file %s", coretrace_log_filename);

    // start tracing
    rv = osd_coretracelogger_start(coretracelogger_ctx);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    irv = zlist_append(ctloggers, coretracelogger_ctx);
    assert(irv == 0);

    retval = OSD_OK;
free_return:
    if (OSD_FAILED(retval)) {
        if (coretracelogger_ctx &&
            osd_coretracelogger_is_connected(coretracelogger_ctx)) {
            osd_coretracelogger_disconnect(coretracelogger_ctx);
        }
        osd_coretracelogger_free(&coretracelogger_ctx);
    }
    return retval;
}

static osd_result run_tracing(void)
{
    osd_result rv;
    osd_result retval;

    struct osd_hostmod_ctx *hostmod_enum = NULL;
    rv = osd_hostmod_new(&hostmod_enum, osd_log_ctx, HOSTCTRL_EP, NULL, NULL);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    rv = osd_hostmod_connect(hostmod_enum);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    struct osd_module_desc *modules = NULL;
    size_t modules_len;
    rv = osd_hostmod_get_modules(hostmod_enum, DEVICE_SUBNET_ADDRESS, &modules,
                                 &modules_len);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    for (size_t i = 0; i < modules_len; i++) {
        if (a_coretrace->count && modules[i].vendor == OSD_MODULE_VENDOR_OSD &&
            modules[i].type == OSD_MODULE_TYPE_STD_CTM) {
            rv = run_coretrace(modules[i].addr);
            if (OSD_FAILED(rv)) return rv;
        }
        if (a_systrace->count && modules[i].vendor == OSD_MODULE_VENDOR_OSD &&
            modules[i].type == OSD_MODULE_TYPE_STD_STM) {
            rv = run_systrace(modules[i].addr);
            if (OSD_FAILED(rv)) return rv;
        }
    }

    rv = osd_hostmod_disconnect(hostmod_enum);
    if (OSD_FAILED(rv)) return rv;

    retval = OSD_OK;
free_return:
    free(modules);
    osd_hostmod_free(&hostmod_enum);
    return retval;
}

static osd_result run_terminal(void)
{
    struct osd_module_desc *modules;
    struct osd_hostmod_ctx *hostmod_enum;
    osd_result rv;
    size_t modules_len;

    // We only create the pseudo-terminal if it was explicitly specified
    if (!a_terminal->count) {
        return OSD_OK;
    }

    rv = osd_hostmod_new(&hostmod_enum, osd_log_ctx, HOSTCTRL_EP, NULL, NULL);
    if (OSD_FAILED(rv)) {
        goto free_return;
    }

    rv = osd_hostmod_connect(hostmod_enum);
    if (OSD_FAILED(rv)) {
        goto free_return;
    }

    rv = osd_hostmod_get_modules(hostmod_enum, DEVICE_SUBNET_ADDRESS, &modules,
                                 &modules_len);
    if (OSD_FAILED(rv)) {
        goto free_return;
    }

    for (size_t i = 0; i < modules_len; i++) {
        if (modules[i].vendor == OSD_MODULE_VENDOR_OSD &&
            modules[i].type == OSD_MODULE_TYPE_STD_DEM_UART) {
            rv = osd_terminal_new(&terminal_ctx, osd_log_ctx, HOSTCTRL_EP,
                                  modules[i].addr);
            if (OSD_FAILED(rv)) {
                fatal("osd_terminal_new() failed with code: %i", rv);
                goto free_return;
            }

            rv = osd_terminal_connect(terminal_ctx);
            if (OSD_FAILED(rv)) {
                fatal("osd_terminal_connect() failed with code: %i", rv);

                if (rv != OSD_ERROR_CONNECTION_FAILED) {
                    osd_terminal_disconnect(terminal_ctx);
                }

                osd_terminal_free(&terminal_ctx);
                goto free_return;
            }

            rv = osd_terminal_start(terminal_ctx);
            if (OSD_FAILED(rv)) {
                fatal("osd_terminal_start() failed with code: %i", rv);

                osd_terminal_disconnect(terminal_ctx);
                osd_terminal_free(&terminal_ctx);
                goto free_return;
            }

            // Currently, we can only handle a single osd_terminal.
            break;
        }
    }

    rv = osd_hostmod_disconnect(hostmod_enum);
    if (OSD_FAILED(rv)) {
        goto free_return;
    }

    rv = OSD_OK;
free_return:
    free(modules);
    osd_hostmod_free(&hostmod_enum);
    return rv;
}

int run(void)
{
    osd_result rv;
    int exitcode;

    zsys_init();

    ctloggers = zlist_new();
    assert(ctloggers);
    stloggers = zlist_new();
    assert(stloggers);
    open_files = zlist_new();
    assert(open_files);

    rv = osd_log_new(&osd_log_ctx, cfg.log_level, &osd_log_handler);
    assert(OSD_SUCCEEDED(rv));

    // host controller
    rv = run_hostctrl();
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }

    // device gateway
    rv = run_gateway_glip();
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }

    // setup memory access helper
    struct osd_memaccess_ctx *memaccess_ctx = NULL;
    rv = osd_memaccess_new(&memaccess_ctx, osd_log_ctx, HOSTCTRL_EP);
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }
    rv = osd_memaccess_connect(memaccess_ctx);
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }

    // stop all CPUs on target device
    info("Stopping all CPUs in the system");
    rv = osd_memaccess_cpus_stop(memaccess_ctx, DEVICE_SUBNET_ADDRESS);
    if (OSD_FAILED(rv)) {
        fatal("Unable to stop CPUs on target (%d)", rv);
        exitcode = -1;
        goto free_return;
    }

    // setup tracing
    info("Setting up tracing");
    rv = run_tracing();
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }

    // setup terminal
    info("Setting up terminal");
    rv = run_terminal();
    if (OSD_FAILED(rv)) {
        exitcode = -1;
        goto free_return;
    }

    // load memories
    info("Loading memories ...");
    struct osd_mem_desc *mems;
    size_t mems_len;
    rv = osd_memaccess_find_memories(memaccess_ctx, DEVICE_SUBNET_ADDRESS,
                                     &mems, &mems_len);
    if (OSD_FAILED(rv)) {
        fatal("Unable to enumerate memories on target (%d)", rv);
        exitcode = -1;
        goto free_return;
    }
    for (size_t i = 0; i < mems_len; i++) {
        if (a_verify_memload->count) {
            info("Loading memory at DI address %d with ELF file %s (verifying "
                 "write through readback)",
                 mems[i].di_addr, a_elf_file->filename[0]);
        } else {
            info("Loading memory at DI address %d with ELF file %s (not "
                 "verifying write)",
                 mems[i].di_addr, a_elf_file->filename[0]);
        }
        rv = osd_memaccess_loadelf(memaccess_ctx, &mems[i],
                                   a_elf_file->filename[0],
                                   a_verify_memload->count);
        if (OSD_FAILED(rv)) {
            err("Unable to load memory at DI address %d (%d)", mems[i].di_addr,
                rv);
            // continue anyways
        }
    }
    free(mems);

    // start CPUs on target
    info("Starting all CPUs");
    rv = osd_memaccess_cpus_start(memaccess_ctx, DEVICE_SUBNET_ADDRESS);
    if (OSD_FAILED(rv)) {
        fatal("Unable to start CPUs on target (%d)", rv);
        exitcode = -1;
        goto free_return;
    }

    // memaccess isn't needed any more
    rv = osd_memaccess_disconnect(memaccess_ctx);
    if (OSD_FAILED(rv)) {
        err("Unable to disconnect memory access tool (%d). Ignoring.", rv);
    }
    osd_memaccess_free(&memaccess_ctx);

    // if tracing is enabled, wait for user to cancel the operation
    if (a_coretrace->count || a_systrace->count) {
        info("System is now running. Press CTRL-C to end tracing.");
        while (!zsys_interrupted) {
            pause();
        }
        info("Shutdown signal received, cleaning up.");
    }

    exitcode = 0;

free_return:;
    dbg("Shutting down terminal");
    if (terminal_ctx) {
        rv = osd_terminal_disconnect(terminal_ctx);
        if (OSD_FAILED(rv) && rv != OSD_ERROR_NOT_CONNECTED) {
            fatal("Unable to shut down terminal");
            exitcode = -1;
        }
        osd_terminal_free(&terminal_ctx);
    }

    dbg("Shutting down systrace loggers");
    struct osd_systracelogger_ctx *s = zlist_first(stloggers);
    while (s) {
        osd_systracelogger_stop(s);
        osd_systracelogger_disconnect(s);
        osd_systracelogger_free(&s);
        s = zlist_next(stloggers);
    }
    zlist_destroy(&stloggers);

    dbg("Shutting down core trace loggers");
    struct osd_coretracelogger_ctx *c = zlist_first(ctloggers);
    while (c) {
        osd_coretracelogger_stop(c);
        osd_coretracelogger_disconnect(c);
        osd_coretracelogger_free(&c);
        c = zlist_next(ctloggers);
    }
    zlist_destroy(&ctloggers);

    dbg("Closing open files");
    FILE *f = zlist_first(open_files);
    while (f) {
        fclose(f);
        f = zlist_next(open_files);
    }
    zlist_destroy(&open_files);

    dbg("Disconnecting gateway");
    rv = osd_gateway_glip_disconnect(gateway_glip_ctx);
    if (OSD_FAILED(rv) && rv != OSD_ERROR_NOT_CONNECTED) {
        fatal("Unable to GLIP device gateway (%d)", rv);
        exitcode = -1;
    }
    osd_gateway_glip_free(&gateway_glip_ctx);

    dbg("Stopping host controller");
    rv = osd_hostctrl_stop(hostctrl_ctx);
    if (OSD_FAILED(rv) && rv != OSD_ERROR_NOT_CONNECTED) {
        fatal("Unable to stop host controller (%d)", rv);
        exitcode = -1;
    }
    osd_hostctrl_free(&hostctrl_ctx);

    osd_log_free(&osd_log_ctx);

    return exitcode;
}
