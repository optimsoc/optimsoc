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

#include <mcapi.h>
#include <mcapi_trans.h>
#include <mca_config.h>  /* for MAX_ defines */
//#include <mca_utils.h> /* for mca_set_debug_level */
#include <string.h> /* for strncpy */

/* FIXME: (errata B5) anyone can get an endpoint handle and call receive on it.  should
   this be an error?  It seems like only the node that owns the receive
   endpoint should be able to call receive. */
/* Convenience functions */

  
/* The following  functions are useful for debugging but are not part of the spec */
  #include <stdio.h>
char* mcapi_display_status (mcapi_status_t status,char* status_message, size_t size) {
  if ((size < MCAPI_MAX_STATUS_SIZE) || (status_message == NULL)) {
    fprintf(stderr,"ERROR: size passed to mcapi_display_status must be at least %d and status_message must not be NULL.\n",MCAPI_MAX_STATUS_SIZE);
    return status_message;
  }
  memset(status_message,0,size);
switch (status) {
  case (MCAPI_SUCCESS): return strcpy(status_message,"MCAPI_SUCCESS");
  case (MCAPI_PENDING): return strcpy(status_message,"MCAPI_PENDING");
  case (MCAPI_ERR_PARAMETER): return strcpy(status_message,"MCAPI_ERR_PARAMETER");
  case (MCAPI_ERR_DOMAIN_INVALID): return strcpy(status_message,"MCAPI_ERR_DOMAIN_INVALID");
  case (MCAPI_ERR_NODE_INVALID): return strcpy(status_message,"MCAPI_ERR_NODE_INVALID");
  case (MCAPI_ERR_NODE_INITFAILED): return strcpy(status_message,"MCAPI_ERR_NODE_INITFAILED");
  case (MCAPI_ERR_NODE_INITIALIZED): return strcpy(status_message,"MCAPI_ERR_NODE_INITIALIZED");
  case (MCAPI_ERR_NODE_NOTINIT): return strcpy(status_message,"MCAPI_ERR_NODE_NOTINIT");
  case (MCAPI_ERR_NODE_FINALFAILED): return strcpy(status_message,"MCAPI_ERR_NODE_FINALFAILED");
  case (MCAPI_ERR_PORT_INVALID): return strcpy(status_message,"MCAPI_ERR_INVALID");
  case (MCAPI_ERR_ENDP_INVALID): return strcpy(status_message,"MCAPI_ERR_ENDP_INVALID");
  case (MCAPI_ERR_ENDP_NOPORTS): return strcpy(status_message,"MCAPI_ERR_ENDP_NOPORTS");
  case (MCAPI_ERR_ENDP_LIMIT): return strcpy(status_message,"MCAPI_ERR_ENDP_LIMIT");
  case (MCAPI_ERR_ENDP_EXISTS): return strcpy(status_message,"MCAPI_ERR_ENDP_EXISTS");
  case (MCAPI_ERR_ENDP_NOTOWNER): return strcpy(status_message,"MCAPI_ERR_ENDP_NOTOWNER");
  case (MCAPI_ERR_ENDP_REMOTE): return strcpy(status_message,"MCAPI_ERR_ENDP_REMOTE");
  case (MCAPI_ERR_ATTR_INCOMPATIBLE): return strcpy(status_message,"MCAPI_ERR_ATTR_INCOMPATIBLE");
  case (MCAPI_ERR_ATTR_SIZE): return strcpy(status_message,"MCAPI_ERR_ATTR_SIZE");
  case (MCAPI_ERR_ATTR_NUM): return strcpy(status_message,"MCAPI_ERR_ATTR_NUM");
  case (MCAPI_ERR_ATTR_VALUE): return strcpy(status_message,"MCAPI_ERR_ATTR_VALUE");
  case (MCAPI_ERR_ATTR_NOTSUPPORTED): return strcpy(status_message,"MCAPI_ERR_ATTR_NOTSUPPORTED");
  case (MCAPI_ERR_ATTR_READONLY): return strcpy(status_message,"MCAPI_ERR_ATTR_READONLY");
  case (MCAPI_ERR_MSG_LIMIT): return strcpy(status_message,"MCAPI_ERR_MSG_LIMIT");
  case (MCAPI_ERR_MSG_TRUNCATED): return strcpy(status_message,"MCAPI_ERR_MSG_TRUNCATED");
  case (MCAPI_ERR_TRANSMISSION): return strcpy(status_message,"MCAPI_ERR_TRANSMISSION");
  case (MCAPI_ERR_MEM_LIMIT): return strcpy(status_message,"MCAPI_ERR_MEM_LIMIT");
  case (MCAPI_TIMEOUT): return strcpy(status_message,"MCAPI_TIMEOUT");
  case (MCAPI_ERR_REQUEST_LIMIT): return strcpy(status_message,"MCAPI_ERR_REQUEST_LIMIT");
  case (MCAPI_ERR_PRIORITY): return strcpy(status_message,"MCAPI_ERR_PRIORITY");
  case (MCAPI_ERR_CHAN_OPEN): return strcpy(status_message,"MCAPI_ERR_CHAN_OPEN");
  case (MCAPI_ERR_CHAN_TYPE): return strcpy(status_message,"MCAPI_ERR_CHAN_TYPE");
  case (MCAPI_ERR_CHAN_CONNECTED): return strcpy(status_message,"MCAPI_ERR_CHAN_CONNECTED");
  case (MCAPI_ERR_CHAN_OPENPENDING): return strcpy(status_message,"MCAPI_ERR_CHAN_OPENPENDING");
  case (MCAPI_ERR_CHAN_DIRECTION): return strcpy(status_message,"MCAPI_ERR_CHAN_DIRECTION");
  case (MCAPI_ERR_CHAN_NOTOPEN): return strcpy(status_message,"MCAPI_ERR_CHAN_NOTOPEN");
  case (MCAPI_ERR_CHAN_INVALID): return strcpy(status_message,"MCAPI_ERR_CHAN_INVALID");
  case (MCAPI_ERR_REQUEST_INVALID): return strcpy(status_message,"MCAPI_ERR_REQUEST_INVALID");
  case (MCAPI_ERR_PKT_LIMIT): return strcpy(status_message,"MCAPI_ERR_PKT_LIMIT");
  case (MCAPI_ERR_BUF_INVALID): return strcpy(status_message,"MCAPI_ERR_BUF_INVALID");
  case (MCAPI_ERR_SCL_SIZE): return strcpy(status_message,"MCAPI_ERR_SCL_SIZE");
  case (MCAPI_ERR_REQUEST_CANCELLED): return strcpy(status_message,"MCAPI_ERR_REQUEST_CANCELLED");
  case (MCAPI_ERR_GENERAL): return strcpy(status_message,"MCAPI_ERR_GENERAL");
  case (MCAPI_STATUSCODE_END): return strcpy(status_message,"MCAPI_STATUSCODE_END");
  default : return strcpy(status_message,"UNKNOWN");
  };
}

/*void mcapi_set_debug_level (int d) {
  mca_set_debug_level (d);
}*/

void mcapi_display_state (void* handle) {
  mcapi_trans_display_state(handle);
}


/************************************************************************
mcapi_initialize - Initializes the MCAPI implementation.

DESCRIPTION
mcapi_initialize() initializes the MCAPI environment on a given 
MCAPI node in a given MCAPI domain. It has to be called by each 
node using MCAPI.  init_parameters is used to pass implementation 
specific initialization parameters. mcapi_info is used to obtain 
information from the MCAPI implementation, including MCAPI and 
implementation version numbers, see mcapi.h for specific information. 
A node is a process, a thread, or a processor (or core) with 
an independent program counter running a piece of code. In other 
words, an MCAPI node is an independent thread of control. An 
MCAPI node can call mcapi_initialize() once per node, and it 
is an error to call mcapi_initialize() multiple times from a 
given node, unless mcapi_finalize() is called inbetween. A given 
MCAPI implementation will specify what is a node (i.e., what 
thread of control - process, thread, or other -- is a node) in 
that implementation. A thread and process are just two examples 
of threads of control, and there could be other. 

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below.


ERRORS

MCAPI_ENO_INIT		The MCAPI environment could not be initialized. 

MCAPI_ERR_NODE_INITIALIZED	The MCAPI environment has already been initialized.

MCAPI_ERR_NODE_INVALID	The parameter is not a valid node.

MCAPI_ERR_DOMAIN_INVALID	The parameter is not a valid domain.

***********************************************************************/
void mcapi_initialize(
                      MCAPI_IN mcapi_domain_t domain_id,
                      MCAPI_IN mcapi_node_t node_id,
		      MCAPI_IN mcapi_node_attributes_t* mcapi_node_attributes,
                      MCAPI_IN mcapi_param_t* init_parameters,
                      MCAPI_OUT mcapi_info_t* mcapi_info,
                      MCAPI_OUT mcapi_status_t* mcapi_status) 
{  
  *mcapi_status = MCAPI_ERR_NODE_INITFAILED;
  
  if (!mcapi_trans_valid_node(node_id)) {
    *mcapi_status = MCAPI_ERR_NODE_INVALID;
  } else if (mcapi_trans_initialized(domain_id,node_id)) {
    *mcapi_status = MCAPI_ERR_NODE_INITIALIZED;
  } else if (mcapi_trans_initialize(domain_id,node_id,mcapi_node_attributes)) {
    
    *mcapi_status = MCAPI_SUCCESS;
  }
}

  /*
    MCAPI_NODE_INIT_ATTRIBUTES
    NAME
    mcapi_node_init_attributes
    DESCRIPTION
    This function initializes the values of an mcapi_node_attributes_t structure. For non-default behavior this function should be called prior to calling mcapi_node_set_attribute(). mcapi_node_set_attribute() is then used to change any default values prior to calling mcapi_initialize().
    MCAPI-defined node attributes:
    MCAPI_NODE_ATTR_TYPE_REGULAR The node is regular. Default.
    RETURN VALUE
    On success *mcapi_status is set to MCAPI_SUCCESS. On error, *mcapi_status is set to the appropriate error defined below.
    ERRORS
    MCAPI_ERR_PARAMETER
    Invalid attributes parameter.
    MCAPI_ERR_GENERAL
    Implementation specific error not covered by other status codes. Specifics must be documented.
  */
  
  void mcapi_node_init_attributes(
                                  MCAPI_OUT mcapi_node_attributes_t* mcapi_node_attributes,
                                  MCAPI_OUT mcapi_status_t* mcapi_status
                                  ) {
    *mcapi_status = MCAPI_ERR_GENERAL;
    if (mcapi_node_attributes == NULL) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    } else if (mcapi_trans_node_init_attributes(mcapi_node_attributes,mcapi_status)) {
      *mcapi_status = MCAPI_SUCCESS;
    }
  }
  
  /*
    NAME
    mcapi_node_set_attribute
    
    DESCRIPTION
    This function is used to change default values of an mcapi_node_attributes_t data structure prior to calling mcapi_initialize(). This is a blocking function. Calls to this function have no effect on node attributes once the MCAPI has been initialized. The purpose of this function is to define node specific characteristics, and in this MCAPI version has only attribute. The node attributes are expected to be expanded in future versions.
    MCAPI-defined node attributes:
    MCAPI_NODE_ATTR_TYPE_REGULAR The node is regular. Default.
    RETURN VALUE
    On success *mcapi_status is set to MCAPI_SUCCESS. On error, *mcapi_status is set to the appropriate error defined below.
    ERRORS
    MCAPI_ERR_ATTR_NUM
    Unknown attribute number.
    MCAPI_ERR_ATTR_VALUE
    Incorrect attribute value.
    MCAPI_ERR_ATTR_SIZE
    Incorrect attribute size.
    MCAPI_ERR_ATTR_NOTSUPPORTED
    Attribute not supported by the implementation.
    MCAPI_ERR_ATTR_READONLY
    Attribute cannot be modified.
    MCAPI_ERR_PARAMETER
    Incorrect mcapi_node_attributes or attribute parameter.
    MCAPI_ERR_GENERAL
    Implementation specific error not covered by other status codes. Specifics must be documented.
    IMPLEMENTATION SPECIFIC DETAILS
    [This section reserved for inclusion of implementation specific details]
    MCAPI API Specification V2.015
    The Multicore Association March 25, 2011 Page 39 of 169
    3.2.7 MCAPI_NODE_GET_ATTRIBUTE
    NAME
  */
  
  void mcapi_node_set_attribute(
                                MCAPI_OUT mcapi_node_attributes_t* mcapi_node_attributes,
                                MCAPI_IN mcapi_uint_t attribute_num,
                                MCAPI_IN void* attribute,
                                MCAPI_IN size_t attribute_size,
                                MCAPI_OUT mcapi_status_t* mcapi_status
                                ){
    *mcapi_status = MCAPI_ERR_GENERAL;
    if ((mcapi_node_attributes == NULL) || (attribute == NULL)) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    } else if (mcapi_trans_node_set_attribute(mcapi_node_attributes,attribute_num,attribute,attribute_size,mcapi_status)) {
      *mcapi_status = MCAPI_SUCCESS;
    }
  }

 
  /*
    mcapi_node_get_attribute 
    \u2013 Get node attributes from a remote node.
    
    node attributenum indicates which one of the node attributes is being referenced. This is a blocking function. attribute points to a structure or scalar to be filled with the value of the attribute specified by attribute_num. attribute_size is the size in bytes of the attribute. See Section 2.2 and header files (Section 7) for a description of attributes. The mcapi_node_get_attribute() function returns the requested attribute value by reference.
    MCAPI-defined node attributes:
    MCAPI_NODE_ATTR_TYPE_REGULAR The node is regular. Default.
    RETURN VALUE
    On success, *attribute is filled with the requested attribute and *mcapi_status is set to MCAPI_SUCCESS. On error, *mcapi_status is set to an error code and *attribute is not modified.
    ERRORS
    MCAPI_ERR_NODE_NOTINIT
    The local node is not initialized.
    MCAPI_ERR_DOMAIN_INVALID
    The parameter is not a valid domain.
    MCAPI_ERR_NODE_INVALID
    The parameter is not a valid node.
    MCAPI_ERR_ATTR_NUM
    Unknown attribute number.
    MCAPI_ERR_ATTR_SIZE
    Incorrect attribute size.
    MCAPI_ERR_ATTR_NOTSUPPORTED
    Attribute not supported by the implementation.
    MCAPI_ERR_PARAMETER
    Incorrect attribute parameter.
    MCAPI_ERR_GENERAL
    Implementation specific error not covered by other status codes. Specifics must be documented.
  */
  
  void mcapi_node_get_attribute(
                                MCAPI_IN mcapi_domain_t domain_id,
                                MCAPI_IN mcapi_node_t node_id,
                                MCAPI_IN mcapi_uint_t attribute_num,
                                MCAPI_OUT void* attribute,
                                MCAPI_IN size_t attribute_size,
                                MCAPI_OUT mcapi_status_t* mcapi_status
                                ){
    *mcapi_status = MCAPI_ERR_GENERAL;
    if (attribute == NULL) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    } else if (mcapi_trans_node_get_attribute(domain_id,node_id,attribute_num,attribute,attribute_size,mcapi_status)) {
      *mcapi_status = MCAPI_SUCCESS;
    }
  }


/************************************************************************
mcapi_finalize - Finalizes the MCAPI implementation.

DESCRIPTION
mcapi_finalize() finalizes the MCAPI environment on the local 
MCAPI node. It has to be called by each node using MCAPI.  It 
is an error to call mcapi_finalize() without first calling mcapi_initialize(). 
 An MCAPI node can call mcapi_finalize() once for each call to 
mcapi_initialize(), but it is an error to call mcapi_finalize() 
multiple times from a given node unless mcapi_initialize() has 
been called prior to each mcapi_finalize() call.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below.


ERRORS

MCAPI_ERR_NODE_FINALFAILED		The MCAPI environment could not be finalized.
MCAPI_ERR_PARAMETER

***********************************************************************/

void mcapi_finalize(
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  *mcapi_status = MCAPI_SUCCESS;
  if (!mcapi_trans_finalize()) {
    *mcapi_status = MCAPI_ERR_NODE_FINALFAILED;
  }
}


/************************************************************************
mcapi_domain_id_get - return the domain number associated with the local node

DESCRIPTION
Returns the domain id associated with the local node.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below and 
the return value is set to MCAPI_DOMAIN_INVALID.

ERRORS

MCAPI_ERR_NODE_NOTINIT	The node is not initialized.
 
***********************************************************************/

mcapi_domain_t mcapi_domain_id_get(
        MCAPI_OUT mcapi_status_t* mcapi_status)
{
  mcapi_domain_t domain = MCAPI_DOMAIN_INVALID;
  *mcapi_status = MCAPI_ERR_NODE_NOTINIT;

  if (mcapi_trans_get_domain_num(&domain)) {
    *mcapi_status = MCAPI_SUCCESS;
  }
  
  return domain;
}

  

/************************************************************************
mcapi_node_id_get - return the node number associated with the local node

DESCRIPTION
Returns the node id associated with the local node.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below and 
the return value is set to MCAPI_NODE_INVALID.

ERRORS

MCAPI_ERR_NODE_NOTINIT	The node is not initialized.

MCAPI_ERR_PARAMETER

 
***********************************************************************/

mcapi_node_t mcapi_node_id_get(
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  mcapi_node_t node = MCAPI_NODE_INVALID;
  *mcapi_status = MCAPI_ERR_NODE_NOTINIT;

  if (mcapi_trans_get_node_num(&node)) { 
    *mcapi_status = MCAPI_SUCCESS;
  }
  
  return node;
}


/************************************************************************
mcapi_endpoint_create - create an endpoint.

DESCRIPTION mcapi_endpoint_create() is used to create endpoints, using
the domain_id and node_id of the local node calling the API function
and specific port_id, returning a reference to a globally unique
endpoint which can later be referenced by name using
mcapi_endpoint_get() (see Section 4.2.3). The port_id can be set to
MCAPI_PORT_ANY to request the next available endpoint on the local
node.

MCAPI supports a simple static naming scheme to create endpoints based
on global tuple names, <domain_id, node_id, port_id>. Other nodes can
access the created endpoint by calling mcapi_endpoint_get() and
specifying the appropriate domain, node and port id's. Enpoints can be
passed on to other endpoints and an endpoint created using
MCAPI_PORT_ANY has to be passed on to other endpoints by the creator,
to facilitate communication.

Static naming allows the programmer to define an MCAPI communication
topology at compile time. This facilitates simple initialization.
Section 7.1 illustrates an example of initialization and bootstrapping
using static naming. Creating endpoints using MCAPI_PORT_ANY provides
a convenient method to create endpoints without having to specify the
port_id.

RETURN VALUE

On success, an endpoint is returned and *mcapi_status is set to
MCAPI_SUCCESS. On error, MCAPI_NULL is returned and *mcapi_status is
set to the appropriate error defined below. MCAPI_NULL (or 0) could be
a valid enpoint value <0,0,0> so status has to be checked to ensure
correctness.

ERRORS

MCAPI_ERR_PORT_INVALID	The parameter is not a valid port.

MCAPI_ERR_ENDP_EXISTS	The endpoint is already created.

MCAPI_ERR_NODE_NOTINIT	The node is not initialized.

MCAPI_ERR_ENDP_LIMIT	Exceeded maximum number of endpoints allowed.

MCAPI_ERR_ENDP_NOPORTS	Endpoints cannot be created on this node.
MCAPI_ERR_PARAMETER
NOTE
The node number can only be set using the mcapi_intialize() function.

***********************************************************************/

mcapi_endpoint_t mcapi_endpoint_create(
 	MCAPI_IN mcapi_port_t port_id, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
    mcapi_endpoint_t e;


    mcapi_status_t status;
    *mcapi_status = MCAPI_SUCCESS;
    mcapi_domain_t domain_id = mcapi_domain_id_get(&status);
    if (status != MCAPI_SUCCESS) {
      *mcapi_status = MCAPI_ERR_NODE_NOTINIT;
    } else if (mcapi_trans_endpoint_exists (domain_id,port_id)) {
      *mcapi_status = MCAPI_ERR_ENDP_EXISTS;
    } else if (mcapi_trans_num_endpoints (domain_id) == MCAPI_MAX_ENDPOINTS) {
      *mcapi_status = MCAPI_ERR_ENDP_LIMIT;
    } else if (!mcapi_trans_valid_port(port_id)) {
      *mcapi_status = MCAPI_ERR_PORT_INVALID;  
    } else if (!mcapi_trans_endpoint_create(&e,port_id,MCAPI_FALSE))  {
      *mcapi_status = MCAPI_ERR_ENDP_NOPORTS;
    }
  
  return e;
}



/************************************************************************
mcapi_endpoint_get_i - obtain the endpoint associated with a given tuple.

DESCRIPTION

mcapi_endpoint_get_i() allows other nodes ("third parties") to 
get the endpoint identifier for the endpoint associated with 
a global tuple name <domain_id, node_id, port_id>.  This function 
is non-blocking and will return immediately.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS SUCCESS if 
completed and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.
ERRORS

MCAPI_ERR_PORT_INVALID	The parameter is not a valid port.

MCAPI_ERR_NODE_INVALID	The parameter is not a valid node.

MCAPI_ERR_NODE_NOTINIT	The node is not initialized.


MCAPI_ERR_DOMAIN_INVALID	The parameter is not a valid domain.


MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_ENDP_NOPORTS	Endpoints cannot be created on this node.
MCAPI_ERR_PARAMETER		Incorrectrequest parameter.


NOTE

Use the mcapi_test(), mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.


***********************************************************************/

void mcapi_endpoint_get_i(
                          MCAPI_IN mcapi_domain_t domain_id,
                          MCAPI_IN mcapi_node_t node_id, 
                          MCAPI_IN mcapi_port_t port_id, 
                          MCAPI_OUT mcapi_endpoint_t* endpoint, 
                          MCAPI_OUT mcapi_request_t* request, 
                          MCAPI_OUT mcapi_status_t* mcapi_status)
{
 
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_node (node_id)){
    *mcapi_status = MCAPI_ERR_NODE_INVALID;
  } else if ( ! mcapi_trans_valid_port (port_id)) {
      *mcapi_status = MCAPI_ERR_PORT_INVALID;
  } 
  mcapi_trans_endpoint_get_i (endpoint,domain_id,node_id,port_id,request,mcapi_status); 
}


/************************************************************************
mcapi_endpoint_get - obtain the endpoint associated with a given tuple.

DESCRIPTION

mcapi_endpoint_get() allows other nodes ("third parties") to 
get the endpoint identifier for the endpoint associated with 
a global tuple name <domain_id, node_id, port_id>.  This function 
will block until the specified remote endpoint has been created 
via the mcapi_endpoint_create() call. 


RETURN VALUE

On success, an endpoint is returned and *mcapi_status is set 
to MCAPI_SUCCESS. On error, MCAPI_NULL is returned and *mcapi_status 
is set to the appropriate error defined below. MCAPI_NULL (or 
0) could be a valid enpoint value <0,0,0> so status has to be 
checked to ensure correctness.

ERRORS

MCAPI_ERR_PORT_INVALID	The parameter is not a valid port.

MCAPI_ERR_NODE_INVALID	The parameter is not a valid node.
MCAPI_ERR_DOMAIN_INVALID	The parameter is not a valid domain.

MCAPI_ERR_NODE_NOTINIT	The node is not initialized.

MCAPI_ERR_ENDP_NOPORTS	Endpoints cannot be created on this node.

MCAPI_ERR_PARAMETER



 

***********************************************************************/

mcapi_endpoint_t mcapi_endpoint_get(
    MCAPI_IN mcapi_domain_t domain_id,
 	MCAPI_IN mcapi_node_t node_id, 
 	MCAPI_IN mcapi_port_t port_id, 
	MCAPI_IN mcapi_timeout_t timeout,
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{ 
  mcapi_endpoint_t e;
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_node (node_id)){
      *mcapi_status = MCAPI_ERR_NODE_INVALID;
  } else if ( ! mcapi_trans_valid_port (port_id)) {
    *mcapi_status = MCAPI_ERR_PORT_INVALID;
  } else {
    mcapi_trans_endpoint_get (&e,domain_id,node_id,port_id);
  }
  
  return e;
}


/************************************************************************
mcapi_endpoint_delete - delete an endpoint.

DESCRIPTION

Deletes an MCAPI endpoint. Pending messages are discarded.  If 
an endpoint has been connected to a packet or scalar channel, 
the appropriate close method must be called before deleting the 
endpoint.  Delete is a blocking operation. Since the connection 
is closed before deleting the endpoint, the delete method does 
not require any cross-process synchronization and is guaranteed 
to return in a timely manner (operation will return without having 
to block on any IPC to any remote nodes). It is an error to attempt 
to delete an endpoint that has not been closed. Only the node 
that created an endpoint can delete it.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS.  On error, 
*mcapi_status is set to the appropriate error defined below.


ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.

MCAPI_ERR_CHAN_OPEN		A channel is open, deletion is not allowed.

MCAPI_ECHAN_CONNECTED	A channel is connected, deletion is not allowed.

MCAPI_EENDP_NOTOWNER		An endpoint can only be deleted by its creator.

MCAPI_ERR_PARAMETER
***********************************************************************/

void mcapi_endpoint_delete(
 	MCAPI_IN mcapi_endpoint_t endpoint, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS;
  if ( ! mcapi_trans_valid_endpoint(endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else if (!mcapi_trans_endpoint_isowner (endpoint)) {
      *mcapi_status = MCAPI_ERR_ENDP_NOTOWNER;
  } else if ( mcapi_trans_endpoint_channel_isopen (endpoint)) {
    *mcapi_status = MCAPI_ERR_CHAN_OPEN;
  } else {
      /* delete the endpoint */
    mcapi_trans_endpoint_delete (endpoint);
  }
}


/************************************************************************
mcapi_endpoint_get_attribute- get endpoint attributes.

DESCRIPTION
***********************************************************************/
void mcapi_endpoint_get_attribute(
        MCAPI_IN mcapi_endpoint_t endpoint,
        MCAPI_IN mcapi_uint_t attribute_num,
        MCAPI_OUT void* attribute,
        MCAPI_IN size_t attribute_size,
        MCAPI_OUT mcapi_status_t* mcapi_status)
{
 *mcapi_status = MCAPI_SUCCESS;
  if ( ! mcapi_trans_valid_endpoint(endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else {
    mcapi_trans_endpoint_get_attribute(endpoint,attribute_num,attribute,attribute_size,mcapi_status);

  }
}


/************************************************************************
mcapi_endpoint_set_attribute - set endpoint attributes.

DESCRIPTION

***********************************************************************/


/************************************************************************
mcapi_msg_send_i - sends a (connectionless) message from a send endpoint to a receive endpoint.

DESCRIPTION

Sends a (connectionless) message from a send endpoint to a receive 
endpoint. It is a non-blocking function, and returns immediately. 
send_endpoint, is a local endpoint identifying the send endpoint, 
receive_endpoint identifies a receive endpoint. buffer is the 
application provided buffer, buffer_size is the buffer size in 
bytes, priority determines the message priority and request is 
the identifier used to determine if the send operation has completed 
on the sending endpoint and the buffer can be reused by the application. 
Furthermore, this method will abandon the send and return MCAPI_ERR_MEM_LIMIT 
if the system cannot either wait for sufficient memory to become 
available or allocate enough memory at the send endpoint to queue 
up the outgoing message.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not an endpoint descriptor.


MCAPI_ERR_MSG_LIMIT	The message size exceeds the maximum size allowed by the MCAPI implementation.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_MEM_LIMIT		No memory available.

MCAPI_ERR_PRIORITY		Incorrect priority level.

 

MCAPI_ERR_PARAMETER		Incorrect request or buffer (applies if buffer = NULL and  buffer_size  > 0) parameter.


NOTE

Use the mcapi_test(), mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.

 
***********************************************************************/

void mcapi_msg_send_i(
 	MCAPI_IN mcapi_endpoint_t send_endpoint, 
 	MCAPI_IN mcapi_endpoint_t receive_endpoint, 
 	MCAPI_IN void* buffer, 
 	MCAPI_IN size_t buffer_size, 
 	MCAPI_IN mcapi_priority_t priority, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* MCAPI_ERR_MEM_LIMIT, MCAPI_ENO_REQUEST, and MCAPI_ERR_MEM_LIMIT handled at the transport layer */
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_priority (priority)){
    *mcapi_status = MCAPI_ERR_PRIORITY;
  } else if (!mcapi_trans_valid_endpoints(send_endpoint,receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID; /* FIXME (errata A1) */
  } else if (buffer_size > MCAPI_MAX_MSG_SIZE) {
    *mcapi_status = MCAPI_ERR_MSG_LIMIT;
  }
  mcapi_trans_msg_send_i (send_endpoint,receive_endpoint,buffer,buffer_size,request,mcapi_status);
}



/************************************************************************
mcapi_msg_send - sends a (connectionless) message from a send endpoint to a receive endpoint.

DESCRIPTION

Sends a (connectionless) message from a send endpoint to a receive 
endpoint. It is a blocking function, and returns once the buffer 
can be reused by the application. send_endpoint is a local endpoint 
identifying the send endpoint, receive_endpoint identifies a 
receive endpoint. buffer is the application provided buffer and 
buffer_size is the buffer size in bytes, and priority determines 
the message priority

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below. 
Success means that the entire buffer has been sent. 

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not an endpoint descriptor.



MCAPI_ERR_MSG_LIMIT		The message size exceeds the maximum size allowed by the MCAPI implementation.

MCAPI_ERR_MEM_LIMIT		No memory available.


MCAPI_ERR_PRIORITY		Incorrect priority level.

MCAPI_ERR_TRANSMISSION	Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.

MCAPI_ERR_PARAMETER		Incorrect buffer (applies if buffer = NULL and  buffer_size  > 0) parameter.

MCAPI_TIMEOUT		The operation timed out. Implementations can optionally 
support timeout for this function. The timeout value is set with 
endpoint attributes.
 
***********************************************************************/

void mcapi_msg_send(
 	MCAPI_IN mcapi_endpoint_t  send_endpoint, 
 	MCAPI_IN mcapi_endpoint_t  receive_endpoint, 
 	MCAPI_IN void* buffer, 
 	MCAPI_IN size_t buffer_size, 
 	MCAPI_IN mcapi_priority_t priority, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{  

  /* FIXME: (errata B1) is it an error to send a message to a connected endpoint? */

  /* MCAPI_ERR_MEM_LIMIT handled at the transport layer */  
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_priority (priority)) {
    *mcapi_status = MCAPI_ERR_PRIORITY;
    } else if (!mcapi_trans_valid_endpoints(send_endpoint,receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID; /* FIXME (errata A1) */
  } else if (buffer_size > MCAPI_MAX_MSG_SIZE) {
    *mcapi_status = MCAPI_ERR_MSG_LIMIT;
  } else if ( !mcapi_trans_msg_send (send_endpoint,receive_endpoint,buffer,buffer_size)) {
    /* assume couldn't get a buffer */
    *mcapi_status = MCAPI_ERR_MEM_LIMIT;
  } 
}



/************************************************************************
mcapi_msg_recv_i - receives a (connectionless) message from a receive endpoint.

DESCRIPTION

Receives a (connectionless) message from a receive endpoint. 
It is a non-blocking function, and returns immediately. receive_endpoint 
is a local endpoint identifying the receive endpoint. buffer 
is the application provided buffer, and buffer_size is the buffer 
size in bytes. request is the identifier used to determine if 
the receive operation has completed (all the data is in the buffer). 



RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.



MCAPI_ERR_MSG_TRUNCATED		The message size exceeds the buffer_size.

MCAPI_ERR_TRANSMISSION	Transmission error, could be caused by 
for example out of order message segments (if segmentation is 
used).

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_MEM_LIMIT		No memory available.

MCAPI_ERR_PARAMETER			Incorrect buffer or  request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.

 
***********************************************************************/

void mcapi_msg_recv_i(
 	MCAPI_IN mcapi_endpoint_t  receive_endpoint,  
 	MCAPI_OUT void* buffer, 
 	MCAPI_IN size_t buffer_size, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* MCAPI_ENO_REQUEST handled at the transport layer */  
  *mcapi_status = MCAPI_SUCCESS;
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_buffer_param(buffer)) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    } else if (!mcapi_trans_valid_endpoint(receive_endpoint)) {
      *mcapi_status = MCAPI_ERR_ENDP_INVALID;
    }
    mcapi_trans_msg_recv_i(receive_endpoint,buffer,buffer_size,request,mcapi_status);
  }
}

/************************************************************************
mcapi_msg_recv - receives a (connectionless) message from a receive endpoint.

DESCRIPTION

Receives a (connectionless) message from a receive endpoint. 
It is a blocking function, and returns once a message is available 
and the received data filled into the buffer. receive_endpoint 
is a local endpoint identifying the receive endpoint. buffer 
is the application provided buffer, and buffer_size is the buffer 
size in bytes.  The received_size parameter is filled with the 
actual size of the received message.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below.


ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.


MCAPI_ERR_MSG_TRUNCATED		The message size exceeds the buffer_size.

MCAPI_ERR_TRANSMISSION	Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.

MCAPI_ERR_MEM_LIMIT		No memory available.


MCAPI_ERR_PARAMETER		Incorrect buffer parameter.

MCAPI_TIMEOUT		The operation timed out. Implementations can optionally 
support timeout for this function. The timeout value is set with 
endpoint attributes.





 
***********************************************************************/

void mcapi_msg_recv(
 	MCAPI_IN mcapi_endpoint_t  receive_endpoint,  
 	MCAPI_OUT void* buffer, 
 	MCAPI_IN size_t buffer_size, 
 	MCAPI_OUT size_t* received_size, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* FIXME: (errata B1) is it an error to try to receive a message on a connected endpoint?  */
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_buffer_param(buffer)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else if (!mcapi_trans_valid_endpoint(receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else {
    mcapi_trans_msg_recv(receive_endpoint,buffer,buffer_size,received_size);
    if (*received_size > buffer_size) {
      *received_size = buffer_size;
      *mcapi_status = MCAPI_ERR_MSG_TRUNCATED;
    }  
  }
}



/************************************************************************
mcapi_msg_available - checks if messages are available on a receive endpoint.

DESCRIPTION

Checks if messages are available on a receive endpoint.  The 
function returns in a timely fashion.  The number of "available" 
incoming messages is defined as the number of mcapi_msg_recv() 
operations that are guaranteed to not block waiting for incoming 
data. receive_endpoint is a local identifier for the receive 
endpoint. The call only checks the availability of messages and 
does not de-queue them. mcapi_msg_available() can only be used 
 to check availability on endpoints on the node local to the 
caller. 

RETURN VALUE

On success, the number of available messages is returned and 
*mcapi_status is set to MCAPI_SUCCESS. On error, MCAPI_NULL is 
returned and *mcapi_status is set to the appropriate error defined 
below. MCAPI_NULL (or 0) could be a valid number of available 
messages, so status has to be checked to ensure correctness.


ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.
MCAPI_ERR_PARAMETER
NOTE

The status code must be checked to distinguish between no messages and an error condition.

 
***********************************************************************/

mcapi_uint_t mcapi_msg_available(
 	MCAPI_IN mcapi_endpoint_t receive_endpoint, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  mcapi_uint_t rc = 0;
  *mcapi_status = MCAPI_SUCCESS;
  if( !mcapi_trans_valid_endpoint(receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else {
    rc = mcapi_trans_msg_available(receive_endpoint);
  }
  return rc;
}


/************************************************************************
mcapi_pktchan_connect_i - connects send & receive side endpoints.

DESCRIPTION

Connects a pair of endpoints into a unidirectional FIFO channel. 
 The connect operation can be performed by the sender, the receiver, 
or by a third party. The connect can happen once at the start 
of the program, or dynamically at run time. 

Connect is a non-blocking function. Synchronization to ensure 
the channel has been created is provided by the open call discussed 
later. 

Attempts to make multiple connections to a single endpoint will 
be detected as errors.  The type of channel connected to an endpoint 
must match the type of open call invoked by that endpoint; the 
open function will return an error if the opened channel type 
does not match the connected channel type, or direction .

It is an error to attempt a connection between endpoints whose 
attributes are set in an incompatible way (whether attributes 
are compatible or not is implementation defined).  It is also 
an error to attempt to change the attributes of endpoints that 
are connected.

RETURN VALUE

On success *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.

MCAPI_ERR_CHAN_CONNECTED			A channel connection has already been 
established for one or both of the specified endpoints.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_ATTR_INCOMPATIBLE	Connection of endpoints with incompatible attributes not allowed.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.

 
***********************************************************************/

void mcapi_pktchan_connect_i(
 	MCAPI_IN mcapi_endpoint_t  send_endpoint, 
 	MCAPI_IN mcapi_endpoint_t  receive_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* MCAPI_ENO_REQUEST handled at the transport layer */
  
  *mcapi_status = MCAPI_SUCCESS;
  if ( ! mcapi_trans_valid_endpoints(send_endpoint,receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else if (( mcapi_trans_channel_connected (send_endpoint)) ||  
             ( mcapi_trans_channel_connected (receive_endpoint))) {
    *mcapi_status = MCAPI_ERR_CHAN_CONNECTED;
  } else if (! mcapi_trans_compatible_endpoint_attributes (send_endpoint,receive_endpoint)) {
    *mcapi_status = MCAPI_ERR_ATTR_INCOMPATIBLE;
  } 
    mcapi_trans_pktchan_connect_i (send_endpoint,receive_endpoint,request,mcapi_status);
}
  
  


/************************************************************************
mcapi_pktchan_recv_open_i - Creates a typed and directional, 
local representation of the channel. It also provides synchronization 
for channel creation between two endpoints. Opens are required 
on both receive and send endpoints.

DESCRIPTION

Opens the receive end of a packet channel. The corresponding 
calls are required on both sides for synchronization to ensure 
that the channel has been created. It is a non-blocking function, 
and the receive_handle is filled in upon successful completion. 
No specific ordering of calls between sender and receiver is 
required since the call is non-blocking.  receive_endpoint is 
the endpoint associated with the channel.  The open call returns 
a typed, local handle for the connected channel that is used 
for channel receive operations.

RETURN VALUE

On success, a valid request is returned by and *mcapi_status 
is set to MCAPI_SUCCESS if completed and MCAPI_PENDING if not 
yet completed. On error *mcapi_status is set to the appropriate 
error defined below. 
 
ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.

MCAPI_ERR_ENDP_REMOTE	Channels can only be opened on the local endpoint.

MCAPI_ERR_CHAN_TYPE			Attempt to open a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION				Attempt to open a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.


MCAPI_ERR_CHAN_OPENPENDING	An open request is pending.

MCAPI_ERR_CHAN_OPEN			The channel is already open.

MCAPI_ERR_PARAMETER		Incorrect handle or request parameter.



NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.
 
***********************************************************************/

void mcapi_pktchan_recv_open_i(
 	MCAPI_OUT mcapi_pktchan_recv_hndl_t* recv_handle, 
 	MCAPI_IN mcapi_endpoint_t receive_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status) 
{
  
  *mcapi_status = MCAPI_SUCCESS;   
  if (! request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
  if (! mcapi_trans_valid_endpoint(receive_endpoint) ) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else if ( mcapi_trans_channel_type (receive_endpoint) == MCAPI_SCL_CHAN) {
    *mcapi_status = MCAPI_ERR_CHAN_TYPE;
  } else if (! mcapi_trans_recv_endpoint (receive_endpoint)) {
      *mcapi_status = MCAPI_ERR_CHAN_DIRECTION;
  }
  mcapi_trans_pktchan_recv_open_i(recv_handle,receive_endpoint,request,mcapi_status);
 }
}


/************************************************************************
mcapi_pktchan_send_open_i - Creates a typed and directional, 
local representation of the channel. It also provides synchronization 
for channel creation between two endpoints. Opens are required 
on both receive and send endpoints.

DESCRIPTION

Opens the send end of a packet channel. The corresponding calls 
are required on both sides for synchronization to ensure that 
the channel has been created. It is a non-blocking function, 
and the send_handle is filled in upon successful completion. 
No specific ordering of calls between sender and receiver is 
required since the call is non-blocking.  send_endpoint is the 
endpoint associated with the channel.  The open call returns 
a typed, local handle for the connected endpoint that is used 
by channel send operations.

RETURN VALUE

On success, a valid request is returned and *mcapi_status is 
set to MCAPI_SUCCESS if completed and MCAPI_PENDING if not yet 
completed. On error, *mcapi_status is set to the appropriate 
error defined below.
 

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.

MCAPI_ERR_ENDP_REMOTE	Channels can only be opened on the local endpoint.

MCAPI_ERR_CHAN_TYPE			Attempt to open a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION				Attempt to open a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_CHAN_OPENPENDING	An open request is pending.

 MCAPI_ERR_CHAN_OPEN			The channel is already open.

MCAPI_ERR_PARAMETER		Incorrect handle or request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.
 
***********************************************************************/

void mcapi_pktchan_send_open_i(
 	MCAPI_OUT mcapi_pktchan_send_hndl_t* send_handle, 
 	MCAPI_IN mcapi_endpoint_t  send_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
   if (! mcapi_trans_valid_endpoint(send_endpoint) ) {
    *mcapi_status = MCAPI_ERR_ENDP_INVALID;
  } else if ( mcapi_trans_channel_type (send_endpoint) == MCAPI_SCL_CHAN){
    *mcapi_status = MCAPI_ERR_CHAN_TYPE;
  } else if (! mcapi_trans_send_endpoint (send_endpoint)) {
    *mcapi_status = MCAPI_ERR_CHAN_DIRECTION;
  }
  mcapi_trans_pktchan_send_open_i(send_handle,send_endpoint,request,mcapi_status);
 }
}


/************************************************************************
mcapi_pktchan_send_i - sends a (connected) packet on a channel.


DESCRIPTION

Sends a packet on a connected channel. It is a non-blocking function, 
and returns immediately. buffer is the application provided buffer 
and size is the buffer size. request is the identifier used to 
determine if the send operation has completed on the sending 
endpoint and the buffer can be reused. While this method returns 
immediately, data transfer will not complete until there is sufficient 
free space in the channels receive buffer. A subsequent call 
to mcapi_wait() will block until space becomes available at the 
receiver, the send operation has completed, and the send buffer 
is available for reuse. Furthermore, this method will abandon 
the send and return MCAPI_ERR_MEM_LIMIT if the system cannot 
either wait for sufficient memory to become available or allocate 
enough memory at the send endpoint to queue up the outgoing packet.


RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_PKT_LIMIT	The packet size exceeds the maximum size allowed by the MCAPI implementation.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_MEM_LIMIT		No memory available.

MCAPI_ERR_TRANSMISSION	Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.

MCAPI_ERR_PARAMETER		Incorrect request or buffer (applies if buffer = 0 and  buffer_size  > 0) parameter.


 
NOTE
Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.

 
***********************************************************************/

void mcapi_pktchan_send_i(
 	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle, 
 	MCAPI_IN void* buffer, 
 	MCAPI_IN size_t size, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* MCAPI_ERR_MEM_LIMIT, MCAPI_ENO_REQUEST and MCAPI_ERR_MEM_LIMIT handled at the transport layer */
  *mcapi_status = MCAPI_SUCCESS; 
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_pktchan_send_handle(send_handle) ) {
      *mcapi_status = MCAPI_ERR_CHAN_INVALID;
    } else if ( size > MCAPI_MAX_PKT_SIZE) {
      *mcapi_status = MCAPI_ERR_PKT_LIMIT; 
    }
    mcapi_trans_pktchan_send_i(send_handle,buffer,size,request,mcapi_status);
  }
}


/************************************************************************
mcapi_pktchan_send - sends a (connected) packet on a channel.


DESCRIPTION

Sends a packet on a connected channel. It is a blocking function, 
and returns once the buffer can be reused. send_handle is the 
efficient local send handle which represents the send endpoint 
associated with the channel. buffer is the application provided 
buffer and size is the buffer size. Since channels behave like 
FIFOs, this method will block if there is no free space in the 
channel's receive buffer. When sufficient space becomes available 
(due to receive calls), the function will complete.


RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below. 
Success means that the entire buffer has been sent. 

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_PKT_LIMIT	The message size exceeds the maximum size allowed by the MCAPI implementation.

MCAPI_ERR_MEM_LIMIT		No memory available


MCAPI_ERR_TRANSMISSION	Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.

MCAPI_ERR_PARAMETER		Incorrect buffer (applies if buffer = 0 and  buffer_size  > 0) parameter.
MCAPI_TIMEOUT		The operation timed out. Implementations can optionally 
support timeout for this function. The timeout value is set with 
endpoint attributes.


 
***********************************************************************/

void mcapi_pktchan_send(
 	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle, 
 	MCAPI_IN void* buffer, 
 	MCAPI_IN size_t size, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_pktchan_send_handle(send_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else if ( size > MCAPI_MAX_PKT_SIZE) {
    *mcapi_status = MCAPI_ERR_PKT_LIMIT; 
  } else  {
    if (!mcapi_trans_pktchan_send (send_handle,buffer,size)) {
      *mcapi_status = MCAPI_ERR_MEM_LIMIT;
    }
  }
}



/************************************************************************
mcapi_pktchan_recv_i - receives a (connected) packet on a channel.


DESCRIPTION

Receives a packet on a connected channel. It is a non-blocking 
function, and returns immediately. receive_handle is the receive 
endpoint.  At some point in the future, when the receive operation 
completes, the buffer parameter is filled with the address of 
a system-supplied buffer containing the received packet.  After 
the receive request has completed and the application is finished 
with buffer, buffer should be returned to the system by calling 
mcapi_pktchan_release(). request is the identifier used to determine 
if the receive operation has completed and buffer is ready for 
use; the mcapi_test() , mcapi_wait() or mcapi_wait_any() function 
will return the actual size of the received packet.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_MEM_LIMIT		No memory available


MCAPI_ERR_REQUEST_LIMIT		No more request handles available.

MCAPI_ERR_TRANSMISSION		Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.

MCAPI_ERR_PARAMETER		Incorrect bufferor request parameter.

 
NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.
 
***********************************************************************/

void mcapi_pktchan_recv_i(
 	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle,  
 	MCAPI_OUT void** buffer, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{ 
  /* MCAPI_EPACKLIMIT, MCAPI_ERR_MEM_LIMIT, and MCAPI_ENO_REQUEST are handled at the transport layer */
  *mcapi_status = MCAPI_SUCCESS; 
  if (! request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_buffer_param(buffer)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else if (! mcapi_trans_valid_pktchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  }
  mcapi_trans_pktchan_recv_i (receive_handle,buffer,request,mcapi_status);
 }
}


/************************************************************************
mcapi_pktchan_recv - receives a data packet on a (connected) channel.


DESCRIPTION

Receives a packet on a connected channel. It is a blocking function, 
and returns when the data has been written to the buffer. receive_handle 
is the efficient local representation of the receive endpoint 
associated with the channel.  buffer is filled with a pointer 
to the system-supplied receive buffer and received_size is filled 
with the size of the packet in that buffer.  When the application 
finishes with buffer, it must return it to the system by calling 
mcapi_pktchan_release(). 

RETURN VALUE


On success, *mcapi_status is set to MCAPI_SUCCESS. On error, 
*mcapi_status is set to the appropriate error defined below.


ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.


MCAPI_ERR_MEM_LIMIT		No memory available.

MCAPI_ERR_TRANSMISSION	Transmission failure. This error code 
is optional, and if supported by an implementation, it's functionality 
shall be described.


MCAPI_ERR_PARAMETER		Incorrect buffer parameter.

MCAPI_TIMEOUT		The operation timed out. Implementations can optionally 
support timeout for this function. The timeout value is set with 
endpoint attributes.


 
***********************************************************************/

void mcapi_pktchan_recv(
 	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT void** buffer, 
 	MCAPI_OUT size_t* received_size, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS;   
  if (! mcapi_trans_valid_buffer_param(buffer)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else if (! mcapi_trans_valid_pktchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else  {
    if (mcapi_trans_pktchan_recv (receive_handle,buffer,received_size)) {
      if ( *received_size > MCAPI_MAX_PKT_SIZE) {
        *mcapi_status = MCAPI_ERR_PKT_LIMIT;
      } 
    } else {
      *mcapi_status = MCAPI_ERR_MEM_LIMIT;
    }
  }
}


/************************************************************************
mcapi_pktchan_available - checks if packets are available on a receive endpoint.


DESCRIPTION

Checks if packets are available on a receive endpoint.   This 
function returns in a timely fashion.  The number of available 
packets is defined as the number of receive operations that could 
be performed without blocking to wait for incoming data.  receive_handle 
is the efficient local handle for the packet channel. The call 
only checks the availability of packets and does not de-queue 
them.

RETURN VALUE

On success, the number of available packets are returned and 
*mcapi_status is set to MCAPI_SUCCESS. On error, MCAPI_NULL is 
returned and *mcapi_status is set to the appropriate error defined 
below. MCAPI_NULL (or 0) could be a valid number of available 
packets, so status has to be checked to ensure correctness.


ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
NOTE

The status code must be checked to distinguish between no messages and an error condition.



 
***********************************************************************/

mcapi_uint_t mcapi_pktchan_available(
 	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status) 
{
  int num = 0;
  
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_pktchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else {
    num = mcapi_trans_pktchan_available(receive_handle);
  }
return num;
}


/************************************************************************
mcapi_pktchan_release - releases a packet buffer obtained from a mcapi_pktchan_recv() call.


DESCRIPTION

When a user is finished with a packet buffer obtained from mcapi_pktchan_recv_i() 
or mcapi_pktchan_recv(), they should invoke this function to 
return the buffer to the system.  Buffers can be released in 
any order. This function is guaranteed to return in a timely 
fashion.

RETURN VALUE

On success *mcapi_status is set to MCAPI_SUCCESS. On error, *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_BUF_INVALID		Argument is not a valid buffer descriptor.
MCAPI_ERR_PARAMETER
***********************************************************************/
void mcapi_pktchan_release(
        /*MCAPI_IN*/ void* buffer,
        MCAPI_OUT mcapi_status_t* mcapi_status)
{

    *mcapi_status = MCAPI_SUCCESS;
    if (!mcapi_trans_pktchan_free (buffer)) {
      *mcapi_status = MCAPI_ERR_BUF_INVALID;
    }
}


/************************************************************************
mcapi_pktchan_release_test - tests if a packet buffer obtained 
from a mcapi_pktchan_recv() has been released (with mcapi_pktchan_release).



DESCRIPTION

Checks if a packet buffer has been released. This function returns in a timely fashion.

RETURN VALUE

On success, MCAPI_TRUE is returned and *mcapi_status is set to 
MCAPI_SUCCESS. If the operation has not completed MCAPI_FALSE 
is returned and *mcapi_status is set to MCAPI_PENDING. On error 
MCAPI_FALSE is returned and *mcapi_status is set to the appropriate 
error defined below.

ERRORS

MCAPI_ERR_BUF_INVALID		Argument is not a valid buffer descriptor.


***********************************************************************/


/************************************************************************
mcapi_pktchan_recv_close_i - closes channel on a receive endpoint.

DESCRIPTION

Closes the receive side of a channel. The sender makes the send-side 
call and the receiver makes the receive-side call. The corresponding 
calls are required on both sides to ensure that the channel has 
been properly closed. It is a non-blocking function, and returns 
immediately. receive_handle is the receive endpoint identifier. 
All pending packets are discarded, and any attempt to send more 
packets will give an error. A packet channel is disconnected 
when the last (second) close operation is performed.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_CHAN_TYPE		Attempt to close a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION			Attempt to close a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_CHAN_NOTOPEN		The channel is not open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.

 
***********************************************************************/

void mcapi_pktchan_recv_close_i(
 	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS;  
  if (! request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
   if (! mcapi_trans_valid_pktchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else if (! mcapi_trans_pktchan_recv_isopen (receive_handle)) {
    *mcapi_status = MCAPI_ERR_CHAN_NOTOPEN;
  }
  mcapi_trans_pktchan_recv_close_i (receive_handle,request,mcapi_status);
 }
}



/************************************************************************
mcapi_pktchan_send_close_i - closes channel on a send endpoint.

DESCRIPTION

Closes the send side of a channel. The sender makes the send-side 
call and the receiver makes the receive-side call. The corresponding 
calls are required on both sides to ensure that the channel has 
been properly closed. It is a non-blocking function, and returns 
immediately. send_handle is the send endpoint identifier. Pending 
packets at the receiver are not discarded. A packet channel is 
disconnected when the last (second) close operation is performed.


RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_CHAN_TYPE		Attempt to close a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION		Attempt to close a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_CHAN_NOTOPEN		The channel is not open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.

 
***********************************************************************/

void mcapi_pktchan_send_close_i(
 	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  *mcapi_status = MCAPI_SUCCESS;
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_pktchan_recv_handle(send_handle) ) {
      *mcapi_status = MCAPI_ERR_CHAN_INVALID;
    } else if (! mcapi_trans_pktchan_send_isopen (send_handle)) {
      *mcapi_status = MCAPI_ERR_CHAN_NOTOPEN;
    } 
    mcapi_trans_pktchan_send_close_i (send_handle,request,mcapi_status);
  }
}



/************************************************************************
mcapi_sclchan_connect_i - connects a pair of scalar channel endpoints.

DESCRIPTION

Connects a pair of endpoints.  The connect operation can be performed 
by the sender, the receiver, or by a third party. The connect 
can happen once at the start of the program or dynamically at 
run time. 

mcapi_sclchan_connect_i() is a non-blocking function. Synchronization 
to ensure the channel has been created is provided by the open 
call discussed later. 

Note that this function behaves like the packetchannel connect call.

Attempts to make multiple connections to a single endpoint will 
be detected as errors.  The type of channel connected to an endpoint 
must match the type of open call invoked by that endpoint; the 
open function will return an error if the opened channel type 
does not match the connected channel type, or direction.

It is an error to attempt a connection between endpoints whose 
attributes are set in an incompatible way (whether attributes 
are compatible or not is implementation defined).  It is also 
an error to attempt to change the attributes of endpoints that 
are connected.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS
MCAPI_ERR_ENDP_INVALID		Argument is not a valid endpoint descriptor.

MCAPI_ERR_CHAN_CONNECTED		A channel connection has already been established for one or both of the specified endpoints.

MCAPI_ERR_ATTR_INCOMPATIBLE	Connection of endpoints with incompatible attributes not allowed.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.
 

***********************************************************************/

void  mcapi_sclchan_connect_i(
 	MCAPI_IN mcapi_endpoint_t send_endpoint, 
 	MCAPI_IN mcapi_endpoint_t receive_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS;
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if ( ! mcapi_trans_valid_endpoints(send_endpoint,receive_endpoint)) {
      *mcapi_status = MCAPI_ERR_ENDP_INVALID;
    } else if (( mcapi_trans_channel_connected (send_endpoint)) ||  
               ( mcapi_trans_channel_connected (receive_endpoint))) {
      *mcapi_status = MCAPI_ERR_CHAN_CONNECTED;
    } else if (! mcapi_trans_compatible_endpoint_attributes (send_endpoint,receive_endpoint)) {
      *mcapi_status = MCAPI_ERR_ATTR_INCOMPATIBLE;
    } 
    mcapi_trans_sclchan_connect_i (send_endpoint,receive_endpoint,request,mcapi_status);
  }
}



/************************************************************************
mcapi_sclchan_recv_open_i - Creates a typed, local representation of a scalar channel. 


DESCRIPTION

Opens the receive end of a scalar channel. It also provides synchronization 
for channel creation between two endpoints. The corresponding 
calls are required on both sides to synchronize the endpoints. 
It is a non-blocking function, and the recv_handle is filled 
in upon successful completion.  No specific ordering of calls 
between sender and receiver is required since the call is non-blocking. 
 receive_endpoint is the local endpoint identifier. The call 
returns a local handle for the connected channel.

RETURN VALUE

On success, a channel handle is returned by reference and *mcapi_status 
is set to MCAPI_SUCCESS if completed and MCAPI_PENDING if not 
yet completed. On error, *mcapi_status is set to the appropriate 
error defined below.

ERRORS
MCAPI_ERR_ENDP_INVALID		Argument is not an endpoint descriptor.

MCAPI_ERR_ENDP_REMOTE	Channels can only be opened on the local endpoint.

MCAPI_ERR_CHAN_TYPE		Attempt to open a packet channel on an endpoint 
that has been connected with a different channel type.

MCAPI_ERR_CHAN_DIRECTION			Attempt to open a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_CHAN_OPENPENDING	An open request is pending.

MCAPI_ERR_CHAN_OPEN			The channel is already open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect handle or request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.
 

***********************************************************************/

void mcapi_sclchan_recv_open_i(
 	MCAPI_OUT mcapi_sclchan_recv_hndl_t* receive_handle, 
 	MCAPI_IN mcapi_endpoint_t receive_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status) 
{
  *mcapi_status = MCAPI_SUCCESS;  
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_endpoint(receive_endpoint) ) {
      *mcapi_status = MCAPI_ERR_ENDP_INVALID;
    } else if ( mcapi_trans_channel_type (receive_endpoint) == MCAPI_PKT_CHAN) {
      *mcapi_status = MCAPI_ERR_CHAN_TYPE;
    } else if (! mcapi_trans_recv_endpoint (receive_endpoint)) {
      *mcapi_status = MCAPI_ERR_CHAN_DIRECTION;
    }
    
    mcapi_trans_sclchan_recv_open_i(receive_handle,receive_endpoint,request,mcapi_status);
  }
}


/************************************************************************
mcapi_sclchan_send_open_i - Creates a typed, local representation of a scalar channel.


DESCRIPTION

Opens the send end of a scalar channel. . It also provides synchronization 
for channel creation between two endpoints.  The corresponding 
calls are required on both sides to synchronize the endpoints. 
It is a non-blocking function, and the send_handle is filled 
in upon successful completion.   No specific ordering of calls 
between sender and receiver is required since the call is non-blocking. 
 send_endpoint is the local endpoint identifier. The call returns 
a local handle for connected channel.

RETURN VALUE

On success, a channel handle is returned by reference and *mcapi_status 
is set to MCAPI_SUCCESS if completed and MCAPI_PENDING if not 
yet completed. On error, *mcapi_status is set to the appropriate 
error defined below.

ERRORS

MCAPI_ERR_ENDP_INVALID		Argument is not an endpoint descriptor.

MCAPI_ERR_ENDP_REMOTE	Channels can only be opened on the local endpoint.

MCAPI_ERR_CHAN_TYPE		Attempt to open a packet channel on an endpoint 
that has been connected with a different channel type.

MCAPI_ERR_CHAN_DIRECTION			Attempt to open a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_CHAN_OPENPENDING	An open request is pending.

MCAPI_ERR_CHAN_OPEN	The channel is already open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.
MCAPI_ERR_PARAMETER		Incorrect handle or request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status and mcapi_cancel() function to cancel the 
operation.
 

***********************************************************************/

void mcapi_sclchan_send_open_i(
 	MCAPI_OUT mcapi_sclchan_send_hndl_t* send_handle, 
 	MCAPI_IN mcapi_endpoint_t send_endpoint, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  
  *mcapi_status = MCAPI_SUCCESS;  
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_endpoint(send_endpoint) ) {
      *mcapi_status = MCAPI_ERR_ENDP_INVALID;
    } else if  (mcapi_trans_channel_type (send_endpoint) == MCAPI_PKT_CHAN){
      *mcapi_status = MCAPI_ERR_CHAN_TYPE;
    } else if (! mcapi_trans_send_endpoint (send_endpoint)) {
      *mcapi_status = MCAPI_ERR_CHAN_DIRECTION;
    }
    mcapi_trans_sclchan_send_open_i(send_handle,send_endpoint,request,mcapi_status); 
  }
}


/************************************************************************
mcapi_sclchan_send_uint64 - sends a (connected) 64-bit scalar on a channel.


DESCRIPTION

Sends a scalar on a connected channel. It is a blocking function, 
and returns immediately unless the buffer is full. send_handle 
is the send endpoint identifier. dataword is the scalar. Since 
channels behave like FIFOs, this method will block if there is 
no free space in the channel's receive buffer. When sufficient 
space becomes available (due to receive calls), the function 
will complete.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error *mcapi_status 
is set to the appropriate error defined below.  Optionally, implementations 
may choose to always set *mcapi_status to MCAPI_SUCCESS for performance 
reasons.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER

 

***********************************************************************/

void mcapi_sclchan_send_uint64(
 	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,  
 	MCAPI_IN mcapi_uint64_t dataword, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* FIXME: (errata B3) this function needs to check MCAPI_ERR_MEM_LIMIT */
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_sclchan_send_handle(send_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  }  else if (!mcapi_trans_sclchan_send (send_handle,dataword,8)) {
    *mcapi_status = MCAPI_ERR_MEM_LIMIT;  /* MR: added this  */
  } 
}





/************************************************************************
mcapi_sclchan_send_uint32 - sends a (connected) 32-bit scalar on a channel.

DESCRIPTION

Sends a scalar on a connected channel. It is a blocking function, 
and returns immediately unless the buffer is full. send_handle 
is the send endpoint identifier. dataword is the scalar. Since 
channels behave like FIFOs, this method will block if there is 
no free space in the channel's receive buffer. When sufficient 
space becomes available (due to receive calls), the function 
will complete.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error *mcapi_status 
is set to the appropriate error defined below.  Optionally, implementations 
may choose to always set *mcapi_status to MCAPI_SUCCESS for performance 
reasons.


ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
 

***********************************************************************/

void mcapi_sclchan_send_uint32(
 	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,  
 	MCAPI_IN mcapi_uint32_t dataword, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* FIXME: (errata B3) this function needs to check MCAPI_ERR_MEM_LIMIT */
  if (! mcapi_trans_valid_status_param(mcapi_status)) {
    if (mcapi_status != MCAPI_NULL) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    }
  } else {
    *mcapi_status = MCAPI_SUCCESS; 
    if (! mcapi_trans_valid_sclchan_send_handle(send_handle) ) {
      *mcapi_status = MCAPI_ERR_CHAN_INVALID;
    }  else if (!mcapi_trans_sclchan_send (send_handle,dataword,4)) {
      *mcapi_status = MCAPI_ERR_MEM_LIMIT;
    } 
  }
}
 

/************************************************************************
mcapi_sclchan_send_uint16 - sends a (connected) 16-bit scalar on a channel.

DESCRIPTION

Sends a scalar on a connected channel. It is a blocking function, 
and returns immediately unless the buffer is full. send_handle 
is the send endpoint identifier. dataword is the scalar. Since 
channels behave like FIFOs, this method will block if there is 
no free space in the channel's receive buffer. When sufficient 
space becomes available (due to receive calls), the function 
will complete.


RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error *mcapi_status 
is set to the appropriate error defined below.  Optionally, implementations 
may choose to always set *mcapi_status to MCAPI_SUCCESS for performance 
reasons.


ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
 

***********************************************************************/

void mcapi_sclchan_send_uint16(
 	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,  
 	MCAPI_IN mcapi_uint16_t dataword, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{ 
  /* FIXME: (errata B3) this function needs to check MCAPI_ERR_MEM_LIMIT */
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_sclchan_send_handle(send_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  }  else if (!mcapi_trans_sclchan_send (send_handle,dataword,2)) {
    *mcapi_status = MCAPI_ERR_MEM_LIMIT; 
  }
}



/************************************************************************
mcapi_sclchan_send_uint8 - sends a (connected) 8-bit scalar on a channel.

DESCRIPTION

Sends a scalar on a connected channel. It is a blocking function, 
and returns immediately unless the buffer is full. send_handle 
is the send endpoint identifier. dataword is the scalar. Since 
channels behave like FIFOs, this method will block if there is 
no free space in the channel's receive buffer. When sufficient 
space becomes available (due to receive calls), the function 
will complete.

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error *mcapi_status 
is set to the appropriate error defined below.  Optionally, implementations 
may choose to always set *mcapi_status to MCAPI_SUCCESS for performance 
reasons.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
 

***********************************************************************/

void mcapi_sclchan_send_uint8(
 	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,  
 	MCAPI_IN mcapi_uint8_t dataword, 
        MCAPI_OUT mcapi_status_t* mcapi_status)
{
  /* FIXME: (errata B3) this function needs to check MCAPI_ERR_MEM_LIMIT */
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_sclchan_send_handle(send_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  }  else if (!mcapi_trans_sclchan_send (send_handle,dataword,1)) {
    *mcapi_status = MCAPI_ERR_MEM_LIMIT;    
  }
}


/************************************************************************
mcapi_sclchan_recv_uint64 - receives a (connected) 64-bit scalar on a channel.

DESCRIPTION

Receives a scalar on a connected channel. It is a blocking function, 
and returns when a scalar is available. receive_handle is the 
receive endpoint identifier.

RETURN VALUE

On success, a value of type uint64_t is returned and *mcapi_status 
is set to MCAPI_SUCCESS. On error, the return value is undefined 
and *mcapi_status is set to the appropriate error defined below. 
 Optionally, implementations may choose to always set *mcapi_status 
to MCAPI_SUCCESS for performance reasons.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
MCAPI_ERR_SCL_SIZE		Incorrect scalar size.
MCAPI_ERR_PARAMETER
NOTE

The receive scalar size must match the send size.

 

***********************************************************************/

mcapi_uint64_t mcapi_sclchan_recv_uint64(
 	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  uint64_t dataword = 0;
  uint32_t exp_size = 8; 
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  }else if (! mcapi_trans_sclchan_recv (receive_handle,&dataword,exp_size)) {
    *mcapi_status = MCAPI_ERR_SCL_SIZE;
  }
  
  return dataword;
}


/************************************************************************
mcapi_sclchan_recv_uint32 - receives a 32-bit scalar on a (connected) channel.

DESCRIPTION

Receives a scalar on a connected channel. It is a blocking function, 
and returns when a scalar is available. receive_handle is the 
receive endpoint identifier.

RETURN VALUE

On success, a value of type uint32_t is returned and *mcapi_status 
is set to MCAPI_SUCCESS. On error, the return value is undefined 
and *mcapi_status is set to the appropriate error defined below. 
 Optionally, implementations may choose to always set *mcapi_status 
to MCAPI_SUCCESS for performance reasons.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
MCAPI_ERR_SCL_SIZE		Incorrect scalar size.
MCAPI_ERR_PARAMETER
NOTE

The receive scalar size must match the send size.

 
***********************************************************************/

mcapi_uint32_t mcapi_sclchan_recv_uint32(
 	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  uint64_t dataword = 0;
  uint32_t exp_size = 4; 
  
  *mcapi_status = MCAPI_SUCCESS;
  if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else if (! mcapi_trans_sclchan_recv (receive_handle,&dataword,exp_size)) {
    *mcapi_status = MCAPI_ERR_SCL_SIZE;
  } 
  return dataword;
}



/************************************************************************
mcapi_sclchan_recv_uint16 - receives a 16-bit scalar on a (connected) channel.

DESCRIPTION

Receives a scalar on a connected channel. It is a blocking function, 
and returns when a scalar is available. receive_handle is the 
receive endpoint identifier.

RETURN VALUE

On success, a value of type uint16_t is returned and *mcapi_status 
is set to MCAPI_SUCCESS. On error, the return value is undefined 
and *mcapi_status is set to the appropriate error defined below. 
 Optionally, implementations may choose to always set *mcapi_status 
to MCAPI_SUCCESS for performance reasons.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
MCAPI_ERR_SCL_SIZE		Incorrect scalar size.
MCAPI_ERR_PARAMETER
NOTE

The receive scalar size must match the send size.

 

***********************************************************************/

mcapi_uint16_t mcapi_sclchan_recv_uint16(
 	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{

  uint64_t dataword = 0;
  uint32_t exp_size = 2; 
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else if (! mcapi_trans_sclchan_recv (receive_handle,&dataword,exp_size)) {  
    *mcapi_status = MCAPI_ERR_SCL_SIZE;
  } 
  return dataword;
}



/************************************************************************
mcapi_sclchan_recv_uint8 - receives a (connected) 8-bit scalar on a channel.

DESCRIPTION

Receives a scalar on a connected channel. It is a blocking function, 
and returns when a scalar is available. receive_handle is the 
receive endpoint identifier.

RETURN VALUE

On success, a value of type uint8_t is returned and *mcapi_status 
is set to MCAPI_SUCCESS. On error, the return value is undefined 
and *mcapi_status is set to the appropriate error defined below. 
 Optionally, implementations may choose to always set *mcapi_status 
to MCAPI_SUCCESS for performance reasons.


ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
MCAPI_ERR_SCL_SIZE		Incorrect scalar size.
MCAPI_ERR_PARAMETER
NOTE

The receive scalar size must match the send size.

 
***********************************************************************/

mcapi_uint8_t mcapi_sclchan_recv_uint8(
 	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  uint64_t dataword = 0;
  uint32_t exp_size = 1; 
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else if (! mcapi_trans_sclchan_recv (receive_handle,&dataword,exp_size)) {
    *mcapi_status = MCAPI_ERR_SCL_SIZE;
  }
  return dataword;
}



/************************************************************************
mcapi_sclchan_available - checks if scalars are available on a receive endpoint.

DESCRIPTION

Checks if scalars are available on a receive endpoint. The function 
returns immediately. receive_endpoint is the receive endpoint 
identifier. The call only checks the availability of messages 
does not de-queue them.

RETURN VALUE

On success, the number of available scalars are returned and 
*mcapi_status is set to MCAPI_SUCCESS. On error, MCAPI_NULL is 
returned and *mcapi_status is set to the appropriate error defined 
below. MCAPI_NULL (or 0) could be a valid number of available 
scalars, so status has to be checked to ensure correctness.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.
MCAPI_ERR_PARAMETER
NOTE

The status code must be checked to distinguish between no messages and an error condition.



 
***********************************************************************/

mcapi_uint_t mcapi_sclchan_available (
 	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  int num = 0;
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
    *mcapi_status = MCAPI_ERR_CHAN_INVALID;
  } else {
    num = mcapi_trans_sclchan_available_i(receive_handle);
  }
  return num;
}


/************************************************************************
NAME

mcapi_ sclchan_recv_close_i - closes channel on a receive endpoint.

DESCRIPTION

Closes the receive side of a channel. The corresponding calls are required on both send and receive sides to ensure that the channel is properly closed. It is a non-blocking function, and returns immediately.  receive_handle is the receive endpoint identifier. All pending scalars are discarded, and any attempt to send more scalars will give an error. A scalar channel is disconnected when the last (second) close operation is performed.

RETURN VALUE
On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_CHAN_TYPE		Attempt to close a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION			Attempt to close a send handle on a port that was connected as a receiver, or vice versa.


MCAPI_ERR_CHAN_NOTOPEN		The channel is not open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.

 
***********************************************************************/
void mcapi_sclchan_recv_close_i(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle, 
	MCAPI_OUT mcapi_request_t* request, 
	MCAPI_OUT mcapi_status_t* mcapi_status) 
{

    *mcapi_status = MCAPI_SUCCESS;
    if (!request) {
      *mcapi_status = MCAPI_ERR_PARAMETER;
    } else {
      if (! mcapi_trans_valid_sclchan_recv_handle(receive_handle) ) {
        *mcapi_status = MCAPI_ERR_CHAN_INVALID;
      } else if (! mcapi_trans_sclchan_recv_isopen (receive_handle)) {
        *mcapi_status = MCAPI_ERR_CHAN_NOTOPEN;
      }
      mcapi_trans_sclchan_recv_close_i (receive_handle,request,mcapi_status);
    }
}

/************************************************************************
mcapi_sclchan_send_close_i - closes channel on a send endpoint.

DESCRIPTION

Closes the send side of a channel. The corresponding calls are 
required on both send and receive sides to ensure that the channel 
is properly closed. It is a non-blocking function, and returns 
immediately.  send_handle is the send endpoint identifier. Pending 
scalars at the receiver are not discarded. A scalar channel is 
disconnected when the last (second) close operation is performed.



RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS if completed 
and MCAPI_PENDING if not yet completed. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_CHAN_INVALID	Argument is not a channel handle.

MCAPI_ERR_CHAN_TYPE		Attempt to close a packet channel on an 
endpoint that has been connected with a different channel type.


MCAPI_ERR_CHAN_DIRECTION			Attempt to close a send handle on a port that was connected as a receiver, or vice versa.

MCAPI_ERR_CHAN_NOTOPEN		The channel is not open.

MCAPI_ERR_REQUEST_LIMIT	No more request handles available.

MCAPI_ERR_PARAMETER		Incorrect request parameter.

NOTE

Use the mcapi_test() , mcapi_wait() and mcapi_wait_any() functions 
to query the status of and mcapi_cancel() function to cancel 
the operation.


 
***********************************************************************/

void mcapi_sclchan_send_close_i(
 	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle, 
 	MCAPI_OUT mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  *mcapi_status = MCAPI_SUCCESS;   
  if (!request) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
  } else {
    if (! mcapi_trans_valid_sclchan_recv_handle(send_handle) ) {
      *mcapi_status = MCAPI_ERR_CHAN_INVALID;
    } else if (! mcapi_trans_sclchan_send_isopen (send_handle)) {
      *mcapi_status = MCAPI_ERR_CHAN_NOTOPEN;
    } 
    mcapi_trans_sclchan_send_close_i (send_handle,request,mcapi_status);
  }
}



/************************************************************************
mcapi_test - tests if non-blocking operation has completed.

DESCRIPTION

Checks if a non-blocking operation has completed. The function 
returns in a timely fashion. request is the identifier for the 
non-blocking operation. The call only checks the completion of 
an operation and doesn't affect any messages/packets/scalars. 
 If the specified request completes and the pending operation 
was a send or receive operation, the size parameter is set to 
the number of bytes that were either sent or received by the 
non-blocking transaction.

RETURN VALUE

On success, MCAPI_TRUE is returned and *mcapi_status is set to 
MCAPI_SUCCESS. If the operation has not completed MCAPI_FALSE 
is returned and *mcapi_status is set to MCAPI_PENDING. On error 
MCAPI_FALSE is returned and *mcapi_status is set to the appropriate 
error defined below for parameter errors for the mcapi_test()call 
or errors from the requesting functions, as defined in those 
respective functions.  

ERRORS

MCAPI_ERR_REQUEST_INVALID	Argument is not a valid request handle.

MCAPI_ERR_PARAMETER		Incorrect size parameter.

***********************************************************************/

mcapi_boolean_t mcapi_test(
                           /*MCAPI_IN*/ mcapi_request_t* request, 
 	MCAPI_OUT size_t* size, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  mcapi_boolean_t rc = MCAPI_FALSE;
  *mcapi_status = MCAPI_SUCCESS;
 
  if (! mcapi_trans_valid_size_param(size)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
    rc = MCAPI_TRUE;
  } else if (!mcapi_trans_valid_request_handle(request)) {
    *mcapi_status = MCAPI_ERR_REQUEST_INVALID; 
    rc = MCAPI_TRUE;
  } else {
    rc = mcapi_trans_test_i(request,size,mcapi_status);
  }
 return rc;
}

/************************************************************************
mcapi_wait - waits for a non-blocking operation to complete.

DESCRIPTION

Wait until a non-blocking operation has completed. It is a blocking 
function and returns when the operation has completed, has been 
canceled, or a timeout has occurred. request is the identifier 
for the non-blocking operation. The call only waits for the completion 
of an operation (all buffers referenced in the operation have 
been filled or consumed and can now be safely accessed by the 
application) and doesn't affect any messages/packets/scalars. 
 The size parameter is set to the number of bytes that were either 
sent or received by the non-blocking transaction that completed 
(size is irrelevant for non-blocking connect and close calls). 
 The mcapi_wait() call will return if the request is cancelled 
by a call to mcapi_cancel(), and the returned mcapi_status will 
indicate that the request was cancelled. The units for timeout 
are implementation defined.  If a timeout occurs the returned 
status will indicate that the timeout occurred.    A value of 
MCAPI_INFINITE for the timeout parameter indicates no timeout 
is requested. In regards to timeouts mcapi_wait is non destructive, 
i.e. the request is not cleared in the case of a timeout and 
can be waited upon multiple times.

RETURN VALUE

On success, MCAPI_TRUE is returned and *mcapi_status is set to 
MCAPI_SUCCESS. On error MCAPI_FALSE is returned and *mcapi_status 
is set to the appropriate error defined below for parameter errors 
for the mcapi_wait() call or errors from the requesting functions, 
as defined in those respective functions.  

ERRORS

MCAPI_ERR_REQUEST_INVALID	Argument is not a valid request handle.

MCAPI_ERR_REQUEST_CANCELLED	The request was canceled, by another thread (during the waiting).


MCAPI_TIMEOUT	The operation timed out.

MCAPI_ERR_PARAMETER		Incorrect request or size parameter.

***********************************************************************/

mcapi_boolean_t mcapi_wait(
 	/*MCAPI_IN*/ mcapi_request_t* request, 
 	MCAPI_OUT size_t* size,  
 	MCAPI_IN mcapi_timeout_t timeout,
 	MCAPI_OUT mcapi_status_t* mcapi_status
)
{
  mcapi_boolean_t rc = MCAPI_FALSE;
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_size_param(size)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
    rc = MCAPI_TRUE;
  } else if (!mcapi_trans_valid_request_handle(request)) {
    *mcapi_status = MCAPI_ERR_REQUEST_INVALID;
  } else {
    rc = mcapi_trans_wait(request,size,mcapi_status,timeout);
  }
  return rc;
}


/************************************************************************
mcapi_wait_any - waits for any non-blocking operation in a list to complete.

DESCRIPTION

Wait until any non-blocking operation of a list has completed. 
It is a blocking function and returns the index into the requests 
array (starting from 0) indicating which of any outstanding operations 
has completed. number is the number of requests in the array. 
requests is the array of mcapi_request_t identifiers for the 
non-blocking operations. The call only waits for the completion 
of an operation and doesn't affect any messages/packets/scalars. 
 The size parameter is set to number of bytes that were either 
sent or received by the non-blocking transaction that completed 
(size is irrelevant for non-blocking connect and close calls). 
 The mcapi_wait_any() call will return 0 if all the requests 
are cancelled by calls to mcapi_cancel() (during the waiting). 
0 could be a valid index, so status has to be checked to ensure 
correctness.The returned status will indicate that a request 
was cancelled. The units for timeout are implementation defined. 
 If a timeout occurs the mcapi_status parameter will indicate 
that a timeout occurred.    A value of MCAPI_INFINITE for the 
timeout parameter indicates no timeout is requested. In regards 
to timeouts mcapi_wait_any is non destructive, i.e. the request 
is not cleared in the case of a timeout and can be waited upon 
multiple times.

RETURN VALUE

On success, the index into the requests array of the mcapi_request_t 
identifier that has completed or has been canceled is returned 
and *mcapi_status is set to MCAPI_SUCCESS. On error MCAPI_NULL 
is returned and *mcapi_status is set to the appropriate error 
defined below  for parameter errors for the mcapi_wait_any() 
call or errors from the requesting functions, as defined in those 
respective functions.  

ERRORS

MCAPI_ERR_REQUEST_INVALID	Argument is not a valid request handle.

MCAPI_ERR_REQUEST_CANCELLED	One of the requests was canceled, by another thread (during the waiting).


MCAPI_TIMEOUT	The operation timed out.

MCAPI_ERR_PARAMETER	Incorrect number (if  =  0), requests or size parameter.

***********************************************************************/
  extern unsigned int mcapi_wait_any(
                                     MCAPI_IN size_t number,
                                     /*MCAPI_IN*/ mcapi_request_t** requests,
                                     MCAPI_OUT size_t* size,
                                     MCAPI_IN mcapi_timeout_t timeout,
                                     MCAPI_OUT mcapi_status_t* mcapi_status) 
{
  unsigned int rc = MCAPI_RETURN_VALUE_INVALID; 
  
  *mcapi_status = MCAPI_SUCCESS; 
  if (! mcapi_trans_valid_size_param(size)) {
    *mcapi_status = MCAPI_ERR_PARAMETER;
    rc = MCAPI_TRUE;
  } else {
    rc = mcapi_trans_wait_any(number,requests,size,mcapi_status,timeout);
  }
  return rc;
}



/************************************************************************
mcapi_cancel - cancels an outstanding non-blocking operation.

DESCRIPTION

Cancels an outstanding non-blocking operation. It is a blocking 
function and returns when the operation has been canceled. request 
is the identifier for the non-blocking operation.  Any pending 
calls to mcapi_wait() or mcapi_wait_any() for this request will 
also be cancelled. The returned status of a canceled mcapi_wait() 
or mcapi_wait_any() call will indicate that the request was cancelled. 
 

RETURN VALUE

On success, *mcapi_status is set to MCAPI_SUCCESS. On error *mcapi_status 
is set to the appropriate error defined below.

ERRORS

MCAPI_ERR_REQUEST_INVALID	Argument is not a valid request handle (the operation may have completed).
MCAPI_ERR_PARAMETER
***********************************************************************/

void mcapi_cancel(
 	/*MCAPI_IN*/ mcapi_request_t* request, 
 	MCAPI_OUT mcapi_status_t* mcapi_status)
{
  *mcapi_status = MCAPI_SUCCESS; 
    if (!mcapi_trans_valid_request_handle(request)) {
    *mcapi_status = MCAPI_ERR_REQUEST_INVALID;
  } else {
     mcapi_trans_cancel(request,mcapi_status);
  }
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
