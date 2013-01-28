/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or withoust
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
 *  Source: th_fork.c
 *
 *  Description: defines a fork task management implementation for
 *               use by testcases
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>


typedef struct _pipe_pair {
  int pipe_fds1[2];
  int pipe_fds2[2];
} pipe_pair;

/* this shared data is critical, accesses to it must be surrounded by mutexes */
static pthread_t listener_thread;
static pid_t parent_pid;
static pid_t task_pids[CONKER_MAX_TASKS];
static pipe_pair task_pipes[CONKER_MAX_TASKS];  
static int max_select_fd = -1;
static int listener_flag = 0;

static pthread_mutex_t th_mutex = PTHREAD_MUTEX_INITIALIZER;
static int th_fork_ok = 1;

/* forward declaration of thread listener */
void* listener_thread_body(void* tdata);

/* forward declaration of child at fork handler */
void child_fork_handler(void);

/*****************************************************************************
 *  th_get_impl_string -- get a string indicating the type of testharness
 *****************************************************************************/
char* th_get_impl_string(void)
{
  static char impl_string[] = "fork";
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
 *  th_task_initialize -- called to initialize the task API
 *     
 * th_task_initialize spawns a single listener pthread
 * this thread is solely to listen for messages from child 
 * processes, which will be the task_exit message
 * 
 *****************************************************************************/
th_status th_task_initialize(void)
{
  th_status status = TH_OK;
  int i = 0;

  assert(CONKER_MAX_TASKS < FD_SETSIZE);
  th_log_info("[TH_FORK]    -- th_task_initialize, max tasks = %d\n", 
              CONKER_MAX_TASKS);

  for (i = 0; i < CONKER_MAX_TASKS; i++) {
    task_pids[i] = -1;
    task_pipes[i].pipe_fds1[0] = task_pipes[i].pipe_fds1[1] = -1;
    task_pipes[i].pipe_fds2[0] = task_pipes[i].pipe_fds2[1] = -1;
  }

  listener_flag = 1;

  /* start the listener thread */
  if (pthread_create(&listener_thread, 
                     NULL, 
                     listener_thread_body, 
                     NULL) != 0) {
    status = TH_ERROR;
  }
  
  /* register child 'atfork' function */
  if (pthread_atfork(NULL, NULL, child_fork_handler) != 0) {
    status = TH_ERROR;
  }

  parent_pid = getpid();

  return status;
} 

/*****************************************************************************
 *  th_task_create -- called to create a new task
 *****************************************************************************/
th_status th_task_create(int task_id)
{
  pid_t pid;
  th_task_descriptor* td = NULL;
  int local_err = 0;

  th_log_info("[TH_FORK]    -- th_task_create, id = %d\n", task_id);

  /* BEGIN MUTEX */
  if (pthread_mutex_lock(&th_mutex) != 0) {
    th_log_error("Unable to obtain a mutex lock.\n");
    return TH_ERROR;
  } else {
    td = th_get_task_descriptor(task_id);
    if (td != NULL) {
      
      if (pipe(task_pipes[task_id].pipe_fds1) < 0 ||
          pipe(task_pipes[task_id].pipe_fds2) < 0 ) {
        th_log_error("Failure setting up pipe for task id %d!\n", task_id);
        return TH_ERROR;
      } else if (task_pipes[task_id].pipe_fds1[0] > max_select_fd) {
        max_select_fd = task_pipes[task_id].pipe_fds1[0];
      }
      
      /* optimistically mark it as running */
      td->status = TASK_RUNNING;
      
      if ((pid = fork()) < 0) {
        
        th_log_error("Failure while forking child task id %d\n", task_id);
        td->status = TASK_ERROR;
        
        /* END MUTEX */
        if ((local_err = pthread_mutex_unlock(&th_mutex)) != 0) {
          th_log_error("Unable to release a mutex lock(1), error=[%s].\n", 
                       strerror(local_err));
        }
        
        return TH_ERROR;
        
      } else if (pid == 0) {
        /* Note pid of 0 is not the _true_ pid, it's just what is returned to fork() */
 
        /* NOTE: child already released the mutex lock in the 'at fork' handler */
#if (PROFILING)
        monstartup ((u_long) &_start, (u_long) &etext);        
#endif
        th_log_info("[TH_FORK]    -- child pid for task id %d = %d\n", 
                    task_id, getpid());
        
        td->task_implementation(task_id,td->input_data,td->input_data_size);
        
        /* we end the child here because only the original parent */
        /* should do the work at the end of the testcase */
        
        th_log_info("[TH_FORK]    -- task id %d exiting\n", task_id);

        th_cleanup();
        
        /* finalize the testcase */
        if (tc_finalize() != TC_OK) {
          th_log_error("tc_finalize() returned error\n");
          exit(8);
        }
        
        exit(0);
        
      } else {
        
        /* parent returns from create call after updating data */

        task_pids[task_id] = pid;
        
        /* END MUTEX */
	local_err = pthread_mutex_unlock(&th_mutex);
        if (local_err != 0 && local_err != EPERM) {
          th_log_error("Unable to release a mutex lock(2), error=[%s].\n",
                       strerror(local_err));
          return TH_ERROR;
        }
      }
    } else {
      th_log_error("invalid task id passed to th_task_create, task_id=%d\n",task_id);
      return TH_ERROR;
    }
  }
    
  return TH_OK;
}

/*****************************************************************************
 *  th_task_exit -- called by a task when it exits; note this function
 *                  is called in the child process, the parent process
 *                  performs additional work in the listener thread
 *                  when it receives a msg from this function in the child
 *****************************************************************************/
th_status th_task_exit(int task_id, 
                       void* return_data, 
                       int return_data_size, 
                       task_status_code task_status)
{
  void* temp_buffer;
  int temp_buffer_size;
  char c;
  th_task_descriptor* td = th_get_task_descriptor(task_id);
  th_status status = TH_OK;

  th_log_info("[TH_FORK]    -- th_task_exit, id = %d, return_data = [%s], "
              "return_data_size = %d, status = %d\n", 
              task_id, (char*) return_data, return_data_size, task_status);

  if (td != NULL) {
        
    if (task_status == TASK_ERROR) {
      td->status = task_status;
    }
    
    td->return_data_size = return_data_size;

    /* send the updated task descriptor over the pipe as a single write:
       task descriptor + task output data */
    temp_buffer_size = sizeof(th_task_descriptor) + return_data_size;
    temp_buffer = calloc(temp_buffer_size, sizeof(char));
    
    memcpy(temp_buffer, td, sizeof(th_task_descriptor));
    memcpy(temp_buffer+sizeof(th_task_descriptor),
           return_data, return_data_size);

    if (write(task_pipes[task_id].pipe_fds1[1], 
              temp_buffer, temp_buffer_size) != 
        temp_buffer_size) {
      th_log_error("Unable to write to parent, task id = %d, err=%s\n", 
                   task_id, strerror(errno));
      free(temp_buffer);
      status = TH_ERROR;
    } else {
      
      /* wait for parent to release the child */
      if (read(task_pipes[task_id].pipe_fds2[0], &c, 1) != 1) {
        th_log_error("Unable to read response from parent\n");
      }
      
      if (c != 'p') {
        th_log_error("Incorrect response from parent\n");
        status = TH_ERROR;
      }
      
      th_log_info("[TH_FORK]    -- th_task_exit, task id %d sent %d bytes to parent\n",
                  task_id, temp_buffer_size);
      
      free(temp_buffer);
    }
    } else {
      th_log_error("invalid task id passed to th_task_exit, task_id=%d\n",task_id);
      return TH_ERROR;
    }


  return status;
}

/*****************************************************************************
 *  th_task_wait -- called to wait for a task to exit
 *****************************************************************************/
th_status th_task_wait(int task_id, 
                       void** return_data, 
                       int* return_data_size, 
                       task_status_code* task_status)
{
  th_status status = TH_OK;
  th_task_descriptor* td;
  int local_err = 0;
  int done = 0;

  th_log_info("[TH_FORK]    -- th_task_wait, task id = %d\n", task_id);

  while (!done && status == TH_OK) {
    /* BEGIN MUTEX */
    if (pthread_mutex_lock(&th_mutex) != 0) {
      th_log_error("Unable to obtain a mutex lock.\n");
      status = TH_ERROR;
    } else {
      td = th_get_task_descriptor(task_id);
      if (td != NULL) {     

        /* TODO - why is the status checking different in fork vs. pthreads ?? */

        if (td->status != TASK_RUNNING) {
          
          /* copy the return data */
          *(return_data) = (void *) malloc(td->return_data_size);
          *return_data_size = td->return_data_size;
          memcpy((void *) *(return_data), td->return_data, 
                 td->return_data_size);
          *task_status = td->status;
          
          done = 1;
        }
      } else {
        th_log_error("(1) Unable to locate task descriptor for task %d.\n",task_id);
        status = TH_ERROR;    
      }
      
      /* END MUTEX */
      if ((local_err = pthread_mutex_unlock(&th_mutex)) != 0) {
        th_log_error("Unable to release a mutex lock(3), error=[%s].\n",
                     strerror(local_err));
        status = TH_ERROR;
      }
      
      sched_yield();
    }
  }
  return status;
}

/*****************************************************************************
 *  th_task_finalize -- called to finalize the task API
 *****************************************************************************/
th_status th_task_finalize(void)
{
  th_status status = TH_OK;
  int local_err = 0;

  th_log_info("[TH_FORK]    -- th_task_finalize\n");

  /* BEGIN MUTEX */
  if (pthread_mutex_lock(&th_mutex) != 0) {
    th_log_error("Unable to obtain a mutex lock.\n");
    status = TH_ERROR;
  } else {
    
    /* tell the listener thread it is time to shut down */
    listener_flag = 0;
    
    /* END MUTEX */
    if ((local_err = pthread_mutex_unlock(&th_mutex)) != 0) {
      th_log_error("Unable to release a mutex lock(4), error=[%s].\n",
                   strerror(local_err));
      status = TH_ERROR;
    } else {
      /* calls join in the spawned thread */
      pthread_join(listener_thread, NULL);      
    }
  }

  return status;
}


/*****************************************************************************
 *  child_fork_handler -- releases locks held in parent
 *****************************************************************************/
void child_fork_handler(void)
{
  /* this is the recommended method for releasing the lock, although    */
  /* it seems like it would be fine to release it immediately following */
  /* the fork cakk in th_task_create.  I tested both, and both seem to  */
  /* work fine, but left it this way as recommended by the pthreads std */

  int local_err = 0;

  if (getpid() != parent_pid) {
    /* END MUTEX */
    local_err = pthread_mutex_unlock(&th_mutex);
    switch (local_err) {
    case 0:
    case EPERM:
      /* some pthreads implementations allow the child to release the lock */
      /* others say that the lock does not exists, we are OK with either   */
      break;
    default:
      th_log_error("Unable to release a mutex lock(5), error=[%s].\n",
                   strerror(local_err));
      break;
    }
  }
}


/*****************************************************************************
 *  listener_thread_body -- waits for tasks to call th_task_exit
 *****************************************************************************/
void* listener_thread_body(void* tdata)
{
  fd_set task_fdset;
  struct timeval timeout;
  int status;
  int i, tmp;
  int local_err = 0;
  th_task_descriptor* td_orig;
  th_task_descriptor td_from_child;

  th_log_info("[TH_FORK]    -- listener thread started\n");

  while (listener_flag) {

    /* BEGIN MUTEX */
    if (pthread_mutex_lock(&th_mutex) != 0) {
      th_log_error("(1) Unable to obtain a mutex lock.\n");
    } else {      
      FD_ZERO(&task_fdset);
      for (i = 0; i < tc_get_num_tasks(); i++) {
        if (task_pids[i] >= 0) {
          FD_SET(task_pipes[i].pipe_fds1[0], &task_fdset);
        }
      }
      
      timeout.tv_sec = 0;
      timeout.tv_usec = 10;
      
      th_log_info("[TH_FORK]    -- listener thread calling select ...\n");
      
      status = select(max_select_fd + 1, &task_fdset, NULL, NULL, &timeout);
      
      th_log_info("[TH_FORK]    -- select done, status = %d\n", status);
      
      switch (status) {
      case 0: 
        /* nothing to read */
        break;
      case -1: 
        /* some kind of error */
        th_log_error("Error calling select!\n");
        break;
      default:
        /* read every fd that is set */
        for (i = 0; i < tc_get_num_tasks(); i++) {

          if (task_pids[i] >= 0) {
            
            td_orig = th_get_task_descriptor(i);
            
            if (td_orig != NULL) {
              
              if (FD_ISSET(task_pipes[i].pipe_fds1[0], &task_fdset)) {
                if ((tmp = read(task_pipes[i].pipe_fds1[0], &td_from_child, 
                                sizeof(td_from_child))) != sizeof(td_from_child)) {
                  th_log_error("Failed to read msg part 1 from task id %d.\n", i);
                  td_orig->status = TASK_ERROR;
                  break;
                }
                
                td_orig->return_data = malloc(td_from_child.return_data_size);
                
                if (read(task_pipes[i].pipe_fds1[0], td_orig->return_data, 
                         td_from_child.return_data_size) != td_from_child.return_data_size) {
                  th_log_error("Failed to read msg part 2 from task id %d\n", i);
                  td_orig->status = TASK_ERROR;
                  break;
                }   
                td_orig->return_data_size = td_from_child.return_data_size;
                if (td_from_child.status == TASK_RUNNING) { 
                  td_orig->status = TASK_EXITED;
                } else { 
                  td_orig->status = TASK_ERROR;
                  th_log_error("task %d exited with an error\n", i);
                } 
                
                /* tell the child it is OK to exit now */
                
                if (write(task_pipes[i].pipe_fds2[1], "p",1) != 1) {
                  th_log_error("error in tell_child\n");
                }
                if (td_orig->return_data_size > 0) {
                  th_log_info("[TH_FORK]    -- task exit msg, task id = %d, data = '%s'\n", 
                              i, (char *) td_orig->return_data);
                } else {
                  th_log_info("[TH_FORK]    -- task exit msg, task id = %d, data = <empty>\n", i);
                }
              }            
            } else {
              th_log_error("(2) Unable to locate task descriptor for task %d\n",i);
            }
          }
        }
        break;
      }
      /* END MUTEX */
      if ((local_err = pthread_mutex_unlock(&th_mutex)) != 0) {
        th_log_error("Unable to release a mutex lock(6), error=[%s].\n",
                     strerror(local_err));
      }
    }   
    sched_yield();
  }
  
  return &th_fork_ok;
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
