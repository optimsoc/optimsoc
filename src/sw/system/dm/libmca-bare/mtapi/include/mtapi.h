/*
* Copyright (c) 2011, The Multicore Association All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* mtapi.h
*
 * Version 0.01, October 2012
 *
 */
#ifndef MTAPI_H
#define MTAPI_H

#include "mtapi_impl_spec.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * MTAPI type definitions
 */
typedef mca_int_t			mtapi_int_t;
typedef mca_int8_t			mtapi_int8_t;
typedef mca_int16_t			mtapi_int16_t;
typedef mca_int32_t 		mtapi_int32_t;
typedef mca_int64_t			mtapi_int64_t;
typedef mca_uint_t			mtapi_uint_t;
typedef mca_uint8_t			mtapi_uint8_t;
typedef mca_uint16_t		mtapi_uint16_t;
typedef mca_uint32_t 		mtapi_uint32_t;
typedef mca_uint64_t		mtapi_uint64_t;
typedef mca_boolean_t		mtapi_boolean_t;
typedef mca_node_t			mtapi_node_t;
typedef mca_status_t		mtapi_status_t;
typedef mca_timeout_t		mtapi_timeout_t;
typedef mca_domain_t		mtapi_domain_t;

typedef mtapi_uint_t	mtapi_task_state_t;
typedef mtapi_uint_t 	mtapi_action_id_t;
typedef mtapi_uint_t 	mtapi_queue_id_t;
typedef mtapi_uint_t 	mtapi_task_id_t;
typedef mtapi_uint_t 	mtapi_group_id_t;
typedef mtapi_uint_t	mtapi_notification_t;
typedef mtapi_uint_t	mtapi_size_t;

/* The following constants are not implementation defined */
#define MTAPI_VERSION				0015
#define MTAPI_TRUE					1
#define MTAPI_FALSE					0
#define MTAPI_NULL					0		/* MTAPI Zero value */
#define	MTAPI_INFINITE				(~0)	/* Wait forever, no timeout */
#define MTAPI_NODE_INVALID			(~0)
#define MTAPI_DOMAIN_INVALID		(~0)
#define MTAPI_TIMEOUT_INFINITE 		(~0)
#define MTAPI_TIMEOUT_IMMEDIATE 	(0)
#define MTAPI_MAX_STATUS_SIZE 		32
#define MTAPI_TASK_ID_NONE			0
#define MTAPI_GROUP_NONE			0

/*
 * MTAPI status codes
 */
enum mtapi_status_codes
{
	MTAPI_SUCCESS = 1,					/* Indicates operation was successful */
	MTAPI_PENDING,						/* Indicates operation is pending
											without errors */
	MTAPI_ERR_NODE_INITFAILED,			/* The MTAPI environment could not be
	 										initialized */
	MTAPI_ERR_NODE_INITIALIZED, 		/* The MTAPI environment has already
											been initialized */
	MTAPI_ERR_NODE_INVALID,				/* The node_id parameter is not valid */
	MTAPI_ERR_NODE_NOTINIT,				/* The calling node is not
											initialized */
	MTAPI_ERR_NODE_FINALFAILED,			/* The MTAPI environment could not
											be finalized */
	MTAPI_ERR_DOMAIN_INVALID,			/* The domain_id parameter is not
											valid */
	MTAPI_ERR_DOMAIN_NOTSHARED,			/* This resource cannot be shared
											by this domain */
	MTAPI_ERR_PARAMETER,				/* Invalid mtapi_parameters or
											mtapi_info parameter */
	MTAPI_ERR_ATTR_READONLY,			/* Attribute cannot be modified */
	MTAPI_ERR_ATTR_NUM,					/* Unknown attribute number */
	MTAPI_ERR_ATTR_SIZE,				/* Incorrect attribute size */
	MTAPI_ERR_ACTION_INVALID,			/* The action_id is not a valid action
											id */
	MTAPI_ERR_ACTION_EXISTS,			/* The action is already created */
	MTAPI_ERR_ACTION_LIMIT,				/* Exceeded maximum number of actions
											allowed */
	MTAPI_ERR_ACTION_NOAFFINITY,		/* The action was created with an
											MTAPI_ACTION_AFFINITY attribute
											that has set the affinity to all
											cores of the node to MTAPI_FALSE */
	MTAPI_ERR_ACTION_DELETED,			/* The action has been deleted */
	MTAPI_ERR_TIMEOUT,					/* Timeout was reached */
	MTAPI_ERR_CONTEXT_OUTOFCONTEXT,		/* Not called in the context of a
											task execution */
	MTAPI_ERR_AFFINITY_MASK,			/* Invalid mask parameter */
	MTAPI_ERR_CORE_NUM,					/* Unknown core number */
	MTAPI_ERR_QUEUE_ID_INVALID,			/* The queue_id is not a valid queue
											id */
	MTAPI_ERR_QUEUE_EXISTS,				/* The queue is already created */
	MTAPI_ERR_QUEUE_LIMIT,				/* Exceeded maximum number of queues
											allowed */
	MTAPI_ERR_TASK_INVALID,				/* Task is not valid */
	MTAPI_ERR_TASK_LIMIT,				/* Exceeded maximum number of tasks
											allowed */
	MTAPI_ERR_TASK_CANCELLED,			/* The tasks has been cancelled
											because of mtapi_task_cancel() was
											called before the task was executed
											or the error code was set by
											mtapi_action_status_set() */
	MTAPI_ERR_ARG_SIZE,					/* The size of the arguments expected
											differs from the caller */
	MTAPI_ERR_RESULT_SIZE,				/* The size of the result buffer
											expected differs from the caller */
	MTAPI_ERR_GROUP_LIMIT,				/* Exceeded maximum number of groups
											allowed */
	MTAPI_ERR_GROUP_INVALID,			/* Argument is not a valid group
											handle */
	MTAPI_ERR_UNKNOWN,					/* Unknown eror */
	MTAPI_GROUP_COMPLETED				/* Group completed, i.e. there are no
											more tasks to wait for */
};

/*
 * MTAPI task states
 */
enum mtapi_task_states
{
	MTAPI_TASK_CANCELLED = 1,
	MTAPI_TASK_CREATED,
	MTAPI_TASK_SCHEDULED,
	MTAPI_TASK_RUNNING,
	MTAPI_TASK_WAITING,
	MTAPI_TASK_DELETED,
	MTAPI_TASK_COMPLETED
};

/*
 * MTAPI task to core affinity mask.
 */
typedef struct
{
	mtapi_task_hndl_t task;					/* MTAPI task handle,
												defines the task which has a
												affinity to a specific core */
	mtapi_node_t node;						/* MTAPI node,
												the corresponding node for the
												affinity of the task */
} mtapi_affinity_t;

/*
 * MTAPI Initialization information
 * In addition to the MTAPI defined information implementations may
 * include implementation specific information.
 */
typedef struct
{
	mtapi_uint_t mtapi_version;				/* MTAPI Version, the three last
	 											(rightmost) hex digits are the
												minor number and those left
												of minor the major number */
	mtapi_uint_t organization_id;			/* Implementation
	 											vendor/organization id */
	mtapi_uint_t implementation_version;	/* Implementation version, the
												format is implementation
												≤defined */
	mtapi_uint_t number_of_domains;			/* Number of damins in the
	 											topology */
	mtapi_uint_t number_of_nodes;			/* Number of nodes in the domain,
												can be used for basic per
												domain topology discovery */
} mtapi_info_t;

/*
 * Task context information.
 */
typedef struct
{
	mtapi_task_id_t task_id;			/* Task id */
	mtapi_action_id_t action_id;		/* Action ID */
	mtapi_status_t status;				/* Status information of a running
	 	 	 	 	 	 	 	 	 		action */
	void* data;							/* Generic context data for
											communication to the runtime
											system */
	mtapi_size_t data_size;				/* Size of the context data */
	mtapi_int_t num_instances;			/* Number of instances */
	mtapi_int_t instance;				/* Instance number of the
											corresponding task */
} mtapi_task_context_t;

/* Node attribute numbers */
enum mtapi_node_attribute_numbers {
	MTAPI_NODE_NUMCORES					/* Number of processor cores on the
											node */
};

/* MTAPI_NODE_NUMCORES */
typedef mtapi_uint_t mtapi_node_numcores_t;

/* Default node attributes */
enum mtapi_node_attributes {
	MTAPI_DEFAULT_NODE_ATTRIBUTES = 0
};

/* Node attribute numbers */
enum mtapi_action_attribute_numbers {
	MTAPI_ACTION_GLOBAL = 1,			/* Indicates whether or not this is
	 	 	 	 	 	 	 	 	 		a globally visible action */
	MTAPI_ACTION_AFFINITY,				/* Core affinity of action code */
	MTAPI_DOMAIN_SHARED					/* Indicates whether or not the action
											is shareable across domains */
};

/* Default action attributes */
enum mtapi_action_attributes {
	MTAPI_DEFAULT_ACTION_ATTRIBUTES = 0
};

/* MTAPI_ACTION_GLOBAL */
typedef mtapi_boolean_t mtapi_action_global_t;

/* MTAPI_ACTION_AFFINITY */
typedef mtapi_affinity_t mtapi_action_affinity_t;

/* MTAPI_DOMAIN_SHARED */
typedef mtapi_boolean_t mtapi_domain_shared_t;

/* Queue attribute numbers */
enum mtapi_queue_attribute_numbers {
	MTAPI_QUEUE_GLOBAL = 1,				/* Indicates whether or not this is
	 	 	 	 	 	 	 	 	 		a globally visible queue */
	MTAPI_QUEUE_PRIORITY,				/* Priority of the queue */
	MTAPI_QUEUE_LIMIT					/* Max. number of element in the
	 	 	 	 	 	 	 	 	 		queue */
};

/* MTAPI_QUEUE_GLOBAL */
typedef mtapi_boolean_t mtapi_queue_global_t;

/* MTAPI_QUEUE_PRIORITY */
typedef mtapi_uint_t mtapi_queue_priority_t;

/* MTAPI_QUEUE_LIMIT */
typedef mtapi_uint_t mtapi_queue_limt_t;

/* Task attribute numbers */
enum mtapi_task_attribute_numbers {
	MTAPI_TASK_DETACHED = 1,			/* Indicates whether or not this is
	 	 	 	 	 	 	 	 	 		a detached task */
	MTAPI_TASK_INSTANCES				/* Indicates how many parallel instances
	 	 	 	 	 	 	 	 	 	 	 of a task shall be started */
};

/* MTAPI_TASK_DETACHED */
typedef mtapi_boolean_t mtapi_task_detached_t;

/* MTAPI_TASK_INSTANCES */
typedef mtapi_uint_t mtapi_task_instances_t;

/* Default action attributes */
enum mtapi_task_attributes {
	MTAPI_DEFAULT_TASK_ATTRIBUTES = 0
};

/* In/Out parameter indication macros */
#ifndef MTAPI_IN
#define MTAPI_IN const
#endif /* MTAPI_IN */

#ifndef MTAPI_OUT
#define MTAPI_OUT
#endif /* MTAPI_OUT */

#ifndef MTAPI_INOUT
#define MTAPI_INOUT
#endif /* MTAPI_INOUT */

/*
 * The mtapi_action_function_t represents a function pointer to an action
 * function.
 */
typedef void (*mtapi_action_function_t) (
	MTAPI_IN void* args,						/* arguments */
	MTAPI_IN mtapi_size_t args_size,			/* length of arguments */
	MTAPI_OUT void* result_buffer,				/* buffer for storing results */
	MTAPI_IN mtapi_size_t result_buffer_size,	/* length of result_buffer */
	MTAPI_IN void* node_local_data,				/* node-local data, shared by
													several tasks, executed on
													the same node */
	MTAPI_IN mtapi_size_t node_local_data_size,	/* length of shared data */
	mtapi_task_context_t* const context			/* MTAPI task context used for
													writing results, ... */
	);

/*
 * Function prototypes
 */

/* Initialization */
extern void mtapi_initialize (
	MTAPI_IN mtapi_domain_t domain_id,
	MTAPI_IN mtapi_node_t node_id,
	MTAPI_IN mtapi_node_attributes_t* mtapi_node_attributes,
	MTAPI_OUT mtapi_info_t* mtapi_info,
	MTAPI_OUT mtapi_status_t* status
	);

/* Synchronization */
extern void mtapi_synchronize (
	MTAPI_IN mtapi_domain_t domain_id,
	MTAPI_IN mtapi_node_t node_id,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_node_init_attributes(
	MTAPI_OUT mtapi_node_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_node_set_attribute(
	MTAPI_INOUT mtapi_node_attributes_t* attributes,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_node_get_attribute(
	MTAPI_IN mtapi_node_t node,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_OUT void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_finalize(
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_domain_t mtapi_domain_id_get(
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_node_t mtapi_node_id_get(
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_action_hndl_t mtapi_action_create(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_IN mtapi_action_function_t function,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	MTAPI_IN mtapi_action_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_init_attributes(
	MTAPI_OUT mtapi_action_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_set_attributes(
	MTAPI_INOUT mtapi_action_attributes_t* attributes,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_get_attribute(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_OUT void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_action_hndl_t mtapi_action_get(
	MTAPI_IN mtapi_action_id_t action_id,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_delete(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_disable(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_enable(
	MTAPI_IN mtapi_action_hndl_t action,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_context_set(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_IN mtapi_status_t error_code,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_context_runtime_notify(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_IN mtapi_notification_t notification,
	MTAPI_IN void* data,
	MTAPI_IN mtapi_size_t data_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_task_state_t mtapi_context_taskstate_get(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_uint_t mtapi_context_instnum_get(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_uint_t mtapi_context_numinst_get(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_uint_t mtapi_context_corenum_get(
	MTAPI_IN mtapi_task_context_t* task_context,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_action_init_cores(
	MTAPI_OUT mtapi_affinity_t* mask,
	MTAPI_IN mtapi_boolean_t affinity,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_affinity_set_core(
	MTAPI_OUT mtapi_affinity_t* mask,
	MTAPI_IN mtapi_uint_t core_num,
	MTAPI_IN mtapi_boolean_t affinity,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_affinity_isset_core(
	MTAPI_OUT mtapi_affinity_t* mask,
	MTAPI_IN mtapi_uint_t core_num,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_queue_hndl_t mtapi_queue_create(
	MTAPI_IN mtapi_queue_id_t queue_id,
	MTAPI_IN mtapi_action_hndl_t* actions,
	MTAPI_IN mtapi_int_t num_actions,
	MTAPI_IN mtapi_queue_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_init_attributes(
	MTAPI_IN mtapi_queue_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_set_attribute(
	MTAPI_INOUT mtapi_queue_attributes_t* attributes,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_get_attribute(
	MTAPI_IN mtapi_queue_hndl_t queue,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_queue_hndl_t mtapi_queue_get(
	MTAPI_IN mtapi_queue_id_t* queue_id,
	MTAPI_IN mtapi_domain_t domain_id,
	MTAPI_IN mtapi_node_t node_id,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_delete(
	MTAPI_IN mtapi_queue_hndl_t queue,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_disable(
	MTAPI_IN mtapi_queue_hndl_t queue,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_queue_enable(
	MTAPI_IN mtapi_queue_hndl_t queue,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_task_hndl_t mtapi_task_start(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_action_hndl_t* actions,
	MTAPI_IN mtapi_int_t num_actions,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_task_hndl_t mtapi_task_enqueue(
	MTAPI_IN mtapi_task_id_t task_id,
	MTAPI_IN mtapi_queue_hndl_t queue,
	MTAPI_IN void* arguments,
	MTAPI_IN mtapi_size_t arguments_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN size_t result_size,
	MTAPI_IN mtapi_task_attributes_t* attributes,
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_task_init_attributes(
	MTAPI_OUT mtapi_task_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_task_set_attribute(
	MTAPI_INOUT mtapi_task_attributes_t* attributes,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_task_get_attribute(
	MTAPI_IN mtapi_task_hndl_t task,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_task_cancel(
	MTAPI_IN mtapi_task_hndl_t task,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_task_wait(
	MTAPI_IN mtapi_task_hndl_t task,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern mtapi_group_hndl_t matpi_group_create(
	MTAPI_IN mtapi_group_id_t group_id,
	MTAPI_IN mtapi_group_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_init_attributes(
	MTAPI_OUT mtapi_group_attributes_t* attributes,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_set_attribute(
	MTAPI_INOUT mtapi_group_attributes_t* attributes,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_get_attribute(
	MTAPI_IN mtapi_group_hndl_t task,
	MTAPI_IN mtapi_uint_t attribute_num,
	MTAPI_IN void* attribute,
	MTAPI_IN size_t attribute_size,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_wait_all(
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_wait_any(
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_IN mtapi_timeout_t timeout,
	MTAPI_OUT mtapi_status_t* status
	);

extern void mtapi_group_delete(
	MTAPI_IN mtapi_group_hndl_t group,
	MTAPI_OUT mtapi_status_t* status
	);

/* Convenience functions */
extern char* mtapi_display_status(
		mtapi_status_t status, char* status_message, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MTAPI_H */
