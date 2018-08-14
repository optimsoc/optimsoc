/* Copyright 2017-2018 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <osd/hostmod.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>
#include <osd/module.h>

#include "osd-private.h"
#include "worker.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

/**
 * Host module context
 */
struct osd_hostmod_ctx {
    /** Is the library connected to a device? */
    bool is_connected;

    /** Logging context */
    struct osd_log_ctx *log_ctx;

    /** Address assigned to this module in the debug interconnect */
    uint16_t diaddr;

    /** I/O worker */
    struct worker_ctx *ioworker_ctx;
};

/**
 * I/O thread user context
 */
struct iothread_usr_ctx {
    /** Communication socket with the host controller */
    zsock_t *hostctrl_socket;

    /** ZeroMQ address/URL of the host controller */
    char *host_controller_address;

    /** Event packet handler function */
    osd_hostmod_event_handler_fn event_handler;

    /** Argument passed to event_handler */
    void *event_handler_arg;

    /** Event re-assembly buffer (used to recombine split transactions) */
    zlist_t *event_reassembly_buf;
};

/**
 * Handle an EVENT packet received from the host controller
 *
 * Possible actions include forwarding the packet to the main thread,
 * storing it to be combined with other packets, or forwarded to the
 * registered event handler callback.
 *
 * @param usrctx the user context in the I/O thread
 * @param pkg the packet to be handled, ownership is passed to this function
 * @return a packet to be sent to the main thread (can be NULL)
 */
static struct osd_packet* iothread_handle_in_eventpkg(struct iothread_usr_ctx *usrctx,
                                                      struct osd_packet *pkg)
{
    int rv;
    osd_result osd_rv;

    assert(usrctx);
    assert(pkg);

    // record non-last EVENT packet in reassembly buffer and be done
    if (osd_packet_get_type_sub(pkg) == EV_CONT) {
        rv = zlist_append(usrctx->event_reassembly_buf, pkg);
        assert(rv == 0);

        return NULL;
    }

    struct osd_packet *fwd_pkg = NULL;

    if (osd_packet_get_type_sub(pkg) != EV_LAST) {
        // simply forward packet as-is
        fwd_pkg = pkg;
    } else {
        // reassemble one packet out of the multiple EVENT packets in the
        // reassembly buffer
        struct osd_packet *pkg_inbuf;
        pkg_inbuf = zlist_first(usrctx->event_reassembly_buf);
        while (pkg_inbuf) {

            if (osd_packet_get_src(pkg_inbuf) != osd_packet_get_src(pkg)) {
                // packet belongs to a different transmission, skip
                pkg_inbuf = zlist_next(usrctx->event_reassembly_buf);
                continue;
            }

            if (!fwd_pkg) {
                // first packet
                fwd_pkg = pkg_inbuf;
                osd_rv = osd_packet_set_type_sub(fwd_pkg, EV_LAST);
                assert(OSD_SUCCEEDED(osd_rv));
            } else {
                // subsequent packet: only take payload
                osd_rv = osd_packet_combine(&fwd_pkg, pkg_inbuf);
                assert(OSD_SUCCEEDED(osd_rv));

                osd_packet_free(&pkg_inbuf);
            }

            zlist_remove(usrctx->event_reassembly_buf, pkg_inbuf);
            pkg_inbuf = zlist_next(usrctx->event_reassembly_buf);
        }

        // append/forward the current packet as well
        if (!fwd_pkg) {
            fwd_pkg = pkg;
            // should not be necessary as this is the only packet in the
            // transmission
            osd_rv = osd_packet_set_type_sub(fwd_pkg, EV_LAST);
            assert(OSD_SUCCEEDED(osd_rv));
        } else {
            osd_rv = osd_packet_combine(&fwd_pkg, pkg);
            assert(OSD_SUCCEEDED(osd_rv));
            osd_packet_free(&pkg);
        }
    }


    if (usrctx->event_handler) {
        // Forward EVENT packets to handler function.
        // Ownership of |pkg| is transferred to the event handler.
        osd_rv = usrctx->event_handler(usrctx->event_handler_arg, fwd_pkg);
        if (OSD_FAILED(osd_rv)) {
            // ignore (error in user logic, packet is possibly dropped)
        }
        return NULL;
    }

    return fwd_pkg;
}

/**
 * Process an incoming data message from the host controller
 *
 * @return a message to be sent to the main thread (can be NULL)
 */
static zmsg_t* iothread_handle_in_data_msg(struct iothread_usr_ctx *usrctx,
                                           zmsg_t *msg)
{
    int rv;
    osd_result osd_rv;

    assert(usrctx);
    assert(msg);

    zmsg_first(msg);
    zframe_t *data_frame = zmsg_next(msg);
    assert(data_frame);

    struct osd_packet *pkg;
    osd_rv = osd_packet_new_from_zframe(&pkg, data_frame);
    assert(OSD_SUCCEEDED(osd_rv));

    if (osd_packet_get_type(pkg) == OSD_PACKET_TYPE_EVENT) {
        zmsg_destroy(&msg);

        struct osd_packet *fwd_pkg = iothread_handle_in_eventpkg(usrctx, pkg);
        if (fwd_pkg) {
            // Create new message to forward packet to main thread
            zmsg_t *fwd_msg = zmsg_new();
            rv = zmsg_addstr(fwd_msg, "D");
            assert(rv == 0);
            rv = zmsg_addmem(fwd_msg, fwd_pkg->data_raw,
                             osd_packet_sizeof(fwd_pkg));
            assert(rv == 0);

            osd_packet_free(&fwd_pkg);
            return fwd_msg;
        }
        return NULL;
    }

    osd_packet_free(&pkg);

    // Forward all other data messages to the main thread
    return msg;
}

/**
 * Process incoming messages from the host controller
 *
 * @return 0 if the message was processed, -1 if @p loop should be terminated
 */
static int iothread_rcv_from_hostctrl(zloop_t *loop, zsock_t *reader,
                                      void *thread_ctx_void)
{
    struct worker_thread_ctx *thread_ctx =
        (struct worker_thread_ctx *)thread_ctx_void;
    assert(thread_ctx);

    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);

    int rv;

    zmsg_t *msg = zmsg_recv(reader);
    if (!msg) {
        return -1;  // process was interrupted, terminate zloop
    }

    zframe_t *type_frame = zmsg_first(msg);
    assert(type_frame);
    if (zframe_streq(type_frame, "D")) {
        zmsg_t *out_msg = iothread_handle_in_data_msg(usrctx, msg);

        // possibly send a message to the main thread
        if (out_msg) {
            rv = zmsg_send(&out_msg, thread_ctx->inproc_socket);
            assert(rv == 0);
        }

    } else if (zframe_streq(type_frame, "M")) {
        assert(0 && "TODO: Handle incoming management messages.");

    } else {
        assert(0 && "Message of unknown type received.");
    }

    return 0;
}

/**
 * Obtain a DI address for this host debug module from the host controller
 */
static osd_result iothread_obtain_diaddr(struct worker_thread_ctx *thread_ctx,
                                         uint16_t *di_addr)
{
    int rv;

    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);
    zsock_t *sock = usrctx->hostctrl_socket;

    // request
    zmsg_t *msg_req = zmsg_new();
    assert(msg_req);

    rv = zmsg_addstr(msg_req, "M");
    assert(rv == 0);
    rv = zmsg_addstr(msg_req, "DIADDR_REQUEST");
    assert(rv == 0);
    rv = zmsg_send(&msg_req, sock);
    if (rv != 0) {
        err(thread_ctx->log_ctx,
            "Unable to send DIADDR_REQUEST request to "
            "host controller");
        return OSD_ERROR_CONNECTION_FAILED;
    }

    // response
    errno = 0;
    zmsg_t *msg_resp = zmsg_recv(sock);
    if (!msg_resp) {
        err(thread_ctx->log_ctx,
            "No response received from host controller at %s: %s (%d)",
            usrctx->host_controller_address, strerror(errno), errno);
        return OSD_ERROR_CONNECTION_FAILED;
    }

    zframe_t *type_frame = zmsg_pop(msg_resp);
    assert(zframe_streq(type_frame, "M"));
    zframe_destroy(&type_frame);

    char *addr_string = zmsg_popstr(msg_resp);
    assert(addr_string);
    char *end;
    long int addr = strtol(addr_string, &end, 10);
    assert(!*end);
    assert(addr <= UINT16_MAX);
    *di_addr = (uint16_t)addr;
    free(addr_string);

    zmsg_destroy(&msg_resp);

    dbg(thread_ctx->log_ctx,
        "Obtained DI address %u.%u (%u) from host controller.",
        osd_diaddr_subnet(*di_addr), osd_diaddr_localaddr(*di_addr), *di_addr);

    return OSD_OK;
}

/**
 * Connect to the host controller in the I/O thread
 *
 * This function is called by the I/O worker thread as response to the I-CONNECT
 * message. It creates a new DEALER ZeroMQ socket and uses it to connect to the
 * host controller. After completion the function sends out a I-CONNECT-DONE
 * message. The message value is -1 if the connection failed for any reason,
 * or the DI address assigned to the host module if the connection was
 * successfully established.
 */
static void iothread_connect_to_hostctrl(struct worker_thread_ctx *thread_ctx)
{
    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);

    osd_result retval;
    osd_result osd_rv;

    // create new DEALER socket to connect with the host controller
    usrctx->hostctrl_socket = zsock_new_dealer(usrctx->host_controller_address);
    if (!usrctx->hostctrl_socket) {
        err(thread_ctx->log_ctx, "Unable to connect to %s",
            usrctx->host_controller_address);
        retval = -1;
        goto free_return;
    }
    zsock_set_rcvtimeo(usrctx->hostctrl_socket, ZMQ_RCV_TIMEOUT);

    // Get our DI address
    uint16_t di_addr;
    osd_rv = iothread_obtain_diaddr(thread_ctx, &di_addr);
    if (OSD_FAILED(osd_rv)) {
        retval = -1;
        goto free_return;
    }
    retval = di_addr;

    // register handler for messages coming from the host controller
    int zmq_rv;
    zmq_rv = zloop_reader(thread_ctx->zloop, usrctx->hostctrl_socket,
                          iothread_rcv_from_hostctrl, thread_ctx);
    assert(zmq_rv == 0);
    zloop_reader_set_tolerant(thread_ctx->zloop, usrctx->hostctrl_socket);

free_return:
    if (retval == -1) {
        zsock_destroy(&usrctx->hostctrl_socket);
    }
    worker_send_status(thread_ctx->inproc_socket, "I-CONNECT-DONE", retval);
}

/**
 * Disconnect from the host controller in the I/O thread
 *
 * This function is called when receiving a I-DISCONNECT message in the
 * I/O thread. After the disconnect is done a I-DISCONNECT-DONE message is sent
 * to the main thread.
 */
static void iothread_disconnect_from_hostctrl(
    struct worker_thread_ctx *thread_ctx)
{
    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);

    osd_result retval;

    zloop_reader_end(thread_ctx->zloop, usrctx->hostctrl_socket);
    zsock_destroy(&usrctx->hostctrl_socket);

    retval = OSD_OK;

    worker_send_status(thread_ctx->inproc_socket, "I-DISCONNECT-DONE", retval);
}

static osd_result iothread_handle_inproc_request(
    struct worker_thread_ctx *thread_ctx, const char *name, zmsg_t *msg)
{
    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);

    int rv;

    if (!strcmp(name, "I-CONNECT")) {
        iothread_connect_to_hostctrl(thread_ctx);

    } else if (!strcmp(name, "I-DISCONNECT")) {
        iothread_disconnect_from_hostctrl(thread_ctx);

    } else if (!strcmp(name, "D")) {
        // Forward data packet to the host controller
        rv = zmsg_send(&msg, usrctx->hostctrl_socket);
        assert(rv == 0);

    } else {
        assert(0 && "Received unknown message from main thread.");
    }

    zmsg_destroy(&msg);

    return OSD_OK;
}

static osd_result iothread_destroy(struct worker_thread_ctx *thread_ctx)
{
    assert(thread_ctx);
    struct iothread_usr_ctx *usrctx = thread_ctx->usr;
    assert(usrctx);

    zlist_destroy(&usrctx->event_reassembly_buf);
    free(usrctx->host_controller_address);
    free(usrctx);
    thread_ctx->usr = NULL;

    return OSD_OK;
}

/**
 * Send a DI Packet to the host controller
 *
 * The actual sending is done through the I/O worker.
 */
static osd_result osd_hostmod_send_packet(struct osd_hostmod_ctx *ctx,
                                          const struct osd_packet *packet)
{
    assert(ctx);
    assert(ctx->ioworker_ctx);
    assert(ctx->ioworker_ctx->inproc_socket);

    int rv;
    zmsg_t *msg = zmsg_new();
    assert(msg);

    rv = zmsg_addstr(msg, "D");
    assert(rv == 0);
    rv = zmsg_addmem(msg, packet->data_raw, osd_packet_sizeof(packet));
    assert(rv == 0);

    rv = zmsg_send(&msg, ctx->ioworker_ctx->inproc_socket);
    if (rv != 0) {
        return OSD_ERROR_COM;
    }

    return OSD_OK;
}

/**
 * Receive a DI Packet
 *
 * @return OSD_OK if the operation was successful,
 *         OSD_ERROR_TIMEDOUT if the operation timed out.
 *         OSD_ERROR_FAILURE if the read operation was aborted
 *         Any other value indicates an error
 */
static osd_result osd_hostmod_receive_packet(struct osd_hostmod_ctx *ctx,
                                             struct osd_packet **packet,
                                             int flags)
{
    osd_result osd_rv;

    // block register read indefinitely until response has been received
    bool do_block = (flags & OSD_HOSTMOD_BLOCKING);

    errno = 0;
    zmsg_t *msg;
    do {
        msg = zmsg_recv(ctx->ioworker_ctx->inproc_socket);
    } while (!msg && errno == EAGAIN && do_block);
    if (!msg && errno == EAGAIN) {
        return OSD_ERROR_TIMEDOUT;
    }
    if (!msg) {
        return OSD_ERROR_FAILURE;
    }

    // ensure that the message we got from the I/O thread is packet data
    // XXX: possibly extend to hand off non-packet messages to their appropriate
    // handler
    zframe_t *type_frame = zmsg_pop(msg);
    assert(type_frame);
    assert(zframe_streq(type_frame, "D"));
    zframe_destroy(&type_frame);

    // get osd_packet from frame data
    zframe_t *data_frame = zmsg_pop(msg);
    assert(data_frame);
    struct osd_packet *p;
    osd_rv = osd_packet_new_from_zframe(&p, data_frame);
    assert(OSD_SUCCEEDED(osd_rv));

    zframe_destroy(&data_frame);
    zmsg_destroy(&msg);

    *packet = p;

    return OSD_OK;
}

API_EXPORT
osd_result osd_hostmod_new(struct osd_hostmod_ctx **ctx,
                           struct osd_log_ctx *log_ctx,
                           const char *host_controller_address,
                           osd_hostmod_event_handler_fn event_handler,
                           void *event_handler_arg)
{
    osd_result rv;

    struct osd_hostmod_ctx *c = calloc(1, sizeof(struct osd_hostmod_ctx));
    assert(c);

    c->log_ctx = log_ctx;
    c->is_connected = false;

    // prepare custom data passed to I/O thread
    struct iothread_usr_ctx *iothread_usr_data =
        calloc(1, sizeof(struct iothread_usr_ctx));
    assert(iothread_usr_data);

    iothread_usr_data->event_handler = event_handler;
    iothread_usr_data->event_handler_arg = event_handler_arg;
    iothread_usr_data->host_controller_address =
        strdup(host_controller_address);
    iothread_usr_data->event_reassembly_buf = zlist_new();

    rv = worker_new(&c->ioworker_ctx, log_ctx, NULL, iothread_destroy,
                    iothread_handle_inproc_request, iothread_usr_data);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    *ctx = c;

    return OSD_OK;
}

API_EXPORT
uint16_t osd_hostmod_get_diaddr(struct osd_hostmod_ctx *ctx)
{
    assert(ctx);
    assert(ctx->is_connected);
    return ctx->diaddr;
}

API_EXPORT
osd_result osd_hostmod_connect(struct osd_hostmod_ctx *ctx)
{
    osd_result rv;
    assert(ctx);
    assert(!ctx->is_connected);

    worker_send_status(ctx->ioworker_ctx->inproc_socket, "I-CONNECT", 0);
    int retval;
    rv = worker_wait_for_status(ctx->ioworker_ctx->inproc_socket,
                                "I-CONNECT-DONE", &retval);
    if (OSD_FAILED(rv) || retval == -1) {
        err(ctx->log_ctx, "Unable to establish connection to host controller.");
        return OSD_ERROR_CONNECTION_FAILED;
    }

    ctx->diaddr = retval;
    ctx->is_connected = true;

    dbg(ctx->log_ctx, "Connection established, DI address is %u.", ctx->diaddr);

    return OSD_OK;
}

API_EXPORT
bool osd_hostmod_is_connected(struct osd_hostmod_ctx *ctx)
{
    assert(ctx);
    return ctx->is_connected;
}

API_EXPORT
osd_result osd_hostmod_disconnect(struct osd_hostmod_ctx *ctx)
{
    osd_result rv;

    assert(ctx);

    if (!ctx->is_connected) {
        return OSD_ERROR_NOT_CONNECTED;
    }

    worker_send_status(ctx->ioworker_ctx->inproc_socket, "I-DISCONNECT", 0);
    osd_result retval;
    rv = worker_wait_for_status(ctx->ioworker_ctx->inproc_socket,
                                "I-DISCONNECT-DONE", &retval);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    if (OSD_FAILED(retval)) {
        return retval;
    }

    ctx->is_connected = false;

    return OSD_OK;
}

API_EXPORT
void osd_hostmod_free(struct osd_hostmod_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_hostmod_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    assert(!ctx->is_connected);

    worker_free(&ctx->ioworker_ctx);

    free(ctx);
    *ctx_p = NULL;
}

static osd_result osd_hostmod_regaccess(
    struct osd_hostmod_ctx *ctx, uint16_t module_addr, uint16_t reg_addr,
    enum osd_packet_type_reg_subtype subtype_req,
    enum osd_packet_type_reg_subtype subtype_resp, const uint16_t *wr_data,
    size_t wr_data_len_words, struct osd_packet **response, int flags)
{
    assert(ctx);
    if (!ctx->is_connected) {
        return OSD_ERROR_NOT_CONNECTED;
    }

    *response = NULL;
    osd_result retval = OSD_ERROR_FAILURE;
    osd_result rv;

    // assemble request packet
    struct osd_packet *pkg_req;
    unsigned int pkg_size_words =
        osd_packet_sizeconv_payload2data(1 + wr_data_len_words);
    rv = osd_packet_new(&pkg_req, pkg_size_words);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto err_free_req;
    }

    osd_packet_set_header(pkg_req, module_addr, ctx->diaddr,
                          OSD_PACKET_TYPE_REG, subtype_req);
    pkg_req->data.payload[0] = reg_addr;
    for (unsigned int i = 0; i < wr_data_len_words; i++) {
        pkg_req->data.payload[1 + i] = wr_data[i];
    }

    // send register read request
    rv = osd_hostmod_send_packet(ctx, pkg_req);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto err_free_req;
    }

    // wait for response
    struct osd_packet *pkg_resp;
    rv = osd_hostmod_receive_packet(ctx, &pkg_resp, flags);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto err_free_req;
    }

    // parse response
    assert(osd_packet_get_type(pkg_resp) == OSD_PACKET_TYPE_REG);

    // handle register read error
    if (osd_packet_get_type_sub(pkg_resp) == RESP_READ_REG_ERROR ||
        osd_packet_get_type_sub(pkg_resp) == RESP_WRITE_REG_ERROR) {
        err(ctx->log_ctx,
            "Got RESP_WRITE_REG_ERROR when accessing register %u of module %d",
            reg_addr, module_addr);
        retval = OSD_ERROR_DEVICE_ERROR;
        goto err_free_resp;
    }

    // validate response subtype
    if (osd_packet_get_type_sub(pkg_resp) != subtype_resp) {
        err(ctx->log_ctx, "Expected register response of subtype %d, got %d",
            subtype_resp, osd_packet_get_type_sub(pkg_resp));
        retval = OSD_ERROR_DEVICE_INVALID_DATA;
        goto err_free_resp;
    }

    retval = OSD_OK;
    *response = pkg_resp;
    goto err_free_req;

err_free_resp:
    free(pkg_resp);

err_free_req:
    free(pkg_req);

    return retval;
}

static enum osd_packet_type_reg_subtype get_subtype_reg_read_req(
    unsigned int reg_size_bit)
{
    return (reg_size_bit / 16) - 1;
}

static enum osd_packet_type_reg_subtype get_subtype_reg_read_success_resp(
    unsigned int reg_size_bit)
{
    return get_subtype_reg_read_req(reg_size_bit) | 0b1000;
}

static enum osd_packet_type_reg_subtype get_subtype_reg_write_req(
    unsigned int reg_size_bit)
{
    return ((reg_size_bit / 16) - 1) | 0b0100;
}

API_EXPORT
osd_result osd_hostmod_reg_read(struct osd_hostmod_ctx *ctx, void *reg_val,
                                uint16_t diaddr, uint16_t reg_addr,
                                int reg_size_bit, int flags)
{
    osd_result rv;
    osd_result retval;

    assert(reg_size_bit % 16 == 0 && reg_size_bit <= 128);

    dbg(ctx->log_ctx,
        "Issuing %d bit read request to register 0x%x of module 0x%x",
        reg_size_bit, reg_addr, diaddr);

    struct osd_packet *response_pkg;
    rv = osd_hostmod_regaccess(ctx, diaddr, reg_addr,
                               get_subtype_reg_read_req(reg_size_bit),
                               get_subtype_reg_read_success_resp(reg_size_bit),
                               NULL, 0, &response_pkg, flags);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    // validate response size
    unsigned int exp_data_size_words =
        osd_packet_sizeconv_payload2data(reg_size_bit / 16);
    if (response_pkg->data_size_words != exp_data_size_words) {
        err(ctx->log_ctx,
            "Expected %d 16 bit data words in register read response, got %d.",
            exp_data_size_words, response_pkg->data_size_words);
        retval = OSD_ERROR_DEVICE_INVALID_DATA;
        goto err_free_resp;
    }

    // make result available to caller
    // XXX: this is broken for anything else than 16 bit registers due to
    // endianness issues.
    memcpy(reg_val, response_pkg->data.payload, reg_size_bit / 8);

    retval = OSD_OK;

err_free_resp:
    free(response_pkg);

    return retval;
}

API_EXPORT
osd_result osd_hostmod_reg_write(struct osd_hostmod_ctx *ctx,
                                 const void *reg_val, uint16_t diaddr,
                                 uint16_t reg_addr, int reg_size_bit, int flags)
{
    assert(reg_size_bit % 16 == 0 && reg_size_bit <= 128);

    osd_result rv;
    osd_result retval;

    dbg(ctx->log_ctx,
        "Issuing %d bit write request to register 0x%x of module 0x%x",
        reg_size_bit, reg_addr, diaddr);

    struct osd_packet *response_pkg;
    rv = osd_hostmod_regaccess(
        ctx, diaddr, reg_addr, get_subtype_reg_write_req(reg_size_bit),
        RESP_WRITE_REG_SUCCESS, reg_val, reg_size_bit / 16, &response_pkg, flags);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    // validate response size
    unsigned int data_size_words_exp = osd_packet_sizeconv_payload2data(0);
    if (response_pkg->data_size_words != data_size_words_exp) {
        err(ctx->log_ctx,
            "Invalid write response received. Expected packet with %u data "
            "words, got %u words.",
            data_size_words_exp, response_pkg->data_size_words);
        retval = OSD_ERROR_DEVICE_INVALID_DATA;
        goto err_free_resp;
    }

    retval = OSD_OK;

err_free_resp:
    free(response_pkg);

    return retval;
}

API_EXPORT
osd_result osd_hostmod_reg_setbit(struct osd_hostmod_ctx *hostmod_ctx,
                                  unsigned int bitnum, bool bitval,
                                  uint16_t diaddr, uint16_t reg_addr,
                                  int reg_size_bit, int flags)
{
    osd_result rv;
    uint16_t val;
    rv = osd_hostmod_reg_read(hostmod_ctx, &val, diaddr, reg_addr, 16, flags);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    val = (val & ~(1 << bitnum)) | (bitval << bitnum);
    return osd_hostmod_reg_write(hostmod_ctx, &val, diaddr, reg_addr, 16,
                                 flags);
}

unsigned int osd_hostmod_get_max_event_words(struct osd_hostmod_ctx *ctx,
                                             unsigned int di_addr_target)
{
    // XXX: This should be not a constant but read from the SCM in the target
    // subnet of di_addr_target.
    return osd_packet_sizeconv_data2payload(OSD_MAX_PKG_LEN_WORDS);
}


osd_result osd_hostmod_event_send(struct osd_hostmod_ctx *ctx,
                                  const struct osd_packet* event_pkg)
{
    assert(ctx);
    assert(event_pkg);
    assert(osd_packet_get_type(event_pkg) == OSD_PACKET_TYPE_EVENT);

    if (!osd_hostmod_is_connected(ctx)) {
        return OSD_ERROR_NOT_CONNECTED;
    }

    return osd_hostmod_send_packet(ctx, event_pkg);
}

osd_result osd_hostmod_event_receive(struct osd_hostmod_ctx *ctx,
                                     struct osd_packet **event_pkg,
                                     int flags)
{
    /*
     * This implementation is currently rather naive, as the exact requirements
     * are not yet fixed.
     *
     * - It doesn't guarantee to return only event packets, but any packet
     *   which happens to be received when calling.
     * - Event packets are not treated differently from register access packets,
     *   requiring the source(s) to send them exactly in the expected order.
     * - It doesn't warn the user if he/she set a eventhandler in the
     *   constructor, which takes precedence over this function (no data will be
     *   returned ever).
     *
     * Once the exact requirements are fixed, this could be extended in a couple
     * ways:
     *
     * - Queue event packets when they are received in the iothread, and pop
     *   from this queue when this function is called; or
     * - Use a separate socket for pushing event packets between the iothread
     *   and the main thread, and receive from this socket here.
     */
    return osd_hostmod_receive_packet(ctx, event_pkg, flags);
}

osd_result osd_hostmod_get_modules(struct osd_hostmod_ctx *ctx,
                                   unsigned int subnet_addr,
                                   struct osd_module_desc **modules,
                                   size_t *modules_len)
{
    osd_result retval = OSD_OK;
    osd_result rv;

    uint16_t scm_diaddr = osd_diaddr_build(subnet_addr, 0);

    uint16_t num_modules;
    rv = osd_hostmod_reg_read(ctx, &num_modules, scm_diaddr,
                              OSD_REG_SCM_NUM_MOD, 16, 0);
    if (OSD_FAILED(rv)) {
        err(ctx->log_ctx, "Unable to read NUM_MOD from SCM in subnet %u",
            subnet_addr);
        return rv;
    }
    dbg(ctx->log_ctx, "Debug system with %u modules found.", num_modules);

    struct osd_module_desc *mods;
    mods = calloc(num_modules, sizeof(struct osd_module_desc));

    for (uint16_t localaddr = scm_diaddr; localaddr < num_modules;
         localaddr++) {
        uint16_t module_addr = osd_diaddr_build(subnet_addr, localaddr);

        rv = osd_hostmod_mod_describe(ctx, module_addr, &mods[module_addr]);
        if (OSD_FAILED(rv)) {
            err(ctx->log_ctx, "Failed to obtain information about debug "
                "module at address %u (rv=%d)", module_addr, rv);
            mods[module_addr].addr = module_addr;
            mods[module_addr].vendor = OSD_MODULE_VENDOR_UNKNOWN;
            mods[module_addr].type = OSD_MODULE_TYPE_STD_UNKNOWN;
            mods[module_addr].version = 0;
            retval = OSD_ERROR_PARTIAL_RESULT;
            // continue with the next module anyways
        } else {
            const char* type_name =
                osd_module_get_type_short_name(mods[module_addr].vendor,
                                               mods[module_addr].type);
            dbg(ctx->log_ctx,
                "Found debug module at address %u of type %s (%u.%u, v%u)",
                mods[module_addr].addr, type_name, mods[module_addr].vendor,
                mods[module_addr].type, mods[module_addr].version);
        }
    }
    dbg(ctx->log_ctx, "Enumerated of subnet %u completed.", subnet_addr);

    *modules = mods;
    *modules_len = num_modules;
    return retval;
}

API_EXPORT
osd_result osd_hostmod_mod_describe(struct osd_hostmod_ctx *ctx,
                                    uint16_t di_addr,
                                    struct osd_module_desc *desc)
{
    osd_result rv;

    desc->addr = di_addr;

    rv = osd_hostmod_reg_read(ctx, &desc->vendor, di_addr,
                              OSD_REG_BASE_MOD_VENDOR, 16, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    rv = osd_hostmod_reg_read(ctx, &desc->type, di_addr, OSD_REG_BASE_MOD_TYPE,
                              16, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    rv = osd_hostmod_reg_read(ctx, &desc->version, di_addr,
                              OSD_REG_BASE_MOD_VERSION, 16, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    return OSD_OK;
}

API_EXPORT
osd_result osd_hostmod_mod_set_event_dest(struct osd_hostmod_ctx *ctx,
                                          uint16_t di_addr, int flags)
{
    return osd_hostmod_reg_write(ctx, &ctx->diaddr, di_addr,
                                 OSD_REG_BASE_MOD_EVENT_DEST, 16,
                                 flags);
}

API_EXPORT
osd_result osd_hostmod_mod_set_event_active(struct osd_hostmod_ctx *ctx,
                                            uint16_t di_addr, bool enabled,
                                            int flags)
{
    return osd_hostmod_reg_setbit(ctx, OSD_REG_BASE_MOD_CS_ACTIVE_BIT, enabled,
                                  di_addr, OSD_REG_BASE_MOD_CS, 16,
                                  flags);
}


API_EXPORT
struct osd_log_ctx* osd_hostmod_log_ctx(struct osd_hostmod_ctx *ctx)
{
    return ctx->log_ctx;
}
