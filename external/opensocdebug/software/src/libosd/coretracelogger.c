/* Copyright 2017-2018 The Open SoC Debug Project
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

#include <osd/coretracelogger.h>
#include <osd/module.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/cl_ctm.h>
#include "osd-private.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <gelf.h>

/**
 * A function in a ELF file
 */
struct elf_function_table {
    uint64_t addr;
    char *name;
};

/**
 * Core Trace Logger context
 */
struct osd_coretracelogger_ctx {
    struct osd_hostmod_ctx *hostmod_ctx;
    struct osd_log_ctx *log_ctx;
    uint16_t ctm_di_addr;
    struct osd_ctm_desc ctm_desc;
    struct osd_ctm_event_handler ctm_event_handler;
    FILE *fp_log;
    size_t num_funcs;
    struct elf_function_table *funcs;
};

static void print_with_elfdata(struct osd_coretracelogger_ctx *ctx,
                               const struct osd_ctm_event *event)
{
    assert(ctx);
    assert(event);
    assert(ctx->funcs);

    if (event->is_modechange) {
        fprintf(ctx->fp_log, "%08x change mode to %d\n", event->timestamp,
                event->mode);
        return;
    }

    if (event->is_call) {
        for (size_t f = 0; f < ctx->num_funcs; f++) {
            if (ctx->funcs[f].addr == event->npc) {
                fprintf(ctx->fp_log, "%08x enter %s\n", event->timestamp,
                        ctx->funcs[f].name);
                return;
            }
        }
        return;
    }

    if (event->is_ret) {
        char* to = 0;
        for (size_t f = 1; f <= ctx->num_funcs; f++) {
            if (ctx->funcs[f].addr > event->npc) {
                to = ctx->funcs[f - 1].name;
                break;
            }
            if (f == ctx->num_funcs) {
                to = ctx->funcs[ctx->num_funcs - 1].name;
            }
        }

        for (size_t f = 1; f <= ctx->num_funcs; f++) {
            if (ctx->funcs[f].addr == event->npc) {
                fprintf(ctx->fp_log, "%08x enter %s\n", event->timestamp,
                        ctx->funcs[f].name);
                break;
            }

            if (ctx->funcs[f].addr > event->pc) {
                if (ctx->funcs[f - 1].name != to) {
                    fprintf(ctx->fp_log, "%08x leave %s\n", event->timestamp,
                            ctx->funcs[f - 1].name);
                }
                break;
            }
            if (f == ctx->num_funcs) {
                if (ctx->funcs[ctx->num_funcs - 1].name != to) {
                    fprintf(ctx->fp_log, "%08x leave %s\n", event->timestamp,
                            ctx->funcs[ctx->num_funcs - 1].name);
                }
            }
        }
    }
}

static void ctm_event_handler(void *ctx_void,
                              const struct osd_ctm_desc *ctm_desc,
                              const struct osd_ctm_event *event)
{
    int rv;
    struct osd_coretracelogger_ctx *ctx = ctx_void;

    if (!ctx->fp_log) {
        return;
    }

    if (event->overflow) {
        rv = fprintf(ctx->fp_log, "Overflow, missed %u events\n",
                     event->overflow);
        if (rv < 0) {
            err(ctx->log_ctx, "Unable to write CTM event to log file.");
        }
        return;
    }

    if (!ctx->funcs) {
        rv = fprintf(ctx->fp_log, "%08x %d %d %d %d %016lx %016lx\n",
                     event->timestamp, event->is_modechange, event->is_call,
                     event->is_ret, event->mode, event->pc, event->npc);
        if (rv < 0) {
            err(ctx->log_ctx, "Unable to write CTM event to log file.");
        }
    } else {
        print_with_elfdata(ctx, event);
    }
}

API_EXPORT
osd_result osd_coretracelogger_new(struct osd_coretracelogger_ctx **ctx,
                                   struct osd_log_ctx *log_ctx,
                                   const char *host_controller_address,
                                   uint16_t ctm_di_addr)
{
    osd_result rv;

    struct osd_coretracelogger_ctx *c =
        calloc(1, sizeof(struct osd_coretracelogger_ctx));
    assert(c);

    c->log_ctx = log_ctx;
    c->ctm_di_addr = ctm_di_addr;
    c->ctm_event_handler.cb_fn = ctm_event_handler;
    c->ctm_event_handler.cb_arg = (void*)c;

    struct osd_hostmod_ctx *hostmod_ctx;
    rv = osd_hostmod_new(&hostmod_ctx, log_ctx, host_controller_address,
                         osd_cl_ctm_handle_event, (void*)&c->ctm_event_handler);
    assert(OSD_SUCCEEDED(rv));
    c->hostmod_ctx = hostmod_ctx;

    *ctx = c;

    return OSD_OK;
}

API_EXPORT
osd_result osd_coretracelogger_connect(struct osd_coretracelogger_ctx *ctx)
{
    return osd_hostmod_connect(ctx->hostmod_ctx);
}

API_EXPORT
osd_result osd_coretracelogger_disconnect(struct osd_coretracelogger_ctx *ctx)
{
    return osd_hostmod_disconnect(ctx->hostmod_ctx);
}

API_EXPORT
bool osd_coretracelogger_is_connected(struct osd_coretracelogger_ctx *ctx)
{
    return osd_hostmod_is_connected(ctx->hostmod_ctx);
}

static void free_elf_data(struct osd_coretracelogger_ctx *ctx)
{
    if (ctx->funcs) {
        for (size_t i = 0; i < ctx->num_funcs; i++) {
            free(ctx->funcs[i].name);
        }
        free(ctx->funcs);
    }
}

API_EXPORT
void osd_coretracelogger_free(struct osd_coretracelogger_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_coretracelogger_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    osd_hostmod_free(&ctx->hostmod_ctx);

    free_elf_data(ctx);

    free(ctx);
    *ctx_p = NULL;
}

API_EXPORT
osd_result osd_coretracelogger_start(struct osd_coretracelogger_ctx *ctx)
{
    osd_result rv;

    rv = osd_cl_ctm_get_desc(ctx->hostmod_ctx, ctx->ctm_di_addr, &ctx->ctm_desc);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    ctx->ctm_event_handler.ctm_desc = &ctx->ctm_desc;

    rv = osd_hostmod_mod_set_event_dest(ctx->hostmod_ctx, ctx->ctm_di_addr, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    rv = osd_hostmod_mod_set_event_active(ctx->hostmod_ctx,
                                          ctx->ctm_di_addr, true, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    return OSD_OK;
}

API_EXPORT
osd_result osd_coretracelogger_stop(struct osd_coretracelogger_ctx *ctx)
{
    osd_result rv;
    rv = osd_hostmod_mod_set_event_active(ctx->hostmod_ctx,
                                          ctx->ctm_di_addr, false, 0);
    if (rv == OSD_ERROR_TIMEDOUT) {
        rv = OSD_OK;
    }
    return rv;
}

API_EXPORT
osd_result osd_coretracelogger_set_log(struct osd_coretracelogger_ctx *ctx,
                                       FILE *fp)
{
    ctx->fp_log = fp;
    return OSD_OK;
}

API_EXPORT
osd_result osd_coretracelogger_set_elf(struct osd_coretracelogger_ctx *ctx,
                                       const char* elf_filename)
{
    osd_result retval;

    if (ctx->funcs) {
        free_elf_data(ctx);
    }

    ctx->num_funcs = 0;
    ctx->funcs = NULL;

    if (elf_filename == NULL) {
        return OSD_OK;
    }

    struct elf_function_table *tab = NULL;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        err(ctx->log_ctx, "Version mismatch between elf library and system.");
        return OSD_ERROR_FAILURE;
    }

    int fd = open(elf_filename, O_RDONLY, 0);
    if (fd < 0) {
        err(ctx->log_ctx, "Unable to open file %s: %s (%d)", elf_filename,
            strerror(errno), errno);
        return OSD_ERROR_FILE;
    }

    Elf *elf_object = elf_begin(fd, ELF_C_READ, NULL);
    if (elf_object == NULL) {
        err(ctx->log_ctx, "%s", elf_errmsg(-1));
        retval = OSD_ERROR_FAILURE;
        goto return_free_file;
    }

    Elf_Scn *sec = NULL;
    while ((sec = elf_nextscn(elf_object, sec)) != NULL) {
        GElf_Shdr shdr;
        gelf_getshdr(sec, &shdr);

        if (shdr.sh_type == SHT_SYMTAB) {
            Elf_Data *edata = NULL;
            edata = elf_getdata(sec, edata);

            size_t allsyms = shdr.sh_size / shdr.sh_entsize;

            size_t f = 0;
            for (size_t i = 0; i < allsyms; i++) {
                GElf_Sym sym;
                gelf_getsym(edata, i, &sym);

                if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC
                    || ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE) {
                    f++;
                }
            }

            size_t base = ctx->num_funcs;
            ctx->num_funcs += f;
            tab = realloc(tab,
                          ctx->num_funcs * sizeof(struct elf_function_table));

            f = 0;
            for (size_t i = 0; i < allsyms; i++) {
                GElf_Sym sym;
                gelf_getsym(edata, i, &sym);

                if ((ELF32_ST_TYPE(sym.st_info) == STT_FUNC)
                        || (ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE)) {
                    tab[base + f].addr = sym.st_value;
                    tab[base + f].name = strdup(
                            elf_strptr(elf_object, shdr.sh_link, sym.st_name));
                    f++;
                }
            }
        }
    }

    ctx->funcs = malloc(sizeof(struct elf_function_table) * ctx->num_funcs);

    for (size_t i = 0; i < ctx->num_funcs; i++) {
        uint64_t min = -1;
        struct elf_function_table *minp = NULL;
        for (size_t j = 0; j < ctx->num_funcs; j++) {
            if (tab[j].addr < min) {
                min = tab[j].addr;
                minp = &tab[j];
            }
        }
        ctx->funcs[i].addr = minp->addr;
        ctx->funcs[i].name = minp->name;
        minp->addr = -1;
    }

    retval = OSD_OK;

    free(tab);
    elf_end(elf_object);

return_free_file:
    close(fd);

    return retval;
}
