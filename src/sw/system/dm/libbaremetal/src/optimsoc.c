#include "optimsoc.h"
#include "utils.h"
#include "int.h"

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

unsigned int time(unsigned int x) {
	return 123456;
}
