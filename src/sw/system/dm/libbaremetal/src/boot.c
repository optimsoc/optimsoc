/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Boot code for distributed memory systems.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <utils.h>
#include <assert.h>

extern char* printbuffer;

// The bootcode goes to a special section (@0x1000)
void _optimsoc_dm_boot() __attribute__ ((section(".bootload")));

extern unsigned int _end;
extern void main();
extern const unsigned int *exception_stack;

void _optimsoc_dm_boot() {
#ifdef PAGING
	// At the moment we only need to do something when using paging.

	// Determine the end of the image from the linker script symbol.
	unsigned int endofimage =(unsigned int)&_end;

	// At the moment we simply copy all pages of the image to the local
	// tile memory. It therefore assumes the local memory is sufficiently
	// large.

	for(int i=0x2000;i<endofimage+1;i+=0x2000) {
		// Do DMA copy. We cannot use the dma functions here
		// as they are also not loaded.
		REG32(0xe0201040) = i; // Local address
		REG32(0xe0201044) = 0x800; // Size in words
		REG32(0xe0201048) = 1; // Remote tile
		REG32(0xe020104c) = i; // Remote address
		REG32(0xe0201050) = 1; // R2L
		REG32(0xe0201054) = 1; // valid

		while (REG32(0xe0201054)==0) {}
	}
#endif

	// Set the exception stack that is used by the exception functions
	exception_stack = (unsigned int*) ((unsigned int)malloc(4096) + 4092);
	assert(exception_stack!=0);

	main();
}
