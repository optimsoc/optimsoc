/* Copyright (c) 2012-2013 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */
#include <stdlib.h>
#include <stdio.h>

#include <optimsoc.h>
#include <optimsoc-sysconfig.h>

#include <optimsoc-baremetal.h>
#include <mcapi_trans.h>

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MCAPI_EP_BUFFER_SIZE 16 // must be 2^n
#define MCAPI_EP_BUFFER_MASK (MCAPI_EP_BUFFER_SIZE - 1)


// Indicates whether the transport has been initialized
unsigned int initialized = 0;

mcapi_domain_t mcapi_domainid;
mcapi_node_t mcapi_nodenum;


mcapi_boolean_t mcapi_trans_get_node_num(mcapi_node_t* node_num) {
    *node_num = 0;
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_get_domain_num(mcapi_domain_t* domain_num)
{
    *domain_num = optimsoc_get_tileid();
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_set_node_num(mcapi_uint_t node_num)
{
    return MCAPI_FALSE;
}


/****************** error checking queries *************************/
/* checks if the given node is valid */
mcapi_boolean_t mcapi_trans_valid_node(mcapi_uint_t node_num)
{
 //   if (node_num<4) {
        return MCAPI_TRUE;
 //   } else {
 //       return MCAPI_FALSE;
 //   }
}

mcapi_boolean_t mcapi_trans_initialized (mca_domain_t domain_id,mca_node_t node_id)
{
    return ((initialized>0) ? MCAPI_TRUE : MCAPI_FALSE);
}

mcapi_boolean_t mcapi_trans_valid_priority(mcapi_priority_t priority)
{
    // For the moment we just accept all priorities
    return MCAPI_TRUE;
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
    control_init();

    endpoints_init();

    mcapi_domainid = domain_id;
    mcapi_nodenum = node_num;

    initialized = 1;

#ifdef RUNTIME
    mcapi_syscall_mapper_init();
#endif

    return MCAPI_TRUE;
}



/****************** tear down ******************************/
mcapi_boolean_t mcapi_trans_finalize() {
    printf("mcapi_trans_finalize not implemented!\n");
    return MCAPI_FALSE;
}



/****************** endpoints ******************************/




/****************** msgs **********************************/



/****************** channels general ****************************/




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
    if (port_num == MCAPI_PORT_ANY)
        return MCAPI_FALSE;

    // TODO: Does this involve checking remotely?
    struct endpoint_handle *eph;
    eph = endpoint_get(domain_id,0,port_num);

    return ((eph!=NULL) ? MCAPI_TRUE : MCAPI_FALSE);

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
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) endpoint1;
    if (endpoint_verify(eph)!=eph) {
        return MCAPI_FALSE;
    }
    eph = (struct endpoint_handle*) endpoint2;
    if (endpoint_verify(eph)!=eph) {
        return MCAPI_FALSE;
    }
    return MCAPI_TRUE;
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

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
