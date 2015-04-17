
#include <stdio.h>

#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <optimsoc-mp.h>

#include "gzll.h"

optimsoc_mp_endpoint_handle _gzll_mp_ep_system;
optimsoc_mp_endpoint_handle *_gzll_mp_ep_system_remote;

void communication_init() {
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
    printf("Communication thread started\n");
    while (1) { asm("l.nop"); }

}
