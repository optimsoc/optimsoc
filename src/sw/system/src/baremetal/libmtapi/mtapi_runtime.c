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
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <optimsoc.h>

#include "mtapi_runtime.h"
#include "mca_config.h"

/******************************************************************************
 * Private functions
 *****************************************************************************/

/* Wrapper function for thread calls */
static void _threadFunction ( void* arguments );

/* Inserts a task into the task list */
static void _insertTask( mtapiRT_TaskInfo_t* taskInfo );

/* Deletes a task and removes it from the task list */
static void _deleteTask( mtapiRT_TaskInfo_t* taskInfo );

/* Listener Thread function */
static void _runListener( void* arg );

/* Marshall data from a message to a dynamic sized (void) array. */
static unsigned _marshallMessage(mtapiRT_Message_t *message, char*buffer);

/* Marshall data from a message to a dynamic sized (void) array. */
static mtapi_boolean_t _unmarshallMessage( char* buffer,
                                           unsigned bufferSize,
                                           mtapiRT_Message_t* message );

static void _fillMtapiInfo( MTAPI_OUT mtapi_info_t* mtapi_info );

/*
 * private handler for listener thread
 */

/* Handler for: MTAPIRT_MSG_GET_ACTION  */
static void _lst_getAction( mtapiRT_Message_t* message,
                            mcapi_endpoint_t sendEP,
                            mcapi_endpoint_t remoteEP );

/* Handler for: MTAPIRT_START_TASK  */
static void _lst_startTask( mtapiRT_Message_t* message,
                            mcapi_endpoint_t sendEP,
                            mcapi_endpoint_t remoteEP );

/* Handler for: MTAPIRT_TASK_WAIT  */
static void _lst_waitForTask( mtapiRT_Message_t* message,
                              mcapi_endpoint_t sendEP,
                              mcapi_endpoint_t remoteEP );


/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Initialize the MTAPI runtime.
 */
void mtapiRT_initialize( MTAPI_IN mtapi_domain_t domain_id,
		 	 	 	 	             MTAPI_IN mtapi_node_t node_id,
		 	 	 	 	             MTAPI_OUT mtapi_info_t* mtapi_info,
		 	 	 	 	             mtapi_status_t* status )
{
	int i, j, k;
	mcapi_status_t mcapi_status;
	mtapiRT_ActionInfo_t* aInfo;
	mtapiRT_ActionInfo_t* tmpAInfo;

	dbg_if ( _initialized ) {
	  *status = MTAPI_ERR_NODE_INITIALIZED;
	  return;
	}

	/* determine internal nodeID */
	_intNodeID = -1;
	for (i=0; i<NUM_NODES; ++i) {
	  if ( NODE_INFOS[i].domain == domain_id && NODE_INFOS[i].node == node_id ) {
	    _intNodeID = i;
	    break;
	  }
	}
	dbg_if( _intNodeID == -1 ) {
    *status = MTAPI_ERR_NODE_INITFAILED;
    return;
	}

	/* initialize variables */
	_groups = MTAPI_NULL;
	_lastGroup = MTAPI_NULL;
	_tasks = MTAPI_NULL;
	_lastTask = MTAPI_NULL;
	_listenerActive = 0;
	_taskID = 0;

  /* allocate heap memory for local actions */
  _actions = (mtapiRT_ActionInfo_t*)
        malloc(NODE_INFOS[_intNodeID].num_actions * sizeof(mtapiRT_ActionInfo_t));
  dbg_if(!_actions) {
    *status = MTAPI_ERR_NODE_INITFAILED;
    return;
  }

  /* allocate heap memory for jobs */
  _jobs = (mtapiRT_JobInfo_t*) malloc(NUM_JOBS * sizeof(mtapiRT_JobInfo_t));
  dbg_if(!_jobs) {
    free(_actions);
    *status = MTAPI_ERR_NODE_INITFAILED;
    return;
  }

	/* initialize actions */
	for (i=0; i < NODE_INFOS[_intNodeID].num_actions; i++) {
	  mutex_init(&_actions[i].mutex);
	  _actions[i].jobID = NODE_INFOS[_intNodeID].actions[i];
	  _actions[i].status = MTAPIRT_ACTION_NULL;
	  _actions[i].type = MTAPIRT_LOC_LOCAL;
	  _actions[i].running = 0;
	  _actions[i].next = MTAPI_NULL;
	}

	/* initialize jobs */
	for (i=0; i<NUM_JOBS; ++i) {
		mutex_init(&_jobs[i].mutex);
    _jobs[i].running = 0;
    _jobs[i].actions = MTAPI_NULL;

		/* first local actions */
		for (j=0; j<NODE_INFOS[_intNodeID].num_actions; ++j) {
		  if (_actions[j].jobID == i) {
		    _jobs[i].actions = &_actions[j];
		    break;
		  }
		}

		/* then link actions from other nodes into the job list */
		for (j=0; j<NUM_NODES; ++j) {
		  if (j == _intNodeID) continue;
		  for (k=0; k<NODE_INFOS[j].num_actions; ++k) {

		    if (NODE_INFOS[j].actions[k] != i) continue;
		    aInfo = (mtapiRT_ActionInfo_t*) malloc (sizeof (mtapiRT_ActionInfo_t));
		    aInfo->jobID = i;
        aInfo->type = MTAPIRT_LOC_REMOTE;
        aInfo->location.intNodeID = j;
        aInfo->status = MTAPIRT_ACTION_NULL;
        aInfo->running = 0;
        aInfo->next = MTAPI_NULL;
        if (_jobs[i].actions == MTAPI_NULL)
          _jobs[i].actions = aInfo;
        else {
          tmpAInfo = _jobs[i].actions;
          while ( tmpAInfo->next != MTAPI_NULL )
            tmpAInfo = tmpAInfo->next;
          tmpAInfo->next = aInfo;
        }

		  }
		}
	}

	/* create send and receive endpoints */
	_RecvEndpoint = mcapi_endpoint_create(NODE_INFOS[_intNodeID].req_recv_port,
	                                      &mcapi_status);
	if (mcapi_status != MCAPI_SUCCESS) {
	  free (_actions);
		free (_jobs);
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}
	_SendEndpoint = mcapi_endpoint_create(NODE_INFOS[_intNodeID].req_send_port,
	                                      &mcapi_status);
	if (mcapi_status != MCAPI_SUCCESS) {
		free (_actions);
	  free (_jobs);
		*status = MTAPI_ERR_NODE_INITFAILED;
		return;
	}

	/* start listener thread */
	if (thread_create(&_listenerThread, _runListener, MTAPI_NULL )) {
    free (_actions);
	  free (_jobs);
    *status = MTAPI_ERR_NODE_INITFAILED;
    return;
	}

	/* fill mtapi information */
	_fillMtapiInfo( mtapi_info );

	/* initialize mutexes */
	mutex_init(&_taskListMutex);

	/* set initialized */
	_initialized = MTAPI_TRUE;
	*status = MTAPI_SUCCESS;
}

void mtapiRT_finalize ( MTAPI_OUT mtapi_status_t* status )
{
  int i;
  mtapiRT_ActionInfo_t* action;
  mtapiRT_ActionInfo_t* tmpAction;
  mtapiRT_TaskInfo_t* tmpTask;

  dbg_if( *status != MTAPI_SUCCESS )
    return;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* cancel (and free) tasks */
  tmpTask = _tasks;
  while ( tmpTask != MTAPI_NULL ) {
    switch (tmpTask->action->type) {
    case MTAPIRT_LOC_LOCAL:
      mtapiRT_cancelLocalTask( tmpTask, status );
      break;
    case MTAPIRT_LOC_REMOTE:
      mtapiRT_cancelRemoteTask( tmpTask, status );
      break;
    default:
      *status = MTAPI_ERR_UNKNOWN;
      return;
    }

    _tasks = tmpTask;
    tmpTask = tmpTask->next;
    free( _tasks );
  }

  /* free actions */
  for (i=0; i<NODE_INFOS[_intNodeID].num_actions; ++i)
  {
    action = _actions[i].next;
    while (action != MTAPI_NULL) {
      tmpAction = action;
      action = action->next;
      free( tmpAction );
    }
    free( &_actions[i] );
  }

  /* free jobs */
  for (i=0; i<NUM_JOBS; ++i) {
    free( &_jobs[i] );
  }

  _initialized = 0;
  _listenerActive = 0;
  thread_join(_listenerThread);
  *status = MTAPI_SUCCESS;
}

mtapiRT_TaskInfo_t* mtapiRT_startLocalTask(
	MTAPI_IN mtapi_task_id_t task_id,
	mtapiRT_ActionInfo_t* action,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
	mtapiRT_TaskInfo_t* taskInfo;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

	/* check action */
	dbg_if (action->status != MTAPIRT_ACTION_CREATED ||
		action->type != MTAPIRT_LOC_LOCAL){
		*status = MTAPI_ERR_TASK_INVALID;
		return MTAPI_NULL;
	}

	/* create a new task information plus corresponding context */
	taskInfo = (mtapiRT_TaskInfo_t*) malloc(sizeof(mtapiRT_TaskInfo_t));
	dbg_if (!taskInfo) {
		*status = MTAPI_ERR_TASK_INVALID;
		return MTAPI_NULL;
	}

	taskInfo->context = (mtapi_task_context_t*)
									malloc(sizeof(mtapi_task_context_t));
	dbg_if (taskInfo->context == MTAPI_NULL) {
		free(taskInfo);
		*status = MTAPI_ERR_TASK_INVALID;
		return MTAPI_NULL;
	}

	/* register task */
	taskInfo->task_id = task_id;
	taskInfo->int_task_id = _taskID++;
	taskInfo->action = action;
	taskInfo->arguments = arguments;
	taskInfo->arguments_size = arguments_size;
	taskInfo->result_buffer = result_buffer;
	taskInfo->result_size = result_size;
	taskInfo->attributes = attributes;
  taskInfo->next = MTAPI_NULL;
  taskInfo->prev = MTAPI_NULL;
	taskInfo->context->status = MTAPI_TASK_SCHEDULED;
	mutex_init(&taskInfo->mutex);

	/* create thread */
	taskInfo->thread = (thread_t) malloc( sizeof(thread_t) );
  int rv = thread_create( &taskInfo->thread,
      &_threadFunction,
      (void*) taskInfo );
	dbg_if (taskInfo->thread == MTAPI_NULL || rv ) {
		free (taskInfo->context);
		free (taskInfo);
		*status = MTAPI_ERR_TASK_INVALID;
		return MTAPI_NULL;
	}

	/* link new task into task list */
	_insertTask(taskInfo);

	*status = MTAPI_SUCCESS;
	return taskInfo;
}

mtapiRT_TaskInfo_t* mtapiRT_startRemoteTask(
	MTAPI_IN mtapi_task_id_t task_id,
	mtapiRT_ActionInfo_t* action,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
  mtapiRT_TaskInfo_t* taskInfo;
  mtapiRT_RemoteTaskInfo_t remoteTaskInfo;
  mtapiRT_Message_t package;
  mcapi_endpoint_t remoteEP;
  mcapi_status_t mcapi_status;
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;
  size_t size;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

  /* check action */
  dbg_if ( action->type != MTAPIRT_LOC_REMOTE ){
    *status = MTAPI_ERR_TASK_INVALID;
    return MTAPI_NULL;
  }

  /* create a new task information plus corresponding context */
  taskInfo = (mtapiRT_TaskInfo_t*) malloc( sizeof(mtapiRT_TaskInfo_t) +
                                           arguments_size );
  dbg_if (!taskInfo) {
    *status = MTAPI_ERR_TASK_INVALID;
    return MTAPI_NULL;
  }

  /* register task */
  taskInfo->context = (mtapi_task_context_t*)
                  malloc(sizeof(mtapi_task_context_t));
  dbg_if (taskInfo->context == MTAPI_NULL) {
    free(taskInfo);
    *status = MTAPI_ERR_TASK_INVALID;
    return MTAPI_NULL;
  }

  taskInfo->task_id = task_id;
  taskInfo->action = action;
  taskInfo->arguments = arguments;
  taskInfo->arguments_size = arguments_size;
  taskInfo->result_buffer = result_buffer;
  taskInfo->result_size = result_size;
  taskInfo->attributes = attributes;
  taskInfo->next = MTAPI_NULL;
  taskInfo->prev = MTAPI_NULL;
  taskInfo->context->status = MTAPI_TASK_SCHEDULED;
  taskInfo->thread = MTAPI_NULL;
  mutex_init(&taskInfo->mutex);

  remoteTaskInfo.task_id = task_id;
  remoteTaskInfo.job_id = action->jobID;
  remoteTaskInfo.arguments_size = arguments_size;
  remoteTaskInfo.result_buffer = MTAPI_NULL;
  remoteTaskInfo.result_size = result_size;

  /* request a remote task call */
  dbg_if (*status != MTAPI_SUCCESS) {
    free( taskInfo->context );
    free( taskInfo );
    return MTAPI_NULL;
  }
 remoteEP = mcapi_endpoint_get(
                NODE_INFOS[action->location.intNodeID].domain,
                NODE_INFOS[action->location.intNodeID].node,
                NODE_INFOS[action->location.intNodeID].lst_recv_port,
                MCAPI_TIMEOUT_INFINITE,
                &mcapi_status);
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    free(taskInfo);
    *status = MTAPI_ERR_UNKNOWN;
    return MTAPI_NULL;
  }

  package.type = MTAPIRT_MSG_START_TASK;
  package.sender = _intNodeID;
  package.buffer = (void*) &remoteTaskInfo;
  package.bufferSize = sizeof( mtapiRT_RemoteTaskInfo_t );
  bufferSize = _marshallMessage( &package, buffer );

  /* append arguments to message */
  memcpy ( &buffer[bufferSize],                    /* destination */
           arguments,                              /* source */
           arguments_size );                       /* size */

  /*OPTIMSOC_REPORT(0x657, action->location.intNodeID);
  OPTIMSOC_REPORT(0x658, remoteTaskInfo.job_id);
  OPTIMSOC_REPORT(0x659, *(int*)arguments);*/

  mcapi_msg_send( _SendEndpoint,
                  remoteEP,
                  &buffer,
                  bufferSize + arguments_size,
                  MCAPI_MAX_PRIORITY,
                  &mcapi_status);
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return MTAPI_NULL;
  }

  /* wait on acknowledgment */
  mcapi_msg_recv( _RecvEndpoint,
                  &buffer,
                  MCAPI_MAX_MSG_SIZE,
                  &size,
                  &mcapi_status);
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return MTAPI_NULL;
  }
  _unmarshallMessage(buffer, size, &package);

  dbg_if (package.type != MTAPIRT_MSG_ACK) {
    free(taskInfo);
    *status = MTAPI_ERR_UNKNOWN;
    return MTAPI_NULL;
  }

  /* set action attributes (now we know everything is fine) */
  action->status = MTAPIRT_ACTION_CREATED;

  /* link new task into task list */
  memcpy(&taskInfo->int_task_id, package.buffer, sizeof(mtapiRT_TaskID_t));
  _insertTask(taskInfo);
  *status = MTAPI_SUCCESS;
  return taskInfo;
}

void mtapiRT_cancelLocalTask(
   mtapiRT_TaskInfo_t* taskInfo,
   mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if (taskInfo->action->status != MTAPIRT_ACTION_CREATED ||
          taskInfo->action->type != MTAPIRT_LOC_LOCAL) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return;
  }

  /* cancel task depending on status */
  switch (taskInfo->context->status) {
  case MTAPI_TASK_CANCELLED:
  case MTAPI_TASK_COMPLETED:
    *status = MTAPI_SUCCESS;
    break;
  case MTAPI_TASK_RUNNING:
  case MTAPI_TASK_WAITING:
  case MTAPI_TASK_SCHEDULED:
    taskInfo->context->status = MTAPI_TASK_CANCELLED;
    *status = MTAPI_SUCCESS;
    break;
  default:
    *status = MTAPI_ERR_UNKNOWN;
    break;
  }
}

void mtapiRT_cancelRemoteTask(
   mtapiRT_TaskInfo_t* taskInfo,
   mtapi_status_t* status )
{
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;
  mtapiRT_Message_t message;
  mcapi_endpoint_t remoteEP;
  mtapi_node_t remoteID;
  mcapi_status_t mcapi_status;
  size_t size;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if (taskInfo->action->status != MTAPIRT_ACTION_CREATED ||
          taskInfo->action->type != MTAPIRT_LOC_REMOTE) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return;
  }

  /* send message */
  dbg_if (*status != MTAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }
  remoteID = taskInfo->action->location.intNodeID;
  remoteEP = mcapi_endpoint_get( NODE_INFOS[remoteID].domain,
                                 NODE_INFOS[remoteID].node,
                                 NODE_INFOS[remoteID].lst_recv_port,
                                 MCAPI_TIMEOUT_INFINITE,
                                 &mcapi_status );
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }

  message.sender = mtapi_domain_id_get( status );
  message.type = MTAPIRT_MSG_CANCEL_TASK;
  message.bufferSize = 0;
  bufferSize = _marshallMessage(&message, buffer);
  mcapi_msg_send( _SendEndpoint,
                    remoteEP,
                    &buffer,
                    bufferSize,
                    MCAPI_MAX_PRIORITY,
                    &mcapi_status );
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }

  /* wait on acknowledgment */
  mcapi_msg_recv( remoteEP,
                  &buffer,
                  MCAPI_MAX_MSG_SIZE,
                  &size,
                  &mcapi_status);
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }
  _unmarshallMessage(buffer, size, &message);

  dbg_if ( mcapi_status != MCAPI_SUCCESS ||
           message.type != MTAPIRT_MSG_ACK ){
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }

  taskInfo->context->status = MTAPI_TASK_CANCELLED;
  *status = MTAPI_SUCCESS;
}

void mtapiRT_waitForLocalTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if (taskInfo->action->status != MTAPIRT_ACTION_CREATED ||
          taskInfo->action->type != MTAPIRT_LOC_LOCAL) {
    *status = MTAPI_ERR_TASK_INVALID;
    return;
  }

	switch (taskInfo->context->status) {
	case MTAPI_TASK_CANCELLED:
		*status = MTAPI_ERR_TASK_CANCELLED;
		return;
	case MTAPI_TASK_COMPLETED:
		*status = MTAPI_SUCCESS;
		break;
	case MTAPI_TASK_RUNNING:
	case MTAPI_TASK_SCHEDULED:
	case MTAPI_TASK_WAITING:
		thread_join(taskInfo->thread); /* todo: wait for timeout */
		*status = (taskInfo->context->status == MTAPI_ERR_ARG_SIZE ||
		           taskInfo->context->status == MTAPI_ERR_RESULT_SIZE )
		           ? taskInfo->context->status : MTAPI_SUCCESS;
		break;
	default:
		*status = taskInfo->context->status;
		return;
	}

	_deleteTask( taskInfo );
}

void mtapiRT_waitForRemoteTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;
  mtapiRT_Message_t message;
  mtapiRT_Message_t message2;
  mcapi_endpoint_t remoteEP;
  mtapi_node_t remoteID;
  mcapi_status_t mcapi_status;
  size_t size;
  mtapi_boolean_t ackAvailable = 0;
  int i;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if ( taskInfo->action->status != MTAPIRT_ACTION_CREATED ||
           taskInfo->action->type != MTAPIRT_LOC_REMOTE ) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return;
  }

  /* send message (no timeout => infinite times) */
  remoteID = taskInfo->action->location.intNodeID;
  remoteEP = mcapi_endpoint_get( NODE_INFOS[remoteID].domain,
                                 NODE_INFOS[remoteID].node,
                                 NODE_INFOS[remoteID].lst_recv_port,
                                 MCAPI_TIMEOUT_INFINITE,
                                 &mcapi_status );
  dbg_if (mcapi_status != MCAPI_SUCCESS) {
    *status = MTAPI_ERR_UNKNOWN;
    return;
  }

  /* todo: use timeout */
  while( !ackAvailable ) {
    message.sender = _intNodeID;
    message.type = MTAPIRT_MSG_WAIT_TASK;
    message.buffer = (char*)&taskInfo->int_task_id;
    message.bufferSize = sizeof(taskInfo->int_task_id);
    bufferSize = _marshallMessage(&message, buffer);

    mcapi_msg_send( _SendEndpoint,
                    remoteEP,
                    &buffer,
                    bufferSize,
                    MCAPI_MAX_PRIORITY,
                    &mcapi_status );
    dbg_if (mcapi_status != MCAPI_SUCCESS) {
      *status = MTAPI_ERR_UNKNOWN;
      return;
    }

    /* receive answer */
    mcapi_msg_recv( _RecvEndpoint,
                    &buffer,
                    MCAPI_MAX_MSG_SIZE,
                    &size,
                    &mcapi_status);
    dbg_if (mcapi_status != MCAPI_SUCCESS) {
      *status = MTAPI_ERR_UNKNOWN;
      return;
    }
    _unmarshallMessage(buffer, size, &message2);

    switch (message2.type) {
    case MTAPIRT_MSG_NACK:
      break;
    case MTAPIRT_MSG_ACK:
      memcpy( taskInfo->result_buffer,    /* destination */
              message2.buffer,             /* source */
              taskInfo->result_size );    /* size */

        OPTIMSOC_REPORT(0x540, taskInfo->result_size);
        OPTIMSOC_REPORT(0x541, size);
        /*OPTIMSOC_REPORT(0x558, *(int*)taskInfo->arguments); */
        /*OPTIMSOC_REPORT(0x559, *(int*)taskInfo->result_buffer); */
      ackAvailable = MTAPI_TRUE;
      break;
    default:
      *status = MTAPI_ERR_UNKNOWN;
      return;
    }
  } /* while (!ackAvailable) */

  _deleteTask( taskInfo );
  *status = MTAPI_SUCCESS;
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

mtapiRT_JobInfo_t* mtapiRT_getJob( MTAPI_IN mtapi_job_id_t job_id,
                                   MTAPI_OUT mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

  dbg_if ( job_id < 0 || job_id >= NUM_JOBS ) {
    *status = MTAPI_ERR_JOB_INVALID;
    return MTAPI_NULL;
  }

  *status = MTAPI_SUCCESS;
    return &_jobs[job_id];
}

mtapiRT_ActionInfo_t* mtapiRT_createAction(
  MTAPI_IN mtapi_job_id_t jobID,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

  dbg_if ( jobID < 0 || jobID >= NUM_JOBS ) {
    *status = MTAPI_ERR_JOB_INVALID;
    return MTAPI_NULL;
  }

  mtapiRT_ActionInfo_t* action = _jobs[jobID].actions;
  dbg_if ( action == MTAPI_NULL ) {
    *status = MTAPI_ERR_JOB_INVALID;
    return MTAPI_NULL;
  }
  dbg_if ( action->status != MTAPIRT_ACTION_NULL) {
		*status = MTAPI_ERR_ACTION_EXISTS;
		return MTAPI_NULL;
	}

	/* insert / modify action */
  mutex_lock(&action->mutex);
    action->status = MTAPIRT_ACTION_CREATED;
    action->type = MTAPIRT_LOC_LOCAL;
    action->location.fptr = function;
    action->local_data = node_local_data;
    action->local_data_size = node_local_data_size;
    action->attributes = attributes;
	mutex_unlock(&action->mutex);

	*status = MTAPI_SUCCESS;
	return action;
}

void mtapiRT_deleteLocalAction(
	mtapiRT_ActionInfo_t* action,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if ( action->type != MTAPIRT_LOC_LOCAL ) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return;
  }

  switch ( action->status ) {
  case MTAPIRT_ACTION_NULL:
    *status = MTAPI_ERR_ACTION_INVALID;
    break;
  case MTAPIRT_ACTION_DELETED:
    *status = MTAPI_SUCCESS;
    break;
  default:
    mutex_lock(&action->mutex);
      action->status = MTAPIRT_ACTION_DELETED;
    mutex_unlock(&action->mutex);
    *status = MTAPI_SUCCESS;
    break;
  }
}

void mtapiRT_deleteRemoteAction(
  mtapiRT_ActionInfo_t* action,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status )
{
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;
  mtapiRT_Message_t message;
  mcapi_endpoint_t remoteEP;
  mtapi_node_t remoteID;
  mcapi_status_t mcapi_status;
  size_t size;

  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return;
  }

  /* check action */
  dbg_if ( action->type != MTAPIRT_LOC_REMOTE ) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return;
  }

  switch ( action->status ) {
  case MTAPIRT_ACTION_NULL:
    *status = MTAPI_ERR_ACTION_INVALID;
    break;

  case MTAPIRT_ACTION_DELETED:
    *status = MTAPI_SUCCESS;
    break;

  default:

    /* send message (no timeout => infinite times) */
    remoteID = action->location.intNodeID;
    remoteEP = mcapi_endpoint_get( NODE_INFOS[remoteID].domain,
                                   NODE_INFOS[remoteID].node,
                                   NODE_INFOS[remoteID].lst_recv_port,
                                   MCAPI_TIMEOUT_INFINITE,
                                   &mcapi_status );
    dbg_if (mcapi_status != MCAPI_SUCCESS) {
      *status = MTAPI_ERR_UNKNOWN;
      return;
    }

      message.sender = mtapi_domain_id_get( status );
      message.type = MTAPIRT_MSG_DELETE_ACTION;
      message.buffer = (char*)&action->jobID;
      message.bufferSize = sizeof(action->jobID);
      bufferSize = _marshallMessage(&message, buffer);
      mcapi_msg_send( _SendEndpoint,
                        remoteEP,
                        &buffer,
                        bufferSize,
                        MCAPI_MAX_PRIORITY,
                        &mcapi_status );
      dbg_if (mcapi_status != MCAPI_SUCCESS) {
        *status = MTAPI_ERR_UNKNOWN;
        return;
      }

      /* receive answer */
      mcapi_msg_recv( remoteEP,
                      &buffer,
                      MCAPI_MAX_MSG_SIZE,
                      &size,
                      &mcapi_status);
      dbg_if (mcapi_status != MCAPI_SUCCESS) {
        *status = MTAPI_ERR_UNKNOWN;
        return;
      }
      _unmarshallMessage(buffer, size, &message);

      switch (message.type) {
      case MTAPIRT_MSG_NACK:
        *status = MTAPI_SUCCESS;
        break;
      case MTAPIRT_MSG_ACK:
        mutex_lock(&action->mutex);
        action->status = MTAPIRT_ACTION_DELETED;
        mutex_unlock(&action->mutex);
        *status = MTAPI_SUCCESS;
        break;
      default:
        *status = MTAPI_ERR_UNKNOWN;
        return;
      }
    break;
  }
}

mtapiRT_ActionInfo_t* mtapiRT_getBestAction( mtapiRT_JobInfo_t* job,
                                            mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

  dbg_if( job->actions == MTAPI_NULL ) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return MTAPI_NULL;
  }

  /* Get action with lowest running counter (prefer local action). */
  mtapiRT_ActionInfo_t* tmpAction = job->actions;
  mtapiRT_ActionInfo_t* bestAction = job->actions;

  while (tmpAction != MTAPI_NULL) {
    if (tmpAction->running == 0) {
      bestAction = tmpAction;
      break;
    } else if ( tmpAction->running < bestAction->running ) {
      bestAction = tmpAction;
    }
    tmpAction = tmpAction->next;
  }

  return bestAction;
}

mtapiRT_ActionInfo_t* mtapiRT_getLocalAction( mtapiRT_JobInfo_t* job,
                                              mtapi_status_t* status )
{
  dbg_if ( !_initialized ) {
    *status = MTAPI_ERR_NODE_NOTINIT;
    return MTAPI_NULL;
  }

  dbg_if( job->actions == MTAPI_NULL ||
          job->actions->type != MTAPIRT_LOC_LOCAL ) {
    *status = MTAPI_ERR_ACTION_INVALID;
    return MTAPI_NULL;
  }

  *status = MTAPI_SUCCESS;
  return job->actions;
}

mtapi_boolean_t mtapiRT_isInitialized() {
  return _initialized;
}

//mtapi_boolean_t mtapiRT_isInitialized()
//{
// return _initialized;
//}

/*****************************************************************************
 * Private functions
 *****************************************************************************/

/* Wrapper function for spawning new threads */
static void _threadFunction ( void* arguments )
{
	mtapiRT_TaskInfo_t* taskInfo  = (mtapiRT_TaskInfo_t*) arguments;

	mutex_lock( &_jobs[taskInfo->action->jobID].mutex );
	_jobs[taskInfo->action->jobID].running++;
	mutex_unlock( &_jobs[taskInfo->action->jobID].mutex );
	mutex_lock( &taskInfo->action->mutex );
	  taskInfo->action->running++;
	mutex_unlock( &taskInfo->action->mutex );

	mutex_lock( &taskInfo->mutex );
	  taskInfo->context->status = MTAPI_TASK_RUNNING;

	  /* execute action function with task-parameters */
	  taskInfo->action->location.fptr (
			taskInfo->arguments,
			taskInfo->arguments_size,
			taskInfo->result_buffer,
			taskInfo->result_size,
			taskInfo->action->local_data,
			taskInfo->action->local_data_size,
			(mtapi_task_context_t * const) taskInfo->context );

	  if ( taskInfo->context->status != MTAPI_ERR_RESULT_SIZE ||
	       taskInfo->context->status != MTAPI_ERR_ARG_SIZE )
	    taskInfo->context->status = MTAPI_TASK_COMPLETED;
	mutex_unlock(&taskInfo->mutex);

  mutex_lock( &taskInfo->action->mutex );
    taskInfo->action->running--;
  mutex_unlock( &taskInfo->action->mutex );
	mutex_lock(&_jobs[taskInfo->action->jobID].mutex);
	  _jobs[taskInfo->action->jobID].running--;
	mutex_unlock(&_jobs[taskInfo->action->jobID].mutex);
}

static void _insertTask( mtapiRT_TaskInfo_t* taskInfo )
{
  mutex_lock(&_taskListMutex);
  if (_tasks == MTAPI_NULL) {   /* first task */
	  _tasks = _lastTask = taskInfo;
	}
  else {
	    _lastTask->next = taskInfo;
	    taskInfo->prev = _lastTask;
	    _lastTask = taskInfo;
	}
  mutex_unlock(&_taskListMutex);
}

static void _deleteTask( mtapiRT_TaskInfo_t* taskInfo )
{
  mutex_lock(&_taskListMutex);
  if (taskInfo->prev != MTAPI_NULL)
    taskInfo->prev->next = taskInfo->next;
  if (taskInfo->next != MTAPI_NULL)
    taskInfo->next->prev = taskInfo->prev;
  mutex_unlock(&_taskListMutex);

  mutex_lock(&taskInfo->mutex);
  free(taskInfo->context);
  free(taskInfo);
  mutex_unlock(&taskInfo->mutex);
}

static void _runListener( void* arg )
{
  mcapi_endpoint_t sendEndpoint, recvEndpoint, remoteEndpoint;
  mcapi_status_t mcapi_status;
  mtapiRT_Message_t package;
  size_t msg_size;
  mcapi_uint_t nrMessages;
  char buffer[MCAPI_MAX_MSG_SIZE];

  /* create endpoints */
  recvEndpoint = mcapi_endpoint_create(NODE_INFOS[_intNodeID].lst_recv_port,
                                       &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS);
  sendEndpoint = mcapi_endpoint_create(NODE_INFOS[_intNodeID].lst_send_port,
                                       &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS);

  /* Thread loop for message handling */
  _listenerActive = 1;
  while (_listenerActive) {

    nrMessages = mcapi_msg_available(recvEndpoint, &mcapi_status);
    assert(mcapi_status == MCAPI_SUCCESS);

    if (!nrMessages) {
      thread_yield();
      continue;
    }

    mcapi_msg_recv( recvEndpoint,
                    &buffer,
                    MCAPI_MAX_MSG_SIZE,
                    &msg_size,
                    &mcapi_status);
    assert(mcapi_status == MCAPI_SUCCESS);
    _unmarshallMessage(buffer, msg_size, &package);

    remoteEndpoint = mcapi_endpoint_get(NODE_INFOS[package.sender].domain,
                                        NODE_INFOS[package.sender].node,
                                        NODE_INFOS[package.sender].req_recv_port,
                                        MCAPI_TIMEOUT_INFINITE,
                                        &mcapi_status);
    assert(mcapi_status == MCAPI_SUCCESS);

    /* handle message */
    switch (package.type) {
    case MTAPIRT_MSG_GET_ACTION:
      _lst_getAction( &package, sendEndpoint, remoteEndpoint );
      break;
    case MTAPIRT_MSG_START_TASK:
      _lst_startTask( &package, sendEndpoint, remoteEndpoint );
      break;

    case MTAPIRT_MSG_CANCEL_TASK:

      break;

    case MTAPIRT_MSG_WAIT_TASK:
      _lst_waitForTask( &package, sendEndpoint, remoteEndpoint );
    break;

    default:
      assert(!"Listener thread received an unknown message!");
      break;
    }

    /* free allocated package buffer */
    if (package.buffer)
      free( package.buffer );

  }

  /* delete endpoints */
  //mcapi_endpoint_delete( recvEndpoint, &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS &&
          "Listener thread failed to delete receive endpoint!");
  //mcapi_endpoint_delete( sendEndpoint, &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS &&
          "Listener thread failed to delete send endpoint!");
}

static unsigned _marshallMessage(mtapiRT_Message_t *message, char* buffer)
{
  unsigned i;
  unsigned sizeType = sizeof( message->type );
  unsigned sizeSender = sizeof( message->sender );

  memcpy( &buffer[0], &message->type, sizeType );
  memcpy( &buffer[sizeType], &message->sender, sizeSender );

  for (i=0; i<message->bufferSize; ++i) {
    buffer[i+sizeType+sizeSender] = message->buffer[i];
  }

  return (  sizeType + sizeSender + message->bufferSize );
}

static mtapi_boolean_t _unmarshallMessage(char *buffer,
                                          unsigned bufferSize,
                                          mtapiRT_Message_t* message )
{
  unsigned sizeType = sizeof( message->type );
  unsigned sizeSender = sizeof( message->sender );
  unsigned sum = sizeType + sizeSender;

  message->bufferSize = bufferSize - sum;
  memcpy( &message->type, &buffer[0], sizeType );
  memcpy( &message->sender, &buffer[sizeType], sizeSender );

  // allocate new space for buffer
  char* newBuffer = (char*) malloc( message->bufferSize );
  memcpy( newBuffer, &buffer[sum], message->bufferSize );
  message->buffer = newBuffer;

  return MTAPI_TRUE;
}

static void _fillMtapiInfo( MTAPI_OUT mtapi_info_t* mtapi_info )
{
  mtapi_info->mtapi_version = MTAPI_VERSION;
  mtapi_info->implementation_version = MTAPIRT_MTAPI_VERSION;
  mtapi_info->number_of_domains = NUM_DOMAINS;
  mtapi_info->number_of_nodes = NUM_NODES;
  mtapi_info->organization_id = MTAPIRT_MTAPI_VENDOR;
}

static void _lst_getAction( mtapiRT_Message_t* message,
                            mcapi_endpoint_t sendEP,
                            mcapi_endpoint_t remoteEP )
{
  mtapi_job_id_t jobID;
  mcapi_status_t mcapi_status;
  mtapi_status_t mtapi_status;
  mtapiRT_Message_t answer;
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;

  memcpy(&jobID, message->buffer, sizeof(mtapi_job_id_t));
  mtapiRT_getJob( jobID, &mtapi_status );
  answer.sender = mtapi_domain_id_get( &mtapi_status );
  answer.type = (mtapi_status == MTAPI_SUCCESS) ?
                                    MTAPIRT_MSG_ACK : MTAPIRT_MSG_NACK;

  bufferSize = _marshallMessage(&answer, buffer);
  mcapi_msg_send( sendEP,
                  remoteEP,
                  &buffer,
                  bufferSize,
                  MCAPI_MAX_PRIORITY,
                  &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS);
}

static void _lst_startTask( mtapiRT_Message_t* message,
                                  mcapi_endpoint_t sendEP,
                                  mcapi_endpoint_t remoteEP )
{
  mtapiRT_RemoteTaskInfo_t* rt;
  mtapiRT_Message_t answer;
  mcapi_status_t mcapi_status;
  mtapi_status_t mtapi_status;
  mtapiRT_TaskInfo_t* taskInfo;
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;

  /* allocate heap space for arguments and result buffer */
  rt = (mtapiRT_RemoteTaskInfo_t*) message->buffer;

  void* arguments = malloc(rt->arguments_size);
  rt->result_buffer = malloc(rt->result_size);

  if (!arguments || ! rt->result_buffer) {
    answer.sender = mtapi_domain_id_get( &mtapi_status );
    answer.type = MTAPIRT_MSG_NACK;
    bufferSize = _marshallMessage(&answer, buffer);
    mcapi_msg_send( sendEP,
                    remoteEP,
                    &buffer,
                    bufferSize,
                    MCAPI_MAX_PRIORITY,
                    &mcapi_status);
    assert(mcapi_status == MCAPI_SUCCESS);
    return;
  }

  mtapiRT_ActionInfo_t* action =
      mtapiRT_getLocalAction( &_jobs[rt->job_id], &mtapi_status );
  assert(mtapi_status == MTAPI_SUCCESS);

  /* start local task */
  memcpy ( arguments,                                           /* destination */
           &message->buffer[sizeof(mtapiRT_RemoteTaskInfo_t)],  /* source */
           rt->arguments_size );                                /* size */

  taskInfo = mtapiRT_startLocalTask( rt->task_id,
                                     action,
                                     arguments,
                                     rt->arguments_size,
                                     rt->result_buffer,
                                     rt->result_size,
                                     MTAPI_NULL,
                                     &mtapi_status );
  assert(mtapi_status == MTAPI_SUCCESS);

  answer.sender = _intNodeID;
  answer.type = MTAPIRT_MSG_ACK;
  answer.buffer = (char*)&taskInfo->int_task_id;
  answer.bufferSize = sizeof(taskInfo->int_task_id);
  bufferSize = _marshallMessage(&answer, buffer);

  mcapi_msg_send( sendEP,
                  remoteEP,
                  &buffer,
                  bufferSize,
                  MCAPI_MAX_PRIORITY,
                  &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS);
}

static void _lst_waitForTask( mtapiRT_Message_t* message,
                              mcapi_endpoint_t sendEP,
                              mcapi_endpoint_t remoteEP )
{
  mtapiRT_TaskID_t taskID;
  mtapiRT_Message_t answer;
  mcapi_status_t mcapi_status;
  mtapi_status_t mtapi_status;
  char buffer[MCAPI_MAX_MSG_SIZE];
  unsigned bufferSize;
  int i;

  memcpy( &taskID, message->buffer, sizeof( mtapiRT_TaskID_t ) );

  answer.type = MTAPIRT_MSG_NACK;
  mtapiRT_TaskInfo_t* tInfo = _tasks;
  while (tInfo != MTAPI_NULL) {
    if (tInfo->int_task_id == taskID) {
      mtapiRT_waitForLocalTask( tInfo,
                                MTAPI_TIMEOUT_IMMEDIATE,
                                &mtapi_status );
      if ( mtapi_status != MTAPI_SUCCESS )
        break;

      answer.type = MTAPIRT_MSG_ACK;

      /*OPTIMSOC_REPORT(0x557, *(int*)tInfo->arguments);
      for (i=0; i<tInfo->result_size; ++i) {
        OPTIMSOC_REPORT(0x558, ((char*)tInfo->result_buffer)[i]);
      }*/
      memcpy( answer.buffer, tInfo->result_buffer, tInfo->result_size );
      break;
    }
    tInfo = tInfo->next;
  }

  bufferSize = _marshallMessage( &answer, buffer );
  mcapi_msg_send( sendEP,
                  remoteEP,
                  &buffer,
                  bufferSize,
                  MCAPI_MAX_PRIORITY,
                  &mcapi_status);
  assert(mcapi_status == MCAPI_SUCCESS);
}

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
