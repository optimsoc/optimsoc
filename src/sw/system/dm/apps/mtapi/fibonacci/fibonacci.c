/*
 * fibonacci.c
 *
 *  Created on: Oct 24, 2012
 *      Author: ga49qez
 */
#include "fibonacci.h"
#include "utils.h"
#include "optimsoc.h"

#include <stdlib.h>
#include <stdio.h>

static mtapi_task_id_t tID = 0;
static thread_t mainThread;

void fibonacciActionFunction (
  MTAPI_IN void* args,
  MTAPI_IN mtapi_size_t arg_size,
  MTAPI_OUT void* result_buffer,
  MTAPI_IN mtapi_size_t result_buffer_size,
  MTAPI_IN void* node_local_data,
  MTAPI_IN mtapi_size_t node_local_data_size,
  mtapi_task_context_t* task_context)
{
  mtapi_job_hndl_t jobHndl;
  mtapi_task_hndl_t taskHndl;
  mtapi_group_hndl_t groupHndl = { MTAPI_GROUP_NONE };
  mtapi_status_t status;

  int n;
  int* result;
  int x, y, a, b;

  /* check argument size (1 integer) */
  if (arg_size != sizeof(int)) {
    printf("wrong size of arguments!\n");
    mtapi_context_status_set(task_context, MTAPI_ERR_ARG_SIZE, &status);
    MTAPI_CHECK_STATUS("context set 1", status, MTAPI_TRUE);
    return;
  }

  /* cast arguments to the desired type */
  n = *(int *)args;

  /* check if result buffer is of interest for the caller */
  if (result_buffer == MTAPI_NULL) {
    mtapi_context_status_set(task_context, MTAPI_ERR_RESULT_SIZE, &status);
    MTAPI_CHECK_STATUS("context set 2", status, MTAPI_TRUE);
  } else {
    if (result_buffer_size == sizeof(int)) {
      result = (int *)result_buffer;
    } else {
      printf("wrong size of result buffer\n");
      mtapi_context_status_set(task_context, MTAPI_ERR_RESULT_SIZE, &status);
      MTAPI_CHECK_STATUS("context set 3", status, MTAPI_TRUE);
      return;
    }
  }

  /* calculate */
  if (n < 2)
    *result = n;
  else {

    jobHndl = mtapi_job_get(
        FIBONACCI_ACTION_01,            /* Job id */
        0,                              /* Domain id */
        &status
        );
    MTAPI_CHECK_STATUS ("job get", status, MTAPI_TRUE);

    /* first recursive call spawned as taskHndl (x = fib(n-1);) */
    a = n - 1;
    /* start taskHndl */
    taskHndl = mtapi_task_start (
      tID++,                        /* Optional taskHndl id */
      jobHndl,                      /* Action handles */
      (void *)&a,                   /* Arguments */
      sizeof(int),                  /* Size of arguments */
      (void *)&x,                   /* Result buffer */
      sizeof(int),                  /* Size of result buffer */
      MTAPI_DEFAULT_TASK_ATTRIBUTES,/* Task attributes */
      groupHndl,                    /* Optional Task group */
      &status
      );
    MTAPI_CHECK_STATUS ("taskHndl start", status, MTAPI_TRUE);

    /* second recursive call can be called directly (y = fib(n-2);) */
    b = n - 2;
    fibonacciActionFunction (&b, sizeof(int), &y, sizeof(int),
                 MTAPI_NULL, 0, task_context);

    /* wait for results */
    mtapi_task_wait (taskHndl, MTAPI_INFINITE, &status);
    MTAPI_CHECK_STATUS ("taskHndl wait1", status, MTAPI_TRUE);

    *result = x + y;
  }
}

int fibonacci ( int n,
        int domain_id,
        int node_id ) {
  mtapi_action_hndl_t actionHndl;
  mtapi_task_hndl_t taskHndl;
  mtapi_job_hndl_t jobHndl;
  mtapi_group_hndl_t groupHndl = { MTAPI_GROUP_NONE };
  mtapi_status_t status;

  int result;

  /* create fibonacci action */
  actionHndl = mtapi_action_create (
    FIBONACCI_ACTION_01,              /* Job id */
    (fibonacciActionFunction),        /* Action function */
    MTAPI_NULL,                       /* No shared data */
    0,                                /* Length of shared data */
    MTAPI_DEFAULT_ACTION_ATTRIBUTES,  /* Action attributes */
    &status
    );
  MTAPI_CHECK_STATUS ("action create", status, MTAPI_TRUE);

  jobHndl = mtapi_job_get(
      FIBONACCI_ACTION_01,            /* Job id */
      domain_id,                      /* Domain id */
      &status
      );
  MTAPI_CHECK_STATUS ("job get", status, MTAPI_TRUE);

  /* start taskHndl */
  taskHndl = mtapi_task_start (
    tID++,                            /* Optional taskHndl id */
    jobHndl,                          /* Job handle */
    (void *)&n,                       /* Arguments */
    sizeof(int),                      /* Size of arguments */
    (void *)&result,                  /* Result buffer */
    sizeof(int),                      /* Size of result buffer */
    MTAPI_DEFAULT_TASK_ATTRIBUTES,    /* Task attributes */
    groupHndl,                        /* Optional Task group */
    &status
    );
  MTAPI_CHECK_STATUS ("taskHndl start", status, MTAPI_TRUE);

  /* wait for completion */
  mtapi_task_wait (taskHndl, MTAPI_INFINITE, &status);
  MTAPI_CHECK_STATUS ("taskHndl wait2", status, MTAPI_TRUE);

  return result;
}

void wakeUpFunction (
  MTAPI_IN void* args,
  MTAPI_IN mtapi_size_t arg_size,
  MTAPI_OUT void* result_buffer,
  MTAPI_IN mtapi_size_t result_buffer_size,
  MTAPI_IN void* node_local_data,
  MTAPI_IN mtapi_size_t node_local_data_size,
  mtapi_task_context_t* task_context)
{
  thread_resume(mainThread);
}

void fibonacci_helper_1 ( ) {
  mtapi_task_hndl_t task;
  mtapi_status_t status;

  /* create fibonacci action */
  mtapi_action_create (
    FIBONACCI_ACTION_02,              /* Action id */
    (fibonacciActionFunction),        /* Action function */
    MTAPI_NULL,                       /* No shared data */
    0,                                /* Length of shared data */
    MTAPI_DEFAULT_ACTION_ATTRIBUTES,  /* Action attributes */
    &status
    );
  MTAPI_CHECK_STATUS ("action create", status, MTAPI_TRUE);

  /* create wake up function */
  mtapi_action_create (
    WAKE_UP_FUNCTION,                 /* Action id */
    (wakeUpFunction),                 /* Action function */
    MTAPI_NULL,                       /* No shared data */
    0,                                /* Length of shared data */
    MTAPI_DEFAULT_ACTION_ATTRIBUTES,  /* Action attributes */
    &status
    );
  MTAPI_CHECK_STATUS ("action create", status, MTAPI_TRUE);

  thread_suspend();
}


void init() {

  mtapi_info_t info;
  mtapi_status_t status;
  int result, n, domain, node;
  mtapi_action_hndl_t action;
  mtapi_task_hndl_t task;

  n = 3;
  domain = optimsoc_get_tileid();
  node = optimsoc_get_tileid();

  mainThread = thread_self();

  /* initialize MTAPI */
  mtapi_initialize (
    domain,                           /* Domain id */
    node,                             /* Node id */
    MTAPI_DEFAULT_NODE_ATTRIBUTES,    /* Node attributes */
    &info,                            /* MTAPI info */
    &status                           /* MTAPI status */
    );
  MTAPI_CHECK_STATUS("mtapi init", status, MTAPI_TRUE);

  if (node == 0)   {
    result = fibonacci(n, domain, node);
    OPTIMSOC_REPORT(0x777,result);
  }
  else  {
    fibonacci_helper_1();
  }

  mtapi_finalize( &status );
  MTAPI_CHECK_STATUS("mtapi finalize", status, MTAPI_TRUE);

  /* print result */
  printf ("result: %i\n", result);
}
