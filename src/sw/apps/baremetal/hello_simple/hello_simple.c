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
 * Simple hello world example.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <stdio.h>
#include <optimsoc.h>

unsigned int mylock;

void lock(void* addr) {
  volatile unsigned int ack = 0;
  do {
    REG32(0xe0300000) = addr;
    ack = 0;
    while(ack == 0x00) {
      ack = REG32(0xe0300008);
      OPTIMSOC_TRACE(0x70,ack);
    }
  } while(ack > 1);
}

void unlock(void* addr) {
    unsigned int ack = 0;
    REG32(0xe0300004) = addr;
    while(ack == 0x00) {
        ack = REG32(0xe0300008);
    }
}

void main() {
  //  lock((void*) &mylock);
  printf("Hello World!\n");
  //  unlock((void*) &mylock);
  //  printf("%p\n",malloc(4));
}
