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

#define TEST_SUITE_NAME "check_gateway"

#include "mock_host_controller.h"
#include "testutil.h"

#include <czmq.h>
#include <osd/gateway.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>

struct osd_gateway_ctx *gateway_ctx;
struct osd_log_ctx *log_ctx;

const unsigned int test_device_subnet_addr = 0;

zlist_t *packet_read_from_device_queue;
pthread_mutex_t packet_read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t packet_read_cond = PTHREAD_COND_INITIALIZER;

zlist_t *packet_write_to_device_queue;
pthread_mutex_t packet_write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t packet_write_cond = PTHREAD_COND_INITIALIZER;

volatile int device_is_disconnected = 0;

struct device_queue_item {
    struct osd_packet *pkg;
    osd_result retcode;
};

static osd_result packet_read_from_device(struct osd_packet **pkg, void *cb_arg)
{
    osd_result retcode;
    struct device_queue_item *item = NULL;

    pthread_mutex_lock(&packet_read_mutex);
    if (zlist_size(packet_read_from_device_queue) == 0 &&
        device_is_disconnected) {
        *pkg = NULL;
        retcode = OSD_ERROR_NOT_CONNECTED;
        goto ret;
    }

    while (zlist_size(packet_read_from_device_queue) == 0) {
        pthread_cond_wait(&packet_read_cond, &packet_read_mutex);
    }

    item = zlist_pop(packet_read_from_device_queue);
    ck_assert(item);

    *pkg = item->pkg;
    retcode = item->retcode;

ret:
    free(item);
    pthread_mutex_unlock(&packet_read_mutex);
    return retcode;
}

static osd_result packet_write_to_device(const struct osd_packet *pkg,
                                         void *cb_arg)
{
    struct device_queue_item *item;
    osd_result retcode;

    pthread_mutex_lock(&packet_write_mutex);
    if (zlist_size(packet_write_to_device_queue) == 0 &&
        device_is_disconnected) {
        retcode = OSD_ERROR_NOT_CONNECTED;
        goto ret;
    }

    while (zlist_size(packet_write_to_device_queue) == 0) {
        pthread_cond_wait(&packet_write_cond, &packet_write_mutex);
    }

    item = zlist_pop(packet_write_to_device_queue);
    ck_assert(item);

    ck_assert(osd_packet_equal(pkg, item->pkg));
    retcode = item->retcode;

ret:
    osd_packet_free(&item->pkg);
    free(item);
    pthread_mutex_unlock(&packet_write_mutex);
    return retcode;
}

static struct osd_packet* get_test_packet(void)
{
    osd_result rv;

    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(1));
    ck_assert_int_eq(rv, OSD_OK);
    rv = osd_packet_set_header(pkg, 1025,
                               osd_diaddr_build(test_device_subnet_addr, 2),
                               OSD_PACKET_TYPE_EVENT, 0);
    ck_assert_int_eq(rv, OSD_OK);
    pkg->data.payload[0] = 0xdead;

    return pkg;
}

/**
 * Setup osd_gateway
 */
static void setup_gateway(void)
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    // init
    device_is_disconnected = 0;
    rv = osd_gateway_new(&gateway_ctx, log_ctx, "inproc://testing",
                         test_device_subnet_addr, packet_read_from_device,
                         packet_write_to_device, NULL);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(gateway_ctx, NULL);

    ck_assert_int_eq(osd_gateway_is_connected(gateway_ctx), 0);

    // connect
    mock_host_controller_expect_mgmt_req("GW_REGISTER 0", "ACK");

    rv = osd_gateway_connect(gateway_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_gateway_is_connected(gateway_ctx), 1);
}

static void teardown_gateway(void)
{
    osd_result rv;

    if (osd_gateway_is_connected(gateway_ctx)) {
        mock_host_controller_expect_mgmt_req("GW_UNREGISTER 0", "ACK");

        // Close connection to device: read and write return OSD_ERROR_NOT_CONNECTED
        device_is_disconnected = 1;

        struct device_queue_item *item2 = calloc(1, sizeof(struct device_queue_item));
        item2->pkg = NULL;
        item2->retcode = OSD_ERROR_NOT_CONNECTED;

        pthread_mutex_lock(&packet_read_mutex);
        zlist_append(packet_read_from_device_queue, item2);
        pthread_cond_signal(&packet_read_cond);
        pthread_mutex_unlock(&packet_read_mutex);
    }

    rv = osd_gateway_disconnect(gateway_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_gateway_is_connected(gateway_ctx), 0);

    osd_gateway_free(&gateway_ctx);
    ck_assert_ptr_eq(gateway_ctx, NULL);
}

/**
 * Test fixture: setup (called before each tests)
 */
static void setup(void)
{
    packet_read_from_device_queue = zlist_new();
    packet_write_to_device_queue = zlist_new();

    mock_host_controller_setup();
    setup_gateway();
}

/**
 * Test fixture: setup (called after each test)
 */
static void teardown(void)
{
    mock_host_controller_wait_for_requests();
    mock_host_controller_wait_for_event_tx();

    teardown_gateway();
    mock_host_controller_teardown();

    // ensure that all queued packets have been exchanged
    ck_assert_uint_eq(zlist_size(packet_write_to_device_queue), 0);
    zlist_destroy(&packet_write_to_device_queue);
    ck_assert_uint_eq(zlist_size(packet_read_from_device_queue), 0);
    zlist_destroy(&packet_read_from_device_queue);
}

START_TEST(test_init_base)
{
    setup();
    teardown();
}
END_TEST

/**
 * Test how the gateway copes with the host controller not being reachable
 */
START_TEST(test_init_hostctrl_unreachable)
{
    osd_result rv;

    // log context
    rv = osd_log_new(&log_ctx, LOG_DEBUG, osd_log_handler);
    ck_assert_int_eq(rv, OSD_OK);

    // initialize gateway context
    rv = osd_gateway_new(&gateway_ctx, log_ctx, "inproc://testing",
                         test_device_subnet_addr, packet_read_from_device,
                         packet_write_to_device, NULL);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(gateway_ctx, NULL);

    ck_assert_int_eq(osd_gateway_is_connected(gateway_ctx), 0);

    // try to connect
    rv = osd_gateway_connect(gateway_ctx);
    ck_assert_int_eq(rv, OSD_ERROR_CONNECTION_FAILED);

    ck_assert_int_eq(osd_gateway_is_connected(gateway_ctx), 0);

    osd_gateway_free(&gateway_ctx);
}
END_TEST

/**
 * Test if the gateway shuts down itself if the device signals a broken
 * connection during a device read.
 */
START_TEST(test_shutdown_device_read_err)
{
    osd_result rv;

    setup();

    // when the disconnect is detected the gateway should close the connection
    // to the host controller
    mock_host_controller_expect_mgmt_req("GW_UNREGISTER 0", "ACK");

    // make device read function return a "connection closed" error
    struct device_queue_item *item = calloc(1, sizeof(struct device_queue_item));
    item->pkg = NULL;
    item->retcode = OSD_ERROR_NOT_CONNECTED;

    pthread_mutex_lock(&packet_read_mutex);
    zlist_append(packet_read_from_device_queue, item);
    pthread_cond_signal(&packet_read_cond);
    pthread_mutex_unlock(&packet_read_mutex);


    // wait until read error has been "noticed" by the gateway I/O thread
    while (zlist_size(packet_read_from_device_queue) != 0) {
        usleep(10);
    }

    // It takes a bit until the failing read manifests itself on the main thread
    // of osd_gateway. Tune this sleep number if the test fails. (Or find a more
    // reliable solution.)
    for (int i = 0; i < 1000; i++) {
        if (osd_gateway_is_connected(gateway_ctx) == false) {
            break;
        }
        usleep(100);
    }

    // check if the gateway is now disconnected
    ck_assert_uint_eq(osd_gateway_is_connected(gateway_ctx), false);

    teardown();
}
END_TEST

/**
 * Test if the gateway shuts down itself if the device signals a broken
 * connection during a device write.
 */
START_TEST(test_shutdown_device_write_err)
{
    osd_result rv;

    setup();

    // when the disconnect is detected the gateway should close the connection
    // to the host controller
    mock_host_controller_expect_mgmt_req("GW_UNREGISTER 0", "ACK");

    // make device read function return a "connection closed" error
    struct osd_packet *pkg = get_test_packet();
    struct device_queue_item *item = calloc(1, sizeof(struct device_queue_item));
    item->pkg = pkg;
    item->retcode = OSD_ERROR_NOT_CONNECTED;

    pthread_mutex_lock(&packet_write_mutex);
    zlist_append(packet_write_to_device_queue, item);
    pthread_cond_signal(&packet_write_cond);
    pthread_mutex_unlock(&packet_write_mutex);

    // send write request from host controller to gateway
    mock_host_controller_queue_data_packet(pkg);

    // wait until write error has been "noticed" by the gateway I/O thread
    mock_host_controller_wait_for_event_tx();
    while (zlist_size(packet_write_to_device_queue) != 0) {
        usleep(10);
    }

    // It takes a bit until the failing read manifests itself on the main thread
    // of osd_gateway. Tune this sleep number if the test fails. (Or find a more
    // reliable solution.)
    for (int i = 0; i < 1000; i++) {
        if (osd_gateway_is_connected(gateway_ctx) == false) {
            break;
        }
        usleep(100);
    }

    // check if the gateway is now disconnected
    ck_assert_uint_eq(osd_gateway_is_connected(gateway_ctx), false);

    teardown();
}
END_TEST

/**
 * Read a packet from the device and send it to the host controller
 */
START_TEST(test_core_device_to_hostctrl)
{
    osd_result rv;

    struct osd_packet *pkg = get_test_packet();

    // expect this packet at the host controller
    mock_host_controller_expect_data_req(pkg, NULL);

    // make the device read function return the packet
    struct device_queue_item *item = calloc(1, sizeof(struct device_queue_item));
    item->pkg = pkg;
    item->retcode = OSD_OK;

    pthread_mutex_lock(&packet_read_mutex);
    zlist_append(packet_read_from_device_queue, item);
    pthread_cond_signal(&packet_read_cond);
    pthread_mutex_unlock(&packet_read_mutex);
}
END_TEST

/**
 * Read a packet from the device and send it to the host controller
 *
 * The first read from the device fails with an error which should trigger
 * another read, i.e. the error is handled gracefully.
 */
START_TEST(test_core_device_to_hostctrl_sporadic_readfail)
{
    osd_result rv;

    struct osd_packet *pkg = get_test_packet();

    // expect this packet at the host controller
    mock_host_controller_expect_data_req(pkg, NULL);

    struct device_queue_item *item_temperr = calloc(1, sizeof(struct device_queue_item));
    item_temperr->pkg = NULL;
    item_temperr->retcode = OSD_ERROR_TIMEDOUT;

    struct device_queue_item *item_good = calloc(1, sizeof(struct device_queue_item));
    item_good->pkg = pkg;
    item_good->retcode = OSD_OK;

    pthread_mutex_lock(&packet_read_mutex);
    zlist_append(packet_read_from_device_queue, item_temperr);
    zlist_append(packet_read_from_device_queue, item_good);
    pthread_cond_signal(&packet_read_cond);
    pthread_mutex_unlock(&packet_read_mutex);
}
END_TEST

/**
 * Send a packet from the host controller to the device
 */
START_TEST(test_core_hostctrl_to_device)
{
    osd_result rv;

    // test packet
    struct osd_packet *pkg = get_test_packet();

    // make the device write function return the packet
    struct device_queue_item *item = calloc(1, sizeof(struct device_queue_item));
    item->pkg = pkg;
    item->retcode = OSD_OK;

    pthread_mutex_lock(&packet_write_mutex);
    zlist_append(packet_write_to_device_queue, item);
    pthread_cond_signal(&packet_write_cond);
    pthread_mutex_unlock(&packet_write_mutex);

    // send packet from host controller
    mock_host_controller_queue_data_packet(pkg);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_init, *tc_shutdown, *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    // Initialization
    // As the setup and teardown functions are pretty heavy, we check them
    // here independently and use them as test fixtures after this test
    // succeeds.
    tc_init = tcase_create("Init");
    tcase_add_test(tc_init, test_init_base);
    tcase_add_test(tc_init, test_init_hostctrl_unreachable);
    suite_add_tcase(s, tc_init);

    // Shutdown
    tc_shutdown = tcase_create("Shutdown");
    tcase_add_test(tc_shutdown, test_shutdown_device_read_err);
    tcase_add_test(tc_shutdown, test_shutdown_device_write_err);
    suite_add_tcase(s, tc_shutdown);

    // Core functionality
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_core_device_to_hostctrl);
    tcase_add_test(tc_core, test_core_device_to_hostctrl_sporadic_readfail);
    tcase_add_test(tc_core, test_core_hostctrl_to_device);
    suite_add_tcase(s, tc_core);

    return s;
}

