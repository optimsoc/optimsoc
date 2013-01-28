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
#endif /* __cplusplus */

#ifndef SYSVR4_H
#define SYSVR4_H

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> /* for memset and getpid */
#include <pthread.h>

#include <mrapi.h>

extern int debug;

/*
   System Interface 
   It is expected that these functions will be defined.
   Currently they are defined in baremetal/powerpc/mrapi_powerpc.c and
   in sysvr4/sysvr4.c
*/

/* shared memory management */
uint32_t sys_shmem_create(uint32_t shmkey,int sizeOfShmSeg);
uint32_t sys_shmem_get(uint32_t shmkey,int sizeOfShmSeg);
void* sys_shmem_attach(int id);
mrapi_boolean_t sys_shmem_detach(void *shm_address);
mrapi_boolean_t sys_shmem_delete(uint32_t shmid);

/* semaphore management */
mrapi_boolean_t sys_sem_create(int key,int num_locks,int* id);
mrapi_boolean_t sys_sem_get(int key,int num_locks,int* id);
mrapi_boolean_t sys_sem_lock (int id,int member);
mrapi_boolean_t sys_sem_trylock (int id,int member);
mrapi_boolean_t sys_sem_unlock (int id,int member);
mrapi_boolean_t sys_sem_delete(int sem_id);




#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
