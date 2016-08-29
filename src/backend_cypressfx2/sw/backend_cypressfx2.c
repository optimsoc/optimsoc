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

#include "backend_cypressfx2.h"
#include "glip-protected.h"
#include "cbuf.h"
#include "util.h"

#include <assert.h>
#include <config.h>
#include <errno.h>
#include <libusb.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** A single entry in the list of well-known USB devices */
struct usb_dev_entry {
    /** vendor id (idVendor); may not be 0 */
    uint16_t vid;
    /** product id (idProduct) */
    uint16_t pid;
    /** manufacturer (resolved iManufacturer) */
    const char *manufacturer;
    /** product name (resolved iProduct) */
    const char *product;
};

/** A list of well-known supported devices with Cypress FX2 chip */
static const struct usb_dev_entry usb_devs[] = {
    /* ZTEX 1.15 and 2.13 boards */
    {
        .vid = 0x221a,
        .pid = 0x0100,
        .manufacturer = "TUM LIS",
        .product = "GLIP"
    },
    {} /* leave this as last element! */
};



/* USB device constants */
/** USB write endpoint */
const int USB_WR_EP = 0x02 | LIBUSB_ENDPOINT_OUT; /* EP2 OUT */
/** USB read endpoint */
const int USB_RD_EP = 0x6 | LIBUSB_ENDPOINT_IN; /* EP6 IN */

/**
 * USB read timeout [ms]
 *
 * This timeout should be rather small to achieve good performance even for
 * small reads.
 */
const int USB_RX_TIMEOUT_MS = 2; /* 2 ms */

/**
 * USB write timeout [ms]
 *
 * This value can be rather large without impacting performance.
 */
const int USB_TX_TIMEOUT_MS = 1000; /* 1 second */

/**
 * the timeout after which a USB read or write transfer is triggered [ms]
 */
const int USB_TRANSFER_RETRY_TIMEOUT_MS = 5;

/**
 * Packet/block size for USB bulk transfers [byte]
 *
 * This should be equal to wMaxPacketSize of the endpoint. Note that you *can*
 * send smaller packets (so called "short packets"), but don't achieve optimal
 * performance then obviously.
 *
 * @todo read this from the endpoint descriptor
 */
#define USB_TRANSFER_PACKET_SIZE_BYTES 512

/**
 * Maximum number of packets/blocks in one USB 2.0 transfer
 *
 * In order to achieve good performance, there should always be some packets/
 * blocks in flight. This setting controls how many blocks are sent to the
 * kernel in one transfer. Make sure to have this setting enough to always fill
 * the buffers in case of a continuous transmission.
 *
 * The value of 64 has been determined by tests to achieve good performance for
 * small and large blocking and non-blocking reads and writes.
 */
#define USB_MAX_PACKETS_PER_TRANSFER 64

/**
 * Read/write circular buffer size
 *
 * The read/write circular buffer is located between the glip_read()/
 * glip_write() calls and the USB transfers. It's used to achieve good
 * performance even for small read and write requests.
 *
 * The buffer is able to hold two times the maximum length of one transfer to
 * allow for continuous transmission (one part of the buffer is filled, while
 * the other one is transferred).
 */
#define USB_BUF_SIZE (USB_TRANSFER_PACKET_SIZE_BYTES * \
                      USB_MAX_PACKETS_PER_TRANSFER * 2)

/**
 * GLIP backend context for the Cypress FX2 backend
 */
struct glip_backend_ctx {
    /** libusb-1.0 context */
    struct libusb_context *usb_ctx;
    /** libusb-1.0 device handle */
    struct libusb_device_handle *usb_dev_handle;

    /** USB sending thread */
    pthread_t usb_write_thread;
    /** USB sending thread attributes */
    pthread_attr_t usb_write_thread_attr;

    /** USB receive thread */
    pthread_t usb_read_thread;
    /** USB receive thread attributes */
    pthread_attr_t usb_read_thread_attr;

    /** write circular buffer */
    struct cbuf *write_buf;
    /** semaphore notifying the write thread of a new full packet available to be transferred */
    sem_t write_notification_sem;

    /** read circular buffer */
    struct cbuf *read_buf;
    /** semaphore notifying the USB receiving thread to fetch new data */
    sem_t read_notification_sem;
};

/*
 * libusb < 1.0.9 does not have the function libusb_error_name(). Since this
 * function is non-critical, we can provide a fallback here and enable
 * compilation with older libusb versions.
 */
#ifndef LIBUSB_HAS_ERROR_STRING
#define libusb_error_name(x) "(unknown error, update to libusb > 1.0.8)"
#endif


/**
 * Initialize the backend (constructor)
 *
 * @param ctx  the library context
 * @return
 */
int gb_cypressfx2_new(struct glip_ctx *ctx)
{
    int rv;

    struct glip_backend_ctx *c = calloc(1, sizeof(struct glip_backend_ctx));

    /* initialize libusb */
    rv = libusb_init(&c->usb_ctx);
    if (rv < 0) {
        return rv;
    }

    /* setup vtable */
    ctx->backend_functions.open = gb_cypressfx2_open;
    ctx->backend_functions.close = gb_cypressfx2_close;
    ctx->backend_functions.logic_reset = gb_cypressfx2_logic_reset;
    ctx->backend_functions.read = gb_cypressfx2_read;
    ctx->backend_functions.read_b = gb_cypressfx2_read_b;
    ctx->backend_functions.write = gb_cypressfx2_write;
    ctx->backend_functions.write_b = gb_cypressfx2_write_b;
    ctx->backend_functions.get_fifo_width = gb_cypressfx2_get_fifo_width;
    ctx->backend_functions.get_channel_count = gb_cypressfx2_get_channel_count;

    ctx->backend_ctx = c;

    return 0;
}

/**
 * Open a target connection
 *
 * See the information about device auto-discovery at the start of this file.
 *
 * @return 0 if the connection was successfully established
 * @return -EINVAL if no usable USB device was found or the given options are
 *                 invalid.
 * @return any other value indicates an error
 *
 * @see glip_open()
 */
int gb_cypressfx2_open(struct glip_ctx *ctx, unsigned int num_channels)
{
    int rv;

    if (ctx->backend_ctx->usb_dev_handle != NULL) {
        err(ctx, "Already connected to USB device - disconnect first!\n");
        return -1;
    }

    bool manual_config = false;

    /* check manual device selection */
    uint8_t cfg_bus;
    uint8_t cfg_addr;
    bool cfg_bus_found = (glip_option_get_uint8(ctx, "usb_dev_bus",
                                                &cfg_bus) == 0);
    bool cfg_addr_found = (glip_option_get_uint8(ctx, "usb_dev_addr",
                                                 &cfg_addr) == 0);
    if ((cfg_bus_found && !cfg_addr_found) ||
        (!cfg_bus_found && cfg_addr_found)) {

        err(ctx, "You need to specify both options, usb_dev_bus and "
            "usb_dev_addr, or none.\n");
        return -EINVAL;
    }
    manual_config = cfg_bus_found;

    /* get VID/PID/product name/manufacturer from options (if given) */
    const struct usb_dev_entry *dev_searchlist = usb_devs;
    struct usb_dev_entry usb_devs_cfg[2] = {{}};
    if (!manual_config) {
        bool cfg_vid_found = (glip_option_get_uint16(ctx, "usb_vid",
                                                     &usb_devs_cfg[0].vid) == 0);
        bool cfg_pid_found = (glip_option_get_uint16(ctx, "usb_pid",
                                                     &usb_devs_cfg[0].pid) == 0);
        if ((cfg_vid_found && !cfg_pid_found) || (!cfg_vid_found && cfg_pid_found)) {
            err(ctx, "If using the usb_vid and usb_pid options, both "
                "need to be given.\n");
            return -EINVAL;
        }
        glip_option_get_char(ctx, "usb_manufacturer", &usb_devs_cfg[0].manufacturer);
        glip_option_get_char(ctx, "usb_product", &usb_devs_cfg[0].product);

        /* if available, prefer the configuration over the built-in search list */
        if (cfg_vid_found) {
            info(ctx, "Using custom filter to look for USB device, "
                 "idVendor = 0x%04x, idProduct = 0x%04x, iManufacturer = %s, "
                 "iProduct = %s\n",
                 usb_devs_cfg[0].vid, usb_devs_cfg[0].pid,
                 (!usb_devs_cfg[0].manufacturer ? "[unused]" : usb_devs_cfg[0].manufacturer),
                 (!usb_devs_cfg[0].product ? "[unused]" : usb_devs_cfg[0].product));
            dev_searchlist = usb_devs_cfg;
        }
    }

    /* determine which USB device to use */
    libusb_device **dev_list;

    ssize_t cnt = libusb_get_device_list(ctx->backend_ctx->usb_ctx, &dev_list);
    if (cnt < 0) {
        err(ctx, "Unable to retrieve USB device list.\n");
        return -1;
    }

    int found_cnt = 0;
    int found_dev_idx = -1;
    for (int dev_idx = 0; dev_idx < cnt; dev_idx++) {
        libusb_device *device = dev_list[dev_idx];

        /* check manual configuration */
        if (manual_config) {
            if (libusb_get_bus_number(device) == cfg_bus &&
                libusb_get_device_address(device) == cfg_addr) {

                found_cnt = 1;
                found_dev_idx = dev_idx;
                break;
            }

            continue; /* skip automatic discovery below */
        }

        /* auto-discovery based on filters */
        struct libusb_device_descriptor desc;
        rv = libusb_get_device_descriptor(device, &desc);
        if (rv < 0) {
            dbg(ctx, "Unable to get USB device descriptor.\n");
            continue;
        }

        /* search list of VID/PIDs */
        for (int dev_searchlist_idx = 0;
             dev_searchlist[dev_searchlist_idx].vid != 0; dev_searchlist_idx++) {

            if (dev_searchlist[dev_searchlist_idx].vid != desc.idVendor ||
                dev_searchlist[dev_searchlist_idx].pid != desc.idProduct) {
                /* VID or PID didn't match */
                continue;
            }

            libusb_device_handle *handle;
            rv = libusb_open(device, &handle);
            if (rv < 0) {
                info(ctx, "Unable to open USB device.\n");
                continue;
            }

            /* check manufacturer name */
            char str[256];
            if (dev_searchlist[dev_searchlist_idx].manufacturer != NULL) {
                rv = libusb_get_string_descriptor_ascii(handle,
                                                        desc.iManufacturer,
                                                        (unsigned char*)str,
                                                        sizeof(str));
                if (rv < 0) {
                    info(ctx, "Unable to read manufacturer name from device.\n");
                    continue;
                }
                if (strncmp(str, dev_searchlist[dev_searchlist_idx].manufacturer,
                            sizeof(str))) {
                    /* manufacturer name does not match */
                    libusb_close(handle);
                    continue;
                }
            }

            /* check product name */
            if (dev_searchlist[dev_searchlist_idx].product != NULL) {
                rv = libusb_get_string_descriptor_ascii(handle,
                                                        desc.iProduct,
                                                        (unsigned char*)str,
                                                        sizeof(str));
                if (rv < 0) {
                    info(ctx, "Unable to read product name from device.\n");
                    continue;
                }
                if (strncmp(str, dev_searchlist[dev_searchlist_idx].product, rv)) {
                    /* product name does not match */
                    libusb_close(handle);
                    continue;
                }
            }

            libusb_close(handle);

            /* if we're here we found a match! */
            found_cnt++;
            found_dev_idx = dev_idx;
            break;
        }
    }

    if (found_cnt == 0) {
        err(ctx, "No suitable USB device was found.\n");
        return -EINVAL;
    }

    if (found_cnt > 1) {
        info(ctx, "%d USB devices were found matching the given "
             "filters, using device on bus %u with address %d. Use the "
             "usb_dev_bus and usb_dev_addr options to override this!\n",
             found_cnt, libusb_get_bus_number(dev_list[found_dev_idx]),
             libusb_get_device_address(dev_list[found_dev_idx]));
    }

    rv = libusb_open(dev_list[found_dev_idx],
                     &ctx->backend_ctx->usb_dev_handle);
    if (rv < 0) {
        err(ctx, "Unable to open USB device: %s\n",
            libusb_error_name(rv));
        return -1;
    }

    libusb_free_device_list(dev_list, 1);

    rv = libusb_set_configuration(ctx->backend_ctx->usb_dev_handle, 1);
    if (rv < 0) {
        err(ctx, "Unable to set configuration on USB interface: %s\n",
            libusb_error_name(rv));
        return -1;
    }

    rv = libusb_claim_interface(ctx->backend_ctx->usb_dev_handle, 0);
    if (rv < 0) {
        err(ctx, "Unable to claim interface on USB device: %s\n",
            libusb_error_name(rv));
        return -1;
    }

    /* reset communication */
    rv = libusb_control_transfer(ctx->backend_ctx->usb_dev_handle, 0x40, 0x60,
                                 (1 << 0), 0, 0, 0, USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        err(ctx, "Unable to send USB control message to reset system "
            "(r=1). Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    rv = libusb_control_transfer(ctx->backend_ctx->usb_dev_handle, 0x40, 0x60,
                                 (0 << 0), 0, 0, 0, USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        err(ctx, "Unable to send USB control message to reset system (r=0). "
            "Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    /* initialize write circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->write_buf, USB_BUF_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup write buffer: %d\n", rv);
        return -1;
    }
    cbuf_set_hint_max_read_size(ctx->backend_ctx->write_buf,
                                USB_MAX_PACKETS_PER_TRANSFER *
                                USB_TRANSFER_PACKET_SIZE_BYTES);

    /* initialize read circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->read_buf, USB_BUF_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup read buffer: %d\n", rv);
        return -1;
    }

    /* initialize event notifications */
    sem_init(&ctx->backend_ctx->write_notification_sem, 0, 0);
    sem_init(&ctx->backend_ctx->read_notification_sem, 0, 0);

    /* start thread for sending data to the USB device */
    pthread_attr_init(&ctx->backend_ctx->usb_write_thread_attr);
    pthread_attr_setdetachstate(&ctx->backend_ctx->usb_write_thread_attr,
                                PTHREAD_CREATE_DETACHED);

    rv = pthread_create(&ctx->backend_ctx->usb_write_thread,
                        &ctx->backend_ctx->usb_write_thread_attr,
                        usb_write_thread, (void*)ctx);
    if (rv) {
        err(ctx, "Unable to create USB write thread: %d\n", rv);
        return -1;
    }

    /* start thread for receiving data from the USB device */
    pthread_attr_init(&ctx->backend_ctx->usb_read_thread_attr);
    pthread_attr_setdetachstate(&ctx->backend_ctx->usb_read_thread_attr,
                                PTHREAD_CREATE_DETACHED);

    rv = pthread_create(&ctx->backend_ctx->usb_read_thread,
                        &ctx->backend_ctx->usb_read_thread_attr,
                        usb_read_thread, (void*)ctx);
    if (rv) {
        err(ctx, "Unable to create USB read thread: %d\n", rv);
        return -1;
    }

    return 0;
}

/**
 * Close connection to the target
 *
 * @see glip_close()
 */
int gb_cypressfx2_close(struct glip_ctx *ctx)
{
    if (ctx->backend_ctx->usb_dev_handle == NULL) {
        err(ctx, "Not connected!\n");
        return -1;
    }

    /* tear down event notifications */
    sem_destroy(&ctx->backend_ctx->write_notification_sem);
    sem_destroy(&ctx->backend_ctx->read_notification_sem);

    /* clean-up USB write thread */
    void *status;
    pthread_cancel(ctx->backend_ctx->usb_write_thread);
    pthread_join(ctx->backend_ctx->usb_write_thread, &status);
    pthread_attr_destroy(&ctx->backend_ctx->usb_write_thread_attr);

    /* tear down USB communication */
    int rv = libusb_release_interface(ctx->backend_ctx->usb_dev_handle, 0);
    if (rv < 0) {
        err(ctx, "Unable to release claimed USB interface: %s\n",
            libusb_error_name(rv));
    }

    libusb_close(ctx->backend_ctx->usb_dev_handle);
    ctx->backend_ctx->usb_dev_handle = NULL;

    /* tear down read/write buffer */
    cbuf_free(ctx->backend_ctx->write_buf);
    cbuf_free(ctx->backend_ctx->read_buf);

    return 0;
}

/**
 * Reset the logic on the target
 *
 * @see glip_logic_reset()
 */
int gb_cypressfx2_logic_reset(struct glip_ctx *ctx)
{
    if (ctx->backend_ctx->usb_dev_handle == NULL) {
        err(ctx, "Not connected!\n");
        return -1;
    }

    int rv;

    /* set reset signal */
    rv = libusb_control_transfer(ctx->backend_ctx->usb_dev_handle, 0x40, 0x60,
                                 (1 << 1), 0, 0, 0, USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        err(ctx, "Unable to send USB control message to reset system "
                 "(r=1). Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    /* unset reset signal */
    rv = libusb_control_transfer(ctx->backend_ctx->usb_dev_handle, 0x40, 0x60,
                                 (0 << 1), 0, 0, 0, USB_TX_TIMEOUT_MS);

    if (rv < 0) {
        err(ctx, "Unable to send USB control message to reset system (r=0). "
            "Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    return 0;
}

/**
 * Read from the target device
 *
 * @see glip_read()
 */
int gb_cypressfx2_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                       uint8_t *data, size_t *size_read)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the cypressfx2 backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    size_t fill_level = cbuf_fill_level(bctx->read_buf);
    size_t size_read_req = (size > fill_level ? fill_level : size);

    int rv = cbuf_read(bctx->read_buf, data, size_read_req);
    if (rv < 0) {
        err(ctx, "Unable to get data from read buffer, rv = %d\n", rv);
        return -1;
    }

    /**
     * We request a new read from the USB device if at least one packet fits
     * into the read buffer. The read itself is done by the usb_read_thread.
     */
    if (cbuf_free_level(bctx->read_buf) >= USB_TRANSFER_PACKET_SIZE_BYTES) {
        int sval;
        sem_getvalue(&bctx->read_notification_sem, &sval);
        if (sval == 0) {
            sem_post(&bctx->read_notification_sem);
        }
    }

    *size_read = size_read_req;
    return 0;
}

/**
 * Blocking read from the device
 *
 * @see glip_read_b()
 */
int gb_cypressfx2_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                         uint8_t *data, size_t *size_read,
                         unsigned int timeout)
{
    int rv;
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    if (size > USB_BUF_SIZE) {
        /*
         * This is not a problem for non-blocking reads, but blocking reads will
         * block forever in this case as the maximum amount of data ever
         * available is limited by the buffer size.
         */
        err(ctx, "The read size cannot be larger than %u bytes.", USB_BUF_SIZE);
        return -1;
    }

    /*
     * Wait until sufficient data is available to be read.
     */
    struct timespec ts;
    if (timeout != 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, timeout * 1000 * 1000);
    }

    size_t level = cbuf_fill_level(bctx->read_buf);

    while (level < size) {
        if (timeout == 0) {
            rv = cbuf_wait_for_level_change(bctx->read_buf, level);
        } else {
            rv = cbuf_timedwait_for_level_change(bctx->read_buf, level, &ts);
        }

        if (rv != 0) {
            break;
        }

        level = cbuf_fill_level(bctx->read_buf);
    }

    /*
     * We read whatever data is available, and assume a timeout if the available
     * amount of data does not match the requested amount.
     */
    *size_read = 0;
    rv = gb_cypressfx2_read(ctx, channel, size, data, size_read);
    if (rv == 0 && size != *size_read) {
        return -ETIMEDOUT;
    }
    return rv;
}

/**
 * Write to the target device
 *
 * All data is written through USB 2.0 bulk transfers to the target. Since
 * those transfers are most efficient when sending large amounts of data, we
 * do a client-side buffering of data. The buffer is flushed if one of the
 * two conditions hold:
 *
 * a) we have reached the size of one transfer
 * b) we hit a timeout
 *
 * This strategy ensures that we get high throughput, while at the same time
 * allowing for many small glip_write() calls to be made and combined into one
 * USB transfer.
 *
 * @todo Add a "flush buffer" API call to send off immediately a transfer even
 *       through none of the conditions a) or b) hold.
 *
 * @see glip_write()
 */
int gb_cypressfx2_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                        uint8_t *data, size_t *size_written)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the cypressfx2 backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    unsigned int buf_size_free = cbuf_free_level(bctx->write_buf);
    *size_written = (size > buf_size_free ? buf_size_free : size);

    cbuf_write(bctx->write_buf, data, *size_written);

    /*
     * If half of the write buffer is filled we trigger the USB sending thread
     * to transfer the data to the USB device. Apart from that, the write thread
     * is triggered every USB_TRANSFER_RETRY_TIMEOUT_MS milliseconds, even if
     * less data is available.
     *
     * The threshold "1/2 write buffer size" is a suitable value for
     * high-bandwidth communication; if there is not much data to transfer, the
     * communication latency will be increased to at least
     * USB_TRANSFER_RETRY_TIMEOUT_MS per transfer.
     */
    if (cbuf_fill_level(bctx->write_buf) >= USB_BUF_SIZE / 2) {
        int sval;
        sem_getvalue(&bctx->write_notification_sem, &sval);
        if (sval == 0) {
            sem_post(&bctx->write_notification_sem);
        }
    }

    return 0;
}

/**
 * Blocking write to the target device
 *
 * @see glip_write_b()
 */
int gb_cypressfx2_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                          uint8_t *data, size_t *size_written,
                          unsigned int timeout)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    struct timespec ts;

    if (timeout != 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, timeout * 1000 * 1000);
    }

    size_t size_done = 0; /* number of bytes already written */

    while (1) {
        size_t size_done_tmp = 0;
        gb_cypressfx2_write(ctx, channel, size - size_done, &data[size_done],
                            &size_done_tmp);
        size_done += size_done_tmp;

        if (size_done == size) {
            break;
        }

        if (cbuf_free_level(bctx->write_buf) == 0) {
            if (timeout == 0) {
                cbuf_wait_for_level_change(bctx->write_buf, 0);
            } else {
                cbuf_timedwait_for_level_change(bctx->write_buf, 0, &ts);
            }
        }
    }

    *size_written = size_done;

    if (size != *size_written) {
        return -ETIMEDOUT;
    }
    return 0;
}

/*
 * A general note on the pthread_setcancelstate() calls in the two threads:
 * We cannot cancel the thread (as it is happening when calling
 * gb_cypressfx2_close() ) while libusb is waiting for a transfer. This leaves
 * locks inside libusb in an undefined state and may cause hangs when
 * calling libusb_close(). We thus mark this thread uncancellable for the
 * duration of the bulk transfer call, and set an explicit cancellation
 * point afterwards using pthread_testcancel().
 * Note that this blocks the shutdown of the thread by the bulk read/write
 * timeout!
 */

/**
 * Thread: write data to the USB device
 */
void* usb_write_thread(void* ctx_void)
{
    struct glip_ctx *ctx = ctx_void;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    int rv;

    struct timespec ts;

    while (1) {
        /*
         * Wait for USB_TRANSFER_RETRY_TIMEOUT_MS milliseconds or until the
         * semaphore is posted by the gb_cypressfx2_write() function. See this
         * function for more details on the implications.
         */
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, USB_TRANSFER_RETRY_TIMEOUT_MS * 1000 * 1000);
        sem_timedwait(&ctx->backend_ctx->write_notification_sem, &ts);


        unsigned int write_buf_fill_level = cbuf_fill_level(bctx->write_buf);

        /* no data to transfer */
        if (write_buf_fill_level == 0) {
            continue;
        }

        unsigned int transfer_len = 0;

        if (write_buf_fill_level >= USB_TRANSFER_PACKET_SIZE_BYTES) {
            /*
             * At least one full packet is available. Send out as many
             * full packets as possible, leaving the remainder for the next
             * transfer.
             */
            int num_packets = write_buf_fill_level / USB_TRANSFER_PACKET_SIZE_BYTES;
            if (num_packets > USB_MAX_PACKETS_PER_TRANSFER) {
                num_packets = USB_MAX_PACKETS_PER_TRANSFER;
            }
            transfer_len = num_packets * USB_TRANSFER_PACKET_SIZE_BYTES;

            dbg(ctx, "Transferring %d full packet(s) of %d bytes each.\n",
                num_packets, USB_TRANSFER_PACKET_SIZE_BYTES);
        } else {
            /*
             * We don't have a full packet available. We thus transfer a
             * short packet with whatever data we have available, but always
             * a full word.
             */
            transfer_len = write_buf_fill_level -
                           (write_buf_fill_level % gb_cypressfx2_get_fifo_width(ctx));

            dbg(ctx, "Transferring short packet with %d bytes.\n",
                transfer_len);
        }

        /* we only transfer full words: make sure nothing went wrong before */
        assert((transfer_len % gb_cypressfx2_get_fifo_width(ctx)) == 0);

        /* do the actual transfer */
        unsigned int transfer_len_sent = 0;
        uint8_t* transfer_data;

        rv = cbuf_peek(bctx->write_buf, &transfer_data, transfer_len);
        assert(rv == 0);

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        rv = libusb_bulk_transfer(bctx->usb_dev_handle, USB_WR_EP,
                                  transfer_data, transfer_len,
                                  (int*)&transfer_len_sent, USB_TX_TIMEOUT_MS);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();

        if (rv != 0 && rv != LIBUSB_ERROR_TIMEOUT) {
            /* An error has occurred and no data has been transferred */
            assert(transfer_len_sent == 0);
            err(ctx, "Unable to transfer data to USB device. Error code: %d",
                rv);
            continue;
        }

#ifdef DEBUG
        if (rv == LIBUSB_ERROR_TIMEOUT) {
            dbg(ctx, "Timeout when sending data to USB device.");
        }

        if (transfer_len_sent != transfer_len) {
            dbg(ctx, "Sent only %d of %d bytes of data.",
                transfer_len_sent, transfer_len);
        }
#endif

        cbuf_discard(bctx->write_buf, transfer_len_sent);
    }

    return NULL;
}

/**
 * Thread: read data from the USB device
 */
void* usb_read_thread(void* ctx_void)
{
    struct glip_ctx *ctx = ctx_void;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    int rv;
    int received;
    uint8_t* buf;

    struct timespec ts;

    while (1) {
        /*
         * Try to read a packet if either we have been notified by the
         * gb_cypressfx2_read() that at least one packet fits into the buffer,
         * or if we hit a timeout.
         * The timeout is necessary to initially fill the buffer before the
         * first read occurs.
         *
         * XXX: Replace sem_timedwait() by sem_wait() after the first read()
         * has been done to avoid unnecessary CPU cycles.
         */
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, USB_TRANSFER_RETRY_TIMEOUT_MS * 1000 * 1000);
        sem_timedwait(&ctx->backend_ctx->read_notification_sem, &ts);

        /*
         * Read as many complete packets as the buffer can hold
         */
        unsigned int free_level = cbuf_free_level(bctx->read_buf);
        unsigned int num_packets = free_level / USB_TRANSFER_PACKET_SIZE_BYTES;
        if (num_packets > USB_MAX_PACKETS_PER_TRANSFER) {
            num_packets = USB_MAX_PACKETS_PER_TRANSFER;
        }
        unsigned int buf_size = num_packets * USB_TRANSFER_PACKET_SIZE_BYTES;

        if (buf_size == 0) {
            /*
             * No read buffer space available; try again next time.
             */
            continue;
        }

        /*
         * Reserve this amount of space in the read buffer
         */
        cbuf_reserve(bctx->read_buf, &buf, buf_size);

        /*
         * Read up to buf_size bytes from the USB device. If that much data
         * is not available until the timeout expires, whatever amount of data
         * is available is returned.
         */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        rv = libusb_bulk_transfer(bctx->usb_dev_handle, USB_RD_EP,
                                  (unsigned char*)buf,
                                  buf_size,
                                  &received,
                                  USB_RX_TIMEOUT_MS);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();

        if (rv != 0 && rv != LIBUSB_ERROR_TIMEOUT) {
            /*
             * An error has occurred; usually this means that no data has been
             * received, but this can also happen during thread cancellation.
             */
            err(ctx, "Unable to receive data from USB device. Error code: %d",
                rv);
            continue;
        }

        cbuf_commit(bctx->read_buf, buf, received);
    }

    return NULL;
}

/**
 * Get the number of supported channels by this backend
 *
 * @param  ctx the library context
 * @return always 1
 *
 * @see glip_get_channel_count()
 */
unsigned int gb_cypressfx2_get_channel_count(struct glip_ctx *ctx)
{
    return 1;
}

/**
 * Get the width of the FIFO
 *
 * @param  ctx the library context
 * @return always 2 bytes, i.e. 16 bit
 *
 * @see glip_get_fifo_width()
 */
unsigned int gb_cypressfx2_get_fifo_width(struct glip_ctx *ctx)
{
    return 2;
}
