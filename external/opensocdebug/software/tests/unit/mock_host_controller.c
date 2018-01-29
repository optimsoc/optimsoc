/* Copyright 2017 The Open SoC Debug Project
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

#include <check.h>

#include <czmq.h>
#include <osd/hostmod.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>
#include <pthread.h>

pthread_t mock_host_controller_thread;
volatile int mock_host_controller_ready;
volatile int mock_host_controller_thread_cancel;

zlist_t *mock_exp_req_list;
zlist_t *mock_exp_resp_list;
zlist_t *mock_event_tx_list;

zframe_t *last_hostmod_identity_frame;

/**
 * Wait until all events scheduled to be sent by the host controller are sent
 */
void mock_host_controller_wait_for_event_tx(void)
{
    while (zlist_size(mock_event_tx_list) != 0) {
        usleep(10);
    }
}

/**
 * Wait until expected incoming packets (requests) have been received
 */
void mock_host_controller_wait_for_requests(void)
{
    while (zlist_size(mock_exp_req_list) != 0) {
        usleep(10);
    }
}

/**
 * Called periodically to handle shutdown signal
 *
 * Return -1 to shut down the mock host controller, return 0 to ignore shutdown
 * request.
 */
static int mock_host_controller_shutdown_reactor(zloop_t *loop, int timer_id,
                                                 void *arg)
{
    if (mock_host_controller_thread_cancel) {
        // wait until all event packets have been sent and all expected incoming
        // (request) packets have been received before shutting down
        if (zlist_size(mock_event_tx_list) != 0
            || zlist_size(mock_exp_req_list) != 0) {
            return 0;
        }

        // Returning -1 ends zloop
        return -1;
    }

    return 0;
}

static int mock_host_controller_msg_reactor(zloop_t *loop, zsock_t *reader,
                                            void *arg)
{
    zmsg_t *msg_req = zmsg_recv(reader);
    assert(msg_req);

    printf("Received message: \n");
    zmsg_print(msg_req);

    zmsg_t *msg_req_exp = zlist_pop(mock_exp_req_list);
    ck_assert_msg(msg_req_exp,
                  "Received message, but no message was expected.\n");
    printf("Expecting message: \n");
    zmsg_print(msg_req_exp);

    // save the message source as destination for the response
    zframe_t *src_frame = zmsg_pop(msg_req);

    // additionally, save the message source for sending event packets (which
    // don't follow a strict request-response model)
    zframe_destroy(&last_hostmod_identity_frame);
    last_hostmod_identity_frame = zframe_dup(src_frame);

    // ensure that the request message is what we expect
    zframe_t *f_rcv = zmsg_first(msg_req);
    zframe_t *f_exp = zmsg_first(msg_req_exp);

    // do we expect a packet data message?
    bool is_data_msg_rcv = zframe_streq(f_rcv, "D");
    bool is_data_msg_exp = zframe_streq(f_exp, "D");

    unsigned int frame_idx = 0;

    while (f_rcv && f_exp) {
        bool frame_is_expected = zframe_eq(f_rcv, f_exp);
        ck_assert_msg(frame_is_expected,
                      "Received unexpected data in frame %u.", frame_idx);

        f_rcv = zmsg_next(msg_req);
        f_exp = zmsg_next(msg_req_exp);

        ck_assert_msg((f_rcv && f_exp) || (!f_rcv && !f_exp),
                      "Number of received and expected frames doesn't match.");
        frame_idx++;
    }

    zmsg_destroy(&msg_req);
    zmsg_destroy(&msg_req_exp);

    // send response message
    zmsg_t *msg_resp = zlist_pop(mock_exp_resp_list);

    zframe_t *f_type = zmsg_first(msg_resp);
    bool is_null_msg = zframe_streq(f_type, "N");
    if (is_null_msg) {
        zmsg_destroy(&msg_resp);
        printf("Not sending response as requested by queued null message.\n");
    }

    if (msg_resp && !is_null_msg) {
        zmsg_prepend(msg_resp, &src_frame);
        zmsg_send(&msg_resp, reader);
    }

    zframe_destroy(&src_frame);

    return 0;
}

/**
 * Send a DI packet (a data message) of type EVENT to the host module
 *
 * This function is triggered by a timer. Use
 * mock_host_controller_queue_event_packet() to queue DI event packets for
 * sending.
 */
static int mock_host_controller_event_tx_reactor(zloop_t *loop, int timer_id,
                                                 void *sock_void)
{
    zsock_t *sock = sock_void;

    zmsg_t *event_msg = zlist_pop(mock_event_tx_list);
    if (!event_msg) {
        return 0;
    }

    zframe_t *identity_frame = zframe_dup(last_hostmod_identity_frame);
    zmsg_prepend(event_msg, &identity_frame);
    zmsg_send(&event_msg, sock);

    return 0;
}

static void *mock_host_controller(void *arg)
{
    int rv;

    zsock_t *server_socket;
    zloop_t *mock_host_controller_loop;

    // init
    server_socket = zsock_new_router("inproc://testing");
    assert(server_socket);

    mock_host_controller_loop = zloop_new();
    rv = zloop_reader(mock_host_controller_loop, server_socket,
                      mock_host_controller_msg_reactor, NULL);
    ck_assert_int_eq(rv, 0);
    zloop_reader_set_tolerant(mock_host_controller_loop, server_socket);

    zloop_timer(mock_host_controller_loop, 100, 0,
                mock_host_controller_shutdown_reactor, NULL);
    ck_assert_int_eq(rv, 0);

    zloop_timer(mock_host_controller_loop, 10, 0,
                mock_host_controller_event_tx_reactor, server_socket);
    ck_assert_int_eq(rv, 0);

    // start processing
    mock_host_controller_ready = 1;
    zloop_start(mock_host_controller_loop);

    // cleanup
    mock_host_controller_ready = 0;
    zloop_destroy(&mock_host_controller_loop);
    zsock_destroy(&server_socket);

    return 0;
}

static void queue_data_packet(zlist_t *list, const struct osd_packet *packet)
{
    int rv;

    zmsg_t *msg = zmsg_new();
    ck_assert_ptr_ne(msg, NULL);

    rv = zmsg_addstr(msg, "D");
    ck_assert_int_eq(rv, 0);
    rv = zmsg_addmem(msg, packet->data_raw, osd_packet_sizeof(packet));
    ck_assert_int_eq(rv, 0);

    rv = zlist_append(list, msg);
    ck_assert_int_eq(rv, 0);
}

/**
 * Queue a "null packet"
 *
 * If the response queue contains a null packet, the host controller does not
 * send this packet as response, but instead doesn't respond at all. Use this
 * feature to test the error path in the packet processing (i.e. if the calling
 * function recognizes a timeout).
 */
static void queue_null_packet(zlist_t *list)
{
    int rv;

    zmsg_t *msg = zmsg_new();
    ck_assert_ptr_ne(msg, NULL);

    rv = zmsg_addstr(msg, "N");
    ck_assert_int_eq(rv, 0);

    rv = zlist_append(list, msg);
    ck_assert_int_eq(rv, 0);
}

/**
 * Queue a data packet to be sent by the host controller
 *
 * The queued packets are sent in regular time intervals in the order they were
 * queued (FIFO). Use mock_host_controller_expect_data_req() to send a packet
 * as response to a received packet.
 *
 * @see mock_host_controller_expect_data_req()
 */
osd_result mock_host_controller_queue_data_packet(const struct osd_packet *pkg)
{
    queue_data_packet(mock_event_tx_list, pkg);
    return OSD_OK;
}

/**
 * Expect a management message with a given command and a given response
 */
void mock_host_controller_expect_mgmt_req(const char *cmd, const char *resp)
{
    int rv;

    // request
    zmsg_t *req_msg = zmsg_new();
    ck_assert_ptr_ne(req_msg, NULL);
    rv = zmsg_addstr(req_msg, "M");
    ck_assert_int_eq(rv, 0);
    rv = zmsg_addstr(req_msg, cmd);
    ck_assert_int_eq(rv, 0);
    rv = zlist_append(mock_exp_req_list, req_msg);
    ck_assert_int_eq(rv, 0);

    // response
    zmsg_t *resp_msg = zmsg_new();
    ck_assert_ptr_ne(req_msg, NULL);
    rv = zmsg_addstr(resp_msg, "M");
    ck_assert_int_eq(rv, 0);
    rv = zmsg_addstr(resp_msg, resp);
    ck_assert_int_eq(rv, 0);
    rv = zlist_append(mock_exp_resp_list, resp_msg);
    ck_assert_int_eq(rv, 0);
}

/**
 * Expect a DI packet to be received by the host controller
 *
 * Optionally, a response can be triggered upon reception of the packet.
 * Use mock_host_controller_queue_data_packet() to send a data packet from the
 * host controller without a previously received request.
 *
 * @see mock_host_controller_queue_data_packet
 */
void mock_host_controller_expect_data_req(const struct osd_packet *req,
                                          const struct osd_packet *resp)
{
    queue_data_packet(mock_exp_req_list, req);
    if (!resp) {
        queue_null_packet(mock_exp_resp_list);
    } else {
        queue_data_packet(mock_exp_resp_list, resp);
    }
}

/**
 * Expect a request for a DI address from the module
 */
void mock_host_controller_expect_diaddr_req(unsigned int diaddr)
{
    char diaddr_str[12];
    snprintf(diaddr_str, 12, "%d", diaddr);
    mock_host_controller_expect_mgmt_req("DIADDR_REQUEST", diaddr_str);
}

/**
 * Add a 16 bit register read access to the mock
 *
 * Note that the @p ret_value is not checked by the mock, you need to check
 * in the test if the returned value arrived where it should.
 */
void mock_host_controller_expect_reg_read(unsigned int src, unsigned int dest,
                                          unsigned int reg_addr,
                                          uint16_t ret_value)
{
    osd_result rv;

    // request
    struct osd_packet *pkg_req;
    rv = osd_packet_new(&pkg_req, osd_packet_sizeconv_payload2data(1));
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(pkg_req, NULL);

    osd_packet_set_header(pkg_req, dest, src, OSD_PACKET_TYPE_REG,
                          REQ_READ_REG_16);
    pkg_req->data.payload[0] = reg_addr;

    // response
    struct osd_packet *pkg_resp;
    rv = osd_packet_new(&pkg_resp, osd_packet_sizeconv_payload2data(1));
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_set_header(pkg_resp, src, dest, OSD_PACKET_TYPE_REG,
                          RESP_READ_REG_SUCCESS_16);
    pkg_resp->data.payload[0] = ret_value;

    mock_host_controller_expect_data_req(pkg_req, pkg_resp);
    osd_packet_free(&pkg_req);
    osd_packet_free(&pkg_resp);
}

/**
 * Add a 16 bit register read access to the mock, but generate no response
 *
 * This function will cause the register read to time out (or block forever).
 * Use this function to test the handling of register read errors.
 */
void mock_host_controller_expect_reg_read_noresp(unsigned int src,
                                                 unsigned int dest,
                                                 unsigned int reg_addr)
{
    osd_result rv;

    // request
    struct osd_packet *pkg_req;
    rv = osd_packet_new(&pkg_req, osd_packet_sizeconv_payload2data(1));
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(pkg_req, NULL);

    osd_packet_set_header(pkg_req, dest, src, OSD_PACKET_TYPE_REG,
                          REQ_READ_REG_16);
    pkg_req->data.payload[0] = reg_addr;

    mock_host_controller_expect_data_req(pkg_req, NULL);
    osd_packet_free(&pkg_req);
}

/**
 * Expect a register read of the module's description register
 */
void mock_host_controller_expect_mod_describe(unsigned int src,
                                              unsigned int dest,
                                              uint16_t vendor, uint16_t type,
                                              uint16_t version)
{
    mock_host_controller_expect_reg_read(src, dest,
                                         OSD_REG_BASE_MOD_VENDOR, vendor);
    mock_host_controller_expect_reg_read(src, dest,
                                         OSD_REG_BASE_MOD_TYPE, type);
    mock_host_controller_expect_reg_read(src, dest,
                                         OSD_REG_BASE_MOD_VERSION, version);
}

/**
 * Add a 16 bit register write access to the mock
 *
 * Note that the @p ret_value is not checked by the mock, you need to check
 * in the test if the returned value arrived where it should.
 */
void mock_host_controller_expect_reg_write(unsigned int src, unsigned int dest,
                                           unsigned int reg_addr,
                                           uint16_t exp_write_value)
{
    osd_result rv;

    // request
    struct osd_packet *pkg_req;
    rv = osd_packet_new(&pkg_req, osd_packet_sizeconv_payload2data(2));
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(pkg_req, NULL);

    osd_packet_set_header(pkg_req, dest, src, OSD_PACKET_TYPE_REG,
                          REQ_WRITE_REG_16);
    pkg_req->data.payload[0] = reg_addr;
    pkg_req->data.payload[1] = exp_write_value;

    // response
    struct osd_packet *pkg_resp;
    rv = osd_packet_new(&pkg_resp, osd_packet_sizeconv_payload2data(0));
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_set_header(pkg_resp, src, dest, OSD_PACKET_TYPE_REG,
                          RESP_WRITE_REG_SUCCESS);

    mock_host_controller_expect_data_req(pkg_req, pkg_resp);
    osd_packet_free(&pkg_req);
    osd_packet_free(&pkg_resp);
}

/**
 * Setup the ZeroMQ router standing in for the host controller in this test
 */
void mock_host_controller_setup(void)
{
    int rv;

    mock_host_controller_thread_cancel = 0;
    mock_host_controller_ready = 0;
    rv = pthread_create(&mock_host_controller_thread, 0, mock_host_controller,
                        NULL);
    ck_assert_int_eq(rv, 0);

    mock_exp_req_list = zlist_new();
    mock_exp_resp_list = zlist_new();
    mock_event_tx_list = zlist_new();

    // it takes a bit for the ZeroMQ socket to be ready
    while (!mock_host_controller_ready) {
        usleep(10);
    }
}

void mock_host_controller_teardown(void)
{
    // wait until all event packets have been sent
    mock_host_controller_thread_cancel = 1;

    pthread_join(mock_host_controller_thread, NULL);

    // make sure all expected requests and responses have been received/sent
    ck_assert_uint_eq(zlist_size(mock_exp_req_list), 0);
    ck_assert_uint_eq(zlist_size(mock_exp_resp_list), 0);
    ck_assert_uint_eq(zlist_size(mock_event_tx_list), 0);

    zlist_destroy(&mock_exp_req_list);
    zlist_destroy(&mock_exp_resp_list);
    zlist_destroy(&mock_event_tx_list);
}
