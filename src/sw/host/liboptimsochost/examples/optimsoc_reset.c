/**
 * This file is part of liboptimsoc.
 *
 * liboptimsoc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with liboptimsoc. If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * OpTiMSoC Debug Command Line Interface
 *
 * This example program uses liboptimsoc to create a simple command-line
 * interface for talking to the OpTiMSoC system.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Michael Tempelmeier, michael.tempelmeier@tum.de
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <signal.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <optimsochost/liboptimsochost.h>

struct optimsoc_ctx *ctx;
FILE *trace_file;
FILE *nrm_stat_file;

static void connect(optimsoc_backend_id backend)
{
    int err;

    err = optimsoc_new(&ctx, backend, 0, 0);
    if (err < 0) {
        printf("Unable to create liboptimsoc context object.\n");
        exit(EXIT_FAILURE);
    }

    err = optimsoc_connect(ctx);
    if (err < 0) {
        printf("Unable to connect to target system.\n");
        exit(EXIT_FAILURE);
    }
}

static int disconnect(void)
{
    int err;

    printf("Disconnecting from system...");
    fflush(stdout);

    err = optimsoc_disconnect(ctx);
    if (err < 0) {
        return err;
    }

    err = optimsoc_free(ctx);
    if (err < 0) {
        return err;
    }

    if (trace_file) {
        fclose(trace_file);
    }

    printf(" done\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int c;
    int interactive_mode = 0;
    optimsoc_backend_id backend = OPTIMSOC_BACKEND_DBGNOC;

    trace_file = 0;

    /* connect to target system */
    connect(backend);

    optimsoc_reset(ctx);

    disconnect();

    return EXIT_SUCCESS;
}
