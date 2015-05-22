
#include <stdio.h>
#include <assert.h>

#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <optimsoc-mp.h>

#include "gzll.h"
#include "messages.h"

optimsoc_mp_endpoint_handle _gzll_mp_ep_system;
optimsoc_mp_endpoint_handle *_gzll_mp_ep_system_remote;

typedef void (*gzll_message_handler_fptr)(struct gzll_message *);

gzll_message_handler_fptr gzll_message_handlers[GZLL_NUM_MESSAGE_TYPES];

void communication_init() {
    // Initialize function pointers
    gzll_message_handlers[GZLL_NODE_NEW] = &gzll_message_node_new_handler;

    optimsoc_mp_endpoint_create(&_gzll_mp_ep_system, 0, 0,
                                OPTIMSOC_MP_EP_CONNECTIONLESS, 32, 64);

    printf("Local endpoint created: %p\n", _gzll_mp_ep_system);

    _gzll_mp_ep_system_remote = calloc(optimsoc_get_numct(),
                                       sizeof(optimsoc_mp_endpoint_handle));

    for (int rank = 0; rank < optimsoc_get_numct(); rank++) {
        if (rank != gzll_rank) {
            optimsoc_mp_endpoint_handle *ep;
            uint32_t tile;

            ep = &_gzll_mp_ep_system_remote[rank];
            tile = optimsoc_get_ranktile(rank);

            optimsoc_mp_endpoint_get(ep, tile, 0, 0);
            printf("Remote endpoint @%d: %p\n", rank, _gzll_mp_ep_system_remote[rank]);
        }
    }
}

void communication_thread() {
    uint8_t buffer[256];
    struct gzll_message *msg = (struct gzll_message*) buffer;

    printf("Communication thread started\n");

    while (1) {
        uint32_t received;
        optimsoc_mp_msg_recv(_gzll_mp_ep_system, buffer, 256, &received);
        assert((received > 1) && (received == msg->len));
        assert(msg->type < GZLL_NUM_MESSAGE_TYPES);
        gzll_message_handlers[msg->type](msg);
    }

}

void message_send_node_new(uint32_t app_id, uint32_t app_nodeid,
                           uint32_t nodeid) {
    uint32_t msg_size;

    msg_size = sizeof(struct gzll_message) +
            sizeof(struct gzll_message_node_new);

    struct gzll_message *msg;
    struct gzll_message_node_new *msg_node_new;

    msg = calloc(1, msg_size);
    msg->len = msg_size;
    msg->source_rank = gzll_rank;
    msg->type = GZLL_NODE_NEW;

    msg_node_new = (struct gzll_message_node_new*) &msg->data[0];
    msg_node_new->app_id = app_id;
    msg_node_new->app_nodeid = app_nodeid;

    for (int r = 0; r < optimsoc_get_numct(); ++r) {
        if (r == gzll_rank) {
            continue;
        }
        optimsoc_mp_msg_send(_gzll_mp_ep_system, _gzll_mp_ep_system_remote[r],
                             (uint8_t*) msg, msg_size);
    }
    free(msg);
}

void gzll_message_node_new_handler(struct gzll_message *msg) {
    struct gzll_message_node_new *msg_node;
    msg_node = (struct gzll_message_node_new*) msg->data;

    printf("Received new node information\n");
    printf("  on rank: %d\n", msg->source_rank);
    printf("  appid: %d, nodeid: %d, ranknode: %d\n", msg_node->app_id,
           msg_node->app_nodeid, msg_node->rank_nodeid);

}
