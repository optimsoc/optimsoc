const unsigned int optimsoc_system_clock = 0; // Not needed
const unsigned int optimsoc_ticks = 0; // Not needed
const unsigned char optimsoc_has_hostlink = 0; // Needed at later stages
const unsigned short optimsoc_hostlink = 0; // Not needed
const unsigned int optimsoc_mainmem_size = 0; // Not needed here

const unsigned int optimsoc_compute_tile_num = 1;
const unsigned int optimsoc_compute_tiles[1] = { 0 };

const unsigned int optimsoc_compute_tile_memsize = 128*1024;

// Maximum packet size in flits
extern const unsigned int optimsoc_noc_maxpacketsize = 0; // Not needed here
