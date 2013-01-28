/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

(3) Neither the name of the Multicore Association nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __cplusplus
 extern "C" {
#endif 

#ifndef LOCK_H
#define LOCK_H 

#if HAVE_CONFIG_H
#  include <mca_config.h>
#endif /* HAVE_CONFIG_H */


/*
  The likely() and unlikely() directives instruct GCC that the results of the
  expression contained within are likely true or unlikely true. GCC can then
  perform block reordering and other optimizations to improve the performance
  of conditional branches taken based on the results of these expressions. They
  should only be used if the result is overwhelmingly in one direction; for
  example, marking error conditions as unlikely().
 */
#define likely(x)       __builtin_expect (!!(x), 1)
#define unlikely(x)     __builtin_expect (!!(x), 0)

#define __ASM           __asm__ __volatile__
	 
#define lw_sync()       __ASM ("lwsync" : : : "memory")
#define sync()          __ASM ("sync" : : : "memory" )
#define isync()         __ASM ("isync" : : : "memory")

/* 
   Simple mutex implementation
 */
#define LOCK_LOCKED			1
#define LOCK_UNLOCKED			0
	

/*
   Make a single attempt to acquire a a specified lock.
   \return Previous value of the lock.  The "try" was successful of the
   lock was previously LOCK_UNLOCKED, and the "try" fails if the previous
   value of the lock was LOCK_LOCKED.
*/
static inline uint32_t try_lock(volatile struct lock_t *lock) 
{
	uint32_t tmp = 0;

	asm volatile (
		"1: 	lwarx	%0,  0, %2	\n"
		"	cmpwi	 0, %0,  0	\n"
		"	bne-	2f		\n"
		"	stwcx.  %1,  0, %2	\n"
		"	bne-	1b		\n"
		"       isync			\n"
		"2:				"
		: "=&r" (tmp)
		: "r" (LOCK_LOCKED), "r" (&lock->lock)
		: "cr0", "memory"
	);
	return tmp;
}	

/*
   Repeatedly call try_lock until it is successful.
 */
static inline void acquire_lock(volatile struct lock_t *lock) 
{
	do {
	} while (unlikely(try_lock(lock) != LOCK_UNLOCKED));
}

/*
   Release an acquired lock.
 */
static inline void release_lock(volatile struct lock_t *lock) 
{
  lw_sync();	
  lock->lock = LOCK_UNLOCKED;
}


#endif /* ifndef LOCK_H */

#ifdef __cplusplus
 } /* extern "C" */
#endif
