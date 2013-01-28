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

typedef struct {
	
} optimsoc_conf;

extern unsigned int optimsoc_tileid;

extern void optimsoc_init(optimsoc_conf *config);

static inline unsigned int optimsoc_get_tileid() {
	return optimsoc_tileid;
}

#endif
