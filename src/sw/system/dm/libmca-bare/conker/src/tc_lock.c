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
 *  Source: tc_lock.c
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
#include <pthread.h>

#include <mrapi.h>
#include <mca_config.h>

#define NUM_TASKS 4
#define TC_LOCK_MUTEX_ID 911
#define TC_LOCK_SEM_ID 1776
#define TC_LOCK_RWL_ID 2001

/* testcase config options */
typedef enum _op_types {
  USE_UNDEFINED = 0,
  USE_PTHREAD_MUTEX,
  USE_MRAPI_MUTEX,
  USE_SVR4_SEMAPHORE,
  USE_MRAPI_SEMAPHORE,
  USE_SVR4_RWL,
  USE_MRAPI_RWL
} op_types;

#define BIG_COUNT 10000 /* initial value of process counter */
#define SEMKEY ((key_t) 23456L)
#define RWLKEY ((key_t) 34567L)

/* Define the semaphore operation arrays for the semop() calls */
static struct sembuf    op_lock[2] = {
  { 2, 0, 0 } ,         /* wait for [2] (lock) to equal 0 */
  { 2, 1, SEM_UNDO }    /* then increment [2] to 1 - this locks it */
                        /* UNDO to release the lock if processes exits */
                        /* before explicitly unlocking */
};

static struct sembuf    op_endcreate[2] = {
  { 1, -1, SEM_UNDO } , /* decrement [1] (proc counter) with undo on exit */
                        /* UNDO to adjust proc counter if process exits */
                        /* before explicitly calling sem_close() */
  { 2, -1, SEM_UNDO }   /* then decrement [2] (lock) back to 0 */
};

static struct sembuf    op_open[1] = {
  { 1, -1, SEM_UNDO }   /* decrement [1] (proc counter) with undo on exit */
};

static struct sembuf    op_close[3] = {
  { 2, 0, 0 },          /* wait for [2] (lock) to equal to 0 */
  { 2, 1, SEM_UNDO },   /* then increment [2] to 1 - this locks it */
  { 1, 1, SEM_UNDO }    /* then increment [1] (proc counter) */
};

static struct sembuf    op_unlock[1] = {
  { 2, -1, SEM_UNDO }   /* decrement [2] (lock) back to 0 */
};

static struct sembuf    op_op[1] = {
  { 0, 99, SEM_UNDO }   /* decrement or increment [0] with undo on exit */
                        /* the 99 is set to the actual amount to add or */
                        /* subtract (positive or negative) */
};

union{
      int val;
      struct semid_ds *buf;
      ushort *array;
}semctl_arg;



static op_types  op_type = USE_UNDEFINED;
static int       op_count = 1;
static int       read_op_count = 0;
static int       max_nonexc_locks = 0;
static int       num_tasks = NUM_TASKS;
static int       using_pthreads = 0;
static int       using_trylocks = 0;
static char*     filename = NULL;
static FILE*     outfile;
static tc_status final_status = TC_OK;
static char status_buff[MRAPI_MAX_STATUS_SIZE];

static pthread_mutex_t p_mutex=PTHREAD_MUTEX_INITIALIZER;

/* forward declaration of the task_bodies */
void task_body(int task_id, void* input_data, int input_data_size);

/* forward declaration of the arg handler */
int arg_handler (char opt, const char* value);

/* forward declaration of the task_bodies -- they setup the call to lock_driver */
void task_body_pthread_mutex(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_mutex(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_mutex_try(int task_id, void* input_data, int input_data_size);
void task_body_svr4_semaphore(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_semaphore(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_semaphore_try(int task_id, void* input_data, int input_data_size);
void task_body_svr4_rwl(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_rwl(int task_id, void* input_data, int input_data_size);
void task_body_mrapi_rwl_try(int task_id, void* input_data, int input_data_size);


/* other misc functions */
char* tc_get_testcase_name(void) { return "tc_lock"; }
int tc_get_num_tasks(void) { return num_tasks; }

char* op_type_string(void);

int sem_create(key_t key, int initval);
int sem_open(key_t key);
int sem_rm(int id);
int sem_close(int id);
int sem_lock(int id, int num_locks);
int sem_unlock(int id, int num_locks);
int sem_op(int id, int value);


/*****************************************************************************
 *  tc_print_arg_usage -- print out the descriptions of arguments
 *****************************************************************************/
void tc_print_arg_usage(void)
{
  if (using_pthreads) {
    th_log("  -m : test using pthread mutexes\n");
  }
  th_log("  -M : test using MRAPI mutexes                            [default]\n");
  th_log("  -s : test using SVR4 semaphores\n");
  th_log("  -S : test using MRAPI semaphores\n");
  th_log("  -w : test using SVR4-based reader/writer locks\n");
  th_log("  -W : test using MRAPI reader/writer locks\n");
  th_log("  -l : number of lock operations                           [default = 1]\n");
  th_log("  -n : number of lockers                                   [default = %d]\n", NUM_TASKS);
  th_log("  -r : number of reads for each write                      [default = 1]\n");
  th_log("  -c : max concurrent non-exclusive locks                  [default = 1]\n");
  th_log("  -a : use 'try' locks (asynch)\n");
  th_log("  -f <filename> : append stats to file\n");
  th_log("\n  NOTES: \n");
  if (using_pthreads) {
    th_log("    -m -M -s -S -w -W are mutually exclusive\n");
  } else {
    th_log("    -M -s -S -w -W are mutually exclusive\n");
  }
  th_log("    -r is only valid with -w or -W\n");
  th_log("    -a is only valid with -M -S or -W\n");
}


/*****************************************************************************
 *  arg_handler
 *****************************************************************************/
int arg_handler (char opt, const char* value)
{
  int error = 0;

  switch (opt) {

  case 'a':     /* use try locks */
    using_trylocks = 1;
    break;
    
  case 'm':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_PTHREAD_MUTEX;
    } else {
      error = 1;
    }
    break;      
    
  case 'M':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_MRAPI_MUTEX;
    } else {
      error = 1;
    }
    break;      
    
  case 's':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_SVR4_SEMAPHORE;
    } else {
      error = 1;
    }
    break;      
    
  case 'S':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_MRAPI_SEMAPHORE;
    } else {
      error = 1;
    }
    break;      
    
  case 'w':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_SVR4_RWL;
    } else {
      error = 1;
    }
    break;      
    
  case 'W':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_MRAPI_RWL;
    } else {
      error = 1;
    }
    break;      
    
  case 'f':     /* filename */
    if (value) {
      filename = malloc(strlen(value) + 1);
      strcpy(filename, value);
    } else {
      error = 1;
    }
    break;      
    
  case 'l':     /* number of operations */
    if (value) {
      op_count = atoi(value);
    } else {
      error = 1;
    }
    break;      
         
  case 'n':     /* number of tasks */
    if (value) {
      num_tasks = atoi(value);
    } else {
      error = 1;
    }
    break;      
         
  case 'r':     /* number of reads ops per each write op*/
    if (value) {
      read_op_count = atoi(value);
    } else {
      error = 1;
    }
    break;      
    
  case 'c':     /* max non-exclusive locks */
    if (value) {
      max_nonexc_locks = atoi(value);
    } else {
      error = 1;
    }
    break;      
    
  default:
    break;
  }

  return error;
}

/*****************************************************************************
 *  tc_setup -- do any basic setup here, including registering a handler
 *              for command line args.  Args will be parsed between the call
 *              to tc_setup and the call to tc_initialize.
 *****************************************************************************/
tc_status tc_setup(void)
{
  tc_status status = TC_OK;
  
  if (strcmp(th_get_impl_string(),"pthread") == 0) {
    using_pthreads = 1;
  }
  
  th_register_args("amMsSwWf:l:n:r:c:",arg_handler);
   
  return status;
}


/*****************************************************************************
 *  tc_initialize -- called by test harness to initialize the testcase
 *****************************************************************************/
tc_status tc_initialize(void)
{
  tc_status status = TC_OK;
  int i = 0;
  tc_task_body_fptr_t fp = NULL;

  if (op_type == USE_UNDEFINED) {
    op_type = USE_MRAPI_MUTEX;
  }

  /* check the operation count */
  if (op_count < 1) {
    th_log_error("number of lock operations must be > 0\n");
    status = TC_ERROR;
  }

  /* check the number of tasks */
  if (num_tasks < 1 || num_tasks > CONKER_MAX_TASKS) {
    th_log_error("number of lockers out of range [1-%d]\n", CONKER_MAX_TASKS);
    status = TC_ERROR;
  }

  if (op_type == USE_PTHREAD_MUTEX && !using_pthreads) {
      th_log_error("%s not valid with fork test harness\n",op_type_string());
      status = TC_ERROR;
  }

  if (op_type == USE_MRAPI_RWL || op_type == USE_SVR4_RWL) {
    if (read_op_count < 0) {
      th_log_error("read operation count less than one\n");
      status = TC_ERROR;
    } else if (read_op_count == 0) {
      read_op_count = 1;
    }
  } else {
    if (read_op_count != 0) {
      th_log_error("-r argument not valid for %s\n",op_type_string());
      status = TC_ERROR;
    }
  }

  if (op_type == USE_MRAPI_SEMAPHORE || op_type == USE_SVR4_SEMAPHORE ||
      op_type == USE_MRAPI_RWL || op_type == USE_SVR4_RWL) {
    if (max_nonexc_locks < 0) {
      th_log_error("max non-exclusive locks less than one\n");
      status = TC_ERROR;
    } else if (max_nonexc_locks == 0) {
      max_nonexc_locks = 1;
    }
  } else {
    if (max_nonexc_locks != 0) {
      th_log_error("-c argument not valid for %s\n",op_type_string());
      status = TC_ERROR;
    }
  }

  if (using_trylocks && 
      ((op_type == USE_PTHREAD_MUTEX) ||
       (op_type == USE_SVR4_SEMAPHORE) ||
       (op_type == USE_SVR4_RWL))) {
      th_log_error("-a argument not valid for %s\n",op_type_string());
      status = TC_ERROR;
  }

  if (status == TC_ERROR) return status;

  /* if we got here we must have good args ... */
  th_log("[TC_LOCK]    -- tc_initialize, pid = %d\n", getpid());

  /* display the config */
  th_log("[TC_LOCK]    -- using %s\n",op_type_string());
  if (using_trylocks) {
    th_log("[TC_LOCK]    -- using try locks\n");
  }
  th_log("[TC_LOCK]    -- node offset: %d\n", th_get_node_offset());
  th_log("[TC_LOCK]    -- number of lockers: %d\n",num_tasks);
  th_log("[TC_LOCK]    -- number of lock operations: %d\n",op_count);
  if (read_op_count) {
    th_log("[TC_LOCK]    -- number of lock reads per write: %d\n",
           read_op_count);
  }

  if (filename) {
    /* TODO -- how to deal with output log files in bare metal */
    th_log("[TC_LOCK]    -- results will be appended to file: %s\n",filename);
    outfile = fopen(filename, "a+");
  }

  for (i = 0; status == TC_OK && i < num_tasks; i++) {
    
    switch (op_type) 
      {
      case USE_PTHREAD_MUTEX:
        fp = task_body_pthread_mutex;
        break;
      case USE_MRAPI_MUTEX:
        if (!using_trylocks) {
          fp = task_body_mrapi_mutex;
        } else {
          fp = task_body_mrapi_mutex_try;
        }
        break;
      case USE_SVR4_SEMAPHORE:
        fp = task_body_svr4_semaphore;
        break;
      case USE_MRAPI_SEMAPHORE:
        if (!using_trylocks) {
          fp = task_body_mrapi_semaphore;
        } else {
          fp = task_body_mrapi_semaphore_try;
        }
        break;
      case USE_SVR4_RWL:
        fp = task_body_svr4_rwl;
        break;
      case USE_MRAPI_RWL:
        if (!using_trylocks) {
          fp = task_body_mrapi_rwl;
        } else {
          fp = task_body_mrapi_rwl_try;
        }
        break;
      default:
        /* TODO - this is an error and should be handled !! */
        break;
      }
        
    if (th_register_task(i,"lock_task",fp,NULL,0) != TH_OK) {
      th_log_error("Unable to register task number %d.  Status = %d\n",
                   i, status);
      status = TC_ERROR;
    } else {
      th_log_info("[TC_LOCK]    -- task registered, id = %d\n", i);
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
  int i = 0;
  void* return_data;
  int return_data_size;
  task_status_code task_status;

  /* create the tasks */
  for (i = 0; i < num_tasks; i++) {
    if ((status = th_task_create(i)) != TH_OK) {
      th_log_error("Unable to create task number %d.  Status = %d\n",
                   i, status);
      return TC_ERROR;
    } else {
      th_log("[TC_LOCK]    -- task created, id = %d\n", i);
    }
  }  

  /* wait for tasks to complete */
  for (i = 0; i < num_tasks; i++) {
    if (th_task_wait(i,&return_data,&return_data_size,&task_status) != TH_OK) {
      th_log_error("error waiting for task %d\n",i);
      return TC_ERROR;
    } else {
      if (task_status == TASK_EXITED) {
        /* print out the message from the task */
        if (return_data_size > 0) {
          th_log_info("[TC_LOCK]    -- return data from task %d = '%s'\n", i, 
                      (char *) (return_data));
        } else {
          th_log_info("[TC_LOCK]    -- return data from task %d = <empty>\n", i);
        }  
      } else {
        final_status = TC_ERROR;
        th_log_error("Task %d exited with error.\n", i);
      }
      /* now free the return data */
      free(return_data);
    }
  }

  return final_status;
}

/*****************************************************************************
 *  tc_report -- called by test harness for testcase to report summary results
 *****************************************************************************/
tc_status tc_report(void) 
{
  th_log("[TC_LOCK]    -- tc_report\n");
  return TC_OK;
}

/*****************************************************************************
 *  tc_finalize -- called by test harness to alow testcase to clean up
 *****************************************************************************/
tc_status tc_finalize(void) 
{

  if (final_status == TC_OK) {
    th_log("[TC_LOCK]    -- testcase finished successfully, pid = %d\n", getpid());
  } else {
    th_log("[TC_LOCK]    -- testcase finished with errors, pid = %d\n", getpid());
  }

  if (filename) {
    fclose(outfile);
    free(filename);
    filename = NULL;
  }

  return TC_OK;
}


/*****************************************************************************
 *  task_body_pthread_mutex
 *****************************************************************************/
void task_body_pthread_mutex(int task_id, 
                             void* input_data, 
                             int input_data_size) 
{
  int i;
  long double delta = 0.0;

  for (i = 0; i < op_count; i++) {
    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }
    
    if (pthread_mutex_lock(&p_mutex) != 0) {
      th_log_error("Task id %d failed to lock: %s\n",
                   task_id, strerror(errno));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
    
    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }
    
    if (pthread_mutex_unlock(&p_mutex) != 0) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, strerror(errno));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }
  
  if (task_id == 0) {
    
    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }
  
  th_task_exit(task_id,NULL,0,TC_OK);
}

/*****************************************************************************
 *  task_body_mrapi_mutex
 *****************************************************************************/
void task_body_mrapi_mutex(int task_id, 
                           void* input_data, 
                           int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_mutex_hndl_t mutex;
  mrapi_key_t key;
  int i;
  long double delta = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a mutex
  // attempt to lock it N times in a loop
  // measure the total time and divide by N
  mutex = mrapi_mutex_create(TC_LOCK_MUTEX_ID, MRAPI_NULL, &status);
  if (status == MRAPI_ERR_MUTEX_EXISTS) {
    mutex = mrapi_mutex_get(TC_LOCK_MUTEX_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {
    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }
    
    mrapi_mutex_lock(mutex, &key, MRAPI_TIMEOUT_INFINITE, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to lock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }

    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }
    
    mrapi_mutex_unlock(mutex, &key, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {

    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }
      
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  th_task_exit(task_id,NULL,0,TC_OK);
}

/*****************************************************************************
 *  task_body_mrapi_mutex_try
 *****************************************************************************/
void task_body_mrapi_mutex_try(int task_id, 
                               void* input_data, 
                               int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_mutex_hndl_t mutex;
  mrapi_key_t key;
  mrapi_boolean_t locked = MRAPI_FALSE;
  int i;
  long double delta = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a mutex
  // attempt to lock it N times in a loop
  // measure the total time and divide by N
  mutex = mrapi_mutex_create(TC_LOCK_MUTEX_ID, MRAPI_NULL, &status);
  if (status == MRAPI_ERR_MUTEX_EXISTS) {
    mutex = mrapi_mutex_get(TC_LOCK_MUTEX_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {

    locked = MRAPI_FALSE;

    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }
    
    while (locked == MRAPI_FALSE) {
      locked = mrapi_mutex_trylock(mutex, &key, &status);
      if ((status != MRAPI_SUCCESS) && (status != MRAPI_ERR_MUTEX_LOCKED)) {
        th_log_error("Task id %d failed to lock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
    }

    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }
    
    mrapi_mutex_unlock(mutex, &key, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {

    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }
      
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  th_task_exit(task_id,NULL,0,TC_OK);
}

/*****************************************************************************
 *  task_body_svr4_semaphore
 *****************************************************************************/
void task_body_svr4_semaphore(int task_id, 
                              void* input_data, 
                              int input_data_size) 
{
  long double delta = 0.0;
  int i;
  int semid;

  if ((semid = sem_create(SEMKEY,1)) < 0) {
    th_log_error("Task id %d failed to create semaphore\n", task_id);
    th_task_exit(task_id, NULL, 0, TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {

    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }

    if (sem_lock(semid,1) < 0) {
      th_log_error("Task id %d failed to lock semaphore\n", task_id);
      th_task_exit(task_id, NULL, 0, TC_ERROR);
    }

    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }

    if (sem_unlock(semid,1) < 0) {
      th_log_error("Task id %d failed to lock semaphore\n", task_id);
      th_task_exit(task_id, NULL, 0, TC_ERROR);
    }
  }

  if (task_id == 0) {
    
    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }

  if (sem_close(semid) < 0) {
    th_log_error("Task id %d failed to close semaphore\n", task_id);
    th_task_exit(task_id, NULL, 0, TC_ERROR);
  }

  th_task_exit(task_id,NULL,0,TC_OK);
}


/*****************************************************************************
 *  task_body_mrapi_semaphore
 *****************************************************************************/
void task_body_mrapi_semaphore(int task_id, 
                               void* input_data, 
                               int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_sem_hndl_t semaphore;
  int i;
  long double delta = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a semaphore
  // attempt to lock it N times in a loop
  // measure the total time and divide by N
  semaphore = mrapi_sem_create(TC_LOCK_SEM_ID, MRAPI_NULL, max_nonexc_locks, &status);
  if (status == MRAPI_ERR_SEM_EXISTS) {
    semaphore = mrapi_sem_get(TC_LOCK_SEM_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  for (i = 0; i < op_count; i++) {

    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }

    mrapi_sem_lock(semaphore, MRAPI_TIMEOUT_INFINITE, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to lock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }

    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }

    mrapi_sem_unlock(semaphore, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {
    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }
  
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }


  th_task_exit(task_id,NULL,0,TC_OK);
}


/*****************************************************************************
 *  task_body_mrapi_semaphore_try
 *****************************************************************************/
void task_body_mrapi_semaphore_try(int task_id, 
                               void* input_data, 
                               int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_sem_hndl_t semaphore;
  int i;
  mrapi_boolean_t locked = MRAPI_FALSE;
  long double delta = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a semaphore
  // attempt to lock it N times in a loop
  // measure the total time and divide by N
  semaphore = mrapi_sem_create(TC_LOCK_SEM_ID, MRAPI_NULL, max_nonexc_locks, &status);
  if (status == MRAPI_ERR_SEM_EXISTS) {
    semaphore = mrapi_sem_get(TC_LOCK_SEM_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  for (i = 0; i < op_count; i++) {

    locked = MRAPI_FALSE;

    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }

    while (locked == MRAPI_FALSE) {
      locked = mrapi_sem_trylock(semaphore, &status);
      if ((status != MRAPI_SUCCESS) && (status != MRAPI_ERR_SEM_LOCKED)) {
        th_log_error("Task id %d failed to lock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
    }

    if (task_id == 0) {
      delta += th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }

    mrapi_sem_unlock(semaphore, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {
    th_log("Locks acquired:\t%d\tAvg Lock Time (in seconds):\t%Lf"
           "\n\tTotal:\t%Lf\n",
           op_count, delta / op_count, delta);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, delta/op_count);
    }
  }
  
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }


  th_task_exit(task_id,NULL,0,TC_OK);
}

/*****************************************************************************
 *  task_body_svr4_rwl
 *****************************************************************************/
void task_body_svr4_rwl(int task_id, 
                        void* input_data, 
                        int input_data_size) 
{
  int i,j;
  long double delta_read = 0.0;
  long double delta_write = 0.0;
  int semid;

  if ((semid = sem_create(SEMKEY,max_nonexc_locks)) < 0) {
    th_log_error("Task id %d failed to create writer semaphore\n", task_id);
    th_task_exit(task_id, NULL, 0, TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {
    for (j = 0; j < read_op_count; j++) {
      /* get and then release a reader lock */

      if (task_id == 0) {
        /* begin timer in MASTER only*/
        if (th_start_timing() < 0) {
          th_log_error("Task id %d failed to acquire start time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        }
      }

      if (sem_lock(semid,1) < 0) {
        th_log_error("Task id %d failed to lock reader semaphore\n", task_id);
        th_task_exit(task_id, NULL, 0, TC_ERROR);
      }

      if (task_id == 0) {
        delta_read += th_end_timing();
        if (delta_read < 0) {
          th_log_error("Task id %d failed to acquire end time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        } 
      }

      if (sem_unlock(semid,1) < 0) {
        th_log_error("Task id %d failed to lock reader semaphore\n", task_id);
        th_task_exit(task_id, NULL, 0, TC_ERROR);
      }
    }
    
    /* get and then release a writer lock */
    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }

    if (sem_lock(semid,max_nonexc_locks) < 0) {
      th_log_error("Task id %d failed to lock writer semaphore\n", task_id);
      th_task_exit(task_id, NULL, 0, TC_ERROR);
    }

    if (task_id == 0) {
      delta_write += th_end_timing();
      if (delta_write < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }
    
    /* get and then release a writer lock */
    if (sem_unlock(semid,max_nonexc_locks) < 0) {
      th_log_error("Task id %d failed to unlock writer semaphore\n", task_id);
      th_task_exit(task_id, NULL, 0, TC_ERROR);
    }
  }

  if (task_id == 0) {
    th_log("Locks acquired:\t%d\tAverage Read Time (in seconds):\t%Lf\n"
           "Average Write Time (in seconds):\t%Lf\n"
           "Total read:\t%Lf\t\tTotal write:\t%Lf\n",
           op_count, delta_read / (read_op_count * op_count), delta_write / op_count, delta_read, delta_write);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%d\t%Lf\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, read_op_count,
              delta_read / (read_op_count * op_count), 
              delta_write / op_count);
    }
  }

  if (sem_close(semid) < 0) {
    th_log_error("Task id %d failed to close semaphore\n", task_id);
    th_task_exit(task_id, NULL, 0, TC_ERROR);
  }

  th_task_exit(task_id,NULL,0,TC_OK);
}


/*****************************************************************************
 *  task_body_mrapi_rwl
 *****************************************************************************/
void task_body_mrapi_rwl(int task_id, 
                         void* input_data, 
                         int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_rwl_hndl_t rwl;
  int i,j;
  long double delta_read = 0.0;
  long double delta_write = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a reader/writer lock
  // attempt to lock it M read times in a loop then one write lock
  // measure the total time and divide by N
  rwl = mrapi_rwl_create(TC_LOCK_RWL_ID, MRAPI_NULL, max_nonexc_locks, &status);
  if (status == MRAPI_ERR_RWL_EXISTS) {
    rwl = mrapi_rwl_get(TC_LOCK_RWL_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {
    for (j = 0; j < read_op_count; j++) {

      if (task_id == 0) {
        /* begin timer in MASTER only*/
        if (th_start_timing() < 0) {
          th_log_error("Task id %d failed to acquire start time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        }
      }

      mrapi_rwl_lock(rwl, MRAPI_RWL_READER, MRAPI_TIMEOUT_INFINITE, &status);
      if (status != MRAPI_SUCCESS) {
        th_log_error("Task id %d failed to lock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
      
      if (task_id == 0) {
        delta_read += th_end_timing();
        if (delta_read < 0) {
          th_log_error("Task id %d failed to acquire end time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        } 
      }

      mrapi_rwl_unlock(rwl, MRAPI_RWL_READER ,&status);
      if (status != MRAPI_SUCCESS) {
        th_log_error("Task id %d failed to unlock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
    }

    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }

    mrapi_rwl_lock(rwl, MRAPI_RWL_WRITER, MRAPI_TIMEOUT_INFINITE, &status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to lock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
    
    if (task_id == 0) {
      delta_write += th_end_timing();
      if (delta_write < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }

    mrapi_rwl_unlock(rwl, MRAPI_RWL_WRITER ,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {
    th_log("Locks acquired:\t%d\tAverage Read Time (in seconds):\t%Lf\n"
           "Average Write Time (in seconds):\t%Lf\n"
           "Total read:\t%Lf\t\tTotal write:\t%Lf\n",
           op_count, delta_read / (read_op_count * op_count), delta_write / op_count, delta_read, delta_write);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%d\t%Lf\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, read_op_count,
              delta_read / (read_op_count * op_count), 
              delta_write / op_count);
    }
  }
      
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }


  th_task_exit(task_id,NULL,0,TC_OK);
}


/*****************************************************************************
 *  task_body_mrapi_rwl_try
 *****************************************************************************/
void task_body_mrapi_rwl_try(int task_id, 
                               void* input_data, 
                               int input_data_size) 
{
  mrapi_status_t status;
  mrapi_info_t version;
  mrapi_parameters_t parms = 0;
  mrapi_rwl_hndl_t rwl;
  int i,j;
  mrapi_boolean_t locked = MRAPI_FALSE;
  long double delta_read = 0.0;
  long double delta_write = 0.0;

  mrapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), 
                   parms,&version,&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  // get a reader/writer lock
  // attempt to lock it M read times in a loop then one write lock
  // measure the total time and divide by N
  rwl = mrapi_rwl_create(TC_LOCK_RWL_ID, MRAPI_NULL, max_nonexc_locks, &status);
  if (status == MRAPI_ERR_RWL_EXISTS) {
    rwl = mrapi_rwl_get(TC_LOCK_RWL_ID,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to initialize: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff))); 
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  } else if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));

    th_task_exit(task_id,NULL,0,TC_ERROR);
  }

  sleep(1);

  for (i = 0; i < op_count; i++) {
    for (j = 0; j < read_op_count; j++) {

      locked = MRAPI_FALSE;

      if (task_id == 0) {
        /* begin timer in MASTER only*/
        if (th_start_timing() < 0) {
          th_log_error("Task id %d failed to acquire start time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        }
      }
      
      while (locked == MRAPI_FALSE) {
        locked = mrapi_rwl_trylock(rwl, MRAPI_RWL_READER, &status);
        if ((status != MRAPI_SUCCESS) && (status != MRAPI_ERR_RWL_LOCKED)) {
          th_log_error("Task id %d failed to reader lock [attempt:%d]: %s\n",
                       task_id, j, mrapi_display_status(status,status_buff,
                                                     sizeof(status_buff)));
          th_task_exit(task_id,NULL,0,TC_ERROR);
          exit(1);
        }
      }
      
      if (task_id == 0) {
        delta_read += th_end_timing();
        if (delta_read < 0) {
          th_log_error("Task id %d failed to acquire end time\n", task_id);
          th_task_exit(task_id,NULL,0,TC_ERROR);    
        } 
      }
      
      mrapi_rwl_unlock(rwl, MRAPI_RWL_READER ,&status);
      if (status != MRAPI_SUCCESS) {
        th_log_error("Task id %d failed to unlock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
    }
    
    locked = MRAPI_FALSE;
    
    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      }
    }
    
    while (locked == MRAPI_FALSE) {
      locked = mrapi_rwl_trylock(rwl, MRAPI_RWL_WRITER, &status);
      if ((status != MRAPI_SUCCESS) && (status != MRAPI_ERR_RWL_LOCKED)) {
        th_log_error("Task id %d failed to writer lock: %s\n",
                     task_id, mrapi_display_status(status,status_buff,
                                                   sizeof(status_buff)));
        th_task_exit(task_id,NULL,0,TC_ERROR);
      }
    }
    
    if (task_id == 0) {
      delta_write += th_end_timing();
      if (delta_write < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        th_task_exit(task_id,NULL,0,TC_ERROR);    
      } 
    }

    mrapi_rwl_unlock(rwl, MRAPI_RWL_WRITER ,&status);
    if (status != MRAPI_SUCCESS) {
      th_log_error("Task id %d failed to unlock: %s\n",
                   task_id, mrapi_display_status(status,status_buff,
                                                 sizeof(status_buff)));
      th_task_exit(task_id,NULL,0,TC_ERROR);
    }
  }

  if (task_id == 0) {
    th_log("Locks acquired:\t%d\tAvg Read Time (in seconds):\t%Lf\n"
           "Avg Write Time (in seconds):\t%Lf\n"
           "Total Read Time:\t%Lf\tTotal Write Time:\t%Lf\n",
           op_count, delta_read / (read_op_count * op_count), 
           delta_write / op_count, delta_read, delta_write);
    
    /* TODO - how to handle files for bare metal */
    if (filename) {
      fprintf(outfile, "%s\t%s\t%d\t%d\t%d\t%Lf\t%Lf\n", th_get_impl_string(),
              op_type_string(), num_tasks, op_count, read_op_count,
              delta_read / (read_op_count * op_count), 
              delta_write / op_count);
    }
  }
      
  mrapi_finalize(&status);
  if (status != MRAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mrapi_display_status(status,status_buff,sizeof(status_buff)));
    th_task_exit(task_id,NULL,0,TC_ERROR);
  }


  th_task_exit(task_id,NULL,0,TC_OK);
}

/************************************************************/
/*                 U T I L I T I E S                        */
/************************************************************/

/*****************************************************************************
 * op_type_string
 *****************************************************************************/
char* op_type_string()
{
  switch (op_type) 
    {
    case USE_PTHREAD_MUTEX:
      return "pthread mutex";
      break;
    case USE_MRAPI_MUTEX:
      return "MRAPI mutex";
      break;
    case USE_SVR4_SEMAPHORE:
      return "SVR4 semaphore";
      break;
    case USE_MRAPI_SEMAPHORE:
      return "MRAPI semaphore";
      break;
    case USE_SVR4_RWL:
      return "SVR4-based reader/writer locks";
      break;
    case USE_MRAPI_RWL:
      return "MRAPI reader/writer locks";
      break;
    default:
      return "FIXME";
      break;
    }
}

/*---------------------------------------------------------------------------
 Create a semaphore with a specified initial value.
 If the semaphore already exists, we don't initialize it (of course).
 We return the semaphore ID if all OK, else -1.
 ---------------------------------------------------------------------------*/
int sem_create(key_t key, int initval)
{
  int id, semval;
  
  if (key == IPC_PRIVATE)
    return (-2);  /* not intended for private semaphores */
  else if (key == (key_t) -1)
    return (-1); /* probably an ftok() error by caller */
  
 again:
  if ((id = semget(key, 3, 0666 | IPC_CREAT)) < 0)
    return(-1); /* permission problem or tables full */
  /*
    When the semaphore is created, we know that the value of all 3 member is 0.
    Get a lock on the semaphore by waiting for [2] to equal to 0, then
    increment it.
    
    There is a race condition here. There is a possibility that
    between the semget() above and the semop() below, another
    process can call our sem_close() function which can remove
    the semaphore if that process is the last one using it.
    Therefore, we handle the error condition of an invalid
    semaphore ID specially below, and if it does happen, we just
    go back and create it again.
  */
  
  if (semop(id, &op_lock[0], 2) < 0){
    if (errno == EINVAL) goto again;
    th_log_error("Failed to lock semaphore [%s]\n", strerror(errno));
    return(-1);
  }
  
  /*
    Get the value of the process counter. If it equals 0,
    then no one has initialized the semaphore yet.
  */
  
  semctl_arg.val = 0;
  if ((semval = semctl(id, 1, GETVAL, semctl_arg)) < 0)
    th_log_error("Can't GETVAL [%s]\n", strerror(errno));
  
  if (semval == 0){
    /*
      We could initialize by doing a SETALL, but that
      would clear the adjust value that we set when we
      locked the semaphore above. Instead, we'll do 2
      system calls to initialize [0] and [1].
    */
    
    semctl_arg.val = initval;
    if (semctl(id, 0, SETVAL, semctl_arg) < 0)
      th_log_error("can't SETVAL[0] [%s]\n", strerror(errno));
    
    semctl_arg.val = BIG_COUNT;
    if (semctl(id, 1, SETVAL, semctl_arg) < 0)
      th_log_error("can't SETVAL[1] [%s]\n", strerror(errno));
  }
  
  /* Decrement the process counter and then release the lock. */
  if (semop(id, &op_endcreate[0], 2) < 0)
    th_log_error("can't end create [%s]\n", strerror(errno));
  
  return(id);

}


/*-------------------------------------------------------------------------
 Open a semaphore that must already exist.
 This function should be used, instead of sem_create(), if the caller
 knows that the semaphore must already exist. For example a client
 from a client-server pair would use this, if it is the server's
 responsiblity to create the semaphore.
 We return the semaphore ID if all OK, else -1.
 -------------------------------------------------------------------------*/
int sem_open(key_t key)
{
  int id;
  if (key == IPC_PRIVATE)
    return(-1); /* not intended for private semaphores */
  else if (key == (key_t) -1)
    return(-1); /* probably an ftok() error by caller */
  
  if ((id = semget(key, 3, 0)) < 0)
    return(-1); /* doesn't exist, or tables full */
  
  /* Decrement the process counter. We don't need a lock to do this */
  if (semop(id, &op_open[0], 1) < 0)
    th_log_error("can't open [%s]\n", strerror(errno));

  return(id);
}

/*-------------------------------------------------------------------------
 Remove a semaphore
 This call is intended to be called by a server, for example,
 when it is being shut down, as we do an IPC_RMID on the semaphore,
 regardless whether other processes may be using it or not.
 Most other processes should use sem_close() below.
 ------------------------------------------------------------------------*/
int sem_rm(int id)
{
  semctl_arg.val = 0;
  if (semctl(id, 0, IPC_RMID, semctl_arg) < 0) {
    th_log_error("can't IPC_RMID [%s]\n", strerror(errno));
  }
  return 0;
}
  
/*------------------------------------------------------------------------
 Close a semaphore
 Unlike the remove function above, this function is for a process
 to call before it exits, when it is done with the semaphore.
 We "decrement" the counter of processes using the semaphore, and
 if this was the last one, we can remove the semaphore.
 ------------------------------------------------------------------------*/
int sem_close(int id)
{
  int semval;
  
  /* The following semop() first gets a lock on the semaphore, 
     then increments [1] - the process counter */
  if (semop(id, &op_close[0], 3) < 0) {
    th_log_error("can't semop [%s]\n", strerror(errno));
    return -1;
  }
  
  /* Now that we have a lock, read the value of the process counter to see
     if this is the last reference to the semaphore.
     There is a race condition here - see the comments in sem_create(). */
  semctl_arg.val = 0;
  if ((semval = semctl(id, 1, GETVAL, semctl_arg)) < 0) {
    th_log_error("can't GETVAL [%s]\n", strerror(errno));
    return -1;
  }
  
  if (semval > BIG_COUNT) {
    th_log_error("sem[1] > BIG_COUNT");
    return -1;
  }
  else if (semval == BIG_COUNT) {
    if (sem_rm(id) < 0) {
      return -1;
    }
  }
  else if (semop(id, &op_unlock[0], 1) < 0) {
    th_log_error("can't unlock [%s]\n", strerror(errno));
    return -1;
  }
  
  return 0;
}
  
/*------------------------------------------------------------------------
 Wait until a semaphore's value is greater than 0, then decrement it by 1
 Dijkstra's P operation. Tanenbaum's DOWN operation.
 -----------------------------------------------------------------------*/
int sem_lock(int id, int num_locks)
{
  return sem_op(id, -1 * num_locks);
}

/*------------------------------------------------------------------------
 Increment a semaphore by 1.
 Dijkstra's V operation. Tanenbaum's UP operation.
 -----------------------------------------------------------------------*/
int sem_unlock(int id, int num_locks)
{
  return sem_op(id, num_locks);
}

/*------------------------------------------------------------------------
 General semaphore operation. Increment or decrement by a user-specified
 amount (positive or negative; amount can't be zero).
 -----------------------------------------------------------------------*/
int sem_op(int id, int value)
{
  if ((op_op[0].sem_op = value) == 0) {
    th_log_error("can't have value == 0\n");
    return -1;
  }
  
  return semop(id, &op_op[0], 1);
}
  

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
