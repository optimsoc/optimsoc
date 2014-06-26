#include "include/optimsoc-sysconfig.h"
#include "include/optimsoc-baremetal.h"

int optimsoc_ctrank(void) {
  return optimsoc_tilerank(optimsoc_get_tileid());
}

int optimsoc_tilerank(unsigned int tile) {
  for (int i = 0; i < _optimsoc_compute_tile_num; i++) {
      if (_optimsoc_compute_tiles[i] == tile) {
	  return i;
      }
  }
  return -1;
}

int optimsoc_ranktile(unsigned int rank) {
  return _optimsoc_compute_tiles[rank];
}

void optimsoc_init(optimsoc_conf *config) {

}

int optimsoc_ctnum(void) {
  return _optimsoc_compute_tile_num;
}

uint32_t optimsoc_noc_maxpacketsize(void) {
  return 8;
}

void optimsoc_trace_definesection(int id, char* name) {
  OPTIMSOC_TRACE(0x20,id);
  while (*name!=0) {
      OPTIMSOC_TRACE(0x21,*name);
      name = name + 1;
  }
}

void optimsoc_trace_defineglobalsection(int id, char* name) {

}

void optimsoc_trace_section(int id) {
  OPTIMSOC_TRACE(0x22,id);
}

void optimsoc_trace_kernelsection(void) {
  OPTIMSOC_TRACE(0x23,0);
}
