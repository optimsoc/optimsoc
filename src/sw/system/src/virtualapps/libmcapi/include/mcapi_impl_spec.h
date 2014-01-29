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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <stddef.h>  /* for size_t */
#include <stdint.h>
  
#include <mca.h>
  
  // We need the optimsoc header
//#include <optimsoc.h>

/******************************************************************
           definitions and constants
 ******************************************************************/
#ifndef MCAPI_MAX_PRIORITY
#define MCAPI_MAX_PRIORITY 10
#endif

#ifndef MAX_QUEUE_ELEMENTS
#define MAX_QUEUE_ELEMENTS 64
#endif

#ifndef MAX_NUM_ATTRIBUTES
#define MAX_NUM_ATTRIBUTES 4
#endif

/******************************************************************
           datatypes
******************************************************************/

/* mcapi data */
typedef int impl_info_t;  

typedef int mcapi_param_t;

typedef int mcapi_endpoint_t;


/* internal handles */
typedef int mcapi_pktchan_recv_hndl_t;
typedef int mcapi_pktchan_send_hndl_t;
typedef int mcapi_sclchan_send_hndl_t;
typedef int mcapi_sclchan_recv_hndl_t;
typedef int mcapi_request_t;

                                                                                                                                                                                                                                             
/* enum for channel types */                                                                                                                       
typedef enum {
  MCAPI_NO_CHAN = 0,
  MCAPI_PKT_CHAN,
  MCAPI_SCL_CHAN,
} channel_type;
                                                                                                                        

typedef int mcapi_node_attributes_t;
                                                                                                                        
#endif


#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
