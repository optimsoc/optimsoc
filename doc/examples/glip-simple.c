#include <libglip.h>

int rv;
struct glip_ctx *gctx;

/* options passed to the selected backend */
struct glip_option backend_options[] = {
    { .name = "hostname", .value = "localhost" },
    { .name = "port", .value = "12345" },
};

/*
 * Initialize the GLIP context struct |ctx|. Use the backend "tcp" and pass
 * the 2 options inside backend_options to it.
 */
rv = glip_new(&gctx, "tcp", backend_options, 2);

/*
 * (Almost) all GLIP functions return 0 on success and a negative value if an
 * error happened.
 * We don't show the error handling for all functions below for brevity reasons,
 * but that does not exempt you from doing it! :-)
 */
if (rv != 0) {
    printf("An error happened when creating the GLIP context.\n");
    exit(1);
}

/*
 * open a connection to the target (in this case a TCP connection to
 * localhost:12345 with 1 channel.
 */
glip_open(gctx, 1);

/*
 * Send a reset signal to the attached logic. This does *not* reset the
 * communication interface, the signal is only passed through to the user
 * logic.
 */
glip_logic_reset(gctx);

/*
 * Write 4 bytes of data to channel 0 and wait for it up to 1 second
 * (1000 ms).
 */
uint8_t some_data[] = { 0x01, 0x02, 0x03, 0x04 };
size_t size_written;
glip_write_b(gctx, 0, sizeof(some_data), some_data, &size_written, 1*1000);
printf("%zu bytes written to target.\n", size_written);

/*
 * Read up to 4 bytes from the target on channel 0 and wait up to 1 second
 * for it.
 */
uint8_t read_buf[4];
size_t size_read;
glip_read_b(gctx, 0, sizeof(read_buf), read_buf, &size_read, 1*1000);
printf("%zu read from target.\n", size_read);

/* close the connection to the target */
glip_close(gctx);

/* free all resources */
glip_free(gctx);
