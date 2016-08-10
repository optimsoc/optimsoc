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

#include <string.h>
#include <stdlib.h>

void parse_options(char* str, struct glip_option* options[],
                   size_t *num_options);

void parse_options(char* str, struct glip_option* options[],
                   size_t *num_options)
{
    char *opt;
    int count = 0;

    /* count the number of options */
    char *strcp = strdup(str);
    opt = strtok(strcp, ",");
    if (opt != 0) {
        count++;
        while (strtok(0, ",") != 0) {
            count++;
        }
    }
    free(strcp);

    *num_options = count;
    if (count <= 0) {
        return;
    }

    struct glip_option *optvec;
    optvec = calloc(count, sizeof(struct glip_option));

    strcp = strdup(str);
    opt = strtok(str, ",");
    int i = 0;
    do {
        char *sep = index(opt, '=');
        if (sep) {
            optvec[i].name = strndup(opt, sep - opt);
            optvec[i].value = strndup(sep + 1, opt + strlen(opt) - sep);
        } else {
            optvec[i].name = strdup(opt);
            optvec[i].value = 0;
        }
        opt = strtok(0, ",");
        i++;
    } while (opt);

    free(strcp);
    *options = optvec;
}
