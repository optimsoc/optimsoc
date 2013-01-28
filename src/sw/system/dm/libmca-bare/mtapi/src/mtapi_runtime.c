/*
 * mtapi_runtime.c
 *
 *  Created on: Oct 30, 2012
 *      Author: ga49qez
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include "mtapi_runtime.h"

/*
 * Private functions
 */
static void _threadFunction ( void* arguments );
/*static uint _getNoTasks( );*/
static void _insertTask( mtapiRT_TaskInfo_t* taskInfo );

/* 
 * Public functions
 */

/*
 * Initialize the MTAPI runtime.
 */
void mtapiRT_Initialize( MTAPI_IN mtapi_domain_t domain_id,
		 	 	 	 	 MTAPI_IN mtapi_node_t node_id,
		 	 	 	 	 mtapi_status_t* status )
{
	int i;
	mcapi_status_t mcapi_status;

	/* initialize variables */
	_groups = MTAPIRT_NULL;
	_lastGroup = MTAPIRT_NULL;
	_tasks = MTAPIRT_NULL;
	_lastTask = MTAPIRT_NULL;

	/* allocate heap memory for actions */
	_actions = (mtapiRT_ActionInfo_t*)
					  malloc(NUM_ACTIONS * sizeof(mtapiRT_ActionInfo_t));

	/* initialize actions */
	for (i=0; i<NUM_ACTIONS; ++i) {
		mutex_init(&_actions[i].mutex);
		_actions[i].status = MTAPIRT_ACTION_NULL;
		_actions[i].running = 0;
	}

	/* create send and receive endpoints */
	_RecvEndpoint = mcapi_endpoint_create(REQUEST_RECV_PORT, &mcapi_status);
	if (mcapi_status != MCAPI_SUCCESS) {
		free (_actions);
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}
	_SendEndpoint = mcapi_endpoint_create(REQUEST_SEND_PORT, &mcapi_status);
	if (mcapi_status != MCAPI_SUCCESS) {
		free (_actions);
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}


	/* set initialized */
	_initialized = MTAPIRT_TRUE;

	*status = MTAPI_SUCCESS;
}

void mtapiRT_synchronize( MTAPI_IN mtapi_domain_t domain_id,
		 	 	 	  	  MTAPI_IN mtapi_node_t node_id,
		 	 	 	  	  MTAPI_IN mtapi_timeout_t timeout,
		 	 	 	  	  mtapi_status_t* status )
{
	mcapi_endpoint_t endpoints[NO_NODES];
	mcapi_request_t request;
	mcapi_status_t mcapi_status;
	mtapiRT_Message_t message;
	size_t size;
	int i;

	/* check if runtime is already initialized */
	if (!_initialized) {
		*status = MTAPI_ERR_UNKNOWN; /* todo: wrong error code */
		return;
	}

	/* send synchronization messages to siblings */
	message.type = SYNC;
	for (i=0; i<(NO_NODES); ++i) {
		if (i != node_id) {

			/* get remote endpoint */
			endpoints[i] = mcapi_endpoint_get(	NODE_INFOS[i].domain,
			                    NODE_INFOS[i].node,
			                    NODE_INFOS[i].port,
							   					MCAPI_TIMEOUT_INFINITE,
							   					&mcapi_status );
			if (mcapi_status != MCAPI_SUCCESS){
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}

			/* send synchronization message */
			mcapi_msg_send_i( _SendEndpoint,
							  endpoints[i],
							  (void*) &message,
							  sizeof(message),
							  MCAPI_MAX_PRIORITY,
							  &request,
							  &mcapi_status );
			if (mcapi_status != MCAPI_SUCCESS){
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}
		}
	}

	/* handle messages from siblings */
	for (i=0; i<(NO_NODES); ++i) {
		if (i != node_id) {

			/* await sync message from all siblings */
			mcapi_msg_recv(	endpoints[i],
							&message,
							sizeof(message),
							&size,
							&mcapi_status);
			if (mcapi_status != MCAPI_SUCCESS){
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}

			if (message.type != SYNC) {
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}

			/* send acknowledgment message */
			message.type = ACK;
			mcapi_msg_send_i( _SendEndpoint,
							  endpoints[i],
							  (void*) &message,
							  sizeof(message),
							  MCAPI_MAX_PRIORITY,
							  &request,
							  &mcapi_status );
			if (mcapi_status != MCAPI_SUCCESS){
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}

			/* receive acknowledge from siblings */
			mcapi_msg_recv(	endpoints[i],
							&message,
							sizeof(message),
							&size,
							&mcapi_status);
			if (mcapi_status != MCAPI_SUCCESS){
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}

			if (message.type != ACK) {
				*status = MTAPI_ERR_UNKNOWN;
				return;
			}
		}
	}

	*status = MTAPI_SUCCESS;
	return;
}

mtapiRT_TaskInfo_t* mtapiRT_startLocalTask(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_TaskInfo_t* taskInfo;

	/* check action */
	if (_actions[action_id].status != MTAPIRT_ACTION_CREATED &&
		_actions[action_id].type != MTAPIRT_LOC_LOCAL){
		*status = MTAPIRT_ERR_TASK_CREATION;
		return MTAPI_NULL;
	}

	/* create a new task information plus corresponding context */
	taskInfo = (mtapiRT_TaskInfo_t*) malloc(sizeof(mtapiRT_TaskInfo_t));
	if (taskInfo == MTAPIRT_NULL) {
		*status = MTAPIRT_ERR_TASK_CREATION;
		return MTAPI_NULL;
	}

	taskInfo->context = (mtapi_task_context_t*)
									malloc(sizeof(mtapi_task_context_t));
	if (taskInfo->context == MTAPI_NULL) {
		free(taskInfo);
		*status = MTAPIRT_ERR_TASK_CREATION;
		return MTAPI_NULL;
	}

	/* register task */
	taskInfo->context->task_id = task_id;
	taskInfo->context->action_id = action_id;
	taskInfo->arguments = arguments;
	taskInfo->arguments_size = arguments_size;
	taskInfo->result_buffer = result_buffer;
	taskInfo->result_size = result_size;
	taskInfo->attributes = attributes;
	taskInfo->next = MTAPIRT_NULL;
	taskInfo->context->status = MTAPI_TASK_SCHEDULED;
	mutex_init(&taskInfo->mutex);

	/* create thread */
	taskInfo->thread = (thread_t) malloc( sizeof(thread_t) );
	if (taskInfo->thread == MTAPIRT_NULL ||
		thread_create( &taskInfo->thread,
					   &_threadFunction,
					   (void*) taskInfo ) ) {
		free (taskInfo->context);
		free (taskInfo);
		*status = MTAPIRT_ERR_TASK_CREATION;
		return MTAPI_NULL;
	}

	/* link new task into task list */
	_insertTask(taskInfo);

	*status = MTAPI_SUCCESS;
	return taskInfo;
}

mtapiRT_TaskInfo_t* mtapiRT_startRemoteTask(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
	/* todo: missing implementation */
	return MTAPI_NULL;
}

void mtapiRT_waitForLocalTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{

	/* check action */
	if (_actions[taskInfo->context->action_id].status ==
			MTAPIRT_ACTION_DELETED) {
		*status = MTAPI_ERR_ACTION_DELETED;
		return;
	}

	switch (taskInfo->context->status) {
	case MTAPI_TASK_CANCELLED:
		*status = MTAPI_ERR_TASK_CANCELLED;
		break;
	case MTAPI_TASK_COMPLETED:
		*status = MTAPI_SUCCESS;
		break;
	case MTAPI_TASK_RUNNING:
	case MTAPI_TASK_SCHEDULED:
	case MTAPI_TASK_WAITING:
		thread_join(taskInfo->thread);
		*status = MTAPI_SUCCESS;
		break;
	default:
		*status = MTAPI_ERR_UNKNOWN;
		break;
	}
}

void mtapiRT_waitForRemoteTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{

}

void mtapiRT_status_error(char* (*func)(mca_status_t, char*, size_t size),
						  mtapi_status_t status,
						  char* where,
						  mtapi_boolean_t doExit)
{
	char msg[100];
	func(status, msg, 100);
    printf("(%s) MTAPI-Error: %s\n", where, msg);
    if (doExit)
    	exit(0);
}

void mtapiRT_error(char* msg, mtapi_boolean_t doExit)
{
    printf("MTAPI-Error: %s\n", msg);
    if (doExit)
    	exit(0);
}

mcapi_endpoint_t mtapiRT_getEndpoint( mtapi_action_id_t action )
{
	if (!_initialized)
		mtapiRT_error("MTAPI is not initialized.", MTAPIRT_TRUE);

	if (_actions[action].status == MTAPIRT_NULL)
		mtapiRT_error("Action is not registered.", MTAPIRT_TRUE);

	if (_actions[action].type != MTAPIRT_LOC_REMOTE)
		mtapiRT_error("Action is not remote.", MTAPIRT_TRUE);

	return _actions[action].location.endpt;
}

void mtapiRT_setEndpoint(	mtapi_action_id_t action,
							mcapi_endpoint_t endpoint)
{
	if (!_initialized)
		mtapiRT_error("MTAPI is not initialized.", MTAPIRT_TRUE);

	if (_actions[action].status != MTAPIRT_NULL)
		mtapiRT_error("The action has been already defined!", MTAPIRT_TRUE);

	if (_actions[action].type != MTAPIRT_LOC_REMOTE)
		mtapiRT_error("Action is not remote.", MTAPIRT_TRUE);

	mutex_lock(&_actions[action].mutex);
	_actions[action].location.endpt = endpoint;
	mutex_unlock(&_actions[action].mutex);
}

mtapiRT_ActionInfo_t* mtapiRT_createAction(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_IN mtapi_action_hndl_t handle,
	MTAPI_OUT mtapi_status_t* status )
{
	mutex_lock(&_actions[action_id].mutex);

	/* check if action is already defined */
	if (_actions[action_id].status != MTAPIRT_ACTION_NULL) {
		mutex_unlock(&_actions[action_id].mutex);
		*status = MTAPI_ERR_ACTION_EXISTS;
		return MTAPIRT_NULL;
	}

	/* insert / modify action */
	_actions[action_id].status = MTAPIRT_ACTION_CREATED;
	_actions[action_id].type = MTAPIRT_LOC_LOCAL;
	_actions[action_id].location.fptr = function;
	_actions[action_id].local_data = node_local_data;
	_actions[action_id].local_data_size = node_local_data_size;
	_actions[action_id].attributes = attributes;
	_actions[action_id].handle = handle;
	mutex_unlock(&_actions[action_id].mutex);

	*status = MTAPI_SUCCESS;
	return &_actions[action_id];
}

mtapi_action_hndl_t mtapiRT_getAction(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_OUT mtapi_status_t* status )
{
	/* check if action is already created */
	if (_actions[action_id].status != MTAPIRT_ACTION_CREATED) {
		*status = MTAPI_ERR_ACTION_INVALID;
		return MTAPIRT_NULL;
	}

	return _actions[action_id].handle;
}

void mtapiRT_deleteAction(
	mtapiRT_ActionInfo_t* actionInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{

	/* check action */
	if (actionInfo->status == MTAPIRT_ACTION_NULL) {
		*status = MTAPI_ERR_ACTION_INVALID;
		return;
	}

	mutex_lock(&actionInfo->mutex);
	actionInfo->status = MTAPIRT_ACTION_DELETED;
	mutex_unlock(&actionInfo->mutex);

	*status = MTAPI_SUCCESS;
}

mtapi_action_id_t mtapiRT_getActionID(
	mtapi_action_hndl_t action_hndl,
	mtapi_status_t* status )
{
	mtapiRT_ActionInfo_t* action =
			(mtapiRT_ActionInfo_t*) action_hndl->action_descriptor;
	if (action->status == MTAPIRT_ACTION_NULL) {
		*status = MTAPI_ERR_ACTION_INVALID;
		return MTAPI_NULL;
	}

	*status = MTAPI_SUCCESS;
	return ( (action - _actions) / sizeof(_actions) );
}

mtapi_action_hndl_t mtapiRT_getBestAction (
			MTAPI_IN mtapi_action_hndl_t *actions,
			mtapi_int_t num_actions )
{
	/* todo: make it more sophisticated */
	return actions[0];
}

/*****************************************************************************
 * Private functions
 *****************************************************************************/
/* Wrapper function for spawning new threads */
static void _threadFunction ( void* arguments )
{
	mtapiRT_TaskInfo_t* taskInfo  = (mtapiRT_TaskInfo_t*) arguments;
	mtapi_action_id_t actionID = taskInfo->context->action_id;

	/* increment run counter of the corresponding action */
	mutex_lock(&_actions[actionID].mutex);
	_actions[actionID].running++;
	mutex_unlock(&_actions[actionID].mutex);

	/* set new task status and lock context */
	taskInfo->context->status = MTAPI_TASK_RUNNING;
	mutex_lock(&taskInfo->mutex);

	/* execute action function with task-parameters */
	_actions[actionID].location.fptr (
			taskInfo->arguments,
			taskInfo->arguments_size,
			taskInfo->result_buffer,
			taskInfo->result_size,
			_actions[actionID].local_data,
			_actions[actionID].local_data_size,
			(mtapi_task_context_t * const) taskInfo->context );

	if (taskInfo->context->status == MTAPI_TASK_RUNNING)
		taskInfo->context->status = MTAPI_TASK_COMPLETED;

	mutex_unlock(&taskInfo->mutex);

	/* decrement run counter of the corresponding action */
	mutex_lock(&_actions[actionID].mutex);
	_actions[actionID].running--;
	mutex_unlock(&_actions[actionID].mutex);
}

/* get number of registered tasks */
/*static uint _getNoTasks() {
	if (_tasks == MTAPIRT_NULL)
		return 0;
	else
		return ( (_lastTask - _tasks) / sizeof(mtapiRT_TaskInfo_t*) + 1);
}*/

/* Do a sorted insertion of a task into the task list */
static void _insertTask( mtapiRT_TaskInfo_t* taskInfo )
{

	/* first task */
	if (_tasks == MTAPIRT_NULL) {
		_tasks = _lastTask = taskInfo;
		return;
	}

	mutex_lock(&_taskListMutex);

	/* otherwise */
	_lastTask->next = taskInfo;
	_lastTask = taskInfo;

	mutex_unlock(&_taskListMutex);
}

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
