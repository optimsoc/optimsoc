/*
 * mtapi_runtime.h
 *
 *  Created on: Oct 30, 2012
 *      Author: ga49qez
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MTAPI_RUNTIME_H_
#define MTAPI_RUNTIME_H_

#include "mcapi.h"
#include "mtapi.h"
#include "mutex.h"
#include "thread.h"

/******************************************************************
           user definitions and constants
 ******************************************************************/
extern const unsigned int NUM_ACTIONS;
extern const unsigned int REQUEST_SEND_PORT;
extern const unsigned int REQUEST_RECV_PORT;
extern const unsigned int LISTENER_SEND_PORT;
extern const unsigned int LISTENER_RECV_PORT;
extern const unsigned int MAX_TIMEOUT;
extern const unsigned int NO_NODES;
extern const mtapi_action_id_t ACTIONS[];

/* Node information. */
typedef struct {
  mcapi_domain_t domain;                  /* Domain ID of the node */
  mcapi_node_t node;                      /* ID of the node */
  mcapi_port_t port;                      /* Port number */
  const mtapi_action_id_t *actions;       /* Defined actions of the node */
} mtapiRT_NodeInfo_t;

extern const mtapiRT_NodeInfo_t NODE_INFOS[];

/******************************************************************************
           definitions and constants
 *****************************************************************************/
#define MTAPIRT_TRUE			1
#define MTAPIRT_FALSE			0
#define MTAPIRT_NULL			0

/*
 * MTAPI runtime status codes
 */
enum mtapiRT_status_codes
{
	MTAPIRT_SUCCESS = 1,				/* Indicates operation was successful */
	MTAPIRT_ERR_TASK_CREATION			/* Error during task creation */
};

/******************************************************************************
           datatypes & data
******************************************************************************/
/* Global constants for the MTAPI runtime system */
typedef enum {
	MTAPIRT_TASK_INITIALIZED = 0
} mtapiRT_constants_t;

/* Action status type */
typedef enum {
	MTAPIRT_ACTION_NULL = 0,			/* Action is not initialized */
	MTAPIRT_ACTION_INITIALIZED,			/* Action is not created */
	MTAPIRT_ACTION_DELETED,				/* Action has been deleted */
	MTAPIRT_ACTION_CREATED				/* Action is created */
} mtapiRT_action_status_t;

/* Enumeration that describes the location type of an action */
typedef enum {
	MTAPIRT_LOC_LOCAL = 0,				/* Action is on the local node */
	MTAPIRT_LOC_REMOTE					/* Action is on a remote node */
} mtapiRT_location_type_t;

/* Internal message constants for runtime communication. */
typedef enum {
	SYNC = 0,			/* Synchronization message */
	ACK					/* Acknowledgment */
} mtapiRT_MessageType_t;

/* Internal message type for runtime communication. */
typedef struct {
	mtapiRT_MessageType_t type;		/* Message type */
} mtapiRT_Message_t;

/*
 * MTAPI-Runtime action information.
 * Describes where and how an action is stored.
 */
typedef struct
{
	mtapiRT_action_status_t status;		/* Status of the action */
	mtapiRT_location_type_t type;		/* Location type of an action */
	union location_t {					/* Location information of the action */
		mtapi_action_function_t fptr;
		mcapi_endpoint_t		endpt;
	} location;
	MTAPI_IN void* local_data;
	mtapi_size_t local_data_size;
	MTAPI_IN mtapi_action_attributes_t* attributes;
	mtapi_action_hndl_t handle;			/* The handle of the action */
	mtapi_uint_t running;				/* Counts the number of running tasks
											with this action */
	mutex_t mutex;						/* Mutex for multithreaded access */
} mtapiRT_ActionInfo_t;

/*
 *  MTAPI-Runtime group information.
 */
typedef struct mtapiRT_GroupInfo mtapiRT_GroupInfo_t;
struct mtapiRT_GroupInfo
{
	mtapi_group_id_t id;					/* Group ID */
	mtapi_action_hndl_t* actions;			/* Set of actions within the
												group */
	mtapi_size_t num_actions;				/* Number of actions within the
												group */
	MTAPI_IN mtapi_group_attributes_t* attributes;	/* Group attributes */
	mtapiRT_GroupInfo_t* next;				/* Pointer to the next element in
												the linked list */
};

/*
 * MTAPI-Runtime task information.
 */
typedef struct mtapiRT_TaskInfo
{
	MTAPI_IN void* arguments;				/* Task arguments */
	 mtapi_size_t arguments_size;			/* Size of arguments */
	MTAPI_OUT void* result_buffer;			/* Result buffer */
	size_t result_size;						/* Result size */
	MTAPI_IN mtapi_task_attributes_t* attributes; 	/* Task attributes */
	mtapi_group_hndl_t group_hndl;			/* Task group handle */
	mtapi_task_context_t* context;			/* Task context(s) */
	thread_t thread;						/* Thread which executes task */
	mutex_t mutex;							/* Mutex for static variable */
	struct mtapiRT_TaskInfo* next;			/* Next pointer */
} mtapiRT_TaskInfo_t;

/*
 * Global runtime variables.
 */
mtapi_boolean_t _initialized;
mcapi_endpoint_t _SendEndpoint;
mcapi_endpoint_t _RecvEndpoint;

/*
 * Action information.
 */
mtapiRT_ActionInfo_t* _actions;

/*
 * Group information.
 */
mtapiRT_GroupInfo_t* _groups;
mtapiRT_GroupInfo_t* _lastGroup;

/*
 * Task Information.
 */
mtapiRT_TaskInfo_t* _tasks;
mtapiRT_TaskInfo_t* _lastTask;
mutex_t _taskListMutex;

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Initialization of the MTAPI runtime.
 */
void mtapiRT_Initialize( MTAPI_IN mtapi_domain_t domain_id,
						 MTAPI_IN mtapi_node_t node_id,
						 mtapi_status_t* status );

/*
 * Initialization of a specific MTAPI node.
 * This function will be called within the initialization phase and is the
 * place for the synchronization between the nodes.
 */
void mtapiRT_synchronize( MTAPI_IN mtapi_domain_t domain_id,
		 	 	 	 	  MTAPI_IN mtapi_node_t node_id,
		 	 	 	 	  MTAPI_IN mtapi_timeout_t timeout,
		 	 	 	 	  mtapi_status_t* status );

/* Creates a MTAPI action */
mtapiRT_ActionInfo_t* mtapiRT_createAction(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_IN mtapi_action_hndl_t handle,
	MTAPI_OUT mtapi_status_t* status );

/* Returns the action handle corresponding to an action id */
mtapi_action_hndl_t mtapiRT_getAction(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_OUT mtapi_status_t* status );

/* Deletes a specific MTAPI action */
void mtapiRT_deleteAction(
	mtapiRT_ActionInfo_t* actionInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status );

/* Returns the action ID of a specific action handle */
mtapi_action_id_t mtapiRT_getActionID(
	mtapi_action_hndl_t action_hndl,
	mtapi_status_t* status );

/* Start a local task that executes an action with given arguments */
mtapiRT_TaskInfo_t* mtapiRT_startLocalTask(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status );

/* Start a local task that executes an action with given arguments */
mtapiRT_TaskInfo_t* mtapiRT_startRemoteTask(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status );

/* Wait for finishing a local task */
void mtapiRT_waitForLocalTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status );

/* Wait for finishing a remote task */
void mtapiRT_waitForRemoteTask(
	mtapiRT_TaskInfo_t* taskInfo,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status );

/* Error handling procedure */
void mtapiRT_error(char*, mtapi_boolean_t doExit);

/* Get the endpoint that implements a certain action */
mcapi_endpoint_t mtapiRT_getEndpoint( mtapi_action_id_t action );

/* Set the endpoint that implements a certain action */
void mtapiRT_setEndpoint(	mtapi_action_id_t, mcapi_endpoint_t );

/* Choose the favorable action regarding to certain conditions */
mtapi_action_hndl_t mtapiRT_getBestAction (
	MTAPI_IN mtapi_action_hndl_t *actions,
	mtapi_int_t num_actions );

/* Status error handling procedure */
void mtapiRT_status_error(char* (*func)(mca_status_t, char*, size_t size),
		mtapi_status_t status, char* where, mtapi_boolean_t doExit);

#endif /* MTAPI_RUNTIME_H_ */

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
