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

#include <gelf.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define STM_PRINT_CHARS 256

struct stm_log_desc {
    FILE *fh;
    uint16_t id;
    uint16_t xlen;
    char printf_buf[STM_PRINT_CHARS];
};

static void stm_simprint(struct osd_context *ctx, struct stm_log_desc* desc,
                         uint32_t timestamp, char value) {
    FILE *fh = desc->fh;

    int do_print = 0;

    /* simprint */
    if (value == '\n') {
        // Do the actual printf on newline
        do_print = 1;
    } else {
        // Find the actual position of the character by iterating
        for (unsigned int i = 0; i < (STM_PRINT_CHARS); i++) {
            // If this is the current end of string..
            if (desc->printf_buf[i] == '\0') {
                // .. put the character on it
                desc->printf_buf[i] = value;
                // If we approach the end of the text width, we signal this
                // with three dots and force printing
                if (i == (STM_PRINT_CHARS)-4) {
                    desc->printf_buf[STM_PRINT_CHARS-3] = '.';
                    desc->printf_buf[STM_PRINT_CHARS-2] = '.';
                    desc->printf_buf[STM_PRINT_CHARS-1] = '.';
                    desc->printf_buf[STM_PRINT_CHARS] = '\0';
                    do_print = 1;
                } else {
                    // otherwise simply mark new end
                    desc->printf_buf[i+1] = '\0';
                }
                break;
            }
        }
    }

    if (do_print) {
        // DEBUG: Also send to STDOUT
        printf("[STM %03d] %08x %s\n", desc->id, timestamp, desc->printf_buf);

        fprintf(fh, "%08x %s\n", timestamp, desc->printf_buf);
        fflush(fh);
        desc->printf_buf[0] = '\0';
    }
}

static void stm_log_handler(struct osd_context *ctx, void* arg, uint16_t* packet) {
    struct stm_log_desc *desc = (struct stm_log_desc*) arg;
    FILE *fh = desc->fh;

    uint32_t timestamp;
    uint16_t id;

    timestamp = (packet[4] << 16) | packet[3];
    id = packet[5];
    uint64_t value;

    if (desc->xlen == 32) {
        if (packet[0] != 7) {
            assert((packet[2] >> 11) & 0x1);

            fprintf(fh, "Overflow, missed %d events\n", packet[3] & 0x3ff);
            return;
        }

        value = ((uint32_t)packet[7] << 16) | packet[6];
        fprintf(fh, "%08x %04x %08x\n", timestamp, id, (uint32_t)value);
    } else {
        if (packet[0] != 9) {
            assert((packet[2] >> 11) & 0x1);

            fprintf(fh, "Overflow, missed %d events\n", packet[3] & 0x3ff);
            return;
        }
        value = ((uint64_t)packet[9] << 48) | ((uint64_t)packet[8] << 32) | ((uint64_t)packet[7] << 16) | packet[6];
        fprintf(fh, "%08x %04x %016lx\n", timestamp, id, value);
    }

    // Additionally convert printf() into character strings inside the log
    if (id == 4) {
        stm_simprint(ctx, desc, timestamp, value);
    }

    return;
}


OSD_EXPORT
int osd_stm_log(struct osd_context *ctx, uint16_t modid, char *filename) {
    struct osd_stm_descriptor *stm = ctx->system_info->modules[modid].descriptor.stm;
    struct stm_log_desc *d = calloc(sizeof(struct stm_log_desc), 1);

    d->xlen = stm->xlen;
    d->fh = fopen(filename, "w");
    d->id = modid;
    osd_module_claim(ctx, modid);

    osd_module_register_handler(ctx, modid, OSD_EVENT_TRACE, (void*) d,
                                stm_log_handler);
    osd_module_unstall(ctx, modid);
    return 0;
}

struct elf_function_table {
    uint64_t addr;
    char     *name;
};

struct ctm_log_handle {
    FILE    *fh;
    uint16_t addr_width;
    size_t num_funcs;
    struct elf_function_table *funcs;
};

static void ctm_log_handler (struct osd_context *ctx, void* arg, uint16_t* packet) {
    struct ctm_log_handle *log = (struct ctm_log_handle *) arg;
    uint32_t timestamp;
    uint8_t modechange, call, ret, overflow;
    uint8_t mode;
    uint64_t pc, npc;

    overflow = (packet[2] >> 11) & 0x1;

    if (overflow) {
        fprintf(log->fh, "Overflow, missed %d events\n", packet[3] & 0x3ff);
        return;
    }

    timestamp = (packet[4] << 16) | packet[3];
    size_t index;
    if (log->addr_width == 64) {
      npc = ((uint64_t)packet[8] << 48) | ((uint64_t)packet[7] << 32) | ((uint64_t)packet[6] << 16) | packet[5];
      pc = ((uint64_t)packet[12] << 48) | ((uint64_t)packet[11] << 32) | ((uint64_t)packet[10] << 16) | packet[9];
      index = 13;
    } else {
      assert(log->addr_width == 32);
      npc = ((uint64_t)packet[6] << 16) | packet[5];
      pc = ((uint64_t)packet[8] << 16) | packet[7];
      index = 9;
    }
    modechange = (packet[index] >> 4) & 0x1;
    call = (packet[index] >> 3) & 0x1;
    ret = (packet[index] >> 2) & 0x1;
    mode = packet[index] & 0x3;

    if (!log->funcs) {
        fprintf(log->fh, "%08x %d %d %d %d %016lx %016lx\n", timestamp, modechange, call, ret, mode, pc, npc);
    } else {
        if (modechange) {
            fprintf(log->fh, "%08x change mode to %d\n", timestamp, mode);
        } else if (call) {
            for (size_t f = 0; f < log->num_funcs; f++) {
                if (log->funcs[f].addr == npc) {
                    fprintf(log->fh, "%08x enter %s\n", timestamp, log->funcs[f].name);
                    break;
                }
            }
        } else if (ret) {
            char* to = 0;
            for (size_t f = 1; f <= log->num_funcs; f++) {
                if (log->funcs[f].addr > npc) {
                    to = log->funcs[f-1].name;
                    break;
                }
                if (f == log->num_funcs) {
                    to = log->funcs[log->num_funcs-1].name;
                }
            }

            for (size_t f = 1; f <= log->num_funcs; f++) {
                if (log->funcs[f].addr == npc) {
                    fprintf(log->fh, "%08x enter %s\n", timestamp, log->funcs[f].name);
                    break;
                }

                if (log->funcs[f].addr > pc) {
                    if (log->funcs[f-1].name != to) {
                        fprintf(log->fh, "%08x leave %s\n", timestamp, log->funcs[f-1].name);
                    }
                    break;
                }
                if (f == log->num_funcs) {
                    if (log->funcs[log->num_funcs-1].name != to) {
                        fprintf(log->fh, "%08x leave %s\n", timestamp, log->funcs[log->num_funcs-1].name);
                    }
                }
            }
        }
    }
    return;
}

OSD_EXPORT
int osd_ctm_log(struct osd_context *ctx, uint16_t modid, char *filename, char *elffile) {
    struct ctm_log_handle *log = malloc(sizeof(struct ctm_log_handle));
    log->fh = fopen(filename, "w");
    struct osd_ctm_descriptor *ctm = ctx->system_info->modules[modid].descriptor.ctm;
    log->addr_width = ctm->addr_width;
    log->num_funcs = 0;
    log->funcs = 0;
    // Load the symbols from ELF
    do {
        struct elf_function_table *tab = 0;
        int fd = open(elffile, O_RDONLY , 0);
        if (fd < 0) {
            break;
        }

        if (elf_version(EV_CURRENT) == EV_NONE) {
           break;
        }

        Elf *elf_object = elf_begin(fd , ELF_C_READ , NULL);
        if (elf_object == NULL) {
            printf("%s\n", elf_errmsg(-1));
            break;
        }

        Elf_Scn *sec = 0;
        while((sec = elf_nextscn(elf_object, sec)) != NULL)
        {
            GElf_Shdr shdr;
            gelf_getshdr(sec, &shdr);

            if (shdr.sh_type == SHT_SYMTAB) {
                Elf_Data *edata = 0;
                edata = elf_getdata(sec, edata);

                size_t allsyms = shdr.sh_size / shdr.sh_entsize;

                size_t f = 0;
                for(size_t i = 0; i < allsyms; i++)
                {
                    GElf_Sym sym;
                    gelf_getsym(edata, i, &sym);

                    if ((ELF32_ST_TYPE(sym.st_info) == STT_FUNC) ||
                            (ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE)) {
                        f++;
                    }
                }

                size_t base = log->num_funcs;
                log->num_funcs += f;
                tab = realloc(tab, log->num_funcs * sizeof(struct elf_function_table));

                f = 0;
                for(size_t i = 0; i < allsyms; i++)
                {
                    GElf_Sym sym;
                    gelf_getsym(edata, i, &sym);

                    if ((ELF32_ST_TYPE(sym.st_info) == STT_FUNC) ||
                            (ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE)) {
                        tab[base+f].addr = sym.st_value;
                        tab[base+f].name = strdup(elf_strptr(elf_object, shdr.sh_link, sym.st_name));
                        f++;
                    }
                }
            }
        }

        log->funcs = malloc(sizeof(struct elf_function_table) * log->num_funcs);

        for (size_t i = 0; i < log->num_funcs; i++) {
            uint64_t min = -1;
            struct elf_function_table *minp = 0;
            for (size_t j = 0; j < log->num_funcs; j++) {
                if (tab[j].addr < min) {
                    min = tab[j].addr;
                    minp = &tab[j];
                }
            }
            log->funcs[i].addr = minp->addr;
            log->funcs[i].name = minp->name;
            minp->addr = -1;
        }

        free(tab);
    } while (0);

    osd_module_claim(ctx, modid);
    osd_module_register_handler(ctx, modid, OSD_EVENT_TRACE, (void*) log,
                                ctm_log_handler);
    osd_module_unstall(ctx, modid);
    return 0;
}
