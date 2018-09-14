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

#include <osd/memaccess.h>
#include <osd/module.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/cl_scm.h>
#include "osd-private.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <gelf.h>

/**
 * Memory Access context
 */
struct osd_memaccess_ctx {
    struct osd_hostmod_ctx *hostmod_ctx;
    struct osd_log_ctx *log_ctx;
};


API_EXPORT
osd_result osd_memaccess_new(struct osd_memaccess_ctx **ctx,
                             struct osd_log_ctx *log_ctx,
                             const char *host_controller_address)
{
    osd_result rv;

    struct osd_memaccess_ctx *c = calloc(1, sizeof(struct osd_memaccess_ctx));
    assert(c);

    c->log_ctx = log_ctx;

    struct osd_hostmod_ctx *hostmod_ctx;
    rv = osd_hostmod_new(&hostmod_ctx, log_ctx, host_controller_address,
                         NULL, NULL);
    assert(OSD_SUCCEEDED(rv));
    c->hostmod_ctx = hostmod_ctx;

    *ctx = c;

    return OSD_OK;
}

API_EXPORT
osd_result osd_memaccess_connect(struct osd_memaccess_ctx *ctx)
{
    return osd_hostmod_connect(ctx->hostmod_ctx);
}

API_EXPORT
osd_result osd_memaccess_disconnect(struct osd_memaccess_ctx *ctx)
{
    return osd_hostmod_disconnect(ctx->hostmod_ctx);
}

API_EXPORT
bool osd_memaccess_is_connected(struct osd_memaccess_ctx *ctx)
{
    return osd_hostmod_is_connected(ctx->hostmod_ctx);
}

API_EXPORT
void osd_memaccess_free(struct osd_memaccess_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_memaccess_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    osd_hostmod_free(&ctx->hostmod_ctx);

    free(ctx);
    *ctx_p = NULL;
}

API_EXPORT
osd_result osd_memaccess_cpus_start(struct osd_memaccess_ctx *ctx,
                                    unsigned int subnet_addr)
{
    return osd_cl_scm_cpus_start(ctx->hostmod_ctx, subnet_addr);
}

API_EXPORT
osd_result osd_memaccess_cpus_stop(struct osd_memaccess_ctx *ctx,
                                   unsigned int subnet_addr)
{
    return osd_cl_scm_cpus_stop(ctx->hostmod_ctx, subnet_addr);
}

static bool is_supported_mam(struct osd_module_desc *mod)
{
    return mod->vendor == OSD_MODULE_VENDOR_OSD &&
           mod->type == OSD_MODULE_TYPE_STD_MAM &&
           mod->version == 0;
}

API_EXPORT
osd_result osd_memaccess_find_memories(struct osd_memaccess_ctx *ctx,
                                       unsigned int subnet_addr,
                                       struct osd_mem_desc **memories,
                                       size_t *num_memories)
{
    osd_result rv;
    osd_result retval = OSD_OK;
    struct osd_module_desc *mods;
    size_t mods_len;
    rv = osd_hostmod_get_modules(ctx->hostmod_ctx, subnet_addr, &mods,
                                 &mods_len);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    unsigned int mem_cnt = 0;
    for (unsigned int i = 0; i < mods_len; i++) {
        if (is_supported_mam(&mods[i])) {
            mem_cnt++;
        }
    }

    struct osd_mem_desc *mems = calloc(mem_cnt, sizeof(struct osd_mem_desc));

    unsigned int mem_nr = 0;
    for (unsigned int i = 0; i < mods_len; i++) {
        if (is_supported_mam(&mods[i])) {
            rv = osd_cl_mam_get_mem_desc(ctx->hostmod_ctx, mods[i].addr,
                                         &mems[mem_nr]);
            if (OSD_FAILED(rv)) {
                err(ctx->log_ctx, "Unable to get information from MAM module "
                    "at address %u", mods[i].addr);
                retval = OSD_ERROR_PARTIAL_RESULT;
                // continue anyway
            }
            mem_nr++;
        }
    }

    free(mods);

    *memories = mems;
    *num_memories = mem_cnt;

    return retval;
}

API_EXPORT
osd_result osd_memaccess_loadelf(struct osd_memaccess_ctx *ctx,
                                 const struct osd_mem_desc* mem_desc,
                                 const char* elf_file_path, bool verify)
{
    int fd;
    Elf *elf_object;
    size_t num;
    int rv;
    osd_result retval;
    osd_result osd_rv;

    if (!osd_hostmod_is_connected(ctx->hostmod_ctx)) {
        return OSD_ERROR_NOT_CONNECTED;
    }

    if (elf_version(EV_CURRENT) == EV_NONE) {
        err(ctx->log_ctx, "Version mismatch between elf library and system.");
        return OSD_ERROR_FAILURE;
    }

    fd = open(elf_file_path, O_RDONLY, 0);
    if (fd < 0) {
        err(ctx->log_ctx, "Unable to open file %s: %s (%d)", elf_file_path,
            strerror(errno), errno);
        return OSD_ERROR_FILE;
    }

    elf_object = elf_begin(fd, ELF_C_READ, NULL);
    if (elf_object == NULL) {
        err(ctx->log_ctx, "%s", elf_errmsg(-1));
        retval = OSD_ERROR_FAILURE;
        goto return_free_file;
    }

    // Load program headers
    rv = elf_getphdrnum(elf_object, &num);
    if (rv != 0) {
        err(ctx->log_ctx, "%s", elf_errmsg(-1));
        retval = OSD_ERROR_FAILURE;
        goto return_free_elf;
    }

    for (size_t i = 0; i < num; i++) {
        info(ctx->log_ctx, "Load program header %zu", i);
        GElf_Phdr phdr;
        Elf_Data *data;
        if (gelf_getphdr(elf_object, i, &phdr) != &phdr) {
            err(ctx->log_ctx, "%s", elf_errmsg(-1));
            retval = OSD_ERROR_FAILURE;
            goto return_free_elf;
        }

        data = elf_getdata_rawchunk(elf_object, phdr.p_offset, phdr.p_filesz,
                                    ELF_T_BYTE);
        if (data) {
            osd_rv = osd_cl_mam_write(mem_desc, ctx->hostmod_ctx,
                                      data->d_buf, data->d_size, phdr.p_paddr);
            if (OSD_FAILED(osd_rv)) {
                retval = osd_rv;
                goto return_free_elf;
            }
        }

        Elf32_Word init_with_zero = phdr.p_memsz - phdr.p_filesz;
        if (init_with_zero > 0) {
            void *zeroes = calloc(1, phdr.p_memsz - phdr.p_filesz);
            assert(zeroes);
            osd_rv = osd_cl_mam_write(mem_desc, ctx->hostmod_ctx,
                             zeroes, init_with_zero,
                             phdr.p_paddr + phdr.p_filesz);
            free(zeroes);
            if (OSD_FAILED(osd_rv)) {
                retval = osd_rv;
                goto return_free_elf;
            }
        }
    }

    if (!verify) {
        retval = OSD_OK;
        goto return_free_elf;
    }

    for (size_t i = 0; i < num; i++) {
        info(ctx->log_ctx, "Verify program header %zu", i);
        GElf_Phdr phdr;
        Elf_Data *data;
        if (gelf_getphdr(elf_object, i, &phdr) != &phdr) {
            retval = OSD_ERROR_FAILURE;
            goto return_free_elf;
        }

        data = elf_getdata_rawchunk(elf_object, phdr.p_offset, phdr.p_filesz,
                                    ELF_T_BYTE);
        uint8_t *elf_data = data->d_buf;

        uint8_t *memory_data = malloc(data->d_size);
        assert(memory_data);
        osd_rv = osd_cl_mam_read(mem_desc, ctx->hostmod_ctx, memory_data,
                                 data->d_size, phdr.p_paddr);
        if (OSD_FAILED(osd_rv)) {
            free(memory_data);
            retval = osd_rv;
            goto return_free_elf;
        }

        for (size_t b = 0; b < data->d_size; b++) {
            if (memory_data[b] != elf_data[b]) {
                err(ctx->log_ctx, "Memory mismatch at byte 0x%zx. "
                    "Expected %02x, read %02x", b, elf_data[b], memory_data[b]);

                free(memory_data);
                retval = OSD_ERROR_MEM_VERIFY_FAILED;
                goto return_free_elf;
            }
        }

        free(memory_data);
    }

    retval = OSD_OK;

return_free_elf:
    elf_end(elf_object);

return_free_file:
    close(fd);

    return retval;
}
