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
 * I/O stress test
 *
 * Write pseudo random numbers to a GLIP-attached device, or read back from it.
 * Both the host and the GLIP-attached device generate pseudo random numbers
 * by using the same seed.
 *
 * To test the resilience and simulate more realistic traffic patterns it is
 * possible to include random wait-cycles both on the device and the host side.
 *
 * This tool is useful for testing of a new backend, and to check if all data is
 * sent correctly when streaming large amounts of data in one direction.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
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
#include <byteswap.h>

/* default value for the --transfer-size argument (10 MB) */
#define TRANSFER_SIZE_DEFAULT (10)

/* default random seed */
#define RND_SEED_DEFAULT 1

/* write block size in bytes,
 * can be any number smaller 256 or multiples of 256 */
#define WRITE_BLOCK_SIZE_DEFAULT 2048
/* read block size in bytes */
#define READ_BLOCK_SIZE_DEFAULT 2048
/* timeout for blocking reads */
#define READ_TIMEOUT_MS 100

/* arrays for the implementation of the maximal length sequence lfsr. */
/* first element is the LFSR mask, then its a NULL terminated list of taps */
const uint16_t lfsr_taps16[] = { 0xFFFF, (1 << 15), (1 << 14), (1 << 12), (1
        << 3), 0 };
const uint32_t lfsr_taps32[] = { 0xFFFFFFFF, (1 << 31), (1 << 21), (1 << 1), (1
        << 0), 0 };

void display_help(void);
uint16_t lfsr_inc_16(const uint16_t *taps, uint16_t *lfsr);
uint32_t lfsr_inc_32(const uint32_t *taps, uint32_t *lfsr);

void display_help(void)
{
    printf("Usage: glip_io_stress_test -b BACKEND [OPTIONAL_ARGS]\n\n"
           "Write data to the target or read from it, measuring the \n"
           "transfer data rate.\n"
           "\n"
           "REQUIRED ARGUMENTS\n"
           "-b|--backend BACKEND\n"
           "  the backend to be used. See below for available backends.\n"
           "\n"
           "OPTIONAL ARGUMENTS\n"
           "-s|--transfer-size SIZE\n"
           "  number of megabytes to be transferred in the test. To get \n"
           "  meaningful results, choose this parameter large enough to \n"
           "  transfer data for a couple of seconds at least.\n"
           "  SIZE must be in range: 1 <= SIZE <= 65535\n"
           "-S|--seed SEED\n"
           "  seed to initialize the pseudo random number generator with.\n"
           "  Must be greater than zero.\n"
           "-W|--write-block-size SIZE\n"
           "  number of bytes to be transferred in one block (default: %d)\n"
           "-R|--read-block-size SIZE\n"
           "  number of bytes to be read in one block (default: %d)\n"
           "\n"
           "-o|--backend-options\n"
           "  options passed to the backend. Options are key=value pairs \n"
           "  separated by a comma.\n"
           "-r|--read\n"
           "  run read stress test.\n"
           "-w|--wait-random PERCENT\n"
           "  specify a PERCENT chance of waiting for 100ms between two\n"
           "  reads/writes on host side.\n"
           "  PERCENT must be in range: 1 <= PERCENT <= 100\n"
           "-d|--wait-random-device\n"
           "  induce random wait periods between reads/writes on device side.\n"
           "-n|--nonblock\n"
           "  use the non-blocking read/write functions of GLIP\n"
           "-h|--help\n"
           "  print this help message\n"
           "-v|--version\n"
           "  print the GLIP library version information\n"
           "\n"
           "EXAMPLES\n"
           "Run the program with the cypressfx3 backend, connecting to the \n"
           "USB device on bus 1 with address 2:\n"
           "$> glip_io_stress_test -b cypressfx3 -ousb_dev_bus=1,"
           "usb_dev_addr=2\n"
           "\n",
           WRITE_BLOCK_SIZE_DEFAULT, READ_BLOCK_SIZE_DEFAULT);

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

volatile size_t current_byte;
int use_blocking_functions;
int read_mode;
int random_wait_device;
uint8_t random_mode;
unsigned int fifo_width_bytes;
uint32_t lfsr32 = RND_SEED_DEFAULT;
uint16_t lfsr16 = RND_SEED_DEFAULT;
size_t read_block_size;
size_t write_block_size;
unsigned int progress_bar_update;

pthread_t read_thread;
pthread_t progressbar_thread;

void* read_from_target(void* ctx_void);
int write_stress_test(uint8_t *write_data);
int write_stress_test_read(uint8_t *read_data, size_t size_read);
int read_stress_test(void* ctx_void, uint8_t *read_data, uint16_t *rnd_word_16, size_t size_read);
int read_stress_test_compare(uint8_t *read_data, uint16_t *rnd_word_16, size_t size_read);
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
    read_mode = 0;
    random_wait_device = 0;
    random_mode = 0;
    current_byte = 0;
    read_block_size = READ_BLOCK_SIZE_DEFAULT;
    write_block_size = WRITE_BLOCK_SIZE_DEFAULT;
    progress_bar_update = 1;

    while (1) {
        static struct option long_options[] = {
            { "help",               no_argument,       0, 'h' },
            { "version",            no_argument,       0, 'v' },
            { "nonblock",           no_argument,       0, 'n' },
            { "read",               no_argument,       0, 'r' },
            { "wait-random-device", no_argument,       0, 'd' },
            { "backend",            required_argument, 0, 'b' },
            { "backend-options",    required_argument, 0, 'o' },
            { "transfer-size",      required_argument, 0, 's' },
            { "seed",               required_argument, 0, 'S' },
            { "wait-random",        required_argument, 0, 'w' },
            { "read-block-size",    required_argument, 0, 'R' },
            { "write-block-size",   required_argument, 0, 'W' },
            { 0, 0, 0, 0 } };
        int option_index = 0;

        c = getopt_long(argc, argv, "is:vnrdhb:o:s:R:W:S:w:", long_options,
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
            if (transfer_size <= 0 || transfer_size > 65535) {
                fprintf(stderr, "Error: SIZE must be between 1 and 65535.\n");
                display_help();
                return -1;
            }
            break;
        case 'S':
            lfsr32 = strtoul(optarg, NULL, 10);
            /* Check if the seed is valid. */
            if (lfsr32 == 0) {
                fprintf(stderr, "Error: The random seed must not be zero.");
                display_help();
                return -1;
            }
            lfsr16 = (uint16_t) lfsr32;
            break;
        case 'n':
            use_blocking_functions = 0;
            break;
        case 'r':
            read_mode = 1;
            break;
        case 'd':
            random_wait_device = 1;
            break;
        case 'w':
            random_mode = (uint8_t) strtoul(optarg, NULL, 10);
            if (random_mode <= 0 || random_mode > 100) {
                fprintf(stderr, "Error: PERCENT must be between 1 and 100.\n");
                display_help();
                return -1;
            }
            /* Initialize random generator. */
            struct timeval tv;
            gettimeofday(&tv, NULL);
            srand(tv.tv_sec * 1000 + tv.tv_usec);
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
    // on the target side. The I/O stresstest hardware demo uses 16 bit FIFOs,
    // tell GLIP about that.
    if (strcmp(backend_name, "uart") == 0) {
        rv = glip_set_fifo_width(glip_ctx, 2);
        assert(rv == 0);
    }

    fifo_width_bytes = glip_get_fifo_width(glip_ctx);

    if ((transfer_size * 1024 * 1024) % fifo_width_bytes != 0) {
        fprintf(stderr, "ERROR: The transfer size must be a multiple of the "
                "FIFO width, which is %d bytes for the chosen backend.\n",
                fifo_width_bytes);
        return -1;
    }

    printf("Detected interface width: %d-bit.\n", (int) (fifo_width_bytes * 8));

    rv = glip_logic_reset(glip_ctx);
    if (rv < 0) {
        fprintf(stderr, "ERROR: Cannot reset device\n");
        return -1;
    }

    /* create a read thread listening for data from device */
    rv = pthread_create(&read_thread, NULL, read_from_target, (void*) glip_ctx);

    /* use a separate thread to update the progress bar */
    rv = pthread_create(&progressbar_thread, NULL, update_progressbar_thread,
                        NULL);

    struct timespec start, end;

    printf("Running stress test with %zu MB of data, writing %zu bytes "
           "in one block, \n"
           "and reading %zu bytes in one block.\n",
           transfer_size, write_block_size, read_block_size);

    clock_gettime(CLOCK_MONOTONIC, &start);

    /* allocate memory for the test data to be written to the device. */
    uint8_t *write_data = calloc(write_block_size, sizeof(uint8_t));
    uint16_t *write_data_16 = (uint16_t*) write_data;
    uint32_t *write_data_32 = (uint32_t*) write_data;
    assert(write_data);

    /*
     * Set up the first data words to configure the device.
     *
     * The first word determines the test mode:
     *  - host -> device:                           0x420A
     *  - host -> device (with random wait cycles): 0x421A
     *  - device -> host:                           0x420B
     *  - device -> host (with random wait cycles): 0x421B
     *
     * The second word is the random seed the lfsr on the device is initialized
     * with.
     *
     * The third word is the number of megabytes to be read/written in the test.
     */
    if (fifo_width_bytes == 2) {
        write_data_16[0] = read_mode == 0 ? bswap_16(0x420A) : bswap_16(0x420B);
        /* Set up wait cycles on device if specified. */
        if (random_wait_device) {
            write_data_16[0] |= bswap_16(0x0010);
        }
        write_data_16[1] = bswap_16(lfsr16);
        write_data_16[2] = bswap_16((uint16_t) transfer_size);
    } else if (fifo_width_bytes == 4) {
        write_data_32[0] =
                read_mode == 0 ? bswap_32(0x0000420A) : bswap_32(0x0000420B);
        /* Set up wait cycles on device if specified. */
        if (random_wait_device) {
            write_data_32[0] |= bswap_32(0x00000010);
        }
        write_data_32[1] = bswap_32(lfsr32);
        write_data_32[2] = bswap_32((uint32_t) transfer_size);
    } else {
        fprintf(stderr,
                "ERROR: Invalid fifo width detected. Must be 2 or 4 bytes.\n");
        return -1;
    }

    size_t setup_bytes = fifo_width_bytes == 2 ? 6 : 12;
    size_t size_written;

    /* Set up stress test on target device. */
    if (use_blocking_functions) {
        rv = glip_write_b(glip_ctx, 0, setup_bytes, write_data, &size_written,
                          0);
    } else {
        rv = glip_write(glip_ctx, 0, setup_bytes, write_data, &size_written);
    }
    if (rv != 0 && rv != -ETIMEDOUT) {
        fprintf(stderr, "Error while writing to GLIP. rv = %d\n", rv);
        exit_measurement(1);
    }
    if (size_written < setup_bytes) {
        fprintf(stderr, "Error while setting up stress test on device. "
                "Failed to write all setup data to device.\n");
        return -1;
    }

    /*
     * Write stress test.
     */
    if (read_mode == 0) {
        rv = write_stress_test(write_data);
        if (rv < 0) {
            exit_measurement(1);
        }
    }

    /* wait for read to finish */
    pthread_join(read_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double diff = (end.tv_sec * 1.0 + end.tv_nsec / 1000000000.0)
            - (start.tv_sec * 1.0 + start.tv_nsec / 1000000000.0);

    /* final update to the progress bar */
    pthread_cancel(progressbar_thread);
    pthread_join(progressbar_thread, NULL);
    update_progressbar();
    printf("\n\n");

    if (read_mode == 0) {
        printf("Sent %zu bytes in %.03f seconds = %.01lf kiB/s\n", current_byte,
               diff, (current_byte) / diff / 1024);
    } else {
        printf("Received %zu bytes in %.03f seconds = %.01lf kiB/s\n",
               current_byte, diff, (current_byte) / diff / 1024);
    }
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

int write_stress_test(uint8_t *write_data)
{
    uint16_t *write_data_16 = (uint16_t*) write_data;
    uint32_t *write_data_32 = (uint32_t*) write_data;

    int rv;
    size_t size_written;

    /* Fill first block with random data. */
    if (fifo_width_bytes == 2) {
        for (size_t i = 0; i < write_block_size / 2; i++) {
            write_data_16[i] = bswap_16(lfsr_inc_16(lfsr_taps16, &lfsr16));
        }
    } else if (fifo_width_bytes == 4) {
        for (size_t i = 0; i < write_block_size / 4; i++) {
            write_data_32[i] = bswap_32(lfsr_inc_32(lfsr_taps32, &lfsr32));
        }
    }

    size_t transfer_size_bytes = transfer_size * 1024 * 1024;
    while (current_byte < transfer_size_bytes) {
        size_t block_size = write_block_size;

        /*
         * calculate block_size of the last block of a transfer with all
         * remaining data (less than a full block)
         */
        if ((current_byte + write_block_size) > transfer_size_bytes) {
            block_size = transfer_size_bytes - current_byte;
        }

        size_t sub_idx = current_byte % write_block_size;
        if (use_blocking_functions) {
            rv = glip_write_b(glip_ctx, 0, block_size - sub_idx,
                              &write_data[sub_idx], &size_written, 0);
        } else {
            rv = glip_write(glip_ctx, 0, block_size - sub_idx,
                            &write_data[sub_idx], &size_written);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "\nError while writing to GLIP. rv = %d\n", rv);
            return -1;
        }

        /* Generate new random data.*/
        if (fifo_width_bytes == 2) {
            for (size_t i = sub_idx / 2; i < (sub_idx + size_written) / 2;
                    i++) {
                write_data_16[i] = bswap_16(
                        lfsr_inc_16(lfsr_taps16, &lfsr16));
            }
        }
        if (fifo_width_bytes == 4) {
            for (size_t i = sub_idx / 4; i < (sub_idx + size_written) / 4;
                    i++) {
                write_data_32[i] = bswap_32(
                        lfsr_inc_32(lfsr_taps32, &lfsr32));
            }
        }
        current_byte += size_written;

        /* If random wait is enabled, roll for wait. */
        if (random_mode) {
            if ((random() % 100) < random_mode) {
                usleep(100 * 1000);
            }
        }
    }
    return 0;
}

void* read_from_target(void* ctx_void)
{
    struct glip_ctx *ctx = ctx_void;

    uint8_t *read_data = calloc(read_block_size, sizeof(uint8_t));
    assert(read_data);

    /* Used for read mode to generate pseudo random values locally. */
    uint16_t *rnd_word_16 = calloc(read_block_size, sizeof(uint8_t));
    assert(rnd_word_16);

    size_t size_read;

    int rv;
    while (1) {
        /* Read data from GLIP device. */
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

        /*
         * Write stress test:
         * Only messaged received in write mode are error messages or success
         * message at end of test.
         */
        if (read_mode == 0) {
            if (size_read > 0) {
                // stop progress bar update
                progress_bar_update = 0;
                rv = write_stress_test_read(read_data, size_read);
                if (rv == 0) {
                    return NULL;
                }
                exit_measurement(1);
            }
        }
        /*
         * Read stress test:
         * Compare received data to locally generated data.
         */
        else {
            if (size_read > 0) {
                rv = read_stress_test(ctx_void, read_data, rnd_word_16, size_read);
                if (rv > 0) {
                    return NULL;
                } else if (rv < 0) {
                    exit_measurement(1);
                }
            }
        }
    }

    return NULL;
}

int write_stress_test_read(uint8_t *read_data, size_t size_read)
{
    uint16_t *read_data_16 = (uint16_t*) read_data;
    uint32_t *read_data_32 = (uint32_t*) read_data;

    /* 16-bit interface */
    if (fifo_width_bytes == 2) {
        /* Check for success message. */
        if (size_read == 2) {
            if (bswap_16(read_data_16[0]) == 0xCAFE) {
                return 0;
            }
            fprintf(stderr,
                    "\nError reading from GLIP device during write test. "
                    "Received invalid 2-byte message: 0x%04x",
                    bswap_16(read_data_16[0]));
        /*
         * Error messages are 3 words long.
         * The first word always is 0xDEAD.
         * The second word is the word the device received.
         * The third word is the word the device expected.
         */
        } else if (size_read == 6) {
            if (bswap_16(read_data_16[0]) == 0xDEAD) {
                fprintf(stderr,
                        "\nError on GLIP device. Word received: 0x%04x, "
                        "word expected: 0x%04x.\n",
                        bswap_16(read_data_16[1]),
                        bswap_16(read_data_16[2]));
            } else {
                fprintf(stderr,
                        "\nError reading from GLIP device during write test. "
                        "Received invalid error message from device: 0x%04x\n",
                        read_data_16[0]);
            }
        /* All other messages are invalid. */
        } else {
            fprintf(stderr,
                    "\nError reading from GLIP device during write test. "
                    "Received invalid amount of bytes in status message.\n"
                    "%d bytes have been received. Using 16-bit interface:\n",
                    (int) size_read);
            for (size_t i = 0; i < size_read / 2; i++) {
                printf("0x%04x ", bswap_16(read_data_16[i]));
            }
            printf("\n");
        }
    }
    /* 32-bit interface */
    else {
        /* Check for success message. */
        if (size_read == 4) {
            if (bswap_32(read_data_32[0]) == 0x0000CAFE) {
                return 0;
            }
            fprintf(stderr,
                    "\nError reading from GLIP device during write test. "
                    "Received invalid 4-byte message: 0x%08x",
                    bswap_32(read_data_32[0]));
        /*
         * Error messages are 3 words long.
         * The first word always is 0xDEAD.
         * The second word is the word the device received.
         * The third word is the word the device expected.
         */
        } else if (size_read == 12) {
            if (bswap_32(read_data_32[0]) == 0x0000DEAD) {
                fprintf(stderr,
                        "\nError on GLIP device. Word received: 0x%08x, "
                        "word expected: 0x%08x.\n",
                        bswap_32(read_data_32[1]),
                        bswap_32(read_data_32[2]));
            } else {
                fprintf(stderr,
                        "\nError reading from GLIP device during write test. "
                        "Received invalid error message from device: 0x%08x\n",
                        read_data_32[0]);
            }
        /* All other messages are invalid. */
        } else {
            fprintf(stderr,
                    "\nError reading from GLIP device during write test. "
                    "Received invalid amount of bytes in status message.\n"
                    "%d bytes have been received. Using 32-bit interface.\n",
                    (int) size_read);
            for (size_t i = 0; i < size_read / 4; i++) {
                printf("0x%08x ", bswap_32(read_data_32[i]));
            }
            printf("\n");
        }
    }
    /* If we made it here we have an error. */
    return -1;
}

int read_stress_test(void* ctx_void, uint8_t *read_data, uint16_t *rnd_word_16, size_t size_read)
{
    struct glip_ctx *ctx = ctx_void;

    uint16_t *read_data_16 = (uint16_t*) read_data;
    uint32_t *read_data_32 = (uint32_t*) read_data;

    /* Compare read data. */
    int rv = read_stress_test_compare(read_data, rnd_word_16, size_read);

    /*
     * If an error occurred, send an error message to device to stop the
     * stress test.
     */
    if (rv != 0) {
        size_t size_written;
        if (use_blocking_functions) {
            rv = glip_write_b(ctx, 0, fifo_width_bytes, read_data,
                              &size_written, 0);
        } else {
            rv = glip_write(ctx, 0, fifo_width_bytes, read_data,
                            &size_written);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "\nError while writing to GLIP. rv = %d\n",
                    rv);
        }
        if (size_written < fifo_width_bytes) {
            fprintf(stderr,
                    "\nError while sending error message to GLIP device.\n");
        }

        /*
         * Sleep to make sure the message is sent out before the
         * applications quits.
         */
        usleep(10*1000);
        return -1;
    }

    current_byte += size_read;

    /* Check if all data has been received. */
    size_t transfer_size_bytes = transfer_size * 1024 * 1024;
    if (current_byte >= transfer_size_bytes) {
        /* Send success message to device. */
        if (fifo_width_bytes == 2) {
            read_data_16[0] = bswap_16(0xCAFE);
        } else {
            read_data_32[0] = bswap_32(0x0000CAFE);
        }
        size_t size_written;
        if (use_blocking_functions) {
            rv = glip_write_b(ctx, 0, fifo_width_bytes, read_data,
                              &size_written, 0);
        } else {
            rv = glip_write(ctx, 0, fifo_width_bytes, read_data,
                            &size_written);
        }
        if (rv != 0 && rv != -ETIMEDOUT) {
            fprintf(stderr, "\nError while writing to GLIP. rv = %d\n",
                    rv);
            return -1;
        }
        if (size_written < fifo_width_bytes) {
            fprintf(stderr,
                    "\nError while sending error message to GLIP device.\n");
            return -1;
        }

        /*
         * Sleep to make sure the message is sent out before the
         * applications quits.
         */
        usleep(10*1000);
        return 1;
    }

    /* If random wait is enabled, roll for wait. */
    if (random_mode) {
        if ((random() % 100) < random_mode) {
            usleep(100 * 1000);
        }
    }
    return 0;
}

int read_stress_test_compare(uint8_t *read_data, uint16_t *rnd_word_16, size_t size_read)
{
    uint16_t *read_data_16 = (uint16_t*) read_data;
    uint32_t *read_data_32 = (uint32_t*) read_data;
    uint32_t *rnd_word_32 = (uint32_t*) rnd_word_16;

    int rv = 0;

    /* 16-bit interface */
    if (fifo_width_bytes == 2) {
        /* Loop over received data and compare word by word. */
        for (size_t i = 0; i < size_read / 2; i++) {
            rnd_word_16[i] = lfsr_inc_16(lfsr_taps16, &lfsr16);
            /*
             * In case of an error print the corrupted word and the word
             * that was expected.
             * Furthermore, if possible, print the 10 previously and 10
             * subsequently received words as well as the expected
             * words.
             */
            if (bswap_16(read_data_16[i]) != rnd_word_16[i]) {
                // stop progress bar update
                progress_bar_update = 0;

                fprintf(stderr, "\nError comparing data word %zu. "
                        "Expected: 0x%04x, read: 0x%04x.\n",
                        ((current_byte / fifo_width_bytes) + i),
                        rnd_word_16[i], bswap_16(read_data_16[i]));

                size_t last_words = i >= 5 ? 5 : i;
                size_t next_words = ((size_read / 2) - i) >= 5 ?
                        5 : ((size_read / 2) - i);

                printf("With %zu prior and %zu subsequent words "
                       "expected:\t", last_words, next_words);
                for (size_t j = i - last_words; j <= i + next_words;
                        j++) {
                    if (j > i) {
                        rnd_word_16[j] = lfsr_inc_16(lfsr_taps16,
                                                     &lfsr16);
                    }
                    printf("0x%04x ", rnd_word_16[j]);
                }

                printf("\nWith %zu prior and %zu subsequent words "
                       "read:\t", last_words, next_words);
                for (size_t j = i - last_words; j <= i + next_words;
                        j++) {
                    printf("0x%04x ", bswap_16(read_data_16[j]));
                }

                /*
                 * Calculate the 'distance' of the received and the
                 * expected word. The lfsr implements a maximal length
                 * sequence, meaning that every possible value is
                 * reached at some point. The 'distance' is the number
                 * of shift operations that are necessary for the lfsr
                 * to reach the received value when starting with the
                 * expected value.
                 */
                size_t distance = 1;
                uint16_t expected_word = rnd_word_16[i];
                while (bswap_16(read_data_16[i])
                        != lfsr_inc_16(lfsr_taps16, &rnd_word_16[i]))
                    distance++;
                printf("\nDistance from 0x%04x (expected) to 0x%04x (read) is "
                        "%zu.\n",
                       expected_word, bswap_16(read_data_16[i]), distance);

                /* Set error word to be sent to the device. */
                read_data_16[0] = bswap_16(0xDEAD);
                rv = -1;
                break;
            }
        }
    /* 32-bit interface */
    } else {
        /* Loop over received data and compare word by word. */
        for (size_t i = 0; i < size_read / 4; i++) {
            rnd_word_32[i] = lfsr_inc_32(lfsr_taps32, &lfsr32);
            /*
             * In case of an error print the corrupted word and the word
             * that was expected.
             * Furthermore, if possible, print the 10 previously and 10
             * subsequently received words as well as the expected
             * words.
             */
            if (bswap_32(read_data_32[i]) != rnd_word_32[i]) {
                // stop progress bar update
                progress_bar_update = 0;

                fprintf(stderr, "\nError comparing data word %zu. "
                        "Expected: 0x%08x, read: 0x%08x.\n",
                        ((current_byte / fifo_width_bytes) + i),
                        rnd_word_32[i], bswap_32(read_data_32[i]));

                size_t last_words = i >= 5 ? 5 : i;
                size_t next_words = ((size_read / 4) - i) >= 5 ?
                        5 : ((size_read / 4) - i);

                printf("With %zu prior and %zu subsequent words "
                       "expected:\t", last_words, next_words);
                for (size_t j = i - last_words; j <= i + next_words;
                        j++) {
                    if (j > i) {
                        rnd_word_32[j] = lfsr_inc_32(lfsr_taps32,
                                                     &lfsr32);
                    }
                    printf("0x%08x ", rnd_word_32[j]);
                }

                printf("\nWith %zu prior and %zu subsequent words "
                       "read:\t", last_words, next_words);
                for (size_t j = i - last_words; j <= i + next_words;
                        j++) {
                    printf("0x%08x ", bswap_32(read_data_32[j]));
                }

                /*
                 * Calculate the 'distance' of the received and the
                 * expected word. The lfsr implements a maximal length
                 * sequence, meaning that every possible value is
                 * reached at some point. The 'distance' is the number
                 * of shift operations that are necessary for the lfsr
                 * to reach the received value when starting with the
                 * expected value.
                 */
                size_t distance = 1;
                uint32_t expected_word = rnd_word_32[i];
                while (bswap_32(read_data_32[i])
                        != lfsr_inc_32(lfsr_taps32, &rnd_word_32[i]))
                    distance++;
                printf("\nDistance from 0x%04x (expected) to 0x%04x (read) is "
                        "%zu.\n",
                       expected_word, bswap_32(read_data_32[i]), distance);

                /* Set error word to be sent to the device. */
                read_data_32[0] = bswap_32(0x0000DEAD);
                rv = -1;
                break;
            }
        }
    }
    return rv;
}

void* update_progressbar_thread(void* arg)
{
    while (progress_bar_update) {
        update_progressbar();
        usleep(200 * 1000); /* 200 ms update interval */
    }
    return NULL;
}

void update_progressbar()
{
    int w = 50; /* progress bar width */

    double ratio = current_byte / (double) (transfer_size * 1024 * 1024);
    int c = ratio * w;

    static int alive = 0;
    static char cursor = ' ';

    alive++;
    if(alive%5 == 0) {
        cursor = cursor == ' ' ? '_' : ' ';
    }

    printf("% 4.0lf %% [", ratio * 100);

    for (int x = 0; x < c; x++)
        printf("=");
    for (int x = c; x < w; x++)
        printf(" ");
    if (read_mode == 0)
        printf("] % 4.0lf KiB TX%c\r", current_byte / 1024.0, cursor);
    else
        printf("] % 4.0lf KiB RX%c\r", current_byte / 1024.0, cursor);
    fflush(stdout);
}

void exit_measurement(int exit_code)
{
    exit(exit_code);
}

/*
 * Implementation of a 16-bit wide lfsr with maximum length sequence.
 */
uint16_t lfsr_inc_16(const uint16_t *taps, uint16_t *lfsr)
{
    uint16_t tap = 0;
    int i = 1;

    while (taps[i])
        tap ^= !!(taps[i++] & *lfsr);
    *lfsr <<= 1;
    *lfsr |= tap;
    *lfsr &= taps[0];

    return *lfsr;
}

/*
 * Implementation of a 32-bit wide lfsr with maximum length sequence.
 */
uint32_t lfsr_inc_32(const uint32_t *taps, uint32_t *lfsr)
{
    uint32_t tap = 0;
    int i = 1;

    while (taps[i])
        tap ^= !!(taps[i++] & *lfsr);
    *lfsr <<= 1;
    *lfsr |= tap;
    *lfsr &= taps[0];

    return *lfsr;
}
