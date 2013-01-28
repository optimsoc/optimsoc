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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
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
	mcapi_initialize(domain_id,					/* Domain ID */
					 node_id,					/* Node ID */
					 MCAPI_NULL,				/* Attributes */
					 MCAPI_NULL,				/* Init parameters */
					 &mcapi_info,				/* MCAPI info */
					 &mcapi_status);			/* Status information */
	if (mcapi_status != MCAPI_SUCCESS) {
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}

	/* initialize MTAPI runtime */
	mtapiRT_Initialize(domain_id, node_id, status);
	if (*status != MTAPI_SUCCESS)
		return;
	/* TODO: start listener */

	*status = MTAPI_SUCCESS;
	_initialized = MTAPIRT_TRUE;
}

void mtapi_synchronize (
	MTAPI_IN mtapi_domain_t domain_id,
	MTAPI_IN mtapi_node_t node_id,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_synchronize(domain_id, node_id, timeout, status);
}

mtapi_domain_t mtapi_domain_id_get( MTAPI_OUT mtapi_status_t* status )
{
	return (mtapi_domain_t) mcapi_domain_id_get( (mtapi_status_t*) status );
}

mtapi_node_t mtapi_node_id_get( MTAPI_OUT mtapi_status_t* status )
{
	return (mtapi_domain_t) mcapi_node_id_get( (mtapi_status_t*) status );
}

mtapi_action_hndl_t mtapi_action_create(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapi_action_hndl_t actionHndl;

	if (!_initialized) {
		*status = MTAPI_ERR_NODE_NOTINIT;
		return MTAPI_NULL;
	}

	/* check action */
	if (action_id < 0 || action_id >= NUM_ACTIONS ) {
		*status = MTAPI_ERR_ACTION_INVALID;
		return MTAPI_NULL;
	}

	/* create action handle */
	actionHndl = (mtapi_action_hndl_t)
						malloc(sizeof(struct mtapi_action_hndl));

	/* create action by runtime */
	actionHndl->action_descriptor = (void*) mtapiRT_createAction(
				action_id, function, node_local_data, node_local_data_size,
				attributes, actionHndl, status);

	if (*status != MTAPIRT_SUCCESS) {
		free (actionHndl);
		return MTAPI_NULL;
	}

	/* return handle for action */
	return actionHndl;
}

mtapi_action_hndl_t mtapi_action_get(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_OUT mtapi_status_t* status )
{

	/* check action id */
	if (action_id < 0 || action_id >= NUM_ACTIONS ||
		_actions[action_id].status == MTAPIRT_ACTION_NULL ) {
		*status = MTAPI_ERR_ACTION_INVALID;
		return MTAPI_NULL;
	}

	/* return action handle by runtime */
	return mtapiRT_getAction(action_id, status);
}


void mtapi_action_delete(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_ActionInfo_t* actionInfo;
	actionInfo = (mtapiRT_ActionInfo_t*) action->action_descriptor;
	*status = MTAPI_SUCCESS;

	/* todo: consider timeout */

	/* check if this node doesn't hold the given action */
	switch (actionInfo->type) {
	case MTAPIRT_LOC_LOCAL:
		mtapiRT_deleteAction(actionInfo, timeout, status);
		break;
	case MTAPIRT_LOC_REMOTE:
		/* todo: remote deletion */
		break;
	default:
		*status = MTAPI_ERR_ACTION_INVALID;
		break;
	}

	free (action);
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
			return MTAPI_NULL;
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

	*status = MTAPI_SUCCESS;
	return (mtapi_group_hndl_t) tmp;
}

void mtapi_group_delete(
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_GroupInfo_t* tmp;

	for (tmp = _groups; tmp != MTAPI_NULL; tmp = tmp->next) {
		if (tmp->next == (mtapiRT_GroupInfo_t*)group) {
			free (tmp->next);
			tmp->next = tmp->next->next;
			break;
		}
	}
}

mtapi_task_hndl_t mtapi_task_start(
		MTAPI_IN mtapi_task_id_t task_id,
		MTAPI_IN mtapi_action_hndl_t* actions,
		MTAPI_IN mtapi_int_t num_actions,
		MTAPI_IN void* arguments,
		MTAPI_IN mtapi_size_t arguments_size,
		MTAPI_OUT void* result_buffer,
		MTAPI_IN size_t result_size,
		MTAPI_IN mtapi_task_attributes_t* attributes,
		MTAPI_IN mtapi_group_hndl_t group,
		MTAPI_OUT mtapi_status_t* status )
{
	int i;
	int action_id;
	mtapi_action_hndl_t action_hndl;
	mtapi_task_hndl_t task_hndl;

	/* check action id's */
	if (num_actions <= 0) {
		*status = MTAPI_ERR_TASK_CANCELLED; /* todo: wrong error code */
		return MTAPI_NULL;
	}
	for (i=0; i<num_actions; i++) {
		action_id = mtapiRT_getActionID( actions[i], status );
		if (*status != MTAPI_SUCCESS)
			return MTAPI_NULL;
		if (action_id < 0 || action_id >= NUM_ACTIONS ||
			_actions[action_id].status != MTAPIRT_ACTION_CREATED) {
			*status = MTAPI_ERR_TASK_CANCELLED;
			return MTAPI_NULL;
		}
	}

	/* create a new task handle */
	task_hndl = (mtapi_task_hndl_t) malloc(sizeof(struct mtapi_task_hndl));

	/* get action */
	action_hndl = mtapiRT_getBestAction( actions, num_actions );
	action_id = mtapiRT_getActionID( action_hndl, status );
	if (*status != MTAPI_SUCCESS)
		return MTAPI_NULL;

	/* distinguish the location of the action */
	switch (_actions[action_id].type) {
	case MTAPIRT_LOC_LOCAL:
		task_hndl->task_descriptor = (void*) mtapiRT_startLocalTask(
				task_id, action_id, arguments, arguments_size, result_buffer,
				result_size, attributes, status );
		break;
	case MTAPIRT_LOC_REMOTE:
		task_hndl->task_descriptor = (void*) mtapiRT_startRemoteTask(
				task_id, action_id, arguments, arguments_size, result_buffer,
				result_size, attributes, status );
		break;
	default:
		free (task_hndl);
		*status = MTAPI_ERR_TASK_CANCELLED; /* todo: wrong error code */
		return MTAPI_NULL;
	}

	/* check correct task creation */
	if (*status != MTAPI_SUCCESS) {
		free (task_hndl);
		*status = MTAPI_ERR_TASK_CANCELLED; /* todo: wrong error code */
		return MTAPI_NULL;
	}

	*status = MTAPI_SUCCESS;
	return task_hndl;
}

void mtapi_task_wait(
	MTAPI_IN mtapi_task_hndl_t task,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{

	mtapiRT_TaskInfo_t* taskInfo = (mtapiRT_TaskInfo_t*) task->task_descriptor;

	/* distinguish the location of the action */
	switch (_actions[taskInfo->context->action_id].type) {
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
