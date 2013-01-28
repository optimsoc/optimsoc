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

#ifndef MCAPI_DATATYPES_H
#define MCAPI_DATATYPES_H


#include <stddef.h>  /* for size_t */
#include <stdint.h>

#include <mca.h>
#include <mca_impl_spec.h>

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
          a few convenience functions (not part of API) 
******************************************************************/
void mcapi_set_debug_level (int d);
void mcapi_display_state (void* handle);


/******************************************************************
           datatypes
******************************************************************/ 
/* enum for channel types */
typedef enum {
  MCAPI_NO_CHAN = 0,
  MCAPI_PKT_CHAN,
  MCAPI_SCL_CHAN,
} channel_type;


typedef int impl_info_t;  

typedef uint32_t mcapi_endpoint_t;


/* internal handles */     
typedef uint32_t mcapi_pktchan_recv_hndl_t;
typedef uint32_t mcapi_pktchan_send_hndl_t;
typedef uint32_t mcapi_sclchan_send_hndl_t;
typedef uint32_t mcapi_sclchan_recv_hndl_t;

typedef mca_request_t mcapi_request_t;

typedef int mcapi_param_t;

typedef struct {
  mca_boolean_t valid;
  uint16_t attribute_num;
  uint32_t bytes;
  void* attribute_d;  
} attribute_entry_t;

typedef struct {
  attribute_entry_t entries[MAX_NUM_ATTRIBUTES];
} attributes_t;
  
typedef attributes_t mcapi_node_attributes_t;
typedef attributes_t mcapi_endpt_attributes_t;

#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
