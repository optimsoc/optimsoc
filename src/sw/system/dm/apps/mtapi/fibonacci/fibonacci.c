/*
 * fibonacci.c
 *
 *  Created on: Oct 24, 2012
 *      Author: ga49qez
 */
#include "fibonacci.h"
#include <utils.h>

static mtapi_action_hndl_t fibonacciAction;
static mtapi_task_id_t tID = 0;

void fibonacciActionFunction (
	MTAPI_IN void* args,
	MTAPI_IN mtapi_size_t arg_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN mtapi_size_t result_buffer_size,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	mtapi_task_context_t* task_context)
{
	mtapi_task_hndl_t task;
	mtapi_status_t status;

	int n;
	int* result;
	int x, y, a, b;

	/* check argument size (1 integer) */
	if (arg_size != sizeof(int)) {
		printf("wrong size of arguments!\n");
/*
 *		mtapi_context_status_set(task_context, MTAPI_ERR_ARGSIZE, &status);
 *			MTAPI_CHECK_STATUS(status);
 */
		return;
	}

	/* cast arguments to the desired type */
	n = *(int *)args;

	/* check if result buffer is of interest for the caller */
	if (result_buffer == MTAPI_NULL) {
/*
 *		mtapi_context_status_set(task_context, MTAPI_ERR_RESULTSIZE, &status);
 *		MTAPI_CHECK_STATUS(status);
 */
	} else {
		if (result_buffer_size == sizeof(int)) {
			result = (int *)result_buffer;
		} else {
			printf("wrong size of result buffer\n");
/*
 *			mtapi_context_status_set(task_context, MTAPI_ERR_RESULTSIZE,
 *				&status);
 *			MTAPI_CHECK_STATUS(status);
 */
			return;
		}
	}

	/* calculate */
	if (n < 2)
		*result = n;
	else {

		/* first recursive call spawned as task (x = fib(n-1);) */
		a = n - 1;
		/* start task */
		OPTIMSOC_REPORT(0x801,a);
		task = mtapi_task_start (
			tID++,					/* Optional task id */
			&fibonacciAction,					/* Action handles */
			1,									/* Number of action handles */
			(void *)&a,							/* Arguments */
			sizeof(int),						/* Size of arguments */
			(void *)&x,							/* Result buffer */
			sizeof(int),						/* Size of result buffer */
			MTAPI_DEFAULT_TASK_ATTRIBUTES,		/* Task attributes */
			MTAPI_GROUP_NONE,					/* Optional Task group */
			&status
			);
		MTAPI_CHECK_STATUS ("task start", status, MTAPI_TRUE);
		OPTIMSOC_REPORT(0x802,a);

		/* second recursive call can be called directly (y = fib(n-2);) */
		b = n - 2;
		fibonacciActionFunction (&b, sizeof(int), &y, sizeof(int),
								 MTAPI_NULL, 0, task_context);

		/* wait for results */
		mtapiRT_TaskInfo_t* taskInfo  = (mtapiRT_TaskInfo_t*) task->task_descriptor;
		OPTIMSOC_REPORT(0x701,task_context->task_id);
		OPTIMSOC_REPORT(0x702,taskInfo->context->task_id);
		mtapi_task_wait (task, MTAPI_INFINITE, &status);
		MTAPI_CHECK_STATUS ("task wait", status, MTAPI_TRUE);

		*result = x + y;
	}
}

int fibonacci ( int n,
				int domain_id,
				int node_id ) {
	mtapi_task_hndl_t task;
	mtapi_status_t status;

	int result;

	/* create fibonacci action */
	fibonacciAction = mtapi_action_create (
		FIBONACCI_ACTION_01,				/* Action id */
		(fibonacciActionFunction),			/* Action function */
		MTAPI_NULL,							/* No shared data */
		0,									/* Length of shared data */
		MTAPI_DEFAULT_ACTION_ATTRIBUTES,	/* Action attributes */
		&status
		);
	MTAPI_CHECK_STATUS ("action create", status, MTAPI_TRUE);

	/* start task */
	task = mtapi_task_start (
		tID++,								/* Optional task id */
		&fibonacciAction,					/* Action handles */
		1,									/* Number of action handles */
		(void *)&n,							/* Arguments */
		sizeof(int),						/* Size of arguments */
		(void *)&result,					/* Result buffer */
		sizeof(int),						/* Size of result buffer */
		MTAPI_DEFAULT_TASK_ATTRIBUTES,		/* Task attributes */
		MTAPI_GROUP_NONE,					/* Optional Task group */
		&status
		);
	MTAPI_CHECK_STATUS ("task start", status, MTAPI_TRUE);

	/* wait for completion */
	mtapi_task_wait (task, MTAPI_INFINITE, &status);
	MTAPI_CHECK_STATUS ("task wait", status, MTAPI_TRUE);

	return result;
}


void init() {

	mtapi_info_t info;
	mtapi_status_t status;
	int result, n, domain, node;

	/* check arguments */
	/*if (argc != 3) {
		printf ("Wrong number of arguments (should be 3 ints)!\n");
		return -1;
	}
	n = atoi (&argv[0]);
	domain = atoi(&argv[1]);
	node = atoi(&argv[2]);*/
	n = 10;
	domain = 1;
	node = 1;

	/* initialize MTAPI */
	mtapi_initialize (
		domain,							/* Domain id */
		node,							/* Node id */
		MTAPI_DEFAULT_NODE_ATTRIBUTES,	/* Node attributes */
		&info,							/* MTAPI info */
		&status							/* MTAPI status */
		);
	MTAPI_CHECK_STATUS("mtapi init", status, MTAPI_TRUE);

	result = fibonacci (n, domain, node);

	OPTIMSOC_REPORT(0x777,result);

	/* print result */
	printf ("result: %i\n", result);
}
