/*
 * sbrk.h
 *
 *  Created on: Nov 21, 2010
 *      Author: wallento
 */

#ifndef SBRK_H_
#define SBRK_H_

#include <errno.h>
#include <sys/types.h>

caddr_t _sbrk(int nbytes);

#endif /* SBRK_H_ */
