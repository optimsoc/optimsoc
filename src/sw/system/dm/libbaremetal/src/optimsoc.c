#include "optimsoc.h"
#include "utils.h"
#include "int.h"

#include <sysconfig.h>

struct {
    unsigned int dma_present;
    unsigned int mp_simple_present;
} optimsoc_na_config;

unsigned int optimsoc_tileid;

void optimsoc_init(optimsoc_conf *conf) {
    int_init();

    // Read optimsoc tile id
    optimsoc_tileid = REG32(OPTIMSOC_NA_CONF_TILEID);

    unsigned int flags = REG32(OPTIMSOC_NA_CONF_MODS);
    optimsoc_na_config.mp_simple_present = flags & OPTIMSOC_NA_CONF_MPSIMPLE;
    optimsoc_na_config.dma_present       = flags & OPTIMSOC_NA_CONF_DMA;

    if (optimsoc_na_config.mp_simple_present) {
        optimsoc_mp_simple_init();
    }

    //dma_init();
}

// Get the number of compute tiles
int optimsoc_ctnum() {
    return optimsoc_compute_tile_num;
}

// This gives the rank in the set of compute tiles.
// For example in a system where a compute tile is at position 0 and
// one at position 3, they will get this output
//  tile 0 -> ctrank 0
//  tile 3 -> ctrank 1
int optimsoc_ctrank() {
    for (int i=0;i<optimsoc_compute_tile_num;i++) {
        if (optimsoc_compute_tiles[i]==optimsoc_tileid) {
            return i;
        }
    }
    return -1;
}

int optimsoc_tilerank(unsigned int tile) {
    for (int i=0;i<optimsoc_compute_tile_num;i++) {
        if (optimsoc_compute_tiles[i]==tile) {
            return i;
        }
    }
    return -1;
}

int optimsoc_ranktile(unsigned int rank) {
    return optimsoc_compute_tiles[rank];
}

unsigned int time(unsigned int x) {
    return 123456;
}
