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
 *  Source: th_pthreads.c
 *
 *  Description: defines a pthreads task management implementation for
 *               use by testcases
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>

/* this shared data is critical, accesses must be surrounded by mutexes */
static pthread_t threads[CONKER_MAX_TASKS];
static pthread_mutex_t th_mutex = PTHREAD_MUTEX_INITIALIZER;
static int th_pthreads_ok = 1;

/* forward declaration of thread wrapper function */
void* thread_wrapper(void* task_data);


/*****************************************************************************
 *  th_get_impl_string -- get a string indicating the type of testharness
 *****************************************************************************/
char* th_get_impl_string(void)
{
  static char impl_string[] = "pthread";
  return impl_string;
}

/*****************************************************************************
 *  th_task_print_arg_usage
 *****************************************************************************/
void th_task_print_arg_usage(void)
{
  /* EMPTY */
}

/*****************************************************************************
 *  th_task_setup -- do any basic setup here, including registering a handler
 *                   for command line args.  Args will be parsed between the 
 *                   call to th_task_setup and the call to th_task_initialize.
 *****************************************************************************/
extern th_status th_task_setup(void)
{
  th_status status = TH_OK;

  return status;
}

/*****************************************************************************
 *  th_task_initialize -- called to initialize the task API; NOT REENTRANT
 *****************************************************************************/
th_status th_task_initialize(void)
{
  int i = 0;

  th_log_info("[TH_PTHREAD] -- th_task_initialize, max tasks = %d\n", 
              CONKER_MAX_TASKS);

  for (i = 0; i < CONKER_MAX_TASKS; i++) {
    threads[i] = 0;
  }

  return TH_OK;
} 

/*****************************************************************************
 *  th_task_create -- called to create a new task; REENTRANT
 *****************************************************************************/
th_status th_task_create(int task_id)
{
  th_status status = TH_OK;
  th_task_descriptor* td = NULL;
  
  th_log_info("[TH_PTHREAD] -- th_task_create, task id = %d\n", task_id);

  /* BEGIN MUTEX */
  if (pthread_mutex_lock(&th_mutex) != 0) {
    th_log_error("Unable to obtain a mutex lock.\n");
    status = TH_ERROR;
  } else {
    td = th_get_task_descriptor(task_id);
    if (td != NULL) {
      
      
      /* mark it optimistically as running */
      td->status = TASK_RUNNING;
      
      if (pthread_create(&threads[task_id], 
                         NULL, 
                         thread_wrapper, 
                         (void *) td) == 0) {
      } else {
        td->status = TASK_ERROR;
        status = TH_ERROR;
      }
      
    } else {
      status = TH_ERROR;
      th_log_error("invalid task id passed to th_task_create, task_id=%d\n",task_id);
    }
    /* END MUTEX */
    if (pthread_mutex_unlock(&th_mutex) != 0) {
      th_log_error("Unable to release a mutex lock.\n");
      status = TH_ERROR;
    }
  } 
    
  return status;
}

/*****************************************************************************
 *  th_task_exit -- called by a task when it exits; REENTRANT
 *****************************************************************************/
th_status th_task_exit(int task_id, 
                       void* return_data, 
                       int return_data_size, 
                       task_status_code task_status)
{
  th_status status = TH_OK;
  th_task_descriptor* td = NULL;
  
  th_log_info("[TH_PTHREAD] -- th_task_exit, task id = %d\n", task_id);
  
  /* BEGIN MUTEX */
  if (pthread_mutex_lock(&th_mutex) != 0) {
    th_log_error("Unable to obtain a mutex lock.\n");
    status = TH_ERROR;
  } else {
    td = th_get_task_descriptor(task_id);
    if (td != NULL) {
      
      if (task_status == TASK_ERROR) {
        td->status = task_status;
      }

      /* mark the task as completed */
      if (td->status == TASK_RUNNING) {
        td->status = TASK_EXITED;
      } else { 
        th_log_error("task %d exited with an error\n", task_id);
      } 
      
      /* copy the return data */
      td->return_data = (void *) malloc(return_data_size);
      td->return_data_size =return_data_size;
      memcpy(td->return_data, return_data, return_data_size);
      
      if (td->return_data_size > 0) {
        th_log_info("[TH_PTHREAD] -- return data from task %d = '%s'\n", 
                    task_id, (char *) (td->return_data));
      } else {
        th_log_info("[TH_PTHREAD] -- return data from task %d = <empty>\n", 
                    task_id);
      }
    } else {
      th_log_error("Unable to locate task descriptor for task %d.\n",task_id);
      status = TH_ERROR;    
    }
    
    /* END MUTEX */
    if (pthread_mutex_unlock(&th_mutex) != 0) {
      th_log_error("Unable to release a mutex lock.\n");
      status = TH_ERROR;
    }
  }
  
  return status;
}

/*****************************************************************************
 *  th_task_wait -- called to wait for a task to exit; REENTRANT
 *****************************************************************************/
th_status th_task_wait(int task_id, 
                       void** return_data, 
                       int* return_data_size, 
                       task_status_code* task_status)
{
  th_status status = TH_OK;
  th_task_descriptor* td;
  int pthr_status;

  th_log_info("[TH_PTHREAD] -- th_task_wait, task id = %d\n", task_id);
  
  if ((pthr_status = pthread_join(threads[task_id], NULL)) != 0) {
    th_log_error("Unable to join a task thread, id=%d err=[%s].\n", 
                 task_id, strerror(pthr_status));
    status = TH_ERROR;
  } else {
    /* BEGIN MUTEX */
    if (pthread_mutex_lock(&th_mutex) != 0) {
      th_log_error("Unable to obtain a mutex lock.\n");
      status = TH_ERROR;
    } else {      
      td = th_get_task_descriptor(task_id);

      if (td != NULL) {
        
        /* check if the task called exit */
        if (td->status != TASK_EXITED) {
          td->status = TASK_ERROR;
        }
        
        /* copy the return data */
        *(return_data) = (void *) malloc(td->return_data_size);
        *return_data_size = td->return_data_size;
        memcpy((void *) *(return_data), td->return_data, 
               td->return_data_size);
        *task_status = td->status;
      } else {
        th_log_error("Unable to locate task descriptor for task %d.\n",task_id);
        status = TH_ERROR;    
      }
      /* END MUTEX */
      if (pthread_mutex_unlock(&th_mutex) != 0) {
        th_log_error("Unable to release a mutex lock.\n");
        status = TH_ERROR;
      } 
    }
  }
  return status;
}

/*****************************************************************************
 *  th_task_finalize -- called to finalize the task API; NOT REENTRANT
 *****************************************************************************/
th_status th_task_finalize(void)
{
  th_log_info("[TH_PTHREAD] -- th_task_finalize\n");

  return TH_OK;
}


/*****************************************************************************
 *  thread_wrapper -- silly pthreads calling convention stuff that
 *                    we don't want to force on the testcase
 *****************************************************************************/
void* thread_wrapper(void* task_data)
{
  th_task_descriptor* task = (th_task_descriptor *) task_data;

  task->task_implementation(task->task_id,
                            task->input_data,
                            task->input_data_size);

  return &th_pthreads_ok;
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
