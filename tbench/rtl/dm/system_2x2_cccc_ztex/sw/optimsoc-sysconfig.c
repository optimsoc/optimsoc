#include <stdint.h>

// General
const uint8_t  _optimsoc_system_is_dm = 1;

// Clocking
const uint8_t  _optimsoc_system_has_static_clock = 1;
const uint32_t _optimsoc_system_clock = 40000000;
const uint32_t _optimsoc_system_dynclock = 0; // not needed

// NoC parameters
const uint16_t _optimsoc_noc_maxpacketsize = 8;

// Compute tiles
const uint32_t _optimsoc_compute_tile_memsize = 128*1024; // 128 kByte
const uint16_t _optimsoc_compute_tile_num = 4;
const uint16_t _optimsoc_compute_tiles[] = { 0, 1, 2, 3 };
const uint8_t _optimsoc_max_cores_per_compute_tile = 1;
const uint32_t _optimsoc_stacksize = 0x2000;

// Memory tiles
const uint32_t _optimsoc_mainmem_size = 0; // not needed

// Hostlink tiles
const uint8_t  _optimsoc_has_hostlink = 0;
const uint8_t  _optimsoc_hostlink = 0;

// UART tiles
const uint8_t  _optimsoc_has_uart = 0;
const uint16_t _optimsoc_uarttile = 0;
const uint8_t  _optimsoc_uart_lcd_enable = 0;
