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

/*****************************************************************************
 *  Header: utils.h
 *
 *  Description: this file defines common utility functions
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _H_CONKER_UTILS_INCLUDE_
#define _H_CONKER_UTILS_INCLUDE_

#include <inttypes.h>
#include <mca_config.h>
#include <unistd.h> /* for getpid */
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>

extern int utils_start_timing(void); 

extern long double utils_end_timing(void);

extern void utils_print_affinity(void);  

extern void utils_set_affinity(int proc);

const char *conker_print_tid(pthread_t t) {
  static char buffer[100];
  char *p = buffer;

#ifdef __linux
  /* We know that pthread_t is an unsigned long */
  sprintf(p, "%lu", t);
#else
  /* Just print out the contents of the pthread_t */ {
    char *const tend = (char *) ((&t)+1);
    char *tp = (char *) &t;
    while (tp < tend) {
      p += sprintf (p, "%02x", *tp);
      tp++;
      if (tp < tend)
        *p++ = ':';
    }
  }
#endif
  return buffer;
}

/* Inline this so that it can be compiled out if CONKER_WITH_DEBUG is 0 */
inline void conker_dprintf(int level,const char *format, ...) {
  if (MCA_WITH_DEBUG) {
    va_list ap;
    int debug = 9; /* may eventually want a global or something, for now it's hardcoded */
    pthread_t tid = pthread_self();
    va_start(ap,format);
    if (level <= debug){
      printf ("/* CONKER PID:%d TID: %s */   //",getpid(),conker_print_tid(tid));
      /* call variatic printf */
      vprintf(format,ap);
      printf ("\n");
    }
    va_end(ap);
  }
}

#endif



#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
