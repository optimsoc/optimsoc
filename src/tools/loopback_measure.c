/* Copyright (c) 2014-2015 by the author(s)
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

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <libglip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

/* default value for the --transfer-size argument (10 MB) */
#define TRANSFER_SIZE_DEFAULT (10*1024*1024)

/* write block size in bytes,
 * can be any number smaller 256 or multiples of 256 */
#define WRITE_BLOCK_SIZE 6
/* read block size in bytes */
#define READ_BLOCK_SIZE 2048
/* timeout for blocking reads */
#define READ_TIMEOUT_MS 100

extern void parse_options(char* str, struct glip_option* options[],
                          size_t *num_options);

void display_help(void);

void display_help(void)
{
    printf("Usage: glip_loopback_measure -b BACKEND [OPTIONAL_ARGS]\n\n"
           "Write data to the target and read it back, measuring the \n"
           "transfer data rate.\n"
           "\n"
           "REQUIRED ARGUMENTS\n"
           "-b|--backend BACKEND\n"
           "  the backend to be used. See below for available backends.\n"
           "\n"
           "OPTIONAL ARGUMENTS\n"
           "-s|--transfer-size SIZE\n"
           "  number of bytes to be transferred in the test. To get \n"
           "  meaningful results, choose this parameter large enough to \n"
           "  transfer data for a couple of seconds at least.\n"
           "\n"
           "-o|--backend-options\n"
           "  options passed to the backend. Options are key=value pairs \n"
           "  separated by a comma.\n"
           "-n|--nonblock\n"
           "  use the non-blocking read/write functions of GLIP\n"
           "-h|--help\n"
           "  print this help message\n"
           "-v|--version\n"
           "  print the GLIP library version information\n"
           "\n"
           "EXAMPLES\n"
           "Run the program with the cypressfx2 backend, connecting to the \n"
           "USB device on bus 1 with address 2:\n"
           "$> glip_loopback_measure -b cypressfx2 -ousb_dev_bus=1,"
              "usb_dev_addr=2\n"
           "\n");

    printf("AVAILABLE BACKENDS\n");
    const char** name;
    size_t count;
    glip_get_backends(&name, &count);
    for (size_t i = 0; i < count; i++) {
        printf("- %s\n", name[i]);
    }

    free(name);

    printf("\n"
           "Consult the documentation for a list of available options for \n"
           "each backend.\n"
           "\n");
}

void display_version(void);
void display_version(void)
{
    const struct glip_version *v = glip_get_version();
    printf("libglip version: %d.%d.%d%s\n", v->major, v->minor, v->micro,
           v->suffix);
}

struct glip_ctx *glip_ctx;

size_t transfer_size = TRANSFER_SIZE_DEFAULT;

volatile size_t current_sent;
volatile size_t current_received;
volatile int send_done = 0;
int use_blocking_functions;

pthread_t read_thread;
pthread_t progressbar_thread;

void* read_from_target(void* ctx_void);
void exit_measurement(int exit_code);
void update_progressbar(void);
void* update_progressbar_thread(void*);

int main(int argc, char *argv[])
{
    int c, rv;

    int glip_backend_set = 0;
    char* backend_name;

    char *backend_optionstring;
    struct glip_option* backend_options;
    size_t num_backend_options = 0;
    use_blocking_functions = 1;

    assert((WRITE_BLOCK_SIZE < 256) || (WRITE_BLOCK_SIZE % 256 == 0));

    while (1) {
        static struct option long_options[] = {
            {"help",            no_argument,       0, 'h'},
            {"version",         no_argument,       0, 'v'},
            {"nonblock",        no_argument,       0, 'n'},
            {"backend",         required_argument, 0, 'b'},
            {"backend-options", required_argument, 0, 'o'},
            {"transfer-size",   required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "is:vnhb:o:s:", long_options,
                        &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now.   */
            if (long_options[option_index].flag != 0) {
                break;
            }
            break;
        case 'b':
            backend_name = strdup(optarg);
            glip_backend_set = 1;
            break;
        case 'o':
            backend_optionstring = optarg;
            parse_options(backend_optionstring, &backend_options,
                          &num_backend_options);
            break;
        case 's':
            transfer_size = strtoul(optarg, NULL, 10);
            break;
        case 'n':
            use_blocking_functions = 0;
            break;
        case 'v':
            display_version();
            return 0;
        case 'h':
            display_help();
            return 0;
        default:
            display_help();
            return -1;
        }
    }

    if (glip_backend_set == 0) {
        fprintf(stderr, "ERROR: You need to set a backend\n\n");
        display_help();
        return -1;
    }

    glip_new(&glip_ctx, backend_name, backend_options, num_backend_options);

    rv = glip_open(glip_ctx, 1);
    if (rv < 0) {
        fprintf(stderr, "ERROR: Cannot open backend\n");
        return -1;
    }

    rv = glip_logic_reset(glip_ctx);
    if (rv < 0) {
        fprintf(stderr, "ERROR: Cannot reset device\n");
        return -1;
    }

    /* create a dumb read thread, just discarding the data */
    rv = pthread_create(&read_thread, NULL,
                        read_from_target, (void*)glip_ctx);

    /* use a separate thread to update the progress bar */
    rv = pthread_create(&progressbar_thread, NULL, update_progressbar_thread,
                        NULL);

    struct timespec start, end;

    current_sent = 0;
    current_received = 0;

    printf("Running loopback test with %zu bytes of data. This may take a "
           "while ...\n", transfer_size);

    clock_gettime(CLOCK_MONOTONIC, &start);

    uint8_t data[WRITE_BLOCK_SIZE];
    for (size_t i = 0; i < WRITE_BLOCK_SIZE; i++) {
        data[i] = i % 256;
    }
    while (current_sent < transfer_size) {
        size_t size_written;
        size_t block_size = WRITE_BLOCK_SIZE;

        if ((current_sent + WRITE_BLOCK_SIZE) > transfer_size) {
            block_size = transfer_size - current_sent;
        }

        int sub_idx = current_sent % WRITE_BLOCK_SIZE;
        if (use_blocking_functions) {
            rv = glip_write_b(glip_ctx, 0, block_size - sub_idx,
                              &data[sub_idx], &size_written, 0);
        } else {
            rv = glip_write(glip_ctx, 0, block_size - sub_idx,
                            &data[sub_idx], &size_written);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "Error while writing to GLIP. rv = %d\n", rv);
            exit_measurement(1);
        }
        current_sent += size_written;
    }

    send_done = 1;

    /* wait for read to finish */
    pthread_join(read_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double diff = (end.tv_sec * 1.0 + end.tv_nsec / 1000000000.0) -
                  (start.tv_sec * 1.0 + start.tv_nsec / 1000000000.0);

    /* final update to the progress bar */
    pthread_cancel(progressbar_thread);
    pthread_join(progressbar_thread, NULL);
    update_progressbar();
    printf("\n\n");

    printf("Sent and received %zu bytes in %.03f seconds = %.01lf kiB/s "
           "(bidirectional)\n",
           current_sent, diff, (current_sent)/diff/1024);
    printf("Write block size: %u bytes, read block size: %u bytes\n",
           WRITE_BLOCK_SIZE, READ_BLOCK_SIZE);
    if (use_blocking_functions) {
        printf("Used blocking function calls (glip_read_b() and glip_write_b())\n");
    } else {
        printf("Used non-blocking function calls (glip_read() and glip_write())\n");
    }
    glip_close(glip_ctx);
    return 0;
}

void* read_from_target(void* ctx_void)
{
    struct glip_ctx *ctx = ctx_void;

    uint8_t data_read[READ_BLOCK_SIZE] = {0};
    size_t size_read;
    uint8_t data_exp = 0;

    int byte = 0;
    int rv;
    while (1) {
        if (use_blocking_functions) {
            rv = glip_read_b(ctx, 0, READ_BLOCK_SIZE, data_read, &size_read,
                             READ_TIMEOUT_MS);
        } else {
            rv = glip_read(ctx, 0, READ_BLOCK_SIZE, data_read, &size_read);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "Error while reading from GLIP. rv = %d\n", rv);
            exit_measurement(1);
        }
        current_received += size_read;

        /* verify received data */
        for (size_t i = 0; i < size_read; i++) {
            if (data_read[i] != data_exp) {
                fprintf(stderr, "Data verification failed: expected 0x%x, "
                        "got 0x%x at byte %d\n", data_exp, data_read[i], byte);
                exit_measurement(1);
            }
            byte++;
            data_exp = (data_exp + 1) % (WRITE_BLOCK_SIZE < 256 ?
                                         WRITE_BLOCK_SIZE : 256);
        }

        /* done! */
        if (send_done && current_received >= current_sent) {
            return NULL;
        }
    }

    return NULL;
}

void* update_progressbar_thread(void* arg)
{
    while (1) {
        update_progressbar();
        usleep(200*1000); /* 200 ms update interval */
    }
    return NULL;
}

void update_progressbar()
{
    int w = 50; /* progress bar width */

    double ratio  =  current_received/(double)transfer_size;
    int c = ratio * w;

    printf("% 3.0lf %% [", ratio * 100);

    for (int x=0; x<c; x++) printf("=");
    for (int x=c; x<w; x++) printf(" ");
    printf("] % 3.0lf KiB TX/% 3.0lf KiB RX\r", current_sent/1024.0,
           current_received/1024.0);
    fflush(stdout);
}

void exit_measurement(int exit_code)
{
    exit(exit_code);
}
