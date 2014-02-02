/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * Transport layer for MCAPI implementation.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */
#include <stdlib.h>
#include <stdio.h>

#include <sys/optimsoc.h>
#include <sys/syscall.h>
#include <mcapi_syscalls.h>

#include <mcapi_trans.h>

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************/
// Specific endpoint handling
/*******************************************************************/


mcapi_boolean_t mcapi_trans_get_node_num(mcapi_node_t* node_num)
{
    struct mcapi_trans_get_node_num mcapi_param;

    mcapi_param.node_num = node_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_GET_NODE_NUM, (void*) &mcapi_param);
    return mcapi_param.ret;
}

mcapi_boolean_t mcapi_trans_get_domain_num(mcapi_domain_t* domain_num)
{
    struct mcapi_trans_get_domain_num mcapi_param;

    mcapi_param.domain_num = domain_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_GET_DOMAIN_NUM, (void*) &mcapi_param);
    return mcapi_param.ret;
}

mcapi_boolean_t mcapi_trans_set_node_num(mcapi_uint_t node_num)
{
    struct mcapi_trans_set_node_num mcapi_param;

    mcapi_param.node_num = node_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_SET_NODE_NUM, (void*) &mcapi_param);
    return mcapi_param.ret;
}


/****************** error checking queries *************************/
/* checks if the given node is valid */
mcapi_boolean_t mcapi_trans_valid_node(mcapi_uint_t node_num)
{
    struct mcapi_trans_valid_node mcapi_param;

    mcapi_param.node_num = node_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_VALID_NODE, (void*) &mcapi_param);
    return mcapi_param.ret;
}

/* checks to see if the port_num is a valid port_num for this system */
mcapi_boolean_t mcapi_trans_valid_port(mcapi_uint_t port_num)
{
    struct mcapi_trans_valid_port mcapi_param;

    mcapi_param.port_num = port_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_VALID_PORT, (void*) &mcapi_param);
    return mcapi_param.ret;
}

/* checks if the endpoint handle refers to a valid endpoint */
mcapi_boolean_t mcapi_trans_valid_endpoint (mcapi_endpoint_t endpoint)
{
    struct mcapi_trans_valid_endpoint mcapi_param;

    mcapi_param.endpoint = endpoint;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_VALID_ENDPOINT, (void*) &mcapi_param);
    return mcapi_param.ret;
}

/* checks if the channel is open for a given endpoint */
mcapi_boolean_t mcapi_trans_endpoint_channel_isopen (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_channel_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan receive handle */
mcapi_boolean_t mcapi_trans_pktchan_recv_isopen (mcapi_pktchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_pktchan_recv_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan send handle */
mcapi_boolean_t mcapi_trans_pktchan_send_isopen (mcapi_pktchan_send_hndl_t send_handle)
{
    printf("mcapi_trans_pktchan_send_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan receive handle */
mcapi_boolean_t mcapi_trans_sclchan_recv_isopen (mcapi_sclchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_sclchan_recv_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan send handle */
mcapi_boolean_t mcapi_trans_sclchan_send_isopen (mcapi_sclchan_send_hndl_t send_handle)
{
    printf("mcapi_trans_sclchan_send_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given endpoint is owned by the given node */
mcapi_boolean_t mcapi_trans_endpoint_isowner (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_isowner not implemented!\n");
    return MCAPI_FALSE;
}



channel_type mcapi_trans_channel_type (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_channel_type not implemented!\n");
    return 0;
}



mcapi_boolean_t mcapi_trans_channel_connected  (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_channel_connected not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_recv_endpoint (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_recv_endpoint not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_send_endpoint (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_send_endpoint not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given endpoints have compatible attributes */
mcapi_boolean_t mcapi_trans_compatible_endpoint_attributes  (mcapi_endpoint_t send_endpoint, mcapi_endpoint_t recv_endpoint)
{
    printf("mcapi_trans_compatible_endpoint_attributes not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given channel handle is valid */
mcapi_boolean_t mcapi_trans_valid_pktchan_send_handle( mcapi_pktchan_send_hndl_t handle)
{
    printf("mcapi_trans_valid_pktchan_send_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_pktchan_recv_handle( mcapi_pktchan_recv_hndl_t handle)
{
    printf("mcapi_trans_valid_pktchan_recv_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_send_handle( mcapi_sclchan_send_hndl_t handle)
{
    printf("mcapi_trans_valid_sclchan_send_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_recv_handle( mcapi_sclchan_recv_hndl_t handle)
{
    printf("mcapi_trans_valid_sclchan_recv_handle not implemented!\n");
    return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_initialized (mca_domain_t domain_id,mca_node_t node_id)
{
    return MCAPI_FALSE;
    // TODO: This is _not whether it is generically initialized, but if this instance
    //       is initialized. This _may_ mean we also need to check whether the underlying
    //       runtime layer is initialized. Check..
    struct mcapi_trans_initialized mcapi_param;

    mcapi_param.domain_id = domain_id;
    mcapi_param.node_id = node_id;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_INITIALIZED, (void*) &mcapi_param);
    return mcapi_param.ret;
}

mcapi_uint32_t mcapi_trans_num_endpoints()
{
    struct mcapi_trans_num_endpoints mcapi_param;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_NUM_ENDPOINTS, (void*) &mcapi_param);
    return mcapi_param.ret;
}

mcapi_boolean_t mcapi_trans_valid_priority(mcapi_priority_t priority)
{
    // For the moment we just accept all priorities
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_connected(mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_connected not implemented!\n");
    return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_valid_status_param (mca_status_t* mcapi_status){
    printf("mcapi_trans_valid_status_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_version_param (mcapi_info_t* mcapi_version)
{
    printf("mcapi_trans_valid_version_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_buffer_param (void* buffer)
{
    // Why should a pointer be invalid?
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_request_param (mcapi_request_t* request)
{
    printf("mcapi_trans_valid_request_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_size_param (size_t* size)
{
    printf("mcapi_trans_valid_size_param not implemented!\n");
    return MCAPI_TRUE;
}

/****************** initialization *************************/
/* initialize the transport layer */
mcapi_boolean_t mcapi_trans_initialize(mca_domain_t domain_id,
        mcapi_node_t node_num,
        const mcapi_node_attributes_t* node_attrs)
{
    if(!mcapi_trans_initialized(domain_id, node_num)){
        printf("Call mcapi syshandler\n");
	struct mcapi_trans_initialize mcapi_param;
	mcapi_param.domain_id = domain_id;
	mcapi_param.node_num = node_num;
	mcapi_param.node_attrs = (mcapi_node_attributes_t*) node_attrs;
	syscall(SYSCALL_MCAPI, MCAPI_TRANS_INITIALIZE, (void*) &mcapi_param);
    printf("Return from mcapi syshandler\n");
	return mcapi_param.ret;
    }
    return MCAPI_TRUE;
}



/****************** tear down ******************************/
mcapi_boolean_t mcapi_trans_finalize() {
    printf("mcapi_trans_finalize not implemented!\n");
    return MCAPI_FALSE;
}



/****************** endpoints ******************************/
/* create endpoint <node_num,port_num> and return it's handle */
mcapi_boolean_t mcapi_trans_endpoint_create(mcapi_endpoint_t *endpoint,  mcapi_uint_t port_num, mcapi_boolean_t anonymous)
{
    struct mcapi_trans_endpoint_create mcapi_param;
    mcapi_param.endpoint = endpoint;
    mcapi_param.port_num = port_num;
    mcapi_param.anonymous = anonymous;
    syscall(SYSCALL_MCAPI, MCAPI_TRANS_ENDPOINT_CREATE, (void*) &mcapi_param);
    return mcapi_param.ret;
}


/* non-blocking get endpoint for the given <node_num,port_num> and set endpoint parameter to it's handle */
void mcapi_trans_endpoint_get_i(  mcapi_endpoint_t* endpoint, mca_domain_t domain_id,mcapi_uint_t node_num,
        mcapi_uint_t port_num,mcapi_request_t* request,
        mca_status_t* mcapi_status)
{
    printf("mcapi_trans_endpoint_get_i not implemented!\n");
}

/* blocking get endpoint for the given <node_num,port_num> and return it's handle */
/***************************************************************************
NAME:mcapi_trans_endpoint_get
DESCRIPTION:blocking get endpoint for the given <node_num,port_num>
PARAMETERS:
   endpoint - the endpoint handle to be filled in
   node_num - the node id
   port_num - the port id
RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure
 ***************************************************************************/
void mcapi_trans_endpoint_get(mcapi_endpoint_t *endpoint,
        mcapi_domain_t domain_id,
        mcapi_uint_t node_num,
        mcapi_uint_t port_num)
{
    struct mcapi_trans_endpoint_get mcapi_param;

    mcapi_param.endpoint = endpoint;
    mcapi_param.domain_id = domain_id;
    mcapi_param.node_num = node_num;
    mcapi_param.port_num = port_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_ENDPOINT_GET, (void*) &mcapi_param);
}



/* delete the given endpoint */
void mcapi_trans_endpoint_delete( mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_delete not implemented!\n");
}



/* get the attribute for the given endpoint and attribute_num */
void mcapi_trans_endpoint_get_attribute(
        mcapi_endpoint_t endpoint,
        mcapi_uint_t attribute_num,
        void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status)
{
    printf("mcapi_trans_endpoint_get_attribute not implemented!\n");
}



/* set the given attribute on the given endpoint */
void mcapi_trans_endpoint_attribute_set( mcapi_endpoint_t endpoint, mcapi_uint_t attribute_num, const void* attribute, size_t attribute_size)
{
    printf("mcapi_trans_endpoint_attribute_set not implemented!\n");
}




/****************** msgs **********************************/
void mcapi_trans_msg_send_i( mcapi_endpoint_t  send_endpoint,
        mcapi_endpoint_t  receive_endpoint,
        const char* buffer, size_t buffer_size,
        mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_msg_send_i not implemented!\n");
}

mcapi_boolean_t mcapi_trans_msg_send( mcapi_endpoint_t  send_endpoint,
        mcapi_endpoint_t  receive_endpoint,
        const char* buffer, size_t buffer_size)
{
    struct mcapi_trans_msg_send mcapi_param;

    mcapi_param.send_endpoint = send_endpoint;
    mcapi_param.receive_endpoint = receive_endpoint;
    mcapi_param.buffer = (char*)buffer;
    mcapi_param.buffer_size = buffer_size;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_MSG_SEND, (void*) &mcapi_param);
    return mcapi_param.ret;
}

void mcapi_trans_msg_recv_i( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_msg_recv_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_msg_recv( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, size_t* received_size)
{
    struct mcapi_trans_msg_recv mcapi_param;

    mcapi_param.receive_endpoint = receive_endpoint;
    mcapi_param.buffer = buffer;
    mcapi_param.buffer_size = buffer_size;
    mcapi_param.received_size = received_size;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_MSG_RECV, (void*) &mcapi_param);
    return mcapi_param.ret;
}



mcapi_uint_t mcapi_trans_msg_available( mcapi_endpoint_t receive_endpoint)
{
    struct mcapi_trans_msg_available mcapi_param;

    mcapi_param.receive_endpoint = receive_endpoint;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_MSG_AVAILABLE, (void*) &mcapi_param);
    return mcapi_param.ret;
}



/****************** channels general ****************************/

/****************** pkt channels ****************************/
void mcapi_trans_pktchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_connect_i not implemented!\n");
}



void mcapi_trans_pktchan_recv_open_i( mcapi_pktchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_open_i not implemented!\n");
}



void mcapi_trans_pktchan_send_open_i( mcapi_pktchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_open_i not implemented!\n");
}



void  mcapi_trans_pktchan_send_i( mcapi_pktchan_send_hndl_t send_handle,
        const void* buffer, size_t size,
        mcapi_request_t* request,
        mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_i not implemented!\n");
}



mcapi_boolean_t  mcapi_trans_pktchan_send( mcapi_pktchan_send_hndl_t send_handle,
        const void* buffer, size_t size)
{
    printf("mcapi_trans_pktchan_send not implemented!\n");
    return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_i( mcapi_pktchan_recv_hndl_t receive_handle,  void** buffer, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_pktchan_recv( mcapi_pktchan_recv_hndl_t receive_handle, void** buffer, size_t* received_size)
{
    printf("mcapi_trans_pktchan_recv not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_pktchan_available( mcapi_pktchan_recv_hndl_t   receive_handle)
{
    printf("mcapi_trans_pktchan_available not implemented!\n");
    return 0;
}



mcapi_boolean_t mcapi_trans_pktchan_free( void* buffer)
{
    printf("mcapi_trans_pktchan_free not implemented!\n");
    return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_close_i( mcapi_pktchan_recv_hndl_t  receive_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_close_i not implemented!\n");
}



void mcapi_trans_pktchan_send_close_i( mcapi_pktchan_send_hndl_t  send_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_close_i not implemented!\n");
}



/****************** scalar channels ****************************/
void mcapi_trans_sclchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_connect_i not implemented!\n");
}



void mcapi_trans_sclchan_recv_open_i( mcapi_sclchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_recv_open_i not implemented!\n");
}



void mcapi_trans_sclchan_send_open_i( mcapi_sclchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_send_open_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_sclchan_send( mcapi_sclchan_send_hndl_t send_handle,  uint64_t dataword, uint32_t size)
{
    printf("mcapi_trans_sclchan_send not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_sclchan_recv( mcapi_sclchan_recv_hndl_t receive_handle,uint64_t *data,uint32_t size)
{
    printf("mcapi_trans_sclchan_recv not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_sclchan_available_i( mcapi_sclchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_sclchan_available_i not implemented!\n");
    return 0;
}



void mcapi_trans_sclchan_recv_close_i( mcapi_sclchan_recv_hndl_t  recv_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_recv_close_i not implemented!\n");
}



void mcapi_trans_sclchan_send_close_i( mcapi_sclchan_send_hndl_t send_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_send_close_i not implemented!\n");
}



/****************** test,wait & cancel ****************************/
mcapi_boolean_t mcapi_trans_test_i( mcapi_request_t* request, size_t* size,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_test_i not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_wait( mcapi_request_t* request, size_t* size,
        mca_status_t* mcapi_status,
        mca_timeout_t timeout)
{
    printf("mcapi_trans_wait not implemented!\n");
    return MCAPI_FALSE;
}



int mcapi_trans_wait_first( size_t number, mcapi_request_t** requests, size_t* size)
{
    printf("mcapi_trans_wait_first not implemented!\n");
    return 0;
}



void mcapi_trans_cancel( mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_cancel not implemented!\n");
}


/***************************************************************************
  NAME:mcapi_trans_display_state
  DESCRIPTION: This function is useful for debugging.  If the handle is null,
   we'll print out the state of the entire database.  Otherwise, we'll print out
   only the state of the endpoint that the handle refers to.
  PARAMETERS:
     handle
  RETURN VALUE: none
 ***************************************************************************/
void mcapi_trans_display_state (void* handle)
{
    printf("mcapi_trans_display_state not implemented!\n");
}


/***************************************************************************
  NAME:mcapi_trans_node_init_attributes
  DESCRIPTION:
  PARAMETERS:
  RETURN VALUE:
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_init_attributes(
        mcapi_node_attributes_t* mcapi_node_attributes,
        mcapi_status_t* mcapi_status
) {

    printf("mcapi_trans_node_init_attributes not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_node_get_attribute
  DESCRIPTION:
  PARAMETERS:
  RETURN VALUE:
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_get_attribute(
        mcapi_domain_t domain_id,
        mcapi_node_t node_id,
        mcapi_uint_t attribute_num,
        void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status) {

    printf("mcapi_trans_node_get_attribute not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_node_set_attribute
  DESCRIPTION:
  PARAMETERS:
  RETURN VALUE:
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_set_attribute(
        mcapi_node_attributes_t* mcapi_node_attributes,
        mcapi_uint_t attribute_num,
        const void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status
){
    printf("mcapi_trans_node_set_attribute not implemented!\n");
    return MCAPI_FALSE;

}

/***************************************************************************
  NAME: mcapi_trans_endpoint_exists
  DESCRIPTION: checks if an endpoint has been created for this port id
  PARAMETERS: port id
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_endpoint_exists (mcapi_domain_t domain_id,
        uint32_t port_num)
{
    struct mcapi_trans_endpoint_exists mcapi_param;

    mcapi_param.domain_id = domain_id;
    mcapi_param.port_num = port_num;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_ENDPOINT_EXISTS, (void*) &mcapi_param);
    return mcapi_param.ret;
}

/***************************************************************************
  NAME: mcapi_trans_valid_endpoints
  DESCRIPTION: checks if the given endpoint handles refer to valid endpoints
  PARAMETERS: endpoint1, endpoint2
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_valid_endpoints (mcapi_endpoint_t endpoint1,
        mcapi_endpoint_t endpoint2)
{
    struct mcapi_trans_valid_endpoints mcapi_param;

    mcapi_param.endpoint1 = endpoint1;
    mcapi_param.endpoint2 = endpoint2;

    syscall(SYSCALL_MCAPI, MCAPI_TRANS_VALID_ENDPOINTS, (void*) &mcapi_param);
    return mcapi_param.ret;
}

/***************************************************************************
  NAME: mcapi_trans_valid_request_handle
  DESCRIPTION:checks if the given request handle is valid
        This is the parameter test/wait/cancel pass in to be processed.
  PARAMETERS: request
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_valid_request_handle (mcapi_request_t* request)
{
    printf("mcapi_trans_valid_request_handle not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_wait_any
  DESCRIPTION:Tests if any of the requests have completed yet (blocking).
      Note: the request is now cleared if it has been completed or cancelled.
  PARAMETERS:
    send_handle -
    request -
    mcapi_status -
  RETURN VALUE:
 ***************************************************************************/
unsigned mcapi_trans_wait_any(size_t number, mcapi_request_t** requests, size_t* size,
        mcapi_status_t* mcapi_status,
        mcapi_timeout_t timeout)
{
    printf("mcapi_trans_wait_any not implemented!\n");
    return MCA_RETURN_VALUE_INVALID;
}


#endif

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
