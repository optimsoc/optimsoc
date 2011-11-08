/*
 * sbrk.c
 *
 *  Created on: Nov 21, 2010
 *      Author: wallento
 */

#include <errno.h>
#include <sys/types.h>

extern caddr_t _end;

/* just in case, most boards have at least some memory */
#ifndef RAMSIZE
#  define RAMSIZE             (caddr_t)0x100000
#endif

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
caddr_t
_sbrk(nbytes)
     int nbytes;
{
  static caddr_t heap_ptr = 0;
  caddr_t        base;

  if (heap_ptr == 0) {
    heap_ptr = (caddr_t)&_end;
  }

  if ((RAMSIZE - heap_ptr) >= 0) {
    base = heap_ptr;
    heap_ptr += nbytes;
    return (base);
  } else {
    errno = -1;
    return ((caddr_t)-1);
  }
}
