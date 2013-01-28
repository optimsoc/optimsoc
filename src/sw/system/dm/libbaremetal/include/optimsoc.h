#ifndef __OPTIMSOC_H__
#define __OPTIMSOC_H__

#include "dma.h"
#include "mp_simple.h"

#define OPTIMSOC_NA_BASE        0xe0000000
#define OPTIMSOC_NA_CONF        OPTIMSOC_NA_BASE + 0x00000
#define OPTIMSOC_NA_CONF_TILEID OPTIMSOC_NA_CONF + 0x0
#define OPTIMSOC_NA_CONF_XDIM   OPTIMSOC_NA_CONF + 0x4
#define OPTIMSOC_NA_CONF_YDIM   OPTIMSOC_NA_CONF + 0x8

#define OPTIMSOC_NA_CONF_MODS     OPTIMSOC_NA_CONF + 0xc
#define OPTIMSOC_NA_CONF_MPSIMPLE 0x1
#define OPTIMSOC_NA_CONF_DMA      0x2

#define OPTIMSOC_MPSIMPLE       OPTIMSOC_NA_BASE + 0x100000
#define OPTIMSOC_MPSIMPLE_SEND  OPTIMSOC_MPSIMPLE + 0x0
#define OPTIMSOC_MPSIMPLE_RECV  OPTIMSOC_MPSIMPLE + 0x0

#define OPTIMSOC_MPSIMPLE_STATUS_WAITING OPTIMSOC_MPSIMPLE + 0x18

#define OPTIMSOC_DEST_MSB 31
#define OPTIMSOC_DEST_LSB 27
#define OPTIMSOC_CLASS_MSB 26
#define OPTIMSOC_CLASS_LSB 24
#define OPTIMSOC_CLASS_NUM 8
#define OPTIMSOC_SRC_MSB 23
#define OPTIMSOC_SRC_LSB 19

typedef struct {
	
} optimsoc_conf;

extern unsigned int optimsoc_tileid;

extern void optimsoc_init(optimsoc_conf *config);

static inline unsigned int optimsoc_get_tileid() {
	return optimsoc_tileid;
}

// Get the number of compute tiles
int optimsoc_ctnum();

// This gives the rank in the set of compute tiles.
// For example in a system where a compute tile is at position 0 and
// one at position 3, they will get this output
//  tile 0 -> ctrank 0
//  tile 3 -> ctrank 1
int optimsoc_ctrank();

int optimsoc_tilerank(unsigned int tile);

// This is the reverse action of tilerank
int optimsoc_ranktile(unsigned int rank);

void uart_printf(const char *fmt, ...);

#endif
