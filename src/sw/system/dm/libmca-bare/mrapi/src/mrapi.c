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

#include <mrapi.h>

#include <string.h>
#include <stdio.h>
#include <mca_utils.h> // for mca_set_debug

void mrapi_set_debug_level(int d) { mca_set_debug_level(d); }

char* mrapi_display_status (mrapi_status_t status,char* status_message, size_t size) {
  if ((size < MRAPI_MAX_STATUS_SIZE) || (status_message == NULL)) {
    fprintf(stderr,"ERROR: size passed to mrapi_display_status must be at least %d and status_message must not be NULL.\n",MRAPI_MAX_STATUS_SIZE);
    return status_message;
  }
  memset(status_message,0,size);
  switch (status) {
  case (MRAPI_SUCCESS): return strcpy(status_message,"MRAPI_SUCCESS");
  case (MRAPI_TIMEOUT): return strcpy(status_message,"MRAPI_TIMEOUT"); break;
  case (MRAPI_INCOMPLETE): return strcpy(status_message,"MRAPI_INCOMPLETE"); break;
  case (MRAPI_ERR_ATTR_NUM): return strcpy(status_message,"MRAPI_ERR_ATTR_NUM"); break;
  case (MRAPI_ERR_ATTR_READONLY): return strcpy(status_message,"MRAPI_ERR_ATTR_READONLY"); break;
  case (MRAPI_ERR_ATTR_SIZE): return strcpy(status_message,"MRAPI_ERR_ATTR_SIZE"); break;
  case (MRAPI_ERR_DOMAIN_INVALID): return strcpy(status_message,"MRAPI_ERR_DOMAIN_INVALID"); break;
  case (MRAPI_ERR_DOMAIN_NOTSHARED): return strcpy(status_message,"MRAPI_ERR_DOMAIN_NOTSHARED"); break;
  case (MRAPI_ERR_MEM_LIMIT): return strcpy(status_message,"MRAPI_ERR_MEM_LIMIT"); break;
  case (MRAPI_ERR_MUTEX_DELETED): return strcpy(status_message,"MRAPI_ERR_MUTEX_DELETED"); break;
  case (MRAPI_ERR_MUTEX_EXISTS): return strcpy(status_message,"MRAPI_ERR_MUTEX_EXISTS"); break;
  case (MRAPI_ERR_MUTEX_ID_INVALID): return strcpy(status_message,"MRAPI_ERR_MUTEX_ID_INVALID"); break;
  case (MRAPI_ERR_MUTEX_INVALID): return strcpy(status_message,"MRAPI_ERR_MUTEX_INVALID"); break;
  case (MRAPI_ERR_MUTEX_KEY): return strcpy(status_message,"MRAPI_ERR_MUTEX_KEY"); break;
  case (MRAPI_ERR_MUTEX_LIMIT): return strcpy(status_message,"MRAPI_ERR_MUTEX_LIMIT"); break;
  case (MRAPI_ERR_MUTEX_LOCKED): return strcpy(status_message,"MRAPI_ERR_MUTEX_LOCKED"); break;
  case (MRAPI_ERR_MUTEX_LOCKORDER): return strcpy(status_message,"MRAPI_ERR_MUTEX_LOCKORDER"); break;
  case (MRAPI_ERR_MUTEX_NOTLOCKED): return strcpy(status_message,"MRAPI_ERR_MUTEX_NOTLOCKED"); break;
  case (MRAPI_ERR_MUTEX_NOTVALID): return strcpy(status_message,"MRAPI_ERR_MUTEX_NOTVALID"); break;
  case (MRAPI_ERR_NODE_FINALFAILED): return strcpy(status_message,"MRAPI_ERR_NODE_FINALFAILED"); break;
  case (MRAPI_ERR_NODE_INITIALIZED): return strcpy(status_message,"MRAPI_ERR_NODE_INITIALIZED"); break;
  case (MRAPI_ERR_NODE_INVALID): return strcpy(status_message,"MRAPI_ERR_NODE_INVALID"); break;
  case (MRAPI_ERR_NODE_NOTINIT): return strcpy(status_message,"MRAPI_ERR_NODE_NOTINIT"); break;
  case (MRAPI_ERR_NOT_SUPPORTED): return strcpy(status_message,"MRAPI_ERR_NOT_SUPPORTED"); break;
  case (MRAPI_ERR_PARAMETER): return strcpy(status_message,"MRAPI_ERR_PARAMETER"); break;
  case (MRAPI_ERR_REQUEST_CANCELED): return strcpy(status_message,"MRAPI_ERR_REQUEST_CANCELED"); break;
  case (MRAPI_ERR_REQUEST_INVALID): return strcpy(status_message,"MRAPI_ERR_REQUEST_INVALID"); break;
  case (MRAPI_ERR_REQUEST_LIMIT): return strcpy(status_message,"MRAPI_ERR_REQUEST_LIMIT"); break;
  case (MRAPI_ERR_RMEM_ID_INVALID): return strcpy(status_message,"MRAPI_ERR_RMEMID_INVALID"); break;
  case (MRAPI_ERR_RMEM_ATTACH): return strcpy(status_message,"MRAPI_ERR_RMEM_ATTACH"); break;
  case (MRAPI_ERR_RMEM_ATTACHED): return strcpy(status_message,"MRAPI_ERR_RMEM_ATTACHED"); break;
  case (MRAPI_ERR_RMEM_ATYPE): return strcpy(status_message,"MRAPI_ERR_RMEM_ATYPE"); break;
  case (MRAPI_ERR_RMEM_ATYPE_NOTVALID): return strcpy(status_message,"MRAPI_ERR_RMEM_ATYPE_NOTVALID"); break;
  case (MRAPI_ERR_RMEM_BLOCKED): return strcpy(status_message,"MRAPI_ERR_RMEM_BLOCKED"); break;
  case (MRAPI_ERR_RMEM_BUFF_OVERRUN): return strcpy(status_message,"MRAPI_ERR_RMEM_BUFF_OVERRUN"); break;
  case (MRAPI_ERR_RMEM_CONFLICT): return strcpy(status_message,"MRAPI_ERR_RMEM_CONFLICT"); break;
  case (MRAPI_ERR_RMEM_EXISTS): return strcpy(status_message,"MRAPI_ERR_RMEM_EXISTS"); break;
  case (MRAPI_ERR_RMEM_INVALID): return strcpy(status_message,"MRAPI_ERR_RMEM_INVALID"); break;
  case (MRAPI_ERR_RMEM_NOTATTACHED): return strcpy(status_message,"MRAPI_ERR_RMEM_NOTATTACHED"); break;
  case (MRAPI_ERR_RMEM_NOTOWNER): return strcpy(status_message,"MRAPI_ERR_RMEM_NOTOWNER"); break;
  case (MRAPI_ERR_RMEM_STRIDE): return strcpy(status_message,"MRAPI_ERR_RMEM_STRIDE"); break;
  case (MRAPI_ERR_RMEM_TYPENOTVALID): return strcpy(status_message,"MRAPI_ERR_RMEM_TYPENOTVALID"); break;
  case (MRAPI_ERR_RSRC_COUNTER_INUSE): return strcpy(status_message,"MRAPI_ERR_RSRC_COUNTER_INUSE"); break;
  case (MRAPI_ERR_RSRC_INVALID): return strcpy(status_message,"MRAPI_ERR_RSRC_INVALID"); break;
  case (MRAPI_ERR_RSRC_INVALID_CALLBACK): return strcpy(status_message,"MRAPI_ERR_RSRC_INVALID_CALLBACK"); break;
  case (MRAPI_ERR_RSRC_INVALID_EVENT): return strcpy(status_message,"MRAPI_ERR_RSRC_INVALID_EVENT"); break;
  case (MRAPI_ERR_RSRC_INVALID_SUBSYSTEM): return strcpy(status_message,"MRAPI_ERR_RSRC_INVALID_SUBSYSTEM"); break;
  case (MRAPI_ERR_RSRC_INVALID_TREE): return strcpy(status_message,"MRAPI_ERR_RSRC_INVALID_TREE"); break;
  case (MRAPI_ERR_RSRC_NOTDYNAMIC): return strcpy(status_message,"MRAPI_ERR_RSRC_NOTDYNAMIC"); break;
  case (MRAPI_ERR_RSRC_NOTOWNER): return strcpy(status_message,"MRAPI_ERR_RSRC_NOTOWNER"); break;
  case (MRAPI_ERR_RSRC_NOTSTARTED): return strcpy(status_message,"MRAPI_ERR_RSRC_NOTSTARTED"); break;
  case (MRAPI_ERR_RSRC_STARTED): return strcpy(status_message,"MRAPI_ERR_RSRC_STARTED"); break;
  case (MRAPI_ERR_RWL_DELETED): return strcpy(status_message,"MRAPI_ERR_RWL_DELETED"); break;
  case (MRAPI_ERR_RWL_EXISTS): return strcpy(status_message,"MRAPI_ERR_RWL_EXISTS"); break;
  case (MRAPI_ERR_RWL_ID_INVALID): return strcpy(status_message,"MRAPI_ERR_RWL_ID_INVALID"); break;
  case (MRAPI_ERR_RWL_INVALID): return strcpy(status_message,"MRAPI_ERR_RWL_INVALID"); break;
  case (MRAPI_ERR_RWL_LIMIT): return strcpy(status_message,"MRAPI_ERR_RWL_LIMIT"); break;
  case (MRAPI_ERR_RWL_LOCKED): return strcpy(status_message,"MRAPI_ERR_RWL_LOCKED"); break;
  case (MRAPI_ERR_RWL_NOTLOCKED): return strcpy(status_message,"MRAPI_ERR_RWL_NOTLOCKED"); break;
  case (MRAPI_ERR_SEM_DELETED): return strcpy(status_message,"MRAPI_ERR_SEM_DELETED"); break;
  case (MRAPI_ERR_SEM_EXISTS): return strcpy(status_message,"MRAPI_ERR_SEM_EXISTS"); break;
  case (MRAPI_ERR_SEM_ID_INVALID): return strcpy(status_message,"MRAPI_ERR_SEM_ID_INVALID"); break;
  case (MRAPI_ERR_SEM_INVALID): return strcpy(status_message,"MRAPI_ERR_SEM_INVALID"); break;
  case (MRAPI_ERR_SEM_LIMIT): return strcpy(status_message,"MRAPI_ERR_SEM_LIMIT"); break;
  case (MRAPI_ERR_SEM_LOCKED): return strcpy(status_message,"MRAPI_ERR_SEM_LOCKED"); break;
  case (MRAPI_ERR_SEM_LOCKLIMIT): return strcpy(status_message,"MRAPI_ERR_SEM_LOCKLIMIT"); break;
  case (MRAPI_ERR_SEM_NOTLOCKED): return strcpy(status_message,"MRAPI_ERR_SEM_NOTLOCKED"); break;
  case (MRAPI_ERR_SHM_ATTACHED): return strcpy(status_message,"MRAPI_ERR_SHMEM_ATTACHED"); break;
  case (MRAPI_ERR_SHM_ATTCH): return strcpy(status_message,"MRAPI_ERR_SHMEM_ATTCH"); break;
  case (MRAPI_ERR_SHM_EXISTS): return strcpy(status_message,"MRAPI_ERR_SHMEM_EXISTS"); break;
  case (MRAPI_ERR_SHM_ID_INVALID): return strcpy(status_message,"MRAPI_ERR_SHMEM_ID_INVALID"); break;
  case (MRAPI_ERR_SHM_INVALID): return strcpy(status_message,"MRAPI_ERR_SHM_INVALID"); break;
  case (MRAPI_ERR_SHM_NODES_INCOMPAT): return strcpy(status_message,"MRAPI_ERR_SHM_NODES_INCOMPAT"); break;
  case (MRAPI_ERR_SHM_NODE_NOTSHARED): return strcpy(status_message,"MRAPI_ERR_SHM_NODE_NOTSHARED"); break;
  case (MRAPI_ERR_SHM_NOTATTACHED): return strcpy(status_message,"MRAPI_ERR_SHM_NOTATTACHED"); break;
  default: return strcpy(status_message,"UNKNOWN ERROR"); break;
  };
}

/************************************************************************
mrapi_initialize

DESCRIPTION
mrapi_initialize() initializes the MRAPI environment on a given 
MRAPI node in a given MRAPI domain. It has to be called by each 
node using MRAPI. mrapi_parameters is used to pass implementation 
specific initialization parameters. mrapi_info is used to obtain 
information from the MRAPI implementation, including MRAPI and 
the underlying implementation version numbers, implementation 
vendor identification, the number of nodes in the topology, the 
number of ports on the local node and vendor specific implementation 
information, see the header files for additional information. 
A node is a process, a thread, or a processor (or core) with 
an independent program counter running a piece of code. In other 
words, an MRAPI node is an independent thread of control. An 
MRAPI node can call mrapi_initialize() once per node, and it 
is an error to call mrapi_initialize() multiple times from a 
given node, unless mrapi_finalize() is called in between. A given 
MRAPI implementation will specify what is a node (i.e., what 
thread of control  process, thread, or other -- is a node) in 
that implementation. A thread and process are just two examples 
of threads of control, and there could be others. 

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_NOT_INITFAILED 	The MRAPI environment could not be initialized.
MRAPI_ERR_NODE_INITIALIZED 	The MRAPI environment has already been initialized.
MRAPI_ERR_NODE_INVALID The node_id parameter is not valid.
MRAPI_ERR_DOMAIN_INVALID The domain_id parameter is not valid.
MRAPI_ERR_PARAMETER Invalid mrapi_parameters or mrapi_info  parameter.

NOTE

***********************************************************************/
void mrapi_initialize(
 	MRAPI_IN mrapi_domain_t domain_id,
 	MRAPI_IN mrapi_node_t node_id,
 	MRAPI_IN mrapi_parameters_t init_parameters,
 	MRAPI_OUT mrapi_info_t* mrapi_info,
 	MRAPI_OUT mrapi_status_t* status)
{
  *status = MRAPI_ERR_NODE_INITFAILED;

  if (!mrapi_impl_valid_parameters_param(init_parameters)) {
    *status = MRAPI_ERR_PARAMETER;
  } else if (!mrapi_impl_valid_info_param(mrapi_info)) {
    *status = MRAPI_ERR_PARAMETER;
  } else if (!mrapi_impl_valid_node_num(node_id)) {
    *status = MRAPI_ERR_NODE_INVALID;
  } else if (!mrapi_impl_valid_domain_num(domain_id)) {
    *status = MRAPI_ERR_DOMAIN_INVALID;
  } else if (mrapi_impl_initialized(domain_id,node_id)) {
    *status = MRAPI_ERR_NODE_INITIALIZED;
  } else if (mrapi_impl_initialize(domain_id,node_id,status)) {
    (void)strncpy(mrapi_info->mrapi_version,MRAPI_VERSION,sizeof(MRAPI_VERSION));
    /*printf("MRAPI VERSION=%s\n",mrapi_info->mrapi_version);*/
    *status = MRAPI_SUCCESS;
  } 
}

/************************************************************************
mrapi_finalize

DESCRIPTION
mrapi_finalize() finalizes the MRAPI environment on a given MRAPI 
node and domain. It has to be called by each node using MRAPI. 
 It is an error to call mrapi_finalize() without first calling 
mrapi_initialize().  An MRAPI node can call mrapi_finalize() 
once for each call to mrapi_initialize(), but it is an error 
to call mrapi_finalize() multiple times from a given <domain,node> 
unless mrapi_initialize() has been called prior to each mrapi_finalize() 
call.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_NODE_FINALFAILED The MRAPI environment could not be finalized.

NOTE

***********************************************************************/
void mrapi_finalize(
                    MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (! mrapi_impl_finalize()) {
    *status = MRAPI_ERR_NODE_FINALFAILED;
  }
}

/************************************************************************
mrapi_domain_id_get

DESCRIPTION
Returns the domain id associated with the local node.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_NODE_NOTINIT The calling node is not intialized.

NOTE

***********************************************************************/
mrapi_domain_t mrapi_domain_id_get(
 	MRAPI_OUT mrapi_status_t* status)
{
  mca_domain_t domain = MRAPI_DOMAIN_INVALID;
  *status = MRAPI_SUCCESS;

  if  (!mrapi_impl_get_domain_num(&domain)) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  }
  return domain;
}

/************************************************************************
mrapi_node_id_get

DESCRIPTION
Returns the node id associated with the local node and domain.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_NODE_NOTINIT The calling node is not intialized.

NOTE

***********************************************************************/
mrapi_node_t mrapi_node_id_get(
 	MRAPI_OUT mrapi_status_t* status)
{
  mca_node_t node = MRAPI_NODE_INVALID;

  *status = MRAPI_SUCCESS;

  if  (!mrapi_impl_initialized() || !mrapi_impl_get_node_num(&node)) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } 
  return node;
}

/************************************************************************
mrapi_mutex_create

DESCRIPTION
This function creates a mutex.  For non-default behavior, attributes 
must be set before the call to mrapi_mutex_create().  Once a 
mutex has been created, its attributes may not be changed.  If 
the attributes are NULL, then default attributes will be used. 
 The recursive attribute is disabled by default.  If you want 
to enable recursive locking/unlocking then you need to set that 
attribute before the call to create.  If mutex_id is set to MRAPI_MUTEX_ID_ANY, 
then MRAPI will choose an internal id for you.  

RETURN VALUE
On success a mutex handle is returned and *status is set to MRAPI_SUCCESS. 
 On error, *status is set to the appropriate error defined below. 
 In the case where the mutex already exists, status will be set 
to MRAPI_EXISTS and the handle returned will not be a valid handle. 
 

ERRORS
MRAPI_ERR_MUTEX_ID_INVALID	The mutex_id is not a valid mutex id.
MRAPI_ERR_MUTEX_EXISTS	This mutex is already created.
MRAPI_ERR_MUTEX_LIMIT Exceeded maximum number of mutexes allowed.
MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.
MRAPI_ERR_PARAMETER Invalid attributes parameter.

NOTE

***********************************************************************/
mrapi_mutex_hndl_t mrapi_mutex_create(
 	MRAPI_IN mrapi_mutex_id_t mutex_id,
 	MRAPI_IN mrapi_mutex_attributes_t* attributes,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_mutex_hndl_t mutex_hndl;


  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_mutex_validID(mutex_id)) {
    *status = MRAPI_ERR_MUTEX_ID_INVALID;
  } else if (mrapi_impl_mutex_create(&mutex_hndl,mutex_id,attributes,status)){
    *status = MRAPI_SUCCESS;
    return mutex_hndl;
  } else {
    /* assume the mutex already exists */
    *status = MRAPI_ERR_MUTEX_EXISTS;
  }
  return 0 ;
}

/************************************************************************
mrapi_mutex_init_attributes

DESCRIPTION
This function initializes the values of an mrapi_mutex_attributes_t 
structure.  For non-default behavior this function should be 
called prior to calling mrapi_mutex_set_attribute().  You would 
then use mrapi_mutex_set_attribute() to change any default values 
prior to calling mrapi_mutex_create().

RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to 
the appropriate error defined below.

ERRORS
MRAPI_ERR_PARAMETER Invalid attributes parameter.

NOTE

***********************************************************************/
void mrapi_mutex_init_attributes(
                                 MRAPI_OUT mrapi_mutex_attributes_t* attributes,
                                 MRAPI_OUT mrapi_status_t* status)
{
 *status = MRAPI_SUCCESS;
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_mutex_init_attributes(attributes);
  }
}

/************************************************************************
mrapi_mutex_set_attribute

DESCRIPTION
This function is used to change default values of an mrapi_mutex_attributes_t 
data structure prior to calling mrapi_mutex_create().  Calls 
to this function have no effect on mutex attributes once the 
mutex has been created.

MRAPI pre-defined mutex attributes:
Attribute num:	Description:	Datatype:	Default:
MRAPI_MUTEX_RECURSIVE	Indicates whether or not this is a recursive mutex. 	mrapi_boolean_t	MRAPI_FALSE
MRAPI_ERROR_EXT	Indicates whether or not this mutex has extended error checking enabled.  	mrapi_boolean_t	MRAPI_FALSE
MRAPI_DOMAIN_SHARED	Indicates whether or not the mutex is shareable across domains.	mrapi_boolean_t	MRAPI_TRUE

RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_ATTR_READONLY Attribute can not be modified.
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size

NOTE

***********************************************************************/
void mrapi_mutex_set_attribute (
 	MRAPI_OUT mrapi_mutex_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_IN void* attribute,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_mutex_set_attribute(attributes,attribute_num,attribute,attr_size,status);
  }
}

/************************************************************************
mrapi_mutex_get_attribute

DESCRIPTION
Returns the attribute that corresponds to the given attribute_num 
for this mutex.  The attributes may be viewed but may not be 
changed (for this mutex).

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute 
value is filled in.  On error, *status is set to the appropriate 
error defined below and the attribute value is undefined.  The 
attribute identified by the attribute_num is returned in the 
void* attribute parameter.  When extended error checking is enabled, 
if this function is called on a mutex that no longer exists, 
an MRAPI_EDELETED error code will be returned.  When extended 
error checking is disabled, the MRAPI_ERR_MUTEX_INVALID error 
will be returned.

ERRORS
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_MUTEX_INVALID Argument is not a valid mutex handle.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size

NOTE

***********************************************************************/
void mrapi_mutex_get_attribute (
 	MRAPI_IN mrapi_mutex_hndl_t mutex,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_OUT void* attribute,
 	MRAPI_IN size_t attribute_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (attribute == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else if ( mrapi_impl_valid_mutex_hndl(mutex,status)) {
    mrapi_impl_mutex_get_attribute(mutex,attribute_num,attribute,attribute_size,status);
  }
}

/************************************************************************
mrapi_mutex_get

DESCRIPTION
Given a mutex_id, this function returns the MRAPI handle for referencing that mutex.

RETURN VALUE
On success the mutex handle is returned and *status is set to 
MRAPI_SUCCESS.  On error, *status is set to the appropriate error 
defined below.  When extended error checking is enabled, if this 
function is called on a mutex that no longer exists, an MRAPI_EDELETED 
error code will be returned.  When extended error checking is 
disabled, the MRAPI_ERR_MUTEX_INVALID error will be returned.


ERRORS
MRAPI_ERR_MUTEX_ID_INVALID
The mutex_id parameter does not refer to a valid mutex or it is set to MRAPI_MUTEX_ID_ANY.

MRAPI_ERR_NODE_NOTINIT The node/domain is not initialized.

MRAPI_ERR_DOMAIN_NOTSHARED This resource can not be shared by this domain.

MRAPI_ERR_MUTEX_DELETED	If the mutex has been deleted then if 
MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_MUTEX_INVALID. 


NOTE

***********************************************************************/
mrapi_mutex_hndl_t mrapi_mutex_get(
 	MRAPI_IN mrapi_mutex_id_t mutex_id,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_mutex_hndl_t mutex;

  *status = MRAPI_ERR_MUTEX_ID_INVALID;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else  if (mrapi_impl_mutex_get(&mutex,mutex_id)) {
    *status = MRAPI_SUCCESS;
  }
  return mutex;
}

/************************************************************************
mrapi_mutex_delete

DESCRIPTION
This function deletes the mutex.  The mutex may only be deleted 
if it is unlocked.  If the mutex attributes indicate extended 
error checking is enabled then all subsequent lock requests will 
be notified that the mutex was deleted.  When extended error 
checking is enabled, if this function is called on a mutex that 
no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_MUTEX_INVALID 
error will be returned.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_MUTEX_INVALID Argument is not a valid mutex handle.

MRAPI_ERR_MUTEX_LOCKED	The mutex is locked and cannot be deleted.

MRAPI_ERR_MUTEX_DELETED	If the mutex has been deleted then if 
MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_MUTEX_INVALID. 


NOTE

***********************************************************************/
void mrapi_mutex_delete(
 	MRAPI_IN mrapi_mutex_hndl_t mutex,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else  if ( mrapi_impl_valid_mutex_hndl(mutex,status)) {
    if (mrapi_impl_mutex_delete(mutex)) {
      *status = MRAPI_SUCCESS;
    } else {
      *status = MRAPI_ERR_MUTEX_LOCKED;
    }
  }
}

/************************************************************************
mrapi_mutex_lock

DESCRIPTION
This function attempts to lock a mutex and will block if another 
node has a lock on the mutex.  When it obtains the lock, it sets 
up a unique key for that lock and that key is to be passed back 
on the call to unlock.  This key allows us to support recursive 
locking.  The lock_key is only valid if status indicates success. 
 Whether or not a mutex can be locked recursively is controlled 
via the MRAPI_MUTEX_RECURSIVE attribute, and the default is MRAPI_FALSE. 


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below.  When extended 
error checking is enabled, if this function is called on a mutex 
that no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_MUTEX_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_MUTEX_INVALID
Argument is not a valid mutex handle.

MRAPI_ERR_MUTEX_LOCKED 		Mutex is already locked by another node 
or mutex is already locked by this node and is not a recursive 
mutex.MRAPI_ERR_MUTEX_DELETED	If the mutex has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_MUTEX_INVALID. 

MRAPI_TIMEOUT	Timeout was reached.

MRAPI_ERR_PARAMETER Invalid lock_key or timeout parameter.

NOTE

***********************************************************************/
void mrapi_mutex_lock (
 MRAPI_IN mrapi_mutex_hndl_t mutex,
 MRAPI_OUT mrapi_key_t* lock_key,
 MRAPI_IN mrapi_timeout_t timeout,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_mutex_hndl(mutex,status)) {
    mrapi_impl_mutex_lock(mutex,lock_key,timeout,status);
  }
}

/************************************************************************
mrapi_mutex_trylock

DESCRIPTION
This function attempts to obtain a lock on the mutex.  If the 
lock can't be obtained then the function will immediately return 
MRAPI_FALSE.  If the request can't be satisfied for any other 
reason, then this function will immediately return the appropriate 
error code below.  If it is successful in obtaining the lock, 
it sets up a unique key for that lock and that key is to be passed 
back on the call to unlock.  The lock_key is only valid if status 
indicates success and the function returns MRAPI_TRUE.  This 
key allows us to support recursive locking.  Whether or not a 
mutex can be locked recursively is controlled via the MRAPI_MUTEX_RECURSIVE 
attribute, and the default is MRAPI_FALSE. 

RETURN VALUE
Returns MRAPI_TRUE if the lock was acquired, returns MRAPI_FALSE 
otherwise.  If there was an error then *status will be set to 
indicate the error from the table below, otherwise *status will 
indicate MRAPI_SUCCESS.  If the lock could not be obtained then 
*status will be either MRAPI_ELOCKED or one of the error conditions 
in the table below.  When extended error checking is enabled, 
if lock is called on a mutex that no longer exists, an MRAPI_EDELETED 
error code will be returned.  When extended error checking is 
disabled, the MRAPI_ERR_MUTEX_INVALID error will be returned 
and the lock will fail.

ERRORS

MRAPI_ERR_MUTEX_INVALID Argument is not a valid mutex handle.

MRAPI_ERR_MUTEX_DELETED	If the mutex has been deleted then if 
MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_MUTEX_INVALID. 

MRAPI_ERR_MUTEX_LOCKED 
Mutex is already locked by another node or mutex is already 
locked by this node and is not a recursive mutex.

MRAPI_ERR_PARAMETER Invalid lock_key parameter.


NOTE

***********************************************************************/
mrapi_boolean_t mrapi_mutex_trylock(
 MRAPI_IN mrapi_mutex_hndl_t mutex,
 MRAPI_OUT mrapi_key_t* lock_key,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_mutex_hndl(mutex,status)) {
    if ( mrapi_impl_mutex_lock(mutex,lock_key,1,status)) {
      return MRAPI_TRUE;
    } 
  }
  return MRAPI_FALSE;
}

/************************************************************************
mrapi_mutex_unlock

DESCRIPTION
This function unlocks a mutex.  If the mutex is recursive, then 
the lock_key parameter passed in must match the lock_key that 
was returned by the corresponding call to lock the mutex, and 
the set of recursive locks must be released using lock_keys in 
the reverse order that they were obtained.  When extended error 
checking is enabled, if this function is called on a mutex that 
no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_MUTEX_INVALID 
error will be returned.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_MUTEX_INVALID
Argument is not a valid mutex handle.

MRAPI_ERR_MUTEX_NOTLOCKED
Mutex is not locked.

MRAPI_ERR_MUTEX_KEY
lock_key is invalid for this mutex.

MRAPI_ERR_MUTEX_LOCKORDER
The unlock call does not match the lock order for this recursive mutex.

MRAPI_ERR_PARAMETER Invalid lock_key parameter.

MRAPI_ERR_MUTEX_DELETED	If the mutex 
has been deleted then if MRAPI_ERROR_EXT attribute is set, MRAPI 
will  return MRAPI_EDELETED otherwise MRAPI will just return 
MRAPI_ERR_MUTEX_INVALID. 


NOTE

***********************************************************************/
void mrapi_mutex_unlock(
 MRAPI_IN mrapi_mutex_hndl_t mutex,
 MRAPI_IN mrapi_key_t* lock_key,
 MRAPI_OUT mrapi_status_t* status)
{
  

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_mutex_hndl(mutex,status)) {
    mrapi_impl_mutex_unlock(mutex,lock_key,status); 
  } 
}

/************************************************************************
mrapi_sem_create

DESCRIPTION
This function creates a semaphore.  Unless you want the defaults, 
attributes must be set before the call to mrapi_sem_create(). 
 Once a semaphore has been created, its attributes may not be 
changed.  If the attributes are NULL, then implementation defined 
default attributes will be used.  If sem_id is set to MRAPI_SEM_ID_ANY, 
then MRAPI will choose an internal id for you.  The shared_lock_limit 
parameter indicates the maximum number of available locks and 
it must be between 0 and MRAPI_MAX_SEM_SHAREDLOCKS.

RETURN VALUE
On success a semaphore handle is returned and *status is set 
to MRAPI_SUCCESS.  On error, *status is set to the appropriate 
error defined below.   In the case where the semaphore already 
exists, status will be set to MRAPI_EXISTS and the handle returned 
will not be a valid handle.

ERRORS
MRAPI_ERR_SEM_INVALID
The semaphore_id is not a valid semaphore id.
MRAPI_ERR_SEM_EXISTS	This semaphore is already created.
MRAPI_ERR_SEM_LIMIT
Exceeded maximum number of semaphores allowed.
MRAPI_ERR_SEM_LOCKLIMIT	The shared lock limit is out of bounds.
MRAPI_ERR_NODE_NOTINIT
The calling node is not initialized.
MRAPI_ERR_PARAMETER
Invalid attributes parameter.

NOTE

***********************************************************************/
mrapi_sem_hndl_t mrapi_sem_create(
 	MRAPI_IN mrapi_sem_id_t sem_id,
 	MRAPI_IN mrapi_sem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t shared_lock_limit,
 	MRAPI_OUT mrapi_status_t* status)
{

  mrapi_sem_hndl_t sem_hndl;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT; /* fixme: how to handle domain_notinit? */
  } else if (!mrapi_impl_sem_validID(sem_id)) {
    *status = MRAPI_ERR_SEM_ID_INVALID;
  }else if ((shared_lock_limit == 0) || (shared_lock_limit > MRAPI_MAX_SHARED_LOCKS)) {
    *status = MRAPI_ERR_SEM_LOCKLIMIT;
  } else if (mrapi_impl_sem_create(&sem_hndl,sem_id,attributes,shared_lock_limit,status)){
    *status = MRAPI_SUCCESS;
  }
  return sem_hndl;
}

/************************************************************************
mrapi_sem_init_attributes

DESCRIPTION
Unless you want the defaults, this function should be called 
to initialize the values of an mrapi_sem_attributes_t structure 
prior to mrapi_sem_set_attribute().  You would then use mrapi_sem_set_attribute() 
to change any default values prior to calling mrapi_sem_create().


RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to 
the appropriate error defined below.

ERRORS
MRAPI_ERR_PARAMETER Invalid attributes parameter.

NOTE

***********************************************************************/
void mrapi_sem_init_attributes(
                                 MRAPI_OUT mrapi_sem_attributes_t* attributes,
                                 MRAPI_OUT mrapi_status_t* status)
{
  *status = MRAPI_SUCCESS;
   if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_sem_init_attributes(attributes);
  }
}

/************************************************************************
mrapi_sem_set_attribute

DESCRIPTION
This function is used to change default values of an mrapi_sem_attributes_t 
data structure prior to calling mrapi_sem_create().  Calls to 
this function have no effect on semaphore attributes once the 
semaphore has been created.

MRAPI pre-defined semaphore attributes:
Attribute num:	Description:	Datatype:	Default:
MRAPI_ERROR_EXT	Indicates whether or not this semaphore has extended 
error checking enabled.  	mrapi_boolean_t	MRAPI_FALSE
MRAPI_DOMAIN_SHARED	Indicates whether or not this semaphore is shareable across domains.  	
  mrapi_boolean_t	MRAPI_TRUE


RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_ATTR_READONLY Attribute can not be modified.
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size


NOTE

***********************************************************************/
void mrapi_sem_set_attribute(
 	MRAPI_OUT mrapi_sem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_IN void* attribute,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_sem_set_attribute(attributes,attribute_num,attribute,attr_size,status);
  }
}

/************************************************************************
mrapi_sem_get_attribute

DESCRIPTION
Returns the attribute that corresponds to the given attribute_num 
for this semaphore.  The attribute may be viewed but may not 
be changed (for this semaphore).

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute 
value is filled in.  On error, *status is set to the appropriate 
error defined below and the attribute value is undefined.  The 
attribute identified by the attribute_num is returned in the 
void* attribute parameter.  When extended error checking is enabled, 
if this function is called on a semaphore that no longer exists, 
an MRAPI_EDELETED error code will be returned.  When extended 
error checking is disabled, the MRAPI_ERR_SEM_INVALID error will 
be returned.

ERRORS
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_SEM_INVALID Argument is not a valid semaphore handle.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size


NOTE

***********************************************************************/
void mrapi_sem_get_attribute (
 	MRAPI_IN mrapi_sem_hndl_t sem,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_OUT void* attribute,
 	MRAPI_IN size_t attribute_size,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (attribute == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else if (  mrapi_impl_valid_sem_hndl(sem,status)) {
    mrapi_impl_sem_get_attribute(sem,attribute_num,attribute,attribute_size,status);
  }
}

/************************************************************************
mrapi_sem_get

DESCRIPTION
Given a sem_id, this function returns the MRAPI handle for referencing that semaphore.

RETURN VALUE
On success the semaphore handle is returned and *status is set 
to MRAPI_SUCCESS.  On error, *status is set to the appropriate 
error defined below.  When extended error checking is enabled, 
if this function is called on a semaphore that no longer exists, 
an MRAPI_EDELETED error code will be returned.  When extended 
error checking is disabled, the MRAPI_ERR_SEM_INVALID error will 
be returned.

ERRORS
MRAPI_ERR_SEM_ID_INVALID
The sem_id parameter does not refer to a valid semaphore or was called with sem_id set 
to MRAPI_SEM_ID_ANY.

MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.

MRAPI_ERR_DOMAIN_NOTSHARED
This resource can not be shared by this domain. 

MRAPI_ERR_SEM_DELETED	If the semaphore has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_SEM_INVALID. 



NOTE

***********************************************************************/
mrapi_sem_hndl_t mrapi_sem_get(
 	MRAPI_IN mrapi_sem_id_t sem_id,
 	MRAPI_OUT mrapi_status_t* status)
{ 
  mrapi_sem_hndl_t sem;

  *status = MRAPI_ERR_SEM_ID_INVALID;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_sem_get(&sem,sem_id)) {
    *status = MRAPI_SUCCESS;
  }
  return sem;
}

/************************************************************************
mrapi_sem_delete

DESCRIPTION
This function deletes the semaphore.  The semaphore will only 
be deleted if the semaphore is not locked.  If the semaphore 
attributes indicate extended error checking is enabled then all 
subsequent lock requests will be notified that the semaphore 
was deleted.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below. When extended 
error checking is enabled, if this function is called on a semaphore 
that no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_SEM_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_SEM_INVALID Argument is not a valid semaphore handle.

MRAPI_ERR_SEM_DELETED	If the semaphore has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_SEM_INVALID.

MRAPI_ERR_SEM_LOCKED	The semaphore is locked and cannot be deleted.



NOTE

***********************************************************************/
void mrapi_sem_delete(
 	MRAPI_IN mrapi_sem_hndl_t sem,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_sem_hndl(sem,status)) {
    if (mrapi_impl_sem_delete(sem)) {
      *status = MRAPI_SUCCESS;
    } else {
      *status = MRAPI_ERR_SEM_LOCKED;
    }
  }
}

/************************************************************************
mrapi_sem_lock

DESCRIPTION
This function attempts to obtain a single lock on the semaphore 
and will block until a lock is available or the timeout is reached 
(if timeout is non-zero).  If the request can't be satisfied 
for some other reason, this function will return the appropriate 
error code below.  An application may make this call as many 
times as needed to obtain multiple locks, up to the limit specified 
by the shared_lock_limit parameter used when the semaphore was 
created. 

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below. When extended 
error checking is enabled, if lock is called on semaphore that 
no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_SEM_INVALID 
error will be returned and the lock will fail.

ERRORS
MRAPI_ERR_SEM_INVALID Argument is not a valid semaphore handle.

MRAPI_ERR_SEM_DELETED	If the semaphore has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_SEM_INVALID. MRAPI_TIMEOUT	
Timeout was reached.

NOTE

***********************************************************************/
void mrapi_sem_lock(
 MRAPI_IN mrapi_sem_hndl_t sem,
 MRAPI_IN mrapi_timeout_t timeout,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (  mrapi_impl_valid_sem_hndl(sem,status)) {
    mrapi_impl_sem_lock(sem,1,timeout,status);
  }  
}

/************************************************************************
mrapi_sem_trylock

DESCRIPTION
This function attempts to obtain a single lock on the semaphore. 
 If the lock can't be obtained because all the available locks 
are already locked (by this node and/or others) then the function 
will immediately return MRAPI_FALSE.  If the request can't be 
satisfied for any other reason, then this function will immediately 
return the appropriate error code below.

RETURN VALUE
Returns MRAPI_TRUE if the lock was acquired, returns MRAPI_FALSE 
otherwise.  If there was an error then *status will be set to 
indicate the error from the table below, otherwise *status will 
indicate MRAPI_SUCCESS.  If the lock could not be obtained then 
*status will be either MRAPI_ELOCKED or one of the error conditions 
in the table below. When extended error checking is enabled, 
if this function is called on a semaphore that no longer exists, 
an MRAPI_EDELETED error code will be returned.  When extended 
error checking is disabled, the MRAPI_ERR_SEM_INVALID error will 
be returned.

ERRORS
MRAPI_ERR_SEM_INVALID Argument is not a valid semaphore handle.

MRAPI_ERR_SEM_DELETED	If the semaphore has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_SEM_INVALID.


NOTE

***********************************************************************/
mrapi_boolean_t mrapi_sem_trylock(MRAPI_IN mrapi_sem_hndl_t sem,
                                  MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (  mrapi_impl_valid_sem_hndl(sem,status)) {
    if ( mrapi_impl_sem_lock(sem,1,1,status)) {    
      return MRAPI_TRUE;
    } 
  }
  return MRAPI_FALSE;
 }

/************************************************************************
mrapi_sem_unlock

DESCRIPTION
This function releases a single lock.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below. When extended 
error checking is enabled, if this function is called on a semaphore 
that no longer exists, an MRAPI_EDELETED error code will be returned. 
 When extended error checking is disabled, the MRAPI_ERR_SEM_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_SEM_INVALID 	Argument is not a valid semaphore handle.

MRAPI_ERR_SEM_NOTLOCKED This node does not have a lock on this semaphore

MRAPI_ERR_SEM_DELETED	If the semaphore has been deleted then 
if MRAPI_ERROR_EXT attribute is set, MRAPI will  return MRAPI_EDELETED 
otherwise MRAPI will just return MRAPI_ERR_SEM_INVALID. 



NOTE

***********************************************************************/
void mrapi_sem_unlock (
 MRAPI_IN mrapi_sem_hndl_t sem,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (  mrapi_impl_valid_sem_hndl(sem,status)) {
    mrapi_impl_sem_unlock(sem,1,status);
  } 
}

/************************************************************************
mrapi_rwl_create

DESCRIPTION
This function creates a reader/writer lock.  Unless you want 
the defaults, attributes must be set before the call to mrapi_rwl_create(). 
 Once a reader/writer lock has been created, its attributes may 
not be changed.  If the attributes are NULL, then implementation 
defined default attributes will be used.  If rwl_id is set to 
MRAPI_RWL_ID_ANY, then MRAPI will choose an internal id for you. 
 

RETURN VALUE
On success a reader/writer lock handle is returned and *status 
is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate 
error defined below.  In the case where the reader/writer lock 
already exists, status will be set to MRAPI_EXISTS and the handle 
returned will not be a valid handle.  

ERRORS
MRAPI_ERR_RWL_INVALID The rwl_id is not a valid reader/writer lock id.
MRAPI_ERR_RWL_EXISTS	This reader/writer lock is already created.
MRAPI_ERR_RWL_LIMIT Exceeded maximum number of reader/writer locks allowed.
MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.
MRAPI_ERR_PARAMETER Invalid attributes parameter.

NOTE

***********************************************************************/

mrapi_rwl_hndl_t mrapi_rwl_create(
 	MRAPI_IN mrapi_rwl_id_t rwl_id,
 	MRAPI_IN mrapi_rwl_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t reader_lock_limit,
 	MRAPI_OUT mrapi_status_t* status)
{

  mrapi_sem_hndl_t rwl_hndl;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT; /* fixme: how to handle domain_notinit? */
  } else if (!mrapi_impl_rwl_validID(rwl_id)) {
    *status = MRAPI_ERR_RWL_ID_INVALID;
  } else if (mrapi_impl_rwl_create(&rwl_hndl,rwl_id,attributes,reader_lock_limit,status)){
    *status = MRAPI_SUCCESS;
  }
  return rwl_hndl;
}

/************************************************************************
mrapi_rwl_init_attributes

DESCRIPTION
Unless you want the defaults, this call must be used to initialize 
the values of an mrapi_rwl_attributes_t structure prior to mrapi_rwl_set_attribute(). 
 Use mrapi_rwl_set_attribute() to change any default values prior 
to calling mrapi_rwl_create().

RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_PARAMETER Invalid attributes parameter.

NOTE

***********************************************************************/
void mrapi_rwl_init_attributes(
                                 MRAPI_OUT mrapi_rwl_attributes_t* attributes,
                                 MRAPI_OUT mrapi_status_t* status)
{
  *status = MRAPI_SUCCESS;
   if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_rwl_init_attributes(attributes);
  }
}

/************************************************************************
mrapi_rwl_set_attribute

DESCRIPTION
This function is used to change default values of an mrapi_rwl_attributes_t 
data structure prior to calling mrapi_rwl_create().  Calls to 
this function have no effect on mutex attributes once the mutex 
has been created.

MRAPI pre-defined reader/writer lock attributes:
Attribute num:	Description:	Datatype:	Default:
MRAPI_ERROR_EXT	Indicates whether or not this reader/writer lock 
has extended error checking enabled.  	mrapi_boolean_t	MRAPI_FALSE

MRAPI_DOMAIN_SHARED	Indicates whether or not the reader/writer 
lock is shareable across domains.  	mrapi_boolean_t	MRAPI_TRUE


RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_ATTR_READONLY Attribute can not be modified.
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size


NOTE

***********************************************************************/

void mrapi_rwl_set_attribute(
 	MRAPI_OUT mrapi_rwl_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_IN void* attribute,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_rwl_set_attribute(attributes,attribute_num,attribute,attr_size,status);
  }
}





/************************************************************************
mrapi_rwl_get_attribute

DESCRIPTION
Returns the attribute that corresponds to the given attribute_num 
for this reader/writer lock.  The attribute may be viewed but 
may not be changed (for this reader/writer lock).

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute 
value is filled in.  On error, *status is set to the appropriate 
error defined below and the attribute value is undefined.  The 
attribute identified by the attribute_num is returned in the 
void* attribute parameter. When extended error checking is enabled, 
if this function is called on a reader/writer lock that no longer 
exists, an MRAPI_EDELETED error code will be returned.  When 
extended error checking is disabled, the MRAPI_ERR_RWL_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_RWL_INVALID Argument is not a valid rwl handle.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size


NOTE
It is up to the implementation as to whether a reader/writer 
lock may be shared across domains.  This is specified as an attribute 
during creation and the default is MRAPI_FALSE.

***********************************************************************/
void mrapi_rwl_get_attribute (
 	MRAPI_IN mrapi_rwl_hndl_t rwl,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_OUT void* attribute,
 	MRAPI_IN size_t attribute_size,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else  if (attribute == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else if ( mrapi_impl_valid_rwl_hndl(rwl,status)) {
    mrapi_impl_rwl_get_attribute(rwl,attribute_num,attribute,attribute_size,status);
  }
}

/************************************************************************
mrapi_rwl_get

DESCRIPTION
Given a rwl_id, this function returns the MRAPI handle for referencing that reader/writer lock.

RETURN VALUE
On success the reader/writer lock handle is returned and *status 
is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate 
error defined below.

ERRORS
MRAPI_ERR_RWL_ID_INVALID
The rwl_id parameter does not refer to a valid reader/writer 
lock or it was called with rwl_id set to MRAPI_RWL_ID_ANY.

MRAPI_ERR_NODE_NOTINIT
The calling node is not initialized.

MRAPI_ERR_DOMAIN_NOTSHARED
This resource can not be shared by this domain. 



NOTE

***********************************************************************/
mrapi_rwl_hndl_t mrapi_rwl_get(
 	MRAPI_IN mrapi_rwl_id_t rwl_id,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_rwl_hndl_t rwl;

  
  *status = MRAPI_ERR_RWL_ID_INVALID;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_rwl_get(&rwl,rwl_id)) {
    *status = MRAPI_SUCCESS;
  }
  return rwl;
}

/************************************************************************
mrapi_rwl_delete

DESCRIPTION
This function deletes the reader/writer lock.  A reader/writer 
lock can only be deleted if it is not locked.  If the reader/writer 
lock attributes indicate extended error checking is enabled then 
all subsequent lock requests will be notified that the reader/writer 
lock was deleted.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below.  When extended 
error checking is enabled, if this function is called on a reader/writer 
lock that no longer exists, an MRAPI_EDELETED error code will 
be returned.  When extended error checking is disabled, the MRAPI_ERR_RWL_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_RWL_INVALID
Argument is not a valid reader/writer lock handle.

MRAPI_ERR_RWL_LOCKED	The reader/writer lock was locked and cannot be deleted.



NOTE

***********************************************************************/
void mrapi_rwl_delete(
 	MRAPI_IN mrapi_rwl_hndl_t rwl,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (  mrapi_impl_valid_rwl_hndl(rwl,status)) {
    if (mrapi_impl_rwl_delete(rwl)) {
      *status = MRAPI_SUCCESS;
    } else {
      *status = MRAPI_ERR_RWL_LOCKED;
    }
  }
}

/************************************************************************
mrapi_rwl_lock

DESCRIPTION
This function attempts to obtain a single lock on the reader/writer 
lock and will block until a lock is available or the timeout 
is reached (if timeout is non-zero).  A node may only have one 
reader lock or one writer lock at any given time.  The mode parameter 
is used to specify the type of lock: MRAPI_READER (shared) or 
MRAPI_WRITER (exclusive).  If the lock can't be obtained for 
some other reason, this function will return the appropriate 
error code below.  

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below.  When extended 
error checking is enabled, if lock is called on a reader/writer 
lock that no longer exists, an MRAPI_EDELETED error code will 
be returned.  When extended error checking is disabled, the MRAPI_ERR_RWL_INVALID 
error will be returned.  In both cases the attempt to lock will 
fail.

ERRORS
MRAPI_ERR_RWL_INVALID
Argument is not a valid reader/writer lock handle.

MRAPI_ERR_RWL_DELETED	If the reader/writer lock has been deleted 
then if MRAPI_ERROR_EXT attribute is set, MRAPI will  return 
MRAPI_EDELETED otherwise MRAPI will just return MRAPI_ERR_RWL_INVALID. 

MRAPI_TIMEOUT	Timeout was reached.

MRAPI_ERR_RWL_LOCKED	The caller already has a lock MRAPI_ERR_PARAMETER	Invalid mode.

NOTE

***********************************************************************/
void mrapi_rwl_lock(
 MRAPI_IN mrapi_rwl_hndl_t rwl,
 MRAPI_IN mrapi_rwl_mode_t mode,
 MRAPI_IN mrapi_timeout_t timeout,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_rwl_hndl(rwl,status)) {
    mrapi_impl_rwl_lock(rwl,mode,timeout,status);
  }
}

/************************************************************************
mrapi_rwl_trylock

DESCRIPTION
This function attempts to obtain a single lock on the reader/writer 
lock.  A node may only have one reader lock or one writer lock 
at any given time.  The mode parameter is used to specify the 
type of lock: MRAPI_READER (shared) or MRAPI_WRITER (exclusive). 
 If the lock can't be obtained because a reader lock was requested 
and there is already a writer lock or a writer lock was requested 
and there is already any lock then the function will immediately 
return MRAPI_FALSE.  If the request can't be satisfied for any 
other reason, then this function will immediately return the 
appropriate error code below. 

RETURN VALUE
Returns MRAPI_TRUE if the lock was acquired, returns MRAPI_FALSE 
otherwise.  If there was an error then *status will be set to 
indicate the error from the table below, otherwise *status will 
indicate MRAPI_SUCCESS.  If the lock could not be obtained then 
*status will be either MRAPI_ELOCKED or one of the error conditions 
in the table below.  When extended error checking is enabled, 
if trylock is called on a reader/writer lock that no longer exists, 
an MRAPI_EDELETED error code will be returned.  When extended 
error checking is disabled, the MRAPI_ERR_RWL_INVALID error will 
be returned and the lock will fail.

ERRORS
MRAPI_ERR_RWL_INVALID
Argument is not a valid reader/writer lock handle.

MRAPI_ERR_RWL_DELETED	If the reader/writer lock has been deleted 
then if MRAPI_ERROR_EXT attribute is set, MRAPI will  return 
MRAPI_EDELETED otherwise MRAPI will just return MRAPI_ERR_RWL_INVALID. 

MRAPI_ERR_RWL_LOCKED 		The reader/writer lock is already exclusively 
locked.

MRAPI_ERR_PARAMETER	Invalid mode.



NOTE

***********************************************************************/
mrapi_boolean_t mrapi_rwl_trylock(
 MRAPI_IN mrapi_rwl_hndl_t rwl,
 MRAPI_IN mrapi_rwl_mode_t mode,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_rwl_hndl(rwl,status)) {
    if (mrapi_impl_rwl_lock(rwl,mode,1,status)) {
      return MRAPI_TRUE;
    }
  }
  return MRAPI_FALSE;
}

/************************************************************************
mrapi_rwl_unlock

DESCRIPTION
This function releases a single lock.   The lock to be released 
will be either a reader lock or a writer lock, as specified by 
the mode parameter used when the lock was obtained.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below.  When extended 
error checking is enabled, if this function is called on a reader/writer 
lock that no longer exists, an MRAPI_EDELETED error code will 
be returned.  When extended error checking is disabled, the MRAPI_ERR_RWL_INVALID 
error will be returned.

ERRORS
MRAPI_ERR_RWL_INVALID	
Argument is not a valid reader/writer lock handle.

MRAPI_ERR_RWL_NOTLOCKED
This node does not currently hold the given type (reader/writer) of lock.

NOTE

***********************************************************************/
void mrapi_rwl_unlock (
 MRAPI_IN mrapi_rwl_hndl_t rwl,
 MRAPI_IN mrapi_rwl_mode_t mode,
 MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( mrapi_impl_valid_rwl_hndl(rwl,status)) {
    mrapi_impl_rwl_unlock(rwl,mode,status);
  } 
}

/************************************************************************
mrapi_shmem_create

DESCRIPTION
This function creates a shared memory segment.  The size parameter 
specifies the size of the shared memory region in bytes.  Unless 
you want the defaults, attributes must be set before the call 
to mrapi_shmem_create().  A list of nodes that can access the 
shared memory can be passed in the nodes parameter and nodes_size 
should contain the number of nodes in the list.  If nodes is 
NULL, then all nodes will be allowed to access the shared memory. 
 Once a shared memory segment has been created, its attributes 
may not be changed.  If the attributes parameter is NULL, then 
implementation defined default attributes will be used.  In the 
case where the shared memory segment already exists, status will 
be set to MRAPI_EXISTS and the handle returned will not be a 
valid handle.  If shmem_id is set to MRAPI_SHMEM_ID_ANY, then 
MRAPI will choose an internal id for you.  All nodes in the nodes 
list must be initialized nodes in the system.

RETURN VALUE
On success a shared memory segment handle is returned, the address 
is filled in and *status is set to MRAPI_SUCCESS.  On error, 
*status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_SHM_INVALID
The shmem_id is not a valid shared memory segment id.

MRAPI_ERR_SHM_NODES_INCOMPAT	The list of nodes is not compatible 
for setting up shared memory.

MRAPI_ERR_SHM_EXISTS	This shared memory segment is already created.

MRAPI_ERR_MEM_LIMIT No memory available.

MRAPI_ERR_NODE_NOTINIT
The calling node is not initialized or one of the nodes in the list of nodes to share with is not initialized.

MRAPI_ERR_PARAMETER
Incorrect size, attributes, attribute_size,  or nodes_size parameter.

NOTE

***********************************************************************/
mrapi_shmem_hndl_t mrapi_shmem_create(
 	MRAPI_IN mrapi_shmem_id_t shmem_id,
 	MRAPI_IN mrapi_uint_t size,
 	MRAPI_IN mrapi_node_t* nodes,
 	MRAPI_IN mrapi_uint_t nodes_size,
 	MRAPI_IN mrapi_shmem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_shmem_hndl_t shm;


  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT; /* fixme: how to handle domain_notinit? */
  } else if (!mrapi_impl_shmem_validID(shmem_id)) {
    *status = MRAPI_ERR_SHM_ID_INVALID;
  }else if (mrapi_impl_shmem_exists(shmem_id /*key */)) {
    *status = MRAPI_ERR_SHM_EXISTS;
  } else {
    mrapi_impl_shmem_create(&shm,shmem_id /* key */,size,attributes,status);
  } 
  return shm;
}

/************************************************************************
mrapi_shmem_init_attributes

DESCRIPTION
Unless you want the defaults, this call must be used to initialize 
the values of an mrapi_shmem_attributes_t structure prior to 
mrapi_shmem_set_attribute().  You w would then use mrapi_shmem_set_attribute() 
to change any default values prior to calling mrapi_shmem_create().


RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_PARAMETER
Invalid attributes parameter.

NOTE

***********************************************************************/
void mrapi_shmem_init_attributes(
                                 MRAPI_OUT mrapi_shmem_attributes_t* attributes,
                                 MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_shmem_init_attributes(attributes);
  }
}

/************************************************************************
mrapi_shmem_set_attribute

DESCRIPTION
This function is used to change default values of an mrapi_shmem_attributes_t 
data structure prior to calling mrapi_shmem_create().  If the 
user wants to control which physical memory is used, then that 
is done by setting the MRAPI_SHMEM_RESOURCE attribute to the 
resource in the metadata tree.  The user would first need to 
call mrapi_resources_get() and then iterate over the tree to 
find the desired resource (see the example use case for more 
details).

MRAPI pre-defined shared memory attributes:
Attribute num:	Description:	Datatype:	Default:
MRAPI_SHMEM_RESOURCE	The physical memory resource in the metadata 
resource tree that the memory should be allocated from. 	
mrapi_resource_t	MRAPI_SHMEM_ANY

MRAPI_SHMEM_ADDRESS	The requested address for a shared memory region	
mrapi_uint_t	MRAPI_SHMEM_ANY

MRAPI_DOMAIN_SHARED	Indicates whether or not this remote memory 
is shareable across domains.  	
mrapi_boolean_t	MRAPI_TRUE

MRAPI_SHMEM_SIZE	Returns the size of the shared memory segment 
in bytes.  This attribute can only be set through the size parameter 
passed in to create.	
mrapi_size_t	No default.

MRAPI_SHMEM_ADDRESS	if MRAPI_SHMEM_ANY then not necessarily contiguous, 
if <address> then contiguous;non-contiguous should be used with 
care and will not work in contexts that cannot handle virtual 
memory	
mrapi_addr_t	MRAPI_SHMEM_ANY_CONTIGUOUS


RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the
 appropriate error defined below.

ERRORS
MRAPI_ERR_ATTR_READONLY Attribute can not be modified.
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size

NOTE

***********************************************************************/
void mrapi_shmem_set_attribute(
 	MRAPI_OUT mrapi_shmem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_IN void* attribute,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{


  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_shmem_set_attribute(attributes,attribute_num,attribute,attr_size,status);
  }
}

/************************************************************************
mrapi_shmem_get_attribute

DESCRIPTION
Returns the attribute that corresponds to the given attribute_num 
for this shared memory.  The attributes may be viewed but may 
not be changed (for this shared memory).

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute 
value is filled in.  On error, *status is set to the appropriate 
error defined below and the attribute value is undefined.  The 
attribute identified by the attribute_num is returned in the 
void* attribute parameter.

ERRORS
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_SHM_INVALID Argument is not a valid shmem handle.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size

NOTE

***********************************************************************/
void mrapi_shmem_get_attribute(
 	MRAPI_IN mrapi_shmem_hndl_t shmem,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_OUT void* attribute,
 	MRAPI_IN size_t attribute_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  if (attribute == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else if ( ! mrapi_impl_valid_shmem_hndl(shmem)) {
    *status = MRAPI_ERR_SHM_INVALID;
  }else {
    mrapi_impl_shmem_get_attribute(shmem,attribute_num,attribute,attribute_size,status);
  }
}

/************************************************************************
mrapi_shmem_get

DESCRIPTION
Given a shmem_id this function returns the MRAPI handle for referencing that 
shared memory segment.

RETURN VALUE
On success the shared memory segment handle is returned and *status 
is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate 
error defined below.

ERRORS
MRAPI_ERR_SHM_ID_INVALID The shmem_id is not a valid shared memory id or it 
was called with shmem_id set to MRAPI_SHMEM_ID_ANY.

MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.

MRAPI_ERR_SHM_NODE_NOTSHARED	This shared memory is not shareable 
with the calling node.  Which nodes it is shareable with was 
specified on the call to mrapi_shmem_create().

MRAPI_ERR_DOMAIN_NOTSHARED	This resource can not be shared by this domain.


NOTE
Shared memory is the only MRAPI primitive that is always shareable 
across domains.  Which nodes it is shared with is specified in 
the call to mrapi_shmem_create().

***********************************************************************/
mrapi_shmem_hndl_t mrapi_shmem_get(
 	MRAPI_IN mrapi_shmem_id_t shmem_id,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_shmem_hndl_t shmem;


 
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_shmem_exists(shmem_id)) {
    *status = MRAPI_ERR_SHM_ID_INVALID;
  } else if (mrapi_impl_shmem_get(&shmem,shmem_id /*key */)) {
    *status = MRAPI_SUCCESS;
  } else {
    *status = MRAPI_ERR_NODE_NOTINIT;
  }
  return shmem;
}

/************************************************************************
mrapi_shmem_attach

DESCRIPTION
This function attaches the caller to the shared memory segment and returns its address.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_SHM_INVALID Argument is not a valid shared memory segment handle.
MRAPI_ERR_SHM_ATTACHED	The calling node is already attached to the shared memory.

NOTE

***********************************************************************/
void* mrapi_shmem_attach(
 	MRAPI_IN mrapi_shmem_hndl_t shmem,
 	MRAPI_OUT mrapi_status_t* status)
{
  void* addr = NULL;

  *status = MRAPI_ERR_SHM_INVALID; 
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_shmem_hndl(shmem)) {
    *status = MRAPI_ERR_SHM_INVALID;
  } else if (mrapi_impl_shmem_attached(shmem)) {
    *status = MRAPI_ERR_SHM_ATTACHED;
  } else {
    addr = mrapi_impl_shmem_attach(shmem);
    if ( addr != NULL) {
      *status = MRAPI_SUCCESS;
    } 
  }
  return addr;
}

/************************************************************************
mrapi_shmem_detach

DESCRIPTION
This function detaches the caller from the shared memory segment. 
 All nodes must detach before any node can delete the memory.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_SHM_INVALID Argument is not a valid shared memory segment handle.
MRAPI_ERR_SHM_NOTATTACHED	The calling node is not attached to the shared memory.


NOTE

***********************************************************************/
void mrapi_shmem_detach(
 	MRAPI_IN mrapi_shmem_hndl_t shmem,
 	MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_ERR_SHM_INVALID;
  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_valid_shmem_hndl(shmem)) {
  } else if (!mrapi_impl_shmem_attached(shmem)) {
    *status = MRAPI_ERR_SHM_NOTATTACHED;
  } else if (mrapi_impl_shmem_detach(shmem)) {
    *status = MRAPI_SUCCESS;
  }
}

/************************************************************************
mrapi_shmem_delete

DESCRIPTION
This function deletes the shared memory segment if there are 
no nodes still attached to it.  All nodes must detach before 
any node can delete the memory.  Otherwise, delete will fail 
and there are no automatic retries nor deferred delete.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_SHM_INVALID
Argument is not a valid shared memory segment handle.
MRAPI_ERR_SHM_ATTCH
There are nodes still attached to this shared memory segment thus it could not be deleted.


NOTE

***********************************************************************/
void mrapi_shmem_delete(
 	MRAPI_IN mrapi_shmem_hndl_t shmem,
 	MRAPI_OUT mrapi_status_t* status)
{

  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_shmem_hndl(shmem)) {
    *status = MRAPI_ERR_SHM_INVALID;
  }  else if (mrapi_impl_shmem_delete (shmem)) {
    *status = MRAPI_SUCCESS;
  } 
}


/************************************************************************
mrapi_rmem_create

SYNOPSIS
mrapi_rmem_hndl_t mrapi_rmem_create(
        MRAPI_IN mrapi_rmem_id_t rmem_id,
        MRAPI_IN void* mem,
        MRAPI_IN mrapi_rmem_atype_t access_type,
        MRAPI_IN mrapi_rmem_attributes_t* attributes,
        MRAPI_IN mrapi_uint_t size,
        MRAPI_OUT mrapi_status_t* status
);

DESCRIPTION
This function promotes a private or shared memory segment on the calling node 
to a remote memory segment and returns a handle.  The mem parameter is a pointer 
to the base address of the local memory buffer (see Section 3.5.2).  Once a 
memory segment has been created, its attributes may not be changed.  If the 
attributes are NULL, then implementation defined default attributes will be 
used.  If rmem_id is set to MRAPI_RMEM_ID_ANY, then MRAPI will choose an 
internal id.  access_type specifies access semantics.  Access semantics are 
per remote memory buffer instance, and are either strict (meaning all clients
 must use the same access type), or any (meaning that clients may use any type 
supported by the MRAPI implementation).  Implementations may define multiple 
access types (depending on underlying silicon capabilities), but must provide 
at minimum: MRAPI_RMEM_ATYPE_ANY (which indicates any semantics), and  
MRAPI_RMEM_ATYPE_DEFAULT, which has strict semantics  Note that MRAPI_RMEM_ATYPE_ANY 
is only valid for remote memory buffer creation, clients must use 
MRAPI_RMEM_ATYPE_DEFAULT or another specific type of access mechanism provided 
by the MRAPI implementation (DMA, etc.)   Specifying any type of access (even default) 
other than MRAPI_RMEM_ATYPE_ANY forces strict mode.  The access type is explicity 
passed in to create rather than being an attribute because it is so system specific, 
there is no easy way to define an attribute with a default value.

RETURN VALUE
On success a remote memory segment handle is returned, the address is filled in and 
*status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error 
defined below.  In the case where the remote memory segment already exists, status 
will be set to MRAPI_EXISTS and the handle returned will not be a valid handle.

ERRORS
MRAPI_ERR_RMEM_ID_INVALID The rmem_id is not a valid remote memory segment id.
MRAPI_ERR_RMEM_EXISTS   This remote memory segment is already created.
MRAPI_ERR_MEM_LIMIT No memory available.
MRAPI_ERR_RMEM_TYPENOTVALID     Invalid access_type parameter
MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.
MRAPI_ERR_PARAMETER Incorrect attributes, rmem, or size  parameter.
MRAPI_ERR_RMEM_CONFLICT  The memory pointer + size collides with another remote memory.

NOTE
This function is for promoting a segment of local memory (heap or stack, but stack 
would be dangerous and should be done with care) or an already created shared memory 
segment to rmem, but that also should be done with care.


***********************************************************************/

mrapi_rmem_hndl_t mrapi_rmem_create(
 	MRAPI_IN mrapi_rmem_id_t rmem_id,
 	MRAPI_IN void* mem,
 	MRAPI_IN mrapi_rmem_atype_t access_type,
 	MRAPI_IN mrapi_rmem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t size,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_rmem_hndl_t rmem;

  
  *status = MRAPI_ERR_MEM_LIMIT;
  
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_rmem_validID(rmem_id)) {
    *status = MRAPI_ERR_RMEM_ID_INVALID;
  } else if (mrapi_impl_rmem_exists(rmem_id)) {
    *status = MRAPI_ERR_RMEM_EXISTS;
  } else if (!mrapi_impl_valid_atype(access_type)) {
    *status = MRAPI_ERR_RMEM_TYPENOTVALID;
  } else {
    mrapi_impl_rmem_create (&rmem,rmem_id,mem,access_type,attributes,size,status); 
  }
  return rmem;
}

/************************************************************************
mrapi_rmem_init_attributes

DESCRIPTION
Unless you want the defaults, this call must be used to initialize 
the values of an mrapi_rmem_attributes_t structure prior to mrapi_rmem_set_attribute(). 
 You would then use mrapi_rmem_set_attribute() to change any 
default values prior to calling mrapi_rmem_create().

RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_PARAMETER
Invalid attributes parameter
NOTE

***********************************************************************/
void mrapi_rmem_init_attributes(
                                 MRAPI_OUT mrapi_rmem_attributes_t* attributes,
                                 MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_rmem_init_attributes(attributes);
  }
}

/************************************************************************
mrapi_rmem_set_attribute

DESCRIPTION
This function is used to change default values of an mrapi_rmem_attributes_t 
data structure prior to calling mrapi_rmem_create().  

MRAPI pre-defined remote memory attributes:
Attribute num:	Description:	Datatype:	Default:
MRAPI_DOMAIN_SHARED	Indicates whether or not this remote memory 
is shareable across domains.  	mrapi_boolean_t	MRAPI_TRUE

RETURN VALUE
On success *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_ATTR_READONLY Attribute can not be modified.
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size


NOTE
***********************************************************************/
void mrapi_rmem_set_attribute(
 	MRAPI_OUT mrapi_rmem_attributes_t* attributes,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_IN void* attribute,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  if (attributes == NULL) {
    *status = MRAPI_ERR_PARAMETER;
  } else {
    mrapi_impl_rmem_set_attribute(attributes,attribute_num,attribute,attr_size,status);
  }
}

/************************************************************************
mrapi_rmem_get_attribute

DESCRIPTION
Returns the attribute that corresponds to the given attribute_num 
for this remote memory.  The attributes may be viewed but may 
not be changed (for this remote memory).

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute 
value is filled in.  On error, *status is set to the appropriate 
error defined below and the attribute value is undefined.  The 
attribute identified by the attribute_num is returned in the 
void* attribute parameter.


ERRORS
MRAPI_ERR_PARAMETER Invalid attribute parameter.
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory handle.
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size

NOTE

***********************************************************************/
void mrapi_rmem_get_attribute(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_IN mrapi_uint_t attribute_num,
 	MRAPI_OUT void* attribute,
 	MRAPI_IN size_t attribute_size,
 	MRAPI_OUT mrapi_status_t* status)
{

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (attribute == NULL) {
      *status = MRAPI_ERR_PARAMETER;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  }else {
    mrapi_impl_rmem_get_attribute(rmem,attribute_num,attribute,attribute_size,status);
  }
}

/************************************************************************
mrapi_rmem_get

SYNOPSIS
mrapi_rmem_hndl_t mrapi_rmem_get(
MRAPI_IN mrapi_rmem_id_t rmem_id,
        MRAPI_IN mrapi_rmem_atype_t access_type,
        MRAPI_OUT mrapi_status_t* status
);

DESCRIPTION
Given a rmem_id, this function returns the MRAPI handle referencing to that 
remote memory segment. access_type specifies access semantics.  Access semantics 
are per remote memory buffer instance, and are either strict (meaning all clients 
must use the same access type), or any (meaning that clients may use any type 
supported by the MRAPI implementation).  Implementations may define multiple 
access types (depending on underlying silicon capabilities), but must provide at 
minimum: MRAPI_RMEM_ATYPE_ANY (which indicates any semantics), and  
MRAPI_RMEM_ATYPE_DEFAULT, which has strict semantics  Note that MRAPI_RMEM_ATYPE_ANY 
is only valid for remote memory buffer creation, clients must use MRAPI_RMEM_ATYPE_DEFAULT 
or another specific type of access mechanism provided by the MRAPI implementation (DMA, etc.)  
The access type must match the access type that the memory was created with unless the 
memory was created with the MRAPI_RMEM_ATYPE_ANY type.  See Section 3.5.2 for a 
discussion of remote memory access types.


RETURN VALUE
On success the remote memory segment handle is returned and *status is set to 
MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_ID_INVALID
The rmem_id parameter does not refer to a valid remote memory segment or it was called 
with rmem_id set to MRAPI_RMEM_ID_ANY.
MRAPI_ERR_RMEM_ATYPE_NOTVALID   Invalid access_type parameter.
MRAPI_ERR_NODE_NOTINIT The calling node is not initialized.
MRAPI_ERR_DOMAIN_NOTSHARED This resource can not be shared by this domain.
MRAPI_ERR_RMEM_ATYPE    Type specified on attach is incompatible with type specified on create.


NOTE

**********************************************************************/
mrapi_rmem_hndl_t mrapi_rmem_get(
 MRAPI_IN mrapi_rmem_id_t rmem_id,
 	MRAPI_IN mrapi_rmem_atype_t access_type,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_rmem_hndl_t rmem;


  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_valid_atype(access_type)) {
    *status = MRAPI_ERR_RMEM_TYPENOTVALID;
  } else if (mrapi_impl_rmem_get(&rmem,rmem_id)) {
    *status = MRAPI_SUCCESS;
  } else {
    *status = MRAPI_ERR_RMEM_ID_INVALID;
  }
  return rmem;
}
/************************************************************************
mrapi_rmem_attach

DESCRIPTION
This function attaches the caller to the remote memory segment. 
 Once this is done, the caller may use the mrapi_rmem_read() 
and mrapi_rmem_write() functions.  

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_ATTACHED	The calling node is already attached to the remote memory.


NOTE
***********************************************************************/
void mrapi_rmem_attach(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_IN mrapi_rmem_atype_t access_type,
 	MRAPI_OUT mrapi_status_t* status)
{
  

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  } else if (mrapi_impl_rmem_attach(rmem)) {
    *status = MRAPI_SUCCESS;
  } else {
    *status = MRAPI_ERR_RMEM_TYPENOTVALID;
  }
}





/************************************************************************
mrapi_rmem_detach

DESCRIPTION
This function detaches the caller from the remote memory segment. 
 All attached nodes must detach before any node can delete the 
memory. 

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.

NOTE

***********************************************************************/
void mrapi_rmem_detach(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_ERR_RMEM_INVALID;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_valid_rmem_hndl(rmem) &&
             (mrapi_impl_rmem_detach(rmem))) {
    *status = MRAPI_SUCCESS;
  }
}

/************************************************************************
mrapi_rmem_delete

DESCRIPTION
This function demotes the remote memory segment.  All attached 
nodes must detach before the node can delete the memory.  Otherwise, 
delete will fail and there are no automatic retries nor deferred 
delete.  Note that memory is not de-allocated it is just no longer 
accessible via the MRAPI remote memory function calls.  Only 
the node that created the remote memory can delete it.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_ATTACH
Unable to demote the remote memory because other nodes are still attached to it.
MRAPI_ERR_RMEM_NOTOWNER	The calling node is not the one that created the remote memory.

NOTE

***********************************************************************/
void mrapi_rmem_delete(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_OUT mrapi_status_t* status)
{

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  }  else if (mrapi_impl_rmem_delete (rmem)) {
    *status = MRAPI_SUCCESS;
  } 
}

/************************************************************************
mrapi_rmem_read

DESCRIPTION
This function performs num_strides memory reads, where each read 
is of size bytes_per_access bytes.  The i-th read copies bytes_per_access 
bytes of data from rmem with offset rmem_offset + i*rmem_stride 
to local_buf with offset local_offset + i*local_stride, where 
0 <= i < num_strides.  The local_buf_size represents the number 
of bytes in the local_buf.  

This supports scatter/gather type accesses.  To perform a single 
read, without the need for scatter/gather,  set the num_strides 
parameter to 1.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.

MRAPI_ERR_RMEM_BUFF_OVERRUN	rmem_offset + (rmem_stride * num_strides 
) would fall out of bounds of the remote memory buffer.

MRAPI_ERR_RMEM_STRIDE
num_strides>1 and rmem_stride and/or local_stride are less than bytes_per_access.

MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.

MRAPI_ERR_PARAMETER	Either the local_buf is invalid or the buf_size is zero or bytes_per_access is zero.



NOTE

***********************************************************************/

void mrapi_rmem_read(
                     MRAPI_IN mrapi_rmem_hndl_t rmem,
                     MRAPI_IN mrapi_uint32_t rmem_offset,
                     MRAPI_OUT void* local_buf,
                     MRAPI_IN mrapi_uint32_t local_offset,
                     MRAPI_IN mrapi_uint32_t bytes_per_access,
                     MRAPI_IN mrapi_uint32_t num_strides,
                     MRAPI_IN mrapi_uint32_t rmem_stride,
                     MRAPI_IN mrapi_uint32_t local_stride,
                     MRAPI_OUT mrapi_status_t* status)
{


  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  } else if ((rmem_stride < bytes_per_access) || (local_stride < bytes_per_access)) {
    *status = MRAPI_ERR_RMEM_STRIDE;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  }else {
    mrapi_impl_rmem_read (rmem,rmem_offset,local_buf,local_offset,bytes_per_access,num_strides,rmem_stride,local_stride,status); 
  }
}

/************************************************************************
mrapi_rmem_read_i

DESCRIPTION

This (non-blocking) function performs num_strides memory reads, 
where each read is of size bytes_per_access bytes.  The i-th 
read copies bytes_per_access bytes of data from rmem with offset 
rmem_offset + i*rmem_stride to local_buf with offset local_offset 
+ i*local_stride, where 0 <= i < num_strides. 

This supports scatter/gather type accesses.  To perform a single 
read, without the need for scatter/gather,  set the num_strides 
parameter to 1.



RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below. Use mrapi_test(), 
mrapi_wait() or mrapi_wait_any() to test for completion of the 
operation.

ERRORS
MRAPI_ERR_RMEM_INVALID
Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_BUFF_OVERRUN	rmem_offset + (rmem_stride * num_strides 
) would fall out of bounds of the remote memory buffer.
MRAPI_ERR_RMEM_STRIDE
num_strides>1 and rmem_stride and/or local_stride are less than bytes_per_access.
MRAPI_ERR_REQUEST_LIMIT	No more request handles available.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.
MRAPI_ERR_RMEM_BLOCKED	We have hit a hardware limit of the number 
of asynchronous DMA/cache operations that can be pending ("in 
flight") simultaneously.  Thus we now have to block because the 
resource is busy.
MRAPI_ERR_PARAMETER	Either the local_buf is invalid or the buf_size is zero or bytes_per_access is zero.

NOTE

***********************************************************************/
void mrapi_rmem_read_i(
                       MRAPI_IN mrapi_rmem_hndl_t rmem,
                       MRAPI_IN mrapi_uint32_t rmem_offset,
                       MRAPI_OUT void* local_buf,
                       MRAPI_IN mrapi_uint32_t local_offset,
                       MRAPI_IN mrapi_uint32_t bytes_per_access,
                       MRAPI_IN mrapi_uint32_t num_strides,
                       MRAPI_IN mrapi_uint32_t rmem_stride,
                       MRAPI_IN mrapi_uint32_t local_stride,
                       MRAPI_OUT mrapi_request_t* mrapi_request,
                       MRAPI_OUT mrapi_status_t* status)
{
  

   *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  } else if ((rmem_stride < bytes_per_access) || (local_stride < bytes_per_access)) {
    *status = MRAPI_ERR_RMEM_STRIDE;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  } else {
    mrapi_impl_rmem_read_i (rmem,rmem_offset,local_buf,local_offset,bytes_per_access,num_strides,rmem_stride,local_stride,status,mrapi_request);
  }
}

/************************************************************************
mrapi_rmem_write

DESCRIPTION
This function performs num_strides memory writes, where each 
write is of size bytes_per_access bytes.  The i-th write copies 
bytes_per_access bytes of data from local_buf with offset local_offset 
+ i*local_stride to rmem with offset rmem_offset + i*rmem_stride, 
where 0 <= i < num_strides. 

This supports scatter/gather type accesses.  To perform a single 
write, without the need for scatter/gather,  set the num_strides 
parameter to 1.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RMEM_INVALID
Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_BUFF_OVERRUN	rmem_offset + (rmem_stride * num_strides 
) would fall out of bounds of the remote memory buffer.
MRAPI_ERR_RMEM_STRIDE
num_strides>1 and rmem_stride and/or local_stride are less than bytes_per_access.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.
MRAPI_ERR_PARAMETER	Either the local_buf is invalid  or bytes_per_access is zero.


NOTE

***********************************************************************/
void mrapi_rmem_write(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_IN mrapi_uint32_t rmem_offset,
 	MRAPI_IN void* local_buf,
 	MRAPI_IN mrapi_uint32_t local_offset,
 	MRAPI_IN mrapi_uint32_t bytes_per_access,
  MRAPI_IN mrapi_uint32_t num_strides,
 	MRAPI_IN mrapi_uint32_t rmem_stride,
 	MRAPI_IN mrapi_uint32_t local_stride,
 	MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  } else if ((rmem_stride < bytes_per_access) || (local_stride < bytes_per_access)) {
    *status = MRAPI_ERR_RMEM_STRIDE;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  } else {
    mrapi_impl_rmem_write (rmem,rmem_offset,local_buf,local_offset,bytes_per_access,num_strides,rmem_stride,local_stride,status); 
  }

}

/************************************************************************
mrapi_rmem_write_i

DESCRIPTION
This (non-blocking) function performs num_strides memory writes, 
where each write is of size bytes_per_access bytes.  The i-th 
write copies bytes_per_access bytes of data from local_buf with 
offset local_offset + i*local_stride to rmem with offset rmem_offset 
+ i*rmem_stride, where 0 <= i < num_strides. 

This supports scatter/gather type accesses.  To perform a single 
write, without the need for scatter/gather,  set the num_strides 
parameter to 1.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below. Use mrapi_test(), 
mrapi_wait() or mrapi_wait_any() to test for completion of the 
operation.

ERRORS
MRAPI_ERR_RMEM_INVALID
Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_BUFF_OVERRUN	rmem_offset + (rmem_stride * num_strides 
) would fall out of bounds of the remote memory buffer.
MRAPI_ERR_RMEM_STRIDE
num_strides>1 and rmem_stride and/or local_stride are less than bytes_per_access.
MRAPI_ERR_REQUEST_LIMIT	No more request handles available.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.
MRAPI_ERR_RMEM_BLOCKED	We have hit a hardware limit of the number 
of asynchronous DMA/cache operations that can be pending ("in 
flight") simultaneously.  Thus we now have to block because the 
resource is busy.
MRAPI_ERR_PARAMETER	Either the local_buf is invalid or bytes_per_access is zero.


NOTE

***********************************************************************/
void mrapi_rmem_write_i(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_IN mrapi_uint32_t rmem_offset,
 	MRAPI_IN void* local_buf,
 	MRAPI_IN mrapi_uint32_t local_offset,
 	MRAPI_IN mrapi_uint32_t bytes_per_access,
        MRAPI_IN mrapi_uint32_t num_strides,
 	MRAPI_IN mrapi_uint32_t rmem_stride,
 	MRAPI_IN mrapi_uint32_t local_stride,
 	MRAPI_OUT mrapi_request_t* mrapi_request,
 	MRAPI_OUT mrapi_status_t* status)
{
  

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if ( ! mrapi_impl_valid_rmem_hndl(rmem)) {
    *status = MRAPI_ERR_RMEM_INVALID;
  } else if ((rmem_stride < bytes_per_access) || (local_stride < bytes_per_access)) {
    *status = MRAPI_ERR_RMEM_STRIDE;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  } else {
    mrapi_impl_rmem_write_i (rmem,rmem_offset,local_buf,local_offset,bytes_per_access,num_strides,rmem_stride,local_stride,status,mrapi_request); 
  }
}

/************************************************************************
mrapi_rmem_flush

DESCRIPTION
This function flushes the remote memory.  Support for this function 
is optional and on some systems this may not be supportable. 
 However, if an implementation wants to support coherency back 
to main backing store then this is the way to do it.  Note, that 
this is not an automatic synch back to other viewers of the remote 
data and they would need to also perform a synch, so it is application 
managed coherency.  If writes are synchronizing, then a flush 
will be a no-op.


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below. 

ERRORS
MRAPI_ERR_NOT_SUPPORTED	The flush call is not supported
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.

NOTE

***********************************************************************/
void mrapi_rmem_flush(
 	MRAPI_IN mrapi_rmem_hndl_t rmem,
 	MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_ERR_NOT_SUPPORTED;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  } 
}

/************************************************************************
mrapi_rmem_sync

DESCRIPTION
This function synchronizes the remote memory.  This function 
provides application managed coherency.  It does not guarantee 
that all clients of the rmem buffer will see the updates, see 
corresponding mrapi_rmem_flush().  For some underlying hardware 
this may not be possible.  MRAPI implementation can return an 
error if the synch cannot be performed.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below. 

ERRORS
MRAPI_ERR_NOT_SUPPORTED	The synch call is not supported
MRAPI_ERR_RMEM_INVALID Argument is not a valid remote memory segment handle.
MRAPI_ERR_RMEM_NOTATTACHED	The caller is not attached to the remote memory.

NOTE

***********************************************************************/
void mrapi_rmem_sync(
        MRAPI_IN mrapi_rmem_hndl_t rmem,
        MRAPI_OUT mrapi_status_t* status
        )
{
  *status = MRAPI_ERR_NOT_SUPPORTED;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_rmem_attached(rmem)) {
    *status = MRAPI_ERR_RMEM_NOTATTACHED;
  }
}

/************************************************************************
mrapi_test

DESCRIPTION
mrapi_test() checks if a non-blocking operation has completed. 
 The function returns in a timely fashion.  request is the identifier 
for the non-blocking operation.  

RETURN VALUE
On success, MRAPI_TRUE is returned and *status is set to MRAPI_SUCCESS. 
 If the operation has not completed MRAPI_FALSE is returned and 
*status is set to MRAPI_INCOMPLETE.  On error, MRAPI_FALSE is 
returned and *status is se to the appropriate error defined 
below.

ERRORS
MRAPI_ERR_REQUEST_INVALID	Argument is not a valid request handle.
MRAPI_ERR_REQUEST_CANCELED	The request was canceled.

NOTE

***********************************************************************/
mrapi_boolean_t mrapi_test(
        MRAPI_IN mrapi_request_t* request,
        MRAPI_OUT size_t* size,
        MRAPI_OUT mrapi_status_t* status)
{

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (!mrapi_impl_valid_request_hndl(request)) {
    *status = MRAPI_ERR_REQUEST_INVALID;
  } else  {
    return mrapi_impl_test(request,status);
  }
 return MRAPI_FALSE;
}

/************************************************************************
mrapi_wait

DESCRIPTION
mrapi_wait() waits until a non-blocking operation has completed. 
 It is a blocking function and returns when the operation has 
completed, has been canceled, or a timeout has occurred.  request 
is the identifier for the non-blocking operation.  


RETURN VALUE
On success, status is set to MRAPI_SUCCESS.  On error, *status is set to 
the appropriate error defined below.

ERRORS
MRAPI_ERR_REQUEST_INVALID Argument is not a valid request handle.
MRAPI_ERR_REQUEST_CANCELED	The request was canceled.
MRAPI_TIMEOUT	The operation timed out.

***********************************************************************/
mrapi_boolean_t mrapi_wait(
        MRAPI_IN mrapi_request_t* request,
        MRAPI_OUT size_t* size,
        MRAPI_IN mrapi_timeout_t timeout,
        MRAPI_OUT mrapi_status_t* status)
{
  unsigned i = 0;

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else {
    while(i < timeout) {
      i++;
      if (!mrapi_impl_valid_request_hndl(request)) {
        *status = MRAPI_ERR_REQUEST_INVALID;
        return MRAPI_FALSE;
      } else if (mrapi_impl_canceled_request(request)) {
        *status = MRAPI_ERR_REQUEST_CANCELED;
        return MRAPI_FALSE;
      } else {
        if (mrapi_impl_test(request,status)) {
          *status = MRAPI_SUCCESS;
          return MRAPI_TRUE;
        }
      }
    }
    *status = MRAPI_TIMEOUT;
  }
 return MRAPI_FALSE;
}

/************************************************************************
mrapi_wait_any

DESCRIPTION
mrapi_wait_any() waits until any non-blocking operation of a 
list has completed.  It is a blocking function and returns the 
index into the requests array (starting from 0) indicating which 
of any outstanding operation has completed.  If more than one 
request has completed, it will return the first one it finds. 
 number is the number of requests in the array.  requests is 
the array of mrapi_request_t identifiers for the non-blocking 
operations.  

RETURN VALUE
On success, the index into the requests array of the mrapi_request_t 
identifier that has completed or has been canceled is returned 
and *status is set to MRAPI_SUCCESS.  On error, -1 is returned 
and *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_REQUEST_INVALID Argument is not a valid request handle.
MRAPI_ERR_REQUEST_CANCELED	The request was canceled.
MRAPI_TIMEOUT	The operation timed out.
MRAPI_ERR_PARAMETER	Incorrect number (if=0) requests parameter.

NOTE

***********************************************************************/
mrapi_uint_t mrapi_wait_any(
        MRAPI_IN size_t number,
        MRAPI_IN mrapi_request_t* requests,
        MRAPI_OUT size_t* size,
        MRAPI_IN mrapi_timeout_t timeout,
        MRAPI_OUT mrapi_status_t* status)
{
  mrapi_uint_t i = 0;

  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else {
    *status = MRAPI_TIMEOUT;
    do {
      for (i = 0; i < number; i++) {
        if (!mrapi_impl_valid_request_hndl(&requests[i])) {
          *status = MRAPI_ERR_REQUEST_INVALID;
        } else if (mrapi_impl_canceled_request(&requests[i])) {
          *status = MRAPI_ERR_REQUEST_CANCELED;
        } else if (mrapi_impl_test(&requests[i],status)) {
          return i;
        }
      }
      i++;
    } while (i < timeout);
  }
  return MRAPI_RETURN_VALUE_INVALID;
}

/************************************************************************
mrapi_cancel

DESCRIPTION
mrapi_cancel() cancels an outstanding request.  Any pending calls 
to mrapi_wait() or mrapi_wait_any() for this request will also 
be cancelled. The returned status of a canceled mrapi_wait() 
or mrapi_wait_any() call will indicate that the request was cancelled. 
 Only the node that initiated the request may call cancel.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to 
the appropriate error defined below.

ERRORS
MRAPI_ERR_REQUEST_INVALID Argument is not a valid request handle for this node.

NOTE

***********************************************************************/
void mrapi_cancel(
        MRAPI_IN mrapi_request_t* request,
        MRAPI_OUT mrapi_status_t* mrapi_status)
{
  // In our implementation, requests are satisfied immediately and are thus
  // not cancellable.

  *mrapi_status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *mrapi_status = MRAPI_ERR_NODE_NOTINIT;
  }
}

/************************************************************************
mrapi_resources_get

SYNOPSIS
mrapi_resource_t* mrapi_resources_get(
        MRAPI_IN mrapi_rsrc_filter_t subsystem_filter,
        MRAPI_OUT mrapi_status_t* status
);

DESCRIPTION
mrapi_resources_get() returns a tree of system resources available to the calling node, at the point in time when it is called (this is dynamic in nature).  mrapi_resource_get_attribute() can be used to make a specific query of an attribute of a specific system resource. subsystem_filter is an enumerated type that is used as a filter indicating the scope of the desired information MRAPI returns.  See Section 3.6.1 for a description of how to navigate the resource tree as well as section 6.1 for an example use case.

The valid subsystem filters are:
        MRAPI_RSRC_MEM, MRAPI_RSRC_CACHE, MRAPI_RSRC_CPU

RETURN VALUE
On success, returns a pointer to the root of a tree structure containing the available system resources, and *status is set to MRAPI_SUCCESS.  On error, MRAPI_NULL is returned and *status is set to the appropriate error defined below.  The memory associated with the data structures returned by this function is system managed and must be released via a call to mrapi_resource_tree_free().

ERRORS
MRAPI_ERR_RSRC_INVALID_SUBSYSTEM        Argument is not a valid subsystem enum value.

***********************************************************************/
mrapi_resource_t* mrapi_resources_get(
                                      MRAPI_IN mrapi_rsrc_filter_t subsystem_filter,
                                      MRAPI_OUT mrapi_status_t* status)
{
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
    return NULL;
  } else {
    return mrapi_impl_resources_get(subsystem_filter, status);
  }
}


/************************************************************************
mrapi_resource_get_attribute

SYNOPSIS
void mrapi_resource_get_attribute(
        MRAPI_IN mrapi_resource_t* resource,
        MRAPI_IN mrapi_uint_t attribute_num,
        MRAPI_OUT void* attribute,
        MRAPI_IN size_t attribute_size,
        MRAPI_OUT mrapi_status_t* status
);

DESCRIPTION
mrapi_resource_get_attribute() returns the attribute value at the point in time when this function is called (the value of an attribute may be dynamic in nature), given the input resource and attribute number.  resource is a pointer to the respective resource, attribute_num is the number of the attribute to query for that resource, and attribute_size is the size of the attribute.  Resource attributes are read-only.  Attribute numbers are assigned by the MRAPI implementation and are specific to the given resource type (see Section 3.6.1).

The tables below show the valid attribute_nums for each type of resource:

type of mrapi_resource_t = MRAPI_RSRC_MEM
attribute_num:  datatype:
MRAPI_RSRC_MEM_BASEADDR mrapi_addr_t
MRAPI_RSRC_MEM_WORDSIZE mrapi_uint_t
MRAPI_RSRC_MEM_NUMWORDS mrapi_uint_t

type of mrapi_resource_t = MRAPI_RSRC_CACHE
attribute_num:  datatype:
TO BE FILLED IN TO BE FILLED IN

type of mrapi_resource_t = MRAPI_RSRC_CPU
attribute_num:  datatype:
TO BE FILLED IN TO BE FILLED IN

RETURN VALUE
On success *status is set to MRAPI_SUCCESS and the attribute value is filled in.  On error, *status is set to the appropriate error defined below and the attribute value is undefined.  The attribute identified by the attribute_num is returned in the void* attribute parameter.

ERRORS
MRAPI_ERR_RSRC_INVALID  Invalid resource
MRAPI_ERR_ATTR_NUM Unknown attribute number
MRAPI_ERR_ATTR_SIZE Incorrect attribute size
MRAPI_ERR_PARAMETER  Invlid attribute parameter.

***********************************************************************/
void mrapi_resource_get_attribute(
 	MRAPI_IN mrapi_resource_t* resource,
 	MRAPI_IN mrapi_uint_t attribute_number,
 	MRAPI_OUT void* attribute_value,
 	MRAPI_IN size_t attr_size,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_boolean_t is_static;
  mrapi_boolean_t has_started;
  mrapi_boolean_t get_success;

  /* Check for error conditions */

   
   if  (!mrapi_impl_initialized()) {
     *status = MRAPI_ERR_NODE_NOTINIT;
   } else if (mrapi_impl_valid_attribute_number(resource, attribute_number) == MRAPI_FALSE) {
     *status = MRAPI_ERR_ATTR_NUM;
   } else {
     /* Check if this attribute is dynamic and if has been stated */
     has_started = *(resource->attribute_started[attribute_number]);
     is_static = mrapi_impl_is_static(resource, attribute_number);
     if (is_static == MRAPI_FALSE && has_started == MRAPI_FALSE) {
       *status = MRAPI_ERR_RSRC_NOTSTARTED;
       return;
     }
     
     get_success = mrapi_impl_resource_get_attribute(resource,
                                                     attribute_number,
                                                     attribute_value,
                                                     attr_size,
                                                     status);
     assert(get_success == MRAPI_TRUE);
   }
}
 
/************************************************************************
mrapi_dynamic_attribute_start

DESCRIPTION
mrapi_dynamic_attribute_start() sets the system up to begin collection 
of the attribute's value over time.  resource is a pointer to 
the given resource, attribute_num is the number of the attribute 
to start monitoring for that resource.  Attribute numbers are 
specific to the given resource type.
The rollover_callback is an optional function pointer.  If supplied 
the implementation will call the function when the specified 
attribute value rolls over from its maximum value.  If this callback 
is not supplied the attribute will roll over silently.

If you call stop and then start again, the resource will start 
at it's previous value.  To reset it, call mrapi_dynamic_attribute_reset().


RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the 
appropriate error defined below.

ERRORS
MRAPI_ERR_RSRC_INVALID Invalid resource
MRAPI_ERR_ATTR_NUM Invalid attribute number
MRAPI_ERR_RSRC_NOTDYNAMIC
The input attribute is static and not dynamic, and therefore can't be started.
MRAPI_ERR_RSRC_STARTED
The attribute is dynamic and has already been startedMRAPI_ERR_RSRC_COUNTER_INUSE	
The counter is currently in use by another node.

NOTE

***********************************************************************/
void mrapi_dynamic_attribute_start(
 	MRAPI_IN mrapi_resource_t* resource,
 	MRAPI_IN mrapi_uint_t attribute_number,
	MRAPI_FUNCTION_PTR void (*rollover_callback) (void),
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_boolean_t get_success;

  /* Error checking */

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_valid_attribute_number(resource, attribute_number) == MRAPI_FALSE) {
    *status = MRAPI_ERR_ATTR_NUM;
  } else if (mrapi_impl_is_static(resource, attribute_number) == MRAPI_TRUE) {
    *status = MRAPI_ERR_RSRC_NOTDYNAMIC;
  } else {
    
    get_success = mrapi_impl_dynamic_attribute_start(resource, 
                                                     attribute_number,
                                                     rollover_callback,
                                                     status);
    assert(get_success == MRAPI_TRUE);
  }
}

/************************************************************************
mrapi_dynamic_attribute_reset

DESCRIPTION
mrapi_dynamic_attribute_reset() resets the value of the collected 
dynamic attribute.  resource is the given resource, attribute_num 
is the number of the attribute to reset.  Attribute numbers are 
specific to a given resource type.

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_RSRC_INVALID Invalid resource
MRAPI_ERR_ATTR_NUM Invalid attribute number
MRAPI_ERR_RSRC_NOTDYNAMIC
The input attribute is static and not dynamic, and therefore can't be reset.MRAPI_ERR_RSRC_NOTSTARTED
The attribute is not currently started by the calling node.


NOTE
Some dynamic attributes do not have a defined reset value.  In 
this case, calling mrapi_dynamic_attribute_reset() has no effect.


***********************************************************************/
void mrapi_dynamic_attribute_reset(
 	MRAPI_IN mrapi_resource_t *resource,
 	MRAPI_IN mrapi_uint_t attribute_number,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_boolean_t get_success;
  
  /* Error checking */

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_valid_attribute_number(resource, attribute_number) == MRAPI_FALSE) {
    *status = MRAPI_ERR_ATTR_NUM;
  } else if (mrapi_impl_is_static(resource, attribute_number) == MRAPI_TRUE) {
    *status = MRAPI_ERR_RSRC_NOTDYNAMIC;
  } else {
    get_success = mrapi_impl_dynamic_attribute_reset(resource, attribute_number, status);
    assert(get_success == MRAPI_TRUE);
  }
}


/************************************************************************
mrapi_dynamic_attribute_stop

DESCRIPTION
mrapi_dynamic_attribute_stop() stops the system from collecting 
dynamic attribute values.  resource is the given resource, attribute_num 
is the number of the attribute to stop monitoring.  Attribute 
numbers are specific to a given resource type.  If you call stop 
and then start again, the resource will start at it's previous 
value.  To reset it, call mrapi_dynamic_attribute_reset().

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS.  On error, *status is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_RSRC_INVALID Invalid resource
MRAPI_ERR_ATTR_NUM Invalid attribute number
MRAPI_ERR_RSRC_NOTDYNAMIC
The input attribute is static and not dynamic, and therefore can't be stopped.MRAPI_ERR_RSRC_NOTSTARTED
The attribute is dynamic and has not been started by the calling node.


NOTE

***********************************************************************/
void mrapi_dynamic_attribute_stop(
 	MRAPI_IN mrapi_resource_t* resource,
 	MRAPI_IN mrapi_uint_t attribute_number,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_boolean_t get_success;

  /* Error checking */

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else if (mrapi_impl_valid_attribute_number(resource, attribute_number) == MRAPI_FALSE) {
    *status = MRAPI_ERR_ATTR_NUM;
  } else if (mrapi_impl_is_static(resource, attribute_number) == MRAPI_TRUE) {
    *status = MRAPI_ERR_RSRC_NOTDYNAMIC;
  } else {   
    get_success = mrapi_impl_dynamic_attribute_stop(resource, attribute_number, status);
    assert(get_success == MRAPI_TRUE);
  }
}

/************************************************************************
mrapi_resource_register_callback

DESCRIPTION
mrapi_register_callback() registers an  application-defined function 
to be called when a  specific system event occurs.  The set of 
available events is implementation defined.  Some implementations 
may choose not to define any events and thus not to support this 
functionality.  The frequency parameter is used to indicate the 
reporting frequency for which which an event should trigger the 
callback (frequency is specified in terms of number of event 
occurrences, e.g., callback on every nth occurrence where n=frequency). 
 An example usage of
mrapi_register_callback() could be for notification when the 
core experiences a power management event so that the application 
can determine the cause (manual or automatic) and/or the level 
(nap, sleep, or doze, etc.), and use this information to adjust 
resource usages. 

RETURN VALUE
On success, the callback_function() will be registered for the 
event, and *status is set to MRAPI_SUCCESS.  On error, *status 
is set to the appropriate error defined below.

ERRORS
MRAPI_ERR_RSRC_INVALID_EVENT	Invalid event
MRAPI_ERR_RSRC_INVALID_CALLBACK	Invalid callback function

NOTE

***********************************************************************/
void mrapi_resource_register_callback(
        MRAPI_IN mrapi_event_t event,
        MRAPI_IN unsigned int frequency,
        MRAPI_FUNCTION_PTR void (*callback_function) (mrapi_event_t event),
        MRAPI_OUT mrapi_status_t* status)
{
  /* Cast away the const */
/*   void (*callback_function_nonconst) (mrapi_event_t event); */
/*   callback_function_nonconst = (void (*) (mrapi_event_t event)) callback_function; */

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else {
    mrapi_impl_resource_register_callback(event, frequency,
                                          callback_function, status);
  }
 }

/************************************************************************
mrapi_resource_tree_free

DESCRIPTION
mrapi_resource_tree_free() frees the memory in a resource tree. 
 root is the root of a resource tree originally obtained from 
a call to mrapi_resources_get().

RETURN VALUE
On success, *status is set to MRAPI_SUCCESS and root will be 
set to MRAPI_NULL.  On error, *status is set to the appropriate 
error defined below.

ERRORS
MRAPI_ERR_RSRC_INVALID_TREE Invalid resource tree
MRAPI_ERR_RSRC_NOTOWNER	The calling node is not the same node that originally called mrapi_resources_get().


NOTE
Subsequent usage of root will give undefined results.

***********************************************************************/
void mrapi_resource_tree_free(
 	mrapi_resource_t* MRAPI_IN * root_ptr,
 	MRAPI_OUT mrapi_status_t* status)
{
  mrapi_boolean_t get_success;

  *status = MRAPI_SUCCESS;
  if  (!mrapi_impl_initialized()) {
    *status = MRAPI_ERR_NODE_NOTINIT;
  } else {
    get_success = mrapi_impl_resource_tree_free(root_ptr, status);
  }
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
