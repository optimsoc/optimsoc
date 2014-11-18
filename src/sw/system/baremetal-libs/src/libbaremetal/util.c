#include "include/optimsoc-sysconfig.h"
#include "include/optimsoc-baremetal.h"

uint32_t optimsoc_get_numct(void) {
  return _optimsoc_compute_tile_num;
}

int optimsoc_get_ctrank(void) {
  return optimsoc_get_tilerank(optimsoc_get_tileid());
}

int optimsoc_get_tilerank(unsigned int tile) {
  for (int i = 0; i < _optimsoc_compute_tile_num; i++) {
      if (_optimsoc_compute_tiles[i] == tile) {
	  return i;
      }
  }
  return -1;
}

int optimsoc_get_ranktile(unsigned int rank) {
  return _optimsoc_compute_tiles[rank];
}

void optimsoc_init(optimsoc_conf *config) {

}

uint32_t optimsoc_mainmem_size() {
  return OPTIMSOC_NA_GMEM_SIZE;
}

uint32_t optimsoc_mainmem_tile() {
  return OPTIMSOC_NA_GMEM_TILE;
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

/* Disable interrupts and timer only in SPR_SR.
 * Stall timer interrupt, instead of prevent generation in SPR_TTMR */

uint32_t optimsoc_critical_begin() {
	return or1k_critical_start();
}

void optimsoc_critical_end(uint32_t restore) {
	or1k_critical_end(restore);
}
