/* Copyright (c) 2014-2017 by the author(s)
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
 * Loopback speed measurement tool
 *
 * Write data to a GLIP-attached device, and read back from it. Verify that the
 * read data is equal to the written data, and show the speed of the transfer.
 *
 * This tool is useful for testing of a new backend, and to check if the bringup
 * on a board was successful. The demos for different boards provided as part of
 * GLIP also use this tool.
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
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

/* default value for the --transfer-size argument (10 MB) */
#define TRANSFER_SIZE_DEFAULT (10*1024*1024)

/* write block size in bytes,
 * can be any number smaller 256 or multiples of 256 */
#define WRITE_BLOCK_SIZE_DEFAULT 256
/* read block size in bytes */
#define READ_BLOCK_SIZE_DEFAULT 2048
/* timeout for blocking reads */
#define READ_TIMEOUT_MS 100

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
           "--write-block-size SIZE\n"
           "  number of bytes to be transferred in one block (default: %d)\n"
           "--read-block-size SIZE\n"
           "  number of bytes to be read in one block (default: %d)\n"
           "\n"
           "-o|--backend-options\n"
           "  options passed to the backend. Options are key=value pairs \n"
           "  separated by a comma.\n"
           "-n|--nonblock\n"
           "  use the non-blocking read/write functions of GLIP\n"
           "-r|--random-data\n"
           "  write random data instead of linearly increasing sequences\n"
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
           "\n", WRITE_BLOCK_SIZE_DEFAULT, READ_BLOCK_SIZE_DEFAULT);

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
int random_data;
size_t read_block_size;
size_t write_block_size;
uint8_t *write_data;

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
    random_data = 0;
    read_block_size = READ_BLOCK_SIZE_DEFAULT;
    write_block_size = WRITE_BLOCK_SIZE_DEFAULT;

    //assert((WRITE_BLOCK_SIZE < 256) || (WRITE_BLOCK_SIZE % 256 == 0));

    while (1) {
        static struct option long_options[] = {
            {"help",             no_argument,       0, 'h'},
            {"version",          no_argument,       0, 'v'},
            {"nonblock",         no_argument,       0, 'n'},
            {"random-data",      no_argument,       0, 'r'},
            {"backend",          required_argument, 0, 'b'},
            {"backend-options",  required_argument, 0, 'o'},
            {"transfer-size",    required_argument, 0, 's'},
            {"read-block-size",  required_argument, 0, 'R'},
            {"write-block-size", required_argument, 0, 'W'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "is:vnrhb:o:s:R:W:", long_options,
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
            glip_parse_option_string(backend_optionstring, &backend_options,
                                     &num_backend_options);
            break;
        case 's':
            transfer_size = strtoul(optarg, NULL, 10);
            break;
        case 'n':
            use_blocking_functions = 0;
            break;
        case 'r':
            random_data = 1;
            break;
        case 'R':
            read_block_size = strtoul(optarg, NULL, 10);
            break;
        case 'W':
            write_block_size = strtoul(optarg, NULL, 10);
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

    glip_new(&glip_ctx, backend_name, backend_options, num_backend_options,
             NULL /* log_fn */);

    rv = glip_open(glip_ctx, 1);
    if (rv < 0) {
        fprintf(stderr, "ERROR: Cannot open backend\n");
        return -1;
    }

    // The UART backend has no way to auto-discover the width of the FIFO
    // on the target side. The loopback measure hardware demo uses 8 bit FIFOs,
    // tell GLIP about that.
    if (strcmp(backend_name, "uart") == 0) {
        rv = glip_set_fifo_width(glip_ctx, 1);
        assert(rv == 0);
    }

    unsigned int fifo_width_bytes = glip_get_fifo_width(glip_ctx);
    if (transfer_size % fifo_width_bytes != 0) {
        fprintf(stderr, "ERROR: The transfer size must be a multiple of the "
                "FIFO width, which is %d bytes for the chosen backend.\n",
                fifo_width_bytes);
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

    printf("Running loopback test with %zu bytes of data, writing %zu bytes "
           "in one block, \n"
           "and reading %zu bytes in one block.\n",
           transfer_size, write_block_size, read_block_size);

    clock_gettime(CLOCK_MONOTONIC, &start);

    /*
     * create the test data:
     * write_block_size bytes of data, either linearly increasing between 0x00
     * and 0xFF, or random.
     */
    write_data = calloc(write_block_size, sizeof(uint8_t));
    assert(write_data);

    if (random_data) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        srand(tv.tv_sec * 1000 + tv.tv_usec);
        for (size_t i = 0; i < write_block_size; i++) {
            write_data[i] = random() % 256;
        }
    } else {
        for (size_t i = 0; i < write_block_size; i++) {
            write_data[i] = i % 256;
        }
    }

    while (current_sent < transfer_size) {
        size_t size_written;
        size_t block_size = write_block_size;

        /*
         * calculate block_size of the last block of a transfer with all
         * remaining data (less than a full block)
         */
        if ((current_sent + write_block_size) > transfer_size) {
            block_size = transfer_size - current_sent;
        }

        int sub_idx = current_sent % write_block_size;
        if (use_blocking_functions) {
            rv = glip_write_b(glip_ctx, 0, block_size - sub_idx,
                              &write_data[sub_idx], &size_written, 0);
        } else {
            rv = glip_write(glip_ctx, 0, block_size - sub_idx,
                            &write_data[sub_idx], &size_written);
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
    printf("Write block size: %zu bytes, read block size: %zu bytes\n",
           write_block_size, read_block_size);
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

    uint8_t *read_data = calloc(read_block_size, sizeof(uint8_t));
    assert(read_data);
    size_t size_read;
    uint8_t data_exp = 0;

    unsigned int byte = 0;
    int rv;
    while (1) {
        if (use_blocking_functions) {
            rv = glip_read_b(ctx, 0, read_block_size, read_data, &size_read,
                             READ_TIMEOUT_MS);
        } else {
            rv = glip_read(ctx, 0, read_block_size, read_data, &size_read);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "Error while reading from GLIP. rv = %d\n", rv);
            exit_measurement(1);
        }
        current_received += size_read;

        /* verify received data */
        for (size_t i = 0; i < size_read; i++) {
            data_exp = write_data[byte % write_block_size];
            if (read_data[i] != data_exp) {
                fprintf(stderr, "Data verification failed: expected 0x%x, "
                        "got 0x%x at byte %d. Next byte: 0x%x\n",
                        data_exp, read_data[i], byte, read_data[i+1]);
                exit_measurement(1);
            }
            byte++;
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
