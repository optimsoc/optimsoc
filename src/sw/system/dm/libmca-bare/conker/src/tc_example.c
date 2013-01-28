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

/*****************************************************************************
 *  Source: tc_example.c
 *
 *  Description:
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

#define NUM_TASKS 4

/* our array of task descriptors */
static th_task_descriptor tasks[NUM_TASKS];

/* forward declaration of the task_bodies */
void task_body(int task_id, void* input_data, int input_data_size);

/* forward declaration of the arg handler */
void arg_handler (char opt, const char* value);

char* tc_get_testcase_name(void) { return "tc_example"; }
int tc_get_num_tasks(void) { return NUM_TASKS; }


/*****************************************************************************
 *  tc_print_arg_usage -- print out the descriptions of arguments
 *****************************************************************************/
void tc_print_arg_usage(void)
{
}


/*****************************************************************************
 *  arg_handler
 *****************************************************************************/
void arg_handler (char opt, const char* value)
{
}

/*****************************************************************************
 *  tc_setup -- do any basic setup here, including registering a handler
 *              for command line args.  Args will be parsed between the call
 *              to tc_setup and the call to tc_initialize.
 *****************************************************************************/
tc_status tc_setup(void)
{
  tc_status status = TC_OK;

  /* our example does not have any command line args */

  return status;
}


/*****************************************************************************
 *  tc_initialize -- called by test harness to initialize the testcase
 *****************************************************************************/
tc_status tc_initialize(void)
{
  tc_status status = TC_OK;
  int i = 0;
  char msgbuf[32];

  th_log("[TC_EXAMPLE] -- tc_initialize, pid = %d\n", getpid());

  /* register tasks */
  
  for (i = 0; status == TC_OK && i < NUM_TASKS; i++) {
    sprintf(msgbuf, "Hello Task ID %d!", i);
    if (th_register_task(i,"example_task",task_body,
                         msgbuf,sizeof(msgbuf)) != TH_OK) {
      status = TC_ERROR;
    }
  }

  return status;
}

/*****************************************************************************
 *  tc_run -- called by test harness to execute the testcase
 *            tc_run should:
 *            (1) create all tasks
 *            (2) collect results from all tasks
 *            (3) wait for all tasks to complete
 *            (4) return to the test harness
 *
 *            NOTE - this example does prepare args for the task to allow
 *                   for testing of this capability in the test harness
 *****************************************************************************/
tc_status tc_run(void) 
{
  th_status status;
  int i;
  void* return_data;
  int return_data_size;
  task_status_code task_status;
  
  /* create the tasks */

  for (i = 0; i < NUM_TASKS; i++) {
    if ((status = th_task_create(i)) != TH_OK) {
      th_log_error("Unable to create task number %d.  Status = %d\n",
                   i, status);
      return TC_ERROR;
    } else {
      th_log("[TC_EXAMPLE] -- task created, id = %d\n", i);
    }
  }  

  /* wait for tasks */
  for (i = 0; i < NUM_TASKS; i++) {
    if (th_task_wait(i,&return_data,&return_data_size,&task_status) != TH_OK) {
      th_log_error("error waiting for task %d\n",i);
      return TC_ERROR;
    } else {
      /* print out the message from the task */
      th_log("[TC_EXAMPLE] -- return data from task %d = '%s'\n", i, 
             (char *) (return_data));
      if (task_status == TASK_ERROR) {
        status = TC_ERROR;
        th_log_error("Task %d finished with an error\n",i);
      }
      /* now free the return data */
      free(return_data);
    }
  }

  return status;
}

/*****************************************************************************
 *  tc_report -- called by test harness for testcase to report summary results
 *****************************************************************************/
tc_status tc_report(void) 
{
  th_log("[TC_EXAMPLE] -- tc_report\n");
  return TC_OK;
}

/*****************************************************************************
 *  tc_finalize -- called by test harness to alow testcase to clean up
 *****************************************************************************/
tc_status tc_finalize(void) 
{
  int i = 0;

  th_log("[TC_EXAMPLE] -- tc_finalize, pid = %d\n", getpid());

  /* get rid of memory malloced by tescase and test harness */
  for (i = 0; i < NUM_TASKS; i++) {
    free(tasks[i].input_data);
    free(tasks[i].return_data);
  }

  return TC_OK;
}


/*****************************************************************************
 *  task_body
 *****************************************************************************/
void task_body(int task_id, void* input_data, int input_data_size) 
{
  int i = 0;
  char msgbuf[32];

  th_log("[TC_EXAMPLE] -- in task body, id = %d, input = '%s', pid = %d\n", 
         task_id, (char *) input_data, getpid());

  for (i = 0; i < 10; i++) {
    th_log("[TC_EXAMPLE] -- executing task body, id = %d, count = %d\n", 
           task_id, i);
    /* TODO - what to do about this for bare metal ?? */
    sched_yield();
  }

  /* put something in return data */
  sprintf(msgbuf, "Goodbye from task ID %d", task_id);

  th_log("[TC_EXAMPLE] -- task %d will return '%s'\n", 
         task_id, msgbuf);

  th_task_exit(task_id,msgbuf,strlen(msgbuf)+1,TC_OK);

}




#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
