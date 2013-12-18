/* Copyright (c) 2012-2013 by the author(s)
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
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <libusb.h>
#include <string.h>
#include <unistd.h>

#include "backend_dbgnoc_usb.h"
#include "backend_dbgnoc.h"

/**
 * Opaque object anonymous to liboptimsochost and defined here.
 * Stores the backend context.
 */
struct ob_dbgnoc_connection_ctx {
    /** log context */
    struct optimsoc_log_ctx *log_ctx;
    /** libusb-1.0 context */
    struct libusb_context *usb_ctx;
    /** libusb-1.0 device handle */
    struct libusb_device_handle *usb_dev_handle;
};

/* USB device constants */
/** USB vendor ID  (from ZTEX) */
const int OPTIMSOC_USB_DEV_VID = 0x221a;
/** USB product ID (from ZTEX) */
const int OPTIMSOC_USB_DEV_PID = 0x100;
/** USB device product name (used to differentiate different ZTEX boards) */
const char* OPTIMSOC_USB_DEV_PRODUCTNAME = "OpTiMSoC - ZTEX USB 1.15";
/** USB write endpoint */
const int OPTIMSOC_USB_WR_EP = 0x02 | LIBUSB_ENDPOINT_OUT; // EP2 OUT
/** USB read endpoint */
const int OPTIMSOC_USB_RD_EP = 0x6 | LIBUSB_ENDPOINT_IN; // EP6 IN
/** USB read timeout [ms] */
const int OPTIMSOC_USB_RX_TIMEOUT_MS = 1000; /* 1 second */
/** USB write timeout [ms] */
const int OPTIMSOC_USB_TX_TIMEOUT_MS = 1000; /* 1 second */
/** Block size for USB bulk transfers (in 16 bit samples) */
const int OPTIMSOC_USB_NUM_SAMPLES_PER_TRANSFER = 256;
/** Maximum number of 16 bit samples in one USB 2.0 transfer */
const int OPTIMSOC_USB_MAX_NUM_SAMPLES_PER_TRANSFER = 8192;


/*
 * libusb < 1.0.9 does not have the function libusb_error_name(). Since this
 * function is non-critical, we can provide a fallback here and enable
 * compilation with older libusb versions.
 */
#ifndef LIBUSB_HAS_ERROR_STRING
#define libusb_error_name(x) "(unknown error, update to libusb > 1.0.8)"
#endif

int ob_dbgnoc_usb_new(struct ob_dbgnoc_connection_ctx **ctx,
                      struct ob_dbgnoc_connection_interface *calls,
                      struct optimsoc_log_ctx *log_ctx,
                      int num_options,
                      struct optimsoc_backend_option options[])
{
    int rv;
    struct ob_dbgnoc_connection_ctx *c;

    c = calloc(1, sizeof(struct ob_dbgnoc_connection_ctx));
    if (!c) {
        return -ENOMEM;
    }

    rv = libusb_init(&c->usb_ctx);
    if (rv < 0) {
        return rv;
    }

    c->log_ctx = log_ctx;

    calls->read_fn = &ob_dbgnoc_usb_read;
    calls->write_fn = &ob_dbgnoc_usb_write;

    calls->free = &ob_dbgnoc_usb_free;
    calls->connect = &ob_dbgnoc_usb_connect;
    calls->disconnect = &ob_dbgnoc_usb_disconnect;
    calls->connected = &ob_dbgnoc_usb_connected;
    calls->reset = &ob_dbgnoc_usb_reset;

    *ctx = c;

    return 0;
}

int ob_dbgnoc_usb_free(struct ob_dbgnoc_connection_ctx *ctx)
{
    libusb_exit(ctx->usb_ctx);

    free(ctx);
    ctx = 0;
    return 0;
}

int ob_dbgnoc_usb_connect(struct ob_dbgnoc_connection_ctx *ctx)
{
    int rv;

    if (ctx->usb_dev_handle != NULL) {
        err(ctx->log_ctx, "Already connected to USB device - disconnect first!\n");
        return -1;
    }

    /* connect to USB device */
    libusb_device **list;
    libusb_device_handle *handle;
    int device_found = 0;

    ssize_t i = 0;
    ssize_t cnt = libusb_get_device_list(ctx->usb_ctx, &list);
    if (cnt < 0) {
        err(ctx->log_ctx, "Unable to retrieve USB device list.\n");
        return -1;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        struct libusb_device_descriptor desc;
        rv = libusb_get_device_descriptor(device, &desc);
        if (rv < 0) {
            dbg(ctx->log_ctx, "Unable to get USB device descriptor.\n");
            continue;
        }
        if (desc.idProduct == OPTIMSOC_USB_DEV_PID &&
            desc.idVendor == OPTIMSOC_USB_DEV_VID) {
            /*
             * possible device found, we need to open it to read the
             * product string
             */
            rv = libusb_open(device, &handle);
            if (rv < 0) {
                info(ctx->log_ctx, "Unable to open USB device.\n");
                continue;
            }

            char product_name[256];
            rv = libusb_get_string_descriptor_ascii(handle, desc.iProduct,
                                                    (unsigned char*)product_name,
                                                    sizeof(product_name));
            if (rv < 0) {
                info(ctx->log_ctx, "Unable to read product name from device.\n");
                continue;
            }
            if (!strcmp(product_name, OPTIMSOC_USB_DEV_PRODUCTNAME)) {
                device_found = 1;
                ctx->usb_dev_handle = handle;
                break;
            }

            libusb_close(handle);
        }
    }

    libusb_free_device_list(list, 1);

    if (!device_found) {
        err(ctx->log_ctx, "No OpTiMSoC USB device found.\n");
        return -1;
    }

    rv = libusb_set_configuration(ctx->usb_dev_handle, 1);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to set configuration on USB interface: %s\n",
            libusb_error_name(rv));
        return -1;
    }

    rv = libusb_claim_interface(ctx->usb_dev_handle, 0);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to claim interface on USB device: %s\n",
            libusb_error_name(rv));
        return -1;
    }

    /* reset system to get a defined starting point */
    rv = ob_dbgnoc_usb_reset(ctx);
    if (rv < 0) {
        return rv;
    }

    return 0;
}

int ob_dbgnoc_usb_connected(struct ob_dbgnoc_connection_ctx *ctx)
{
    return (ctx->usb_dev_handle != NULL);
}

int ob_dbgnoc_usb_disconnect(struct ob_dbgnoc_connection_ctx *ctx)
{
    if (ctx->usb_dev_handle == NULL) {
        err(ctx->log_ctx, "Not connected!\n");
        return -1;
    }

    int rv = libusb_release_interface(ctx->usb_dev_handle, 0);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to release claimed USB interface: %s\n",
            libusb_error_name(rv));
    }

    libusb_close(ctx->usb_dev_handle);
    ctx->usb_dev_handle = NULL;

    return 0;
}

/**
 * Reset the whole system
 *
 * This is done using a special control message, that triggers custom code
 * in the ztex firmware, that in turn triggers the a reset pin of the FPGA.
 * All send/receive FIFOs of the FX2 chip are cleared as well.
 *
 * \param ctx backend context
 */
int ob_dbgnoc_usb_reset(struct ob_dbgnoc_connection_ctx *ctx)
{
    assert(ctx->usb_dev_handle);

    int rv;

    /* set and hold reset signal */
    rv = libusb_control_transfer(ctx->usb_dev_handle, 0x40, 0x60, 1, 0, 0, 0,
                                 OPTIMSOC_USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to send USB control message to reset system "
                          "(r=1). Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    usleep(1*1000); /* 1 ms, wait for FX2 FIFOs to be cleared */

    /* clear all FIFO caches */
    uint16_t dummy_rx[256];
    while (ob_dbgnoc_usb_read(ctx, dummy_rx, sizeof(dummy_rx)) >= 0);

    /* unset reset signal */
    rv = libusb_control_transfer(ctx->usb_dev_handle, 0x40, 0x60, 0, 0, 0, 0,
                                 OPTIMSOC_USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        err(ctx->log_ctx, "Unable to send USB control message to reset system "
                          "(r=0). Error %d: %s\n", rv, libusb_error_name(rv));
        return -1;
    }

    return 0;
}

int ob_dbgnoc_usb_write(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                        int len)
{
    assert(ctx->usb_dev_handle);

    int rv;

#ifdef DEBUG
    rv = bulk_transfer_size_valid(ctx, len);
    if (rv < 0) {
        return rv;
    }
#endif

    int bytes_transferred;
    rv = libusb_bulk_transfer(ctx->usb_dev_handle, OPTIMSOC_USB_WR_EP,
                              (unsigned char*)buffer,
                              sizeof(uint16_t) * len,
                              &bytes_transferred,
                              OPTIMSOC_USB_TX_TIMEOUT_MS);
    if (rv < 0) {
        info(ctx->log_ctx, "Error writing to USB: %s\n", libusb_error_name(rv));
        return -1;
    }

    return bytes_transferred / sizeof(uint16_t);
}

/**
 * Read data from the USB interface
 *
 * \param ctx    backend context
 * \param buffer buffer to store the read data
 * \param len    number of 16 bit samples requested to read
 * \return       number of 16 bit samples read (the actual length of the data).
 *               Negative values indicate an error (or no data is available).
 */
int ob_dbgnoc_usb_read(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                       int len)
{
    assert(ctx->usb_dev_handle);

    int rv;

#ifdef DEBUG
    rv = bulk_transfer_size_valid(ctx, len);
    if (rv < 0) {
        return rv;
    }
#endif

    int bytes_transferred;

    /*
     * We cannot cancel the thread while libusb is waiting for a transfer, as
     * it will leave locks inside libusb in an undefined state and may cause
     * hangs when calling libusb_close().
     */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    rv = libusb_bulk_transfer(ctx->usb_dev_handle, OPTIMSOC_USB_RD_EP,
                              (unsigned char*)buffer,
                              sizeof(uint16_t) * len,
                              &bytes_transferred,
                              OPTIMSOC_USB_RX_TIMEOUT_MS);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel(); // explicit cancellation point

    if (rv < 0) {
        return -1;
    }

    dbg(ctx->log_ctx, "bytes_transferred: %d\n", bytes_transferred);
    return bytes_transferred / sizeof(uint16_t);
}

int bulk_transfer_size_valid(struct ob_dbgnoc_connection_ctx *ctx, int num_samples)
{
    if (num_samples == 0 || num_samples > OPTIMSOC_USB_MAX_NUM_SAMPLES_PER_TRANSFER) {
        err(ctx->log_ctx, "Frame size %d is illegal, framesize is limited to 0 "
            "< size <= %d.\n", num_samples, OPTIMSOC_USB_MAX_NUM_SAMPLES_PER_TRANSFER);
        return -1;
    }
    if (num_samples % OPTIMSOC_USB_NUM_SAMPLES_PER_TRANSFER != 0) {
        err(ctx->log_ctx, "Frame size %d is not an integer multiply of %d.\n",
            num_samples, OPTIMSOC_USB_NUM_SAMPLES_PER_TRANSFER);
        return -1;
    }
    return 0;
}
