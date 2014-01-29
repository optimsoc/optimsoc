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

#include <optimsoc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mtapi.h"
#include "mtapi_runtime.h"

void mtapi_initialize (
	MTAPI_IN mtapi_domain_t domain_id,
	MTAPI_IN mtapi_node_t node_id,
	MTAPI_IN mtapi_node_attributes_t* mtapi_node_attributes,
	MTAPI_OUT mtapi_info_t* mtapi_info,
	MTAPI_OUT mtapi_status_t* status )
{
	mcapi_status_t mcapi_status;
	mcapi_info_t mcapi_info;

	/* initialize MCAPI */
	mcapi_initialize(domain_id,	/* Domain ID */
					 node_id,					  /* Node ID */
					 MCAPI_NULL,				/* Attributes */
					 MCAPI_NULL,				/* Init parameters */
					 &mcapi_info,				/* MCAPI info */
					 &mcapi_status);		/* Status information */
	if (mcapi_status != MCAPI_SUCCESS) {
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}

	/* initialize MTAPI runtime */
	mtapiRT_initialize(domain_id, node_id, mtapi_info, status);
	if (*status != MTAPI_SUCCESS)
		return;

  OPTIMSOC_REPORT(0x505,node_id);

	*status = MTAPI_SUCCESS;
}

void mtapi_node_init_attributes(
  MTAPI_OUT mtapi_node_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status
  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}
 void mtapi_node_set_attribute(
  MTAPI_INOUT mtapi_node_attributes_t* attributes,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status
  )
 {
   *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
   return;
 }

void mtapi_node_get_attribute(
   MTAPI_IN mtapi_node_t node,
   MTAPI_IN mtapi_uint_t attribute_num,
   MTAPI_OUT void* attribute,
   MTAPI_IN size_t attribute_size,
   MTAPI_OUT mtapi_status_t* status
   )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}


void mtapi_finalize ( MTAPI_OUT mtapi_status_t* status )
{
  mcapi_status_t mcapi_status;
  mcapi_finalize(&mcapi_status); //todo: finalize doesn't work!
  mtapiRT_finalize(status);
}

mtapi_domain_t mtapi_domain_id_get( MTAPI_OUT mtapi_status_t* status )
{
	return (mtapi_domain_t) mcapi_domain_id_get( (mtapi_status_t*) status );
}

mtapi_node_t mtapi_node_id_get( MTAPI_OUT mtapi_status_t* status )
{
  return (mtapi_domain_t) mcapi_domain_id_get( (mtapi_status_t*) status );
	//return (mtapi_domain_t) mcapi_node_id_get( (mtapi_status_t*) status );
}

mtapi_job_hndl_t mtapi_job_get( MTAPI_IN mtapi_job_id_t job_id,
                                MTAPI_IN mtapi_domain_t domain_id,
                                MTAPI_OUT mtapi_status_t* status )
{
  if (!mtapiRT_isInitialized()) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    mtapi_job_hndl_t err = { MTAPI_NULL };
    return err;
  }

  mtapi_job_hndl_t jobHndl = { (void*) mtapiRT_getJob( job_id, status) };
  dbg_if( *status != MTAPI_SUCCESS ) {
    *status = MTAPI_ERR_JOB_INVALID;
    mtapi_job_hndl_t err = { MTAPI_NULL };
    return err;
  }

  return jobHndl;
}

mtapi_action_hndl_t mtapi_action_create(
  MTAPI_IN mtapi_job_id_t job_id,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
  if (!mtapiRT_isInitialized()) {
		*status = MTAPI_ERR_NODE_NOTINIT;
		mtapi_action_hndl_t err = { MTAPI_NULL };
		return err;
	}

	mtapi_action_hndl_t actionHndl =
	  { (void*) mtapiRT_createAction( job_id, function, node_local_data,
	                                 node_local_data_size, attributes, status ) };
	if (*status != MTAPI_SUCCESS) {
    mtapi_action_hndl_t err = { MTAPI_NULL };
    return err;
	}

	return actionHndl;
}

void mtapi_action_init_attributes(
  MTAPI_OUT mtapi_action_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status
  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_action_set_attributes(
  MTAPI_INOUT mtapi_action_attributes_t* attributes,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status
  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

extern void mtapi_action_get_attribute(
  MTAPI_IN mtapi_action_hndl_t action,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_OUT void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status
  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_action_delete(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_ActionInfo_t* actionInfo = (mtapiRT_ActionInfo_t*) action.descriptor;

	/* todo: consider timeout */

	/* check if this node doesn't hold the given action */
	switch (actionInfo->type) {
	case MTAPIRT_LOC_LOCAL:
		mtapiRT_deleteLocalAction(actionInfo, timeout, status);
	  *status = MTAPI_SUCCESS;
		break;
	case MTAPIRT_LOC_REMOTE:
		/* todo: remote deletion */
	  *status = MTAPI_SUCCESS;
		break;
	default:
		*status = MTAPI_ERR_ACTION_INVALID;
		break;
	}
}

void mtapi_action_disable(
  MTAPI_IN mtapi_action_hndl_t action,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_action_enable(
  MTAPI_IN mtapi_action_hndl_t action,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_context_status_set(
  MTAPI_INOUT mtapi_task_context_t* task_context,
  MTAPI_IN mtapi_status_t error_code,
  MTAPI_OUT mtapi_status_t* status )
 {
  if (!mtapiRT_isInitialized()) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  switch (task_context->status) {
  case MTAPI_TASK_DELETED:
  case MTAPI_TASK_COMPLETED:
    *status = MTAPI_ERR_CONTEXT_OUTOFCONTEXT;
    return;
  default:
    *status = MTAPI_SUCCESS;
    break;
  }

  task_context->status = error_code;
  return;
 }

void mtapi_context_runtime_notify(
  MTAPI_IN mtapi_task_context_t* task_context,
  MTAPI_IN mtapi_notification_t notification,
  MTAPI_IN void* data,
  MTAPI_IN mtapi_size_t data_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

mtapi_task_state_t mtapi_context_taskstate_get(
  MTAPI_IN mtapi_task_context_t* task_context,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return MTAPI_NULL;
}

mtapi_uint_t mtapi_context_instnum_get(
  MTAPI_IN mtapi_task_context_t* task_context,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return MTAPI_NULL;
}

mtapi_uint_t mtapi_context_numinst_get(
  MTAPI_IN mtapi_task_context_t* task_context,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return MTAPI_NULL;
}

mtapi_uint_t mtapi_context_corenum_get(
  MTAPI_IN mtapi_task_context_t* task_context,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return MTAPI_NULL;
}

void mtapi_action_init_cores(
  MTAPI_OUT mtapi_affinity_t* mask,
  MTAPI_IN mtapi_boolean_t affinity,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_affinity_set_core(
  MTAPI_OUT mtapi_affinity_t* mask,
  MTAPI_IN mtapi_uint_t core_num,
  MTAPI_IN mtapi_boolean_t affinity,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_affinity_isset_core(
  MTAPI_OUT mtapi_affinity_t* mask,
  MTAPI_IN mtapi_uint_t core_num,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

mtapi_queue_hndl_t mtapi_queue_create(
  MTAPI_IN mtapi_queue_id_t queue_id,
  MTAPI_IN mtapi_action_hndl_t* actions,
  MTAPI_IN mtapi_int_t num_actions,
  MTAPI_IN mtapi_queue_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  mtapi_queue_hndl_t err = { MTAPI_NULL };
  return err;
}

void mtapi_queue_init_attributes(
  MTAPI_IN mtapi_queue_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_queue_set_attribute(
  MTAPI_INOUT mtapi_queue_attributes_t* attributes,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}


void mtapi_queue_get_attribute(
  MTAPI_IN mtapi_queue_hndl_t queue,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

mtapi_queue_hndl_t mtapi_queue_get(
  MTAPI_IN mtapi_queue_id_t* queue_id,
  MTAPI_IN mtapi_domain_t domain_id,
  MTAPI_IN mtapi_node_t node_id,
  MTAPI_OUT mtapi_status_t* status
  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  mtapi_queue_hndl_t err = { MTAPI_NULL };
  return err;
}

void mtapi_queue_delete(
  MTAPI_IN mtapi_queue_hndl_t queue,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_queue_disable(
  MTAPI_IN mtapi_queue_hndl_t queue,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_queue_enable(
  MTAPI_IN mtapi_queue_hndl_t queue,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

mtapi_group_hndl_t matpi_group_create(
	MTAPI_IN mtapi_group_id_t group_id,
	MTAPI_IN mtapi_group_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_GroupInfo_t* tmp;

	/* check group id */
	for (tmp = _groups; tmp != MTAPI_NULL; tmp = tmp->next) {
		if (tmp->id == group_id) {
			*status = MTAPI_ERR_GROUP_INVALID;
		  mtapi_group_hndl_t err = { MTAPI_NULL };
		  return err;
		}
	}

	/* create group */
	tmp = (mtapiRT_GroupInfo_t*) malloc(sizeof(mtapiRT_GroupInfo_t));
	tmp->id = group_id;
	tmp->attributes = attributes;
	tmp->next = MTAPI_NULL;

	if (_lastGroup == MTAPI_NULL)
		_groups = _lastGroup = tmp;
	else {
		_lastGroup->next = tmp;
		_lastGroup = tmp;
	}


  mtapi_group_hndl_t groupHndl = { tmp };
	*status = MTAPI_SUCCESS;
	return groupHndl;
}

void mtapi_group_init_attributes(
  MTAPI_OUT mtapi_group_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_group_set_attribute(
  MTAPI_INOUT mtapi_group_attributes_t* attributes,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status  )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_group_get_attribute(
  MTAPI_IN mtapi_group_hndl_t task,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_group_wait_all(
  MTAPI_IN mtapi_group_hndl_t group,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_group_wait_any(
  MTAPI_IN mtapi_group_hndl_t group,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_group_delete(
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_OUT mtapi_status_t* status )
{
	//mtapiRT_GroupInfo_t* groupInfo = (mtapiRT_GroupInfo_t*) group.descriptor;
}

mtapi_task_hndl_t mtapi_task_start(
		MTAPI_IN mtapi_task_id_t task_id,
		MTAPI_IN mtapi_job_hndl_t jobHndl,
		MTAPI_IN void* arguments,
		MTAPI_IN mtapi_size_t arguments_size,
		MTAPI_OUT void* result_buffer,
		MTAPI_IN size_t result_size,
		MTAPI_IN mtapi_task_attributes_t* attributes,
		MTAPI_IN mtapi_group_hndl_t group,
		MTAPI_OUT mtapi_status_t* status )
{
  mtapi_node_t myID;
	mtapiRT_ActionInfo_t* action;
	mtapiRT_TaskInfo_t* task;

	myID = mtapi_node_id_get( status );
	if ( *status != MTAPI_SUCCESS ) {
	  mtapi_task_hndl_t err = { MTAPI_NULL };
	  return err;
	}

	action = mtapiRT_getBestAction((mtapiRT_JobInfo_t*)jobHndl.descriptor,status);
  if (*status != MTAPI_SUCCESS) {
    mtapi_task_hndl_t err = { MTAPI_NULL };
    return err;
  }

	/* distinguish the location of the action */
	switch (action->type) {
	case MTAPIRT_LOC_LOCAL:
	  task = mtapiRT_startLocalTask( task_id, action, arguments, arguments_size,
	                                 result_buffer, result_size, attributes,
	                                 status );
		break;
	case MTAPIRT_LOC_REMOTE:
		task = mtapiRT_startRemoteTask( task_id, action, arguments, arguments_size,
		                                result_buffer, result_size, attributes,
		                                status );
		break;
	default:
		*status = MTAPI_ERR_TASK_CANCELLED; /* todo: wrong error code */
    mtapi_task_hndl_t err = { MTAPI_NULL };
		return err;
	}
	/* check correct task creation */
	if (*status != MTAPI_SUCCESS) {
		*status = MTAPI_ERR_TASK_CANCELLED; /* todo: wrong error code */
		mtapi_task_hndl_t err = { MTAPI_NULL };
		return err;
	}

	mtapi_task_hndl_t task_hndl = { (void*) task };
	*status = MTAPI_SUCCESS;
	return task_hndl;
}

void mtapi_task_cancel(
  MTAPI_IN mtapi_task_hndl_t task,
  MTAPI_OUT mtapi_status_t* status )
{
  mtapiRT_TaskInfo_t* taskInfo = (mtapiRT_TaskInfo_t*) task.descriptor;

  /* distinguish the location of the action */
  switch (taskInfo->action->type) {
  case MTAPIRT_LOC_LOCAL:
    mtapiRT_cancelLocalTask(taskInfo, status);
    break;
  case MTAPIRT_LOC_REMOTE:
    mtapiRT_cancelRemoteTask(taskInfo, status);
    break;
  default:
    *status = MTAPI_ERR_TASK_INVALID;
    break;
  }

}

void mtapi_task_wait(
	MTAPI_IN mtapi_task_hndl_t task,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_TaskInfo_t* taskInfo = (mtapiRT_TaskInfo_t*) task.descriptor;

	/* distinguish the location of the action */
	switch (taskInfo->action->type) {
	case MTAPIRT_LOC_LOCAL:
		mtapiRT_waitForLocalTask(taskInfo, timeout, status);
		break;
	case MTAPIRT_LOC_REMOTE:
		mtapiRT_waitForRemoteTask(taskInfo, timeout, status);
		break;
	default:
		*status = MTAPI_ERR_TASK_INVALID;
		break;
	}
}

mtapi_task_hndl_t mtapi_task_enqueue(
  MTAPI_IN mtapi_task_id_t task_id,
  MTAPI_IN mtapi_queue_hndl_t queue,
  MTAPI_IN void* arguments,
  MTAPI_IN mtapi_size_t arguments_size,
  MTAPI_OUT void* result_buffer,
  MTAPI_IN size_t result_size,
  MTAPI_IN mtapi_task_attributes_t* attributes,
  MTAPI_IN mtapi_group_hndl_t group,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  mtapi_task_hndl_t err = { MTAPI_NULL };
  return err;
}

void mtapi_task_init_attributes(
  MTAPI_OUT mtapi_task_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_task_set_attribute(
  MTAPI_INOUT mtapi_task_attributes_t* attributes,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

void mtapi_task_get_attribute(
  MTAPI_IN mtapi_task_hndl_t task,
  MTAPI_IN mtapi_uint_t attribute_num,
  MTAPI_IN void* attribute,
  MTAPI_IN size_t attribute_size,
  MTAPI_OUT mtapi_status_t* status )
{
  *status = MTAPI_ERR_FUNC_NOT_IMPLEMENTED;
  return;
}

char* mtapi_display_status(
		mtapi_status_t status, char* status_message, size_t size) {
	if (status_message == MTAPI_NULL) {
		fprintf(stderr,"ERROR: status_message must not be NULL.\n");
		return status_message;
	}
	memset(status_message,0,size);
	switch (status) {
	  case (MTAPI_SUCCESS): return strcpy(status_message,"MTAPI_SUCCESS");
	  default : return strcpy(status_message,"UNKNOWN");
	};
	return MTAPI_NULL;
}

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
