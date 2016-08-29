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
#include <assert.h>

const struct module_types module_lookup[6] = {
        { .name = "HOST" },
        { .name = "SCM" },
        { .name = "DEM-UART" },
        { .name = "MAM" },
        { .name = "STM" },
        { .name = "CTM" }
};

const uint16_t scmid = 0x1;

int osd_system_enumerate(struct osd_context *ctx) {
    uint16_t mod1_id, mod_num;
    osd_reg_read16(ctx, scmid, 0, &mod1_id);

    if (mod1_id != 0x1) {
        return OSD_E_CANNOTENUMERATE;
    }

    osd_reg_read16(ctx, scmid, 0x201, &mod_num);
    mod_num += 1;

    size_t size = sizeof(struct osd_system_info);
    size += sizeof(struct osd_module_info) * mod_num;

    ctx->system_info = calloc(1, size);

    ctx->system_info->num_modules = mod_num;

    osd_reg_read16(ctx, scmid, 0x200, &ctx->system_info->identifier);

    osd_reg_read16(ctx, scmid, 0x202, &ctx->system_info->max_pkt_len);

    ctx->system_info->modules[0].addr = 0;
    ctx->system_info->modules[0].type = 0;
    ctx->system_info->modules[0].version = 0;

    for (size_t i = 1; i < mod_num; i++) {
        struct osd_module_info *mod = &ctx->system_info->modules[i];
        mod->addr = i;
        osd_reg_read16(ctx, mod->addr, 0, &mod->type);
        if (mod->type == OSD_MOD_MAM) {
            ctx->system_info->num_memories++;

            struct osd_memory_descriptor *mem;

            uint16_t tmp;
            osd_reg_read16(ctx, mod->addr, 0x202, &tmp);
            assert (tmp <= 8);

            mem = calloc(1, sizeof(struct osd_memory_descriptor) + tmp*sizeof(uint64_t)*2);
            mod->descriptor.memory = mem;
            mem->num_regions = tmp;

            osd_reg_read16(ctx, mod->addr, 0x200, &mem->data_width);
            osd_reg_read16(ctx, mod->addr, 0x201, &mem->addr_width);

            for (size_t j = 0; j < mem->num_regions; j++) {
                uint16_t regbase = 0x280 + 16*j;
                osd_reg_read16(ctx, mod->addr, regbase, &tmp);
                mem->regions[j].base_addr = tmp;
                osd_reg_read16(ctx, mod->addr, regbase+1, &tmp);
                mem->regions[j].base_addr |= ((uint64_t) tmp << 16);
                osd_reg_read16(ctx, mod->addr, regbase+2, &tmp);
                mem->regions[j].base_addr |= ((uint64_t) tmp << 32);
                osd_reg_read16(ctx, mod->addr, regbase+3, &tmp);
                mem->regions[j].base_addr |= ((uint64_t) tmp << 48);
                osd_reg_read16(ctx, mod->addr, regbase+4, &tmp);
                mem->regions[j].size = tmp;
                osd_reg_read16(ctx, mod->addr, regbase+5, &tmp);
                mem->regions[j].size |= ((uint64_t) tmp << 16);
                osd_reg_read16(ctx, mod->addr, regbase+6, &tmp);
                mem->regions[j].size |= ((uint64_t) tmp << 32);
                osd_reg_read16(ctx, mod->addr, regbase+7, &tmp);
                mem->regions[j].size |= ((uint64_t) tmp << 48);
            }
        } else if (mod->type == OSD_MOD_STM) {
            struct osd_stm_descriptor *stm;
            stm = calloc(1, sizeof(struct osd_stm_descriptor));
            mod->descriptor.stm = stm;

            osd_reg_read16(ctx, mod->addr, 0x200, &stm->xlen);
        } else if (mod->type == OSD_MOD_CTM) {
            struct osd_ctm_descriptor *ctm;
            ctm = calloc(1, sizeof(struct osd_ctm_descriptor));
            mod->descriptor.ctm = ctm;

            osd_reg_read16(ctx, mod->addr, 0x200, &ctm->addr_width);
            osd_reg_read16(ctx, mod->addr, 0x200, &ctm->data_width);
        }

        osd_reg_read16(ctx, mod->addr, 1, &mod->version);
    }

    return OSD_SUCCESS;
}

int osd_get_scm(struct osd_context *ctx, uint16_t *addr) {
    if (ctx->system_info->modules[1].type != OSD_MOD_SCM) {
        return OSD_E_GENERIC;
    }

    *addr = 1;
    return OSD_SUCCESS;
}

/**
 * Get all MAM elements that are available in the system
 *
 * @param[in] ctx the context
 * @param[out] memories an array of memory addresses
 *                      The memory for this array is initialized in this
 *                      function and the caller is responsible for freeing it!
 * @param[out] size number of elements in the @p memories array
 * @return OSD_SUCCESS on success
 * @return one of the OSD_E_* constants in case of an error
 */
OSD_EXPORT
int osd_get_memories(struct osd_context *ctx,
                     uint16_t **memories, size_t *num) {

    uint16_t* memories_i;
    size_t num_memories_i;

    num_memories_i = ctx->system_info->num_memories;
    printf("found %d memories in the system\n", num_memories_i);

    memories_i = malloc(sizeof(uint16_t) * num_memories_i);

    uint16_t num_mod = ctx->system_info->num_modules;

    for (uint16_t i = 0, m = 0; (i < num_mod) && (m < num_memories_i); i++) {
        if (ctx->system_info->modules[i].type == OSD_MOD_MAM) {
            memories_i[m++] = i;
        }
    }

    *memories = memories_i;
    *num = num_memories_i;

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_memory_descriptor(struct osd_context *ctx, uint16_t addr,
                              struct osd_memory_descriptor **desc) {
    if (ctx->system_info->modules[addr].type != OSD_MOD_MAM) {
        return OSD_E_GENERIC;
    }

    unsigned num_mem_regions =
        ctx->system_info->modules[addr].descriptor.memory->num_regions;

    size_t sz = sizeof(struct osd_memory_descriptor)
        + num_mem_regions * sizeof(uint64_t) * 2;

    *desc = malloc(sz);

    memcpy(*desc, ctx->system_info->modules[addr].descriptor.memory, sz);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_system_identifier(struct osd_context *ctx, uint16_t *id) {
    *id = ctx->system_info->identifier;
    return OSD_SUCCESS;
}

OSD_EXPORT
size_t osd_get_max_pkt_len(struct osd_context *ctx) {
    return ctx->system_info->max_pkt_len;
}

OSD_EXPORT
int osd_get_num_modules(struct osd_context *ctx, uint16_t *n) {
    *n = ctx->system_info->num_modules;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_module_name(struct osd_context *ctx, uint16_t id,
                        char **name) {
    uint16_t type = ctx->system_info->modules[id].type;
    if (type > modules_max_id) {
        *name = strdup("UNKNOWN");
    }
    *name = strdup(module_lookup[type].name);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_print_module_info(struct osd_context *ctx, uint16_t id,
                          FILE* fh, int indent) {
    struct osd_module_info *mod = &ctx->system_info->modules[id];

    if (!mod) {
        return OSD_E_GENERIC;
    }

    char *indentstring = malloc(indent+1);
    memset(indentstring, 0x20, indent);
    indentstring[indent] = 0;

    fprintf(fh, "%sversion: %04x\n", indentstring, mod->version);

    struct osd_memory_descriptor *mem;
    struct osd_stm_descriptor *stm;
    struct osd_ctm_descriptor *ctm;
    switch (mod->type) {
        case OSD_MOD_STM:
            stm = mod->descriptor.stm;
            fprintf(fh, "%sxlen: %d\n", indentstring, stm->xlen);
            break;
        case OSD_MOD_CTM:
            ctm = mod->descriptor.ctm;
            fprintf(fh, "%saddr_width: %d\n", indentstring, ctm->addr_width);
            fprintf(fh, "%sdata_width: %d\n", indentstring, ctm->data_width);
            break;
        case OSD_MOD_MAM:
            mem = mod->descriptor.memory;
            fprintf(fh, "%sdata width: %d, ", indentstring,
                    mem->data_width);
            fprintf(fh, "address width: %d\n", mem->addr_width);
            fprintf(fh, "%snumber of regions: %d\n", indentstring, mem->num_regions);
            for (int r = 0; r < mem->num_regions; r++) {
                fprintf(fh, "%s  [%d] base address: 0x%016lx, ", indentstring,
                        r, mem->regions[r].base_addr);
                fprintf(fh, "memory size: %ld Bytes\n", mem->regions[r].size);
            }
            break;
        default:
            break;
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_module_is_terminal(struct osd_context *ctx, uint16_t id) {
    uint16_t type = ctx->system_info->modules[id].type;

    switch (type) {
        case OSD_MOD_DEM_UART:
            return 1;
            break;
        default:
            return 0;
            break;
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
uint16_t osd_modid2addr(struct osd_context *ctx, uint16_t id) {
    return ctx->system_info->modules[id].addr;
}

OSD_EXPORT
uint16_t osd_addr2modid(struct osd_context *ctx, uint16_t addr) {
    for (size_t i = 0; i < ctx->system_info->num_modules; i++) {
        if (ctx->system_info->modules[i].addr == addr) {
            return i;
        }
    }
    return 0;
}
