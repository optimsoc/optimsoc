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

#include <mcapi.h>
#include <mtapi.h>
#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>

/******************************************************************************
           user definitions and constants
 *****************************************************************************/
extern const unsigned int NUM_NODES;
extern const unsigned int NUM_DOMAINS;
extern const unsigned int NUM_JOBS;
extern const unsigned int MAX_TIMEOUT;
extern const mtapi_job_id_t JOBS[];

/* Node information. */
typedef struct {
  mcapi_domain_t domain;              /* Domain ID */
  mcapi_node_t node;                  /* Node ID */
  mcapi_port_t req_send_port;         /* Port number (send, requested) */
  mcapi_port_t req_recv_port;         /* Port number (recv, requested) */
  mcapi_port_t lst_send_port;         /* Port number (send, listener) */
  mcapi_port_t lst_recv_port;         /* Port number (recv, listener) */
  unsigned num_actions;               /* Number of defined actions */
  const mtapi_action_id_t *actions;   /* Defined actions of the node */
} mtapiRT_NodeInfo_t;

extern const mtapiRT_NodeInfo_t NODE_INFOS[];

/******************************************************************************
           definitions and constants
 *****************************************************************************/
#define MTAPIRT_MTAPI_VERSION 001
#define MTAPIRT_MTAPI_VENDOR  001

#define MTAPIRT_DEBUG         1

#ifdef MTAPIRT_DEBUG
#define dbg_if(expr) \
        assert(!(expr)); if (expr)
#else
#define dbg_if(expr) \
        if (expr)
#endif

/******************************************************************************
           datatypes & data
******************************************************************************/

/* MTAPI-runtime type definitions */
typedef unsigned mtapiRT_TaskID_t;    /* Unique (internal) task identifier */

/* Action status type */
typedef enum {
  MTAPIRT_ACTION_NULL = 0,            /* Action is not initialized */
  MTAPIRT_ACTION_INITIALIZED,         /* Action is not created */
  MTAPIRT_ACTION_DELETED,             /* Action has been deleted */
  MTAPIRT_ACTION_CREATED              /* Action is created */
} mtapiRT_action_status_t;

/* Enumeration that describes the location type of an action */
typedef enum {
  MTAPIRT_LOC_LOCAL = 0,              /* Action is on the local node */
  MTAPIRT_LOC_REMOTE                  /* Action is on a remote node */
} mtapiRT_location_type_t;

/* Internal message constants for runtime communication. */
typedef enum {
  MTAPIRT_MSG_SYNC = 0,               /* Synchronization message */
  MTAPIRT_MSG_ACK,                    /* Acknowledgment */
  MTAPIRT_MSG_NACK,                   /* NoAcknowledgment */
  MTAPIRT_MSG_GET_ACTION,             /* Request for an action handle */
  MTAPIRT_MSG_START_TASK,             /* Start a remote task */
  MTAPIRT_MSG_CANCEL_TASK,            /* Cancel a remote task */
  MTAPIRT_MSG_WAIT_TASK,              /* Wait for a remote task */
  MTAPIRT_MSG_DELETE_ACTION           /* Delete a remote action */
} mtapiRT_MessageType_t;

/* Internal message type for runtime communication. */
typedef struct {
  mtapiRT_MessageType_t type;         /* Message type */
  mtapi_node_t sender;                /* Sender's node ID */
  char* buffer;                       /* Message buffer */
  mtapi_uint_t bufferSize;            /* Size of message buffer */
} mtapiRT_Message_t;

/*
 * MTAPI-Runtime action information.
 */
typedef struct mtapiRT_ActionInfo
{
  mtapiRT_action_status_t status;     /* Status of the action */
  mtapi_job_id_t jobID;               /* Corresponding job id */
  mtapiRT_location_type_t type;       /* Location type of the action */
  union location_t {                  /* Location information of the action: */
    mtapi_action_function_t fptr;     /*  local:  function pointer */
    mtapi_node_t intNodeID;           /*  remote: remote node id (internal) */
  } location;
  MTAPI_IN void* local_data;          /* Local data for execution */
  mtapi_size_t local_data_size;       /* Size of local data for execution */
  MTAPI_IN mtapi_action_attributes_t* attributes; /* Action attributes */
  mtapi_uint_t running;               /* Counts the number of running actions */
  struct mtapiRT_ActionInfo* next;    /* next pointer for job handling */
  mutex_t mutex;                      /* Mutex for multithreaded access */
} mtapiRT_ActionInfo_t;

/*
 * MTAPI-Runtime job information.
 */
typedef struct
{
  mtapiRT_ActionInfo_t* actions;      /* The list of available actions */
  mtapi_uint_t running;               /* Counts the number of running tasks
                                          with this job */
  mutex_t mutex;                      /* Mutex for multithreaded access */
} mtapiRT_JobInfo_t;

/*
 *  MTAPI-Runtime group information.
 */
typedef struct mtapiRT_GroupInfo mtapiRT_GroupInfo_t;
struct mtapiRT_GroupInfo
{
  mtapi_group_id_t id;                /* Group ID */
  mtapi_action_hndl_t* actions;       /* Set of actions within the group */
  mtapi_size_t num_actions;           /* Number of actions within the group */
  MTAPI_IN mtapi_group_attributes_t* attributes;  /* Group attributes */
  mtapiRT_GroupInfo_t* next;          /* Pointer to the next element in
                                          the linked list */
};

/*
 * MTAPI-Runtime task information.
 */
typedef struct mtapiRT_TaskInfo
{
  mtapi_task_id_t task_id;            /* External task id */
  mtapiRT_TaskID_t int_task_id;       /* Internal task id */
  mtapiRT_ActionInfo_t* action;       /* The executable action */
  MTAPI_IN void* arguments;           /* Task arguments */
   mtapi_size_t arguments_size;       /* Size of arguments */
  MTAPI_OUT void* result_buffer;      /* Result buffer */
  size_t result_size;                 /* Result size */
  MTAPI_IN mtapi_task_attributes_t* attributes;   /* Task attributes */
  mtapi_task_context_t* context;      /* Task context(s) */
  thread_t thread;                    /* Thread which executes task */
  mutex_t mutex;                      /* Mutex for static variable */
  struct mtapiRT_TaskInfo* next;      /* Next pointer */
  struct mtapiRT_TaskInfo* prev;      /* Prev pointer */
} mtapiRT_TaskInfo_t;

/*
 * MTAPI-Runtime remote task information (to start remote tasks).
 */
typedef struct mtapiRT_RemoteTaskInfo
{
  mtapi_task_id_t task_id;           /* External task id */
  mtapi_job_id_t job_id;             /* Job id */
  mtapi_size_t arguments_size;       /* Size of arguments */
  MTAPI_OUT void* result_buffer;     /* Result buffer */
  size_t result_size;                /* Result size */
} mtapiRT_RemoteTaskInfo_t;

/*
 * Global runtime variables.
 */
mtapi_boolean_t _initialized;
mcapi_endpoint_t _SendEndpoint;
mcapi_endpoint_t _RecvEndpoint;

/*
 * Node information.
 */
mcapi_node_t _intNodeID;

/*
 * Job information.
 */
mtapiRT_JobInfo_t* _jobs;

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
mtapiRT_TaskID_t _taskID;

/*
 * Listener thread
 */
thread_t _listenerThread;
volatile mtapi_boolean_t _listenerActive;

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Initialization of the MTAPI runtime.
 */
void mtapiRT_initialize( MTAPI_IN mtapi_domain_t domain_id,
             MTAPI_IN mtapi_node_t node_id,
             MTAPI_OUT mtapi_info_t* mtapi_info,
             mtapi_status_t* status );

/*
 * Finalize the MTAPI runtime.
 */
void mtapiRT_finalize ( MTAPI_OUT mtapi_status_t* status );

/* Returns a job handle corresponding to the job id */
mtapiRT_JobInfo_t* mtapiRT_getJob( MTAPI_IN mtapi_job_id_t job_id,
                                   MTAPI_OUT mtapi_status_t* status );

/* Creates a MTAPI action */
mtapiRT_ActionInfo_t* mtapiRT_createAction(
  MTAPI_IN mtapi_job_id_t jobID,
  MTAPI_IN mtapi_action_function_t function,
  MTAPI_IN void* node_local_data,
  MTAPI_IN mtapi_size_t node_local_data_size,
  MTAPI_IN mtapi_action_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status );

/* Deletes a specific (local) MTAPI action */
void mtapiRT_deleteLocalAction(
  mtapiRT_ActionInfo_t* action,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status );

/* Deletes a specific (remote) MTAPI action */
void mtapiRT_deleteRemoteAction(
  mtapiRT_ActionInfo_t* action,
  MTAPI_IN mtapi_timeout_t timeout,
  MTAPI_OUT mtapi_status_t* status );

/* Start a local task that executes an action with given arguments */
mtapiRT_TaskInfo_t* mtapiRT_startLocalTask(
  MTAPI_IN mtapi_task_id_t task_id,
  mtapiRT_ActionInfo_t* action,
  MTAPI_IN void* arguments,
  MTAPI_IN mtapi_size_t arguments_size,
  MTAPI_OUT void* result_buffer,
  MTAPI_IN size_t result_size,
  MTAPI_IN mtapi_task_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status );

/* Start a local task that executes an action with given arguments */
mtapiRT_TaskInfo_t* mtapiRT_startRemoteTask(
  MTAPI_IN mtapi_task_id_t task_id,
  mtapiRT_ActionInfo_t* action,
  MTAPI_IN void* arguments,
  MTAPI_IN mtapi_size_t arguments_size,
  MTAPI_OUT void* result_buffer,
  MTAPI_IN size_t result_size,
  MTAPI_IN mtapi_task_attributes_t* attributes,
  MTAPI_OUT mtapi_status_t* status );

/* Cancel a local task */
void mtapiRT_cancelLocalTask(
   mtapiRT_TaskInfo_t* taskInfo,
   mtapi_status_t* status );

/* Cancel a remote task */
void mtapiRT_cancelRemoteTask(
   mtapiRT_TaskInfo_t* taskInfo,
   mtapi_status_t* status );

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

/* Return whether runtime is initialized */
mtapi_boolean_t mtapiRT_isInitialized();

/* Error handling procedure */
void mtapiRT_error(char*, mtapi_boolean_t doExit);

/* Returns the currently best action implementation of a given jobID.*/
mtapiRT_ActionInfo_t* mtapiRT_getBestAction( mtapiRT_JobInfo_t* job,
                                             mtapi_status_t* status );

/* Returns the local action implementation of a given jobID.*/
mtapiRT_ActionInfo_t* mtapiRT_getLocalAction( mtapiRT_JobInfo_t* job,
                                              mtapi_status_t* status );

/* Status error handling procedure */
void mtapiRT_status_error(char* (*func)(mca_status_t, char*, size_t size),
    mtapi_status_t status, char* where, mtapi_boolean_t doExit);

#endif /* MTAPI_RUNTIME_H_ */

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
