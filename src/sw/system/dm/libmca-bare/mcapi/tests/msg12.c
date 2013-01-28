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
/* Test: msg12
   Tests the alignment macros.  FIXME: need to set pass/fail criteria!!!.
   Currently all we're checking is that we were able to compile.
*/

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


int main(int argc, char** argv) {
  short rc = 0;
  short x MCAPI_DECL_ALIGNED = 0;
  short y MCAPI_DECL_ALIGNED = 0;
  short x_na = 0;
  short y_na = 0;
  char* buff;
  char* buff_na;
 

  printf ("alignment check1: x: 0x%02lX\n",(long unsigned int)&x);
  printf ("alignment check1: y: 0x%02lX\n",(long unsigned int)&y);
  printf ("alignment check1: x_na: 0x%02lX\n",(long unsigned int)&x_na);
  printf ("alignment check1: y_na: 0x%02lX\n",(long unsigned int)&y_na);
 
  
  buff = memalign(MCAPI_BUF_ALIGN,sizeof("H"));
  printf ("alignment check3: buff: 0x%02lX\n",(long unsigned int)buff);
  buff_na = malloc (sizeof("H"));
  printf ("alignment check3: buff_na: 0x%02lX\n",(long unsigned int)buff_na);
  

  free(buff_na);
  
  if (!rc) { printf("   Test PASSED\n"); }
  else  { printf("   Test FAILED\n"); }
  
  return rc;
}



  
