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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <mcapi.h>

/* This is a sample stub of the transport layer.  To define a specific implementation,
   you would copy this file and fill in the functions. You would also need to define
   datatypes for the handles defined in mcapi.h */

mcapi_uint_t mcapi_trans_get_node_num(mcapi_uint_t* node_num)
{
  return 0;
}

mcapi_uint_t mcapi_trans_get_domain_num(mcapi_uint_t* domain_num)
{
  return 0;
}

mcapi_boolean_t mcapi_trans_set_node_num(mcapi_uint_t node_num)
{
  return MCAPI_FALSE;
}


/****************** error checking queries *************************/
/* checks if the given node is valid */
mcapi_boolean_t mcapi_trans_valid_node(mcapi_uint_t node_num)
{
  return MCAPI_FALSE;
}



/* checks to see if the port_num is a valid port_num for this system */
mcapi_boolean_t mcapi_trans_valid_port(mcapi_uint_t port_num)
{
  return MCAPI_FALSE;
}



/* checks if the endpoint handle refers to a valid endpoint */
mcapi_boolean_t mcapi_trans_valid_endpoint (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



/* checks if the channel is open for a given endpoint */
mcapi_boolean_t mcapi_trans_endpoint_channel_isopen (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan receive handle */
mcapi_boolean_t mcapi_trans_pktchan_recv_isopen (mcapi_pktchan_recv_hndl_t receive_handle) 
{
  return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan send handle */
mcapi_boolean_t mcapi_trans_pktchan_send_isopen (mcapi_pktchan_send_hndl_t send_handle) 
{
  return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan receive handle */
mcapi_boolean_t mcapi_trans_sclchan_recv_isopen (mcapi_sclchan_recv_hndl_t receive_handle) 
{
  return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan send handle */
mcapi_boolean_t mcapi_trans_sclchan_send_isopen (mcapi_sclchan_send_hndl_t send_handle) 
{
  return MCAPI_FALSE;
}



/* checks if the given endpoint is owned by the given node */
mcapi_boolean_t mcapi_trans_endpoint_isowner (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



channel_type mcapi_trans_channel_type (mcapi_endpoint_t endpoint)
{
  return 0;
}



mcapi_boolean_t mcapi_trans_channel_connected  (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_recv_endpoint (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_send_endpoint (mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}



/* checks if the given endpoints have compatible attributes */
mcapi_boolean_t mcapi_trans_compatible_endpoint_attributes  (mcapi_endpoint_t send_endpoint, mcapi_endpoint_t recv_endpoint)
{
  return MCAPI_FALSE;
}



/* checks if the given channel handle is valid */
mcapi_boolean_t mcapi_trans_valid_pktchan_send_handle( mcapi_pktchan_send_hndl_t handle)
{
  return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_pktchan_recv_handle( mcapi_pktchan_recv_hndl_t handle)
{
  return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_send_handle( mcapi_sclchan_send_hndl_t handle)
{
  return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_recv_handle( mcapi_sclchan_recv_hndl_t handle)
{
  return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_initialized (mca_domain_t domain_id,mca_node_t node_id)
{
  return MCAPI_FALSE;
}

mcapi_uint32_t mcapi_trans_num_endpoints()
{
  return 0;
}

mcapi_boolean_t mcapi_trans_valid_priority(mcapi_priority_t priority)
{
  return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_connected(mcapi_endpoint_t endpoint)
{
  return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_valid_status_param (mca_status_t* mcapi_status){
  return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_version_param (mcapi_info_t* mcapi_version)
{
  return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_buffer_param (void* buffer)
{
  return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_request_param (mcapi_request_t* request)
{
  return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_size_param (size_t* size)
{
  return MCAPI_TRUE;
}

/****************** initialization *************************/
/* initialize the transport layer */
mcapi_boolean_t mcapi_trans_initialize(mcapi_uint_t node_num)
{
  return MCAPI_FALSE;
}



/****************** tear down ******************************/
void mcapi_trans_finalize()
{
}



/****************** endpoints ******************************/
/* create endpoint <node_num,port_num> and return it's handle */
mcapi_boolean_t mcapi_trans_endpoint_create(mcapi_endpoint_t *endpoint,  mcapi_uint_t port_num,mcapi_boolean_t anonymous)
{
  return MCAPI_FALSE;
}



/* non-blocking get endpoint for the given <node_num,port_num> and set endpoint parameter to it's handle */
void mcapi_trans_endpoint_get_i(  mcapi_endpoint_t* endpoint, mcapi_uint_t node_num, mcapi_uint_t port_num,mcapi_request_t* request,mca_status_t* mcapi_status)
{
}




/* blocking get endpoint for the given <node_num,port_num> and return it's handle */
mcapi_boolean_t mcapi_trans_endpoint_get(mcapi_endpoint_t *endpoint,mcapi_uint_t node_num, mcapi_uint_t port_num)
{
  return MCAPI_FALSE;
}



/* delete the given endpoint */
void mcapi_trans_endpoint_delete( mcapi_endpoint_t endpoint)
{
}



/* get the attribute for the given endpoint and attribute_num */
void mcapi_trans_endpoint_get_attribute( mcapi_endpoint_t endpoint, mcapi_uint_t attribute_num, void* attribute, size_t attribute_size)
{
}



/* set the given attribute on the given endpoint */
void mcapi_trans_endpoint_attribute_set( mcapi_endpoint_t endpoint, mcapi_uint_t attribute_num, const void* attribute, size_t attribute_size)
{
}




/****************** msgs **********************************/
void mcapi_trans_msg_send_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, char* buffer, size_t buffer_size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



mcapi_boolean_t mcapi_trans_msg_send( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, char* buffer, size_t buffer_size)
{
  return MCAPI_FALSE;
}



void mcapi_trans_msg_recv_i( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



mcapi_boolean_t mcapi_trans_msg_recv( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, size_t* received_size)
{
  return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_msg_available( mcapi_endpoint_t receive_endoint)
{
  return 0;
}



/****************** channels general ****************************/

/****************** pkt channels ****************************/
void mcapi_trans_pktchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



void mcapi_trans_pktchan_recv_open_i( mcapi_pktchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}

 

void mcapi_trans_pktchan_send_open_i( mcapi_pktchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



void  mcapi_trans_pktchan_send_i( mcapi_pktchan_send_hndl_t send_handle, void* buffer, size_t size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



mcapi_boolean_t  mcapi_trans_pktchan_send( mcapi_pktchan_send_hndl_t send_handle, void* buffer, size_t size)
{
  return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_i( mcapi_pktchan_recv_hndl_t receive_handle,  void** buffer, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



mcapi_boolean_t mcapi_trans_pktchan_recv( mcapi_pktchan_recv_hndl_t receive_handle, void** buffer, size_t* received_size)
{
  return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_pktchan_available( mcapi_pktchan_recv_hndl_t   receive_handle)
{
  return 0;
}



mcapi_boolean_t mcapi_trans_pktchan_free( void* buffer)
{
  return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_close_i( mcapi_pktchan_recv_hndl_t  receive_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



void mcapi_trans_pktchan_send_close_i( mcapi_pktchan_send_hndl_t  send_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



/****************** scalar channels ****************************/
void mcapi_trans_sclchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



void mcapi_trans_sclchan_recv_open_i( mcapi_sclchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}

 

void mcapi_trans_sclchan_send_open_i( mcapi_sclchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
}



mcapi_boolean_t mcapi_trans_sclchan_send( mcapi_sclchan_send_hndl_t send_handle,  uint64_t dataword, uint32_t size)
{
  return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_sclchan_recv( mcapi_sclchan_recv_hndl_t receive_handle,uint64_t *data,uint32_t size)
{
  return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_sclchan_available_i( mcapi_sclchan_recv_hndl_t receive_handle)
{
  return 0;
}



void mcapi_trans_sclchan_recv_close_i( mcapi_sclchan_recv_hndl_t  recv_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
}



void mcapi_trans_sclchan_send_close_i( mcapi_sclchan_send_hndl_t send_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
}



/****************** test,wait & cancel ****************************/
mcapi_boolean_t mcapi_trans_test_i( mcapi_request_t* request, size_t* size,mca_status_t* mcapi_status)
{
  return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_wait( mcapi_request_t* request, size_t* size,mca_status_t* mcapi_status)
{
  return MCAPI_FALSE;
}



int mcapi_trans_wait_first( size_t number, mcapi_request_t** requests, size_t* size)
{
  return 0;
}



void mcapi_trans_cancel( mcapi_request_t* request)
{
}




#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
