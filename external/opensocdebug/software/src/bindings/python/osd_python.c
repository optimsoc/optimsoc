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

#include "osd_python.h"

#include <opensocdebug.h>
#include <unistd.h>

struct osd_context *ctx = 0;

void python_osd_init() {
    if (ctx == 0) {
        osd_new(&ctx, OSD_MODE_DAEMON, 0, 0);

        if (osd_connect(ctx) != OSD_SUCCESS) {
            printf("Cannot connect\n");
            exit(1);
        }

    }
}

void python_osd_reset(int halt) {
    osd_reset_system(ctx, halt);
}

void python_osd_start(void) {
    osd_start_cores(ctx);
}

void python_osd_wait(int secs) {
    sleep(secs);
}

PyObject *python_osd_get_num_modules() {
    uint16_t n;
    osd_get_num_modules(ctx, &n);
    return PyInt_FromLong(n);
}

PyObject *python_osd_get_module_name(uint16_t id) {
    char *name;
    osd_get_module_name(ctx, id, &name);
    PyObject *str = PyString_FromString(name);
    free(name);
    return str;
}

int python_osd_mem_loadelf(size_t modid, char* filename, int verify) {
    return osd_memory_loadelf(ctx, modid, filename, verify);
}

int python_osd_stm_log(size_t modid, char* filename) {
    return osd_stm_log(ctx, modid, filename);
}

int python_osd_ctm_log(size_t modid, char* filename) {
    return osd_ctm_log(ctx, modid, filename, 0);
}

int python_osd_ctm_log_symbols(size_t modid, char* filename, char* elffile) {
    return osd_ctm_log(ctx, modid, filename, elffile);
}
