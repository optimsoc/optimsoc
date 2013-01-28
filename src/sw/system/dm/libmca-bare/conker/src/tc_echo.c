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
 *  Source: tc_echo.c
 *
 *  Description: the echo testcase 
 *
 *****************************************************************************/
 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mca_config.h>

#include <stdio.h>  /* TODO - remove stdio after log file handling resolved */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <poll.h>
#include <mca_utils.h>

/* TODO - how to compile out sockets, pthreads, sched_yield */

/* #include <quantify.h> */

/* testcase config options */
typedef enum _op_types {
  USE_UNDEFINED = 0,
  USE_MESSAGES,
  USE_PACKETS,
  USE_SCALARS,
  USE_PIPES,
  USE_SOCKETS
} op_types;

static op_types  op_type = USE_UNDEFINED;
static int       op_size = 4;
static int       op_count = 1;
static int       num_tasks = 2;
static int       use_async = 0;
static tc_status final_status = TC_OK;
static int       using_pthreads = 0;
static char*     filename = NULL;
static FILE*     outfile;
#define TIMEOUT MCAPI_TIMEOUT_INFINITE

#define WRITE_IDX 1
#define READ_IDX  0

#define MAX_ECHO_SIZE 1024
#define ECHO_BUF_SIZE MAX_ECHO_SIZE + 1

/* the actual algorith resides in echo_driver */
typedef int  (*td_fptr)   (int task_id);
typedef int  (*comm_fptr) (int task_id, char* buf, int buf_size);

void echo_driver(int task_id, 
                 td_fptr f_config, 
                 comm_fptr f_send, 
                 comm_fptr f_recv, 
                 td_fptr f_cleanup);

/* forward declaration of the task_bodies -- they setup the call to echo_driver */
void task_body_msg_blocking(int task_id, void* input_data, int input_data_size);
void task_body_msg_non_blocking(int task_id, void* input_data, int input_data_size);
void task_body_pkt_blocking(int task_id, void* input_data, int input_data_size);
void task_body_pkt_non_blocking(int task_id, void* input_data, int input_data_size);
void task_body_scalar_blocking(int task_id, void* input_data, int input_data_size);
void task_body_pipes_blocking(int task_id, void* input_data, int input_data_size);
void task_body_sockets_blocking(int task_id, void* input_data, int input_data_size);


/* datatypes and functions for pthreads reader/writer locks */
typedef struct rdwr_var {
  int readers_reading;
  int writer_writing;
  pthread_mutex_t mutex;
  pthread_cond_t lock_free;
} pthread_rdwr_t;

static pthread_rdwr_t locks[CONKER_MAX_TASKS];

int pthread_rdwr_init(pthread_rdwr_t* rdwrp);
int pthread_rdwr_rlock(pthread_rdwr_t* rdwrp);
int pthread_rdwr_wlock(pthread_rdwr_t* rdwrp);
int pthread_rdwr_runlock(pthread_rdwr_t* rdwrp);
int pthread_rdwr_wunlock(pthread_rdwr_t* rdwrp);


/* helper functions and datatypes for pipes & sockets, both use file descriptors */
typedef struct _fds_pair {
  int fds[2];
} fds_pair;

static fds_pair fds_array[CONKER_MAX_TASKS];
static int remote_fds[CONKER_MAX_TASKS];

int read_fds(int index, char* buf, int buf_size);
int write_fds(int index, char* buf, int buf_size);

void setup_pipes(void);
void setup_sockets(void);

/* helper functions and datatypes for MCAPI */
typedef struct _endpt_pair {
  mcapi_endpoint_t endpts[2];
} endpt_pair;

typedef struct _pkt_hndl_pair {
  mcapi_pktchan_send_hndl_t send;
  mcapi_pktchan_recv_hndl_t recv;
} pkt_hndl_pair;

typedef struct _scl_hndl_pair {
  mcapi_sclchan_send_hndl_t send;
  mcapi_sclchan_recv_hndl_t recv;
} scl_hndl_pair;

static endpt_pair local_endpts[CONKER_MAX_TASKS];
static mcapi_endpoint_t remote_endpts[CONKER_MAX_TASKS];
static pkt_hndl_pair pkt_hndls[CONKER_MAX_TASKS];
static scl_hndl_pair scl_hndls[CONKER_MAX_TASKS];

int read_mcapi_msg(int index, char* buf, int buf_size);
int write_mcapi_msg(int index, char* buf, int write_size);

int read_mcapi_msg_a(int index, char* buf, int buf_size);
int write_mcapi_msg_a(int index, char* buf, int write_size);

int setup_pkt_channels(int task_id);

int read_mcapi_pkt(int index, char* buf, int buf_size);
int write_mcapi_pkt(int index, char* buf, int write_size);

int read_mcapi_pkt_a(int index, char* buf, int buf_size);
int write_mcapi_pkt_a(int index, char* buf, int write_size);

int setup_scalar_channels(int task_id);

int read_mcapi_scalar(int index, char* buf, int buf_size);
int write_mcapi_scalar(int index, char* buf, int write_size);

int setup_echo_endpoints(int task_id);

int finalize_mcapi(int task_id);

/* other misc functions */
char* op_type_string(void);

char* tc_get_testcase_name(void) { return "tc_echo"; }
int tc_get_num_tasks(void) { return num_tasks; }


/*****************************************************************************
 *  tc_print_arg_usage -- print out the descriptions of arguments
 *****************************************************************************/
void tc_print_arg_usage(void)
{
  th_log("  -m : test using MCAPI messages                           [default]\n");
  th_log("  -p : test using MCAPI packets\n");
  th_log("  -s : test using MCAPI scalars\n");
  th_log("  -P : test using pipes\n");
  th_log("  -S : test using sockets (UDP)\n");
  th_log("  -a : use non-blocking send/recv\n");
  th_log("  -n : number of sends                                     [default = 1]\n");
  th_log("  -b <size> : number of bytes per send                     [default = 4]\n");
  th_log("  -i <hops> : number of intermediate hops                  [default = 1]\n");
  th_log("  -f <filename> : append stats to file\n");
  th_log("\n  NOTES: \n");
  th_log("    maximum message size = %d\n", MCAPI_MAX_MSG_SIZE);
  th_log("    maximum packet  size = %d\n", MCAPI_MAX_PKT_SIZE);
  th_log("    maximum echo    size = %d\n", MAX_ECHO_SIZE);
  th_log("    scalar sizes are {1, 2, 4, 8} bytes only\n");
  th_log("    -m -p -s -f are mutually exclusive\n");
  th_log("    -a -s -P are mutually exclusive\n");
}


/*****************************************************************************
 *  arg_handler
 *****************************************************************************/
int arg_handler (char opt, const char* value)
{
  int error = 0;

  switch (opt) {
    
  case 'm':     /* use messages */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_MESSAGES;
    } else {
      error = 1;
    }
    break;      
    
  case 'p':     /* use packets */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_PACKETS;
    } else {
      error = 1;
    }
    break;      
    
  case 's':     /* use scalars */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_SCALARS;
    } else {
      error = 1;
    }
    break;      
    
  case 'P':     /* use pipes */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_PIPES;
    } else {
      error = 1;
    }
    break;      

  case 'S':     /* use sockets */
    if (op_type == USE_UNDEFINED) {
      op_type = USE_SOCKETS;
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
    
  case 'n':     /* number of sends */
    if (value) {
      op_count = atoi(value);
    } else {
      error = 1;
    }
    break;      
    
  case 'b':     /* number of bytes/send */
    if (value) {
      op_size = atoi(value);
    } else {
      error = 1;
    }
    break;      
    
  case 'i':     /* number of hops (e.g., number slave tasks) */
    if (value) {
      num_tasks = atoi(value) + 1;
    } else {
      error = 1;
    }
    break;      
    
  case 'a':     /* use non-blocking operations for send/recv */
    use_async = 1;
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

  th_register_args("mpsPSaf:n:b:i:",arg_handler);

  return status;
}

/*****************************************************************************
 *  tc_initialize  -- called by test harness to initialize the testcase
 *****************************************************************************/
tc_status tc_initialize(void)
{
  tc_status status = TC_OK;
  int i = 0;
  tc_task_body_fptr_t fp = NULL;

  if (op_type == USE_UNDEFINED) {
    op_type = USE_MESSAGES;
  }

  /* check the operation size & async-ness */
  switch (op_type) 
    {
    case USE_MESSAGES:
      if (op_size < 1 || op_size > MCAPI_MAX_MSG_SIZE || op_size > MAX_ECHO_SIZE) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      }
      break;
    case USE_PACKETS:
      if (op_size < 1 || op_size > MCAPI_MAX_PKT_SIZE || op_size > MAX_ECHO_SIZE) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      }
      break;
    case USE_SCALARS:
      if (op_size != 1 && op_size != 2 && op_size != 4 && op_size != 8) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      } else if (use_async) {
        th_log_error("async cannot be used with scalars\n");
        status = TC_ERROR;
      }
      break;
    case USE_PIPES:
      if (op_size < 1 || op_size > MAX_ECHO_SIZE) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      } else if (use_async) {
        th_log_error("async cannot be used with pipes\n");
        status = TC_ERROR;
      }      
      break;
    case USE_SOCKETS:
      if (op_size < 1 || op_size > MAX_ECHO_SIZE) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      } else if (use_async) {
        th_log_error("async cannot be used with sockets\n");
        status = TC_ERROR;
      }      
      break;
    default:
      break;
    }
  
  /* check the operation count */
  if (op_count < 1) {
    th_log_error("number of sends must be > 0\n");
    status = TC_ERROR;
  }

  /* check the number of hops */
  if (num_tasks < 2 || num_tasks > CONKER_MAX_TASKS) {
    th_log_error("number of hops out of range [2-%d]\n", CONKER_MAX_TASKS);
    status = TC_ERROR;
  }

  /* if we got here we must have good args ... */
  th_log_info("[TC_ECHO]    -- tc_initialize, pid = %d\n", getpid());

  /* additional initialization items */
  for (i = 0; i < CONKER_MAX_TASKS; i++) {
    fds_array[i].fds[READ_IDX] = fds_array[i].fds[WRITE_IDX] = -1;
  }

  if (strcmp(th_get_impl_string(),"pthread") == 0) {
    using_pthreads = 1;
  }

  /* print config */
  switch (op_type) {
    case USE_MESSAGES:
      th_log("[TC_ECHO]    -- using MCAPI messages\n");
      break;
    case USE_PACKETS:
      th_log("[TC_ECHO]    -- using MCAPI packets\n");
      break;
    case USE_SCALARS:
      th_log("[TC_ECHO]    -- using MCAPI scalars\n");
      break;
    case USE_PIPES:
      th_log("[TC_ECHO]    -- using pipes\n");
      setup_pipes();
      break;
    case USE_SOCKETS:
      th_log("[TC_ECHO]    -- using sockets\n");
      setup_sockets();
      break;
    default:
      break;
  }
  if (use_async) {
    th_log("[TC_ECHO]    -- using non-blocking send/recv\n");
  }
  th_log("[TC_ECHO]    -- node offset:\t%d\n", th_get_node_offset());
  th_log("[TC_ECHO]    -- #sends:\t%d\n", op_count);
  th_log("[TC_ECHO]    -- #bytes/send:\t%d\n", op_size);
  th_log("[TC_ECHO]    -- #hops:\t%d\n", num_tasks - 1);
  if (filename) {
    /* TODO -- how to deal with output log files in bare metal */
    th_log("[TC_ECHO]    -- results will be appended to file: %s\n",filename);
    outfile = fopen(filename, "a+");
  }

  for (i = 0; status == TC_OK && i < num_tasks; i++) {
    
    switch (op_type) 
      {
      case USE_MESSAGES:
        if (!use_async) {
          fp = task_body_msg_blocking;
        } else {
          fp = task_body_msg_non_blocking;
        }
        break;
      case USE_PACKETS:
        if (!use_async) {
          fp = task_body_pkt_blocking;
        } else {
          fp = task_body_pkt_non_blocking;
        }
        break;
      case USE_SCALARS:
        fp = task_body_scalar_blocking;
        break;
      case USE_PIPES:
        fp = task_body_pipes_blocking;
        break;
      case USE_SOCKETS:
        fp = task_body_sockets_blocking;
        break;
      default:
        /* TODO - this is an error and should be handled !! */
        break;
      }
        
    if (th_register_task(i,"echo_task",fp,NULL,0) != TH_OK) {
      th_log_error("Unable to register task number %d.  Status = %d\n",
                   i, status);
      status = TC_ERROR;
    } else {
      th_log_info("[TC_ECHO]    -- task registered, id = %d\n", i);
    }
  }

  return status;
}

/*****************************************************************************
 *  tc_run  -- called by test harness to execute the testcase
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
      th_log("[TC_ECHO] -- task created, id = %d\n", i);
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
          th_log_info("[TC_ECHO]    -- return data from task %d = '%s'\n", i, 
                      (char *) (return_data));
        } else {
          th_log_info("[TC_ECHO]    -- return data from task %d = <empty>\n", i);
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
 *  tc_report  -- called by test harness for testcase to report summary results
 *****************************************************************************/
tc_status tc_report(void) 
{
  /* TODO - make reporting work from this function */
  return TC_OK;
}

/*****************************************************************************
 *  tc_finalize  -- called by test harness to alow testcase to clean up
 *****************************************************************************/
tc_status tc_finalize(void) 
{
  if (final_status == TC_OK) {
    th_log("[TC_ECHO]    -- testcase finished successfully, pid = %d\n", getpid());
  } else {
    th_log("[TC_ECHO]    -- testcase finished with errors, pid = %d\n", getpid());
  }

  if (filename) {
    fclose(outfile);
    free(filename);
    filename = NULL;
  }

  return TC_OK;
}

/*****************************************************************************
 * echo_driver - thee actual algorithm, with function pointers for different
 *               test types 
 *****************************************************************************/
void echo_driver(int task_id, 
                 td_fptr f_config, 
                 comm_fptr f_send, 
                 comm_fptr f_recv, 
                 td_fptr f_cleanup)
{
  task_status_code status = TASK_RUNNING;
  int send_idx = task_id;
  int recv_idx = task_id;
  char msgbuf[ECHO_BUF_SIZE] = "";
  char expected_value[ECHO_BUF_SIZE] = "";
  int msgsize = 0;
  int count = 0;
  long double delta = 0.0;


  /* call configuration function */
  if (f_config) {
    if ((*f_config)(task_id) != 0) {
      th_log_error("Task id %d failed to configure properly\n", task_id);
      status = TASK_ERROR;  
    } 
  }

  if (status != TASK_ERROR) {

    if (op_size > 0 && task_id == 0) {
      memset(expected_value, 'A', op_size - 1);
      expected_value[op_size - 1] = 0;
      expected_value[0] += num_tasks - 1; 
    } else {
      expected_value[0] = 0;
    }
    
    if (task_id == 0) {
      /* begin timer in MASTER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        status = TASK_ERROR;  
      }
    }
    
    /* testing loop */


/*     quantify_start_recording_data(); */

    for (count = 0; count < op_count; count++) {
      
      if (task_id == 0) {
        
        if (op_size > 0) {
          memset(msgbuf, 'A', op_size - 1);
          msgbuf[op_size - 1] = 0;
        } else {
          msgbuf[0] = 0;
        }
        
        /* generate a msg, send to neighbor, wait for return */
        th_log_info("[TC_ECHO]    -- Master Sending: '%s', task=%d, send_idx=%d\n", 
                    msgbuf, task_id, send_idx);
               
        if ((f_send)(send_idx, msgbuf, op_size) != op_size) {
          th_log_error("Task id %d failed to send msg\n", task_id);
          status = TASK_ERROR;  
        } else {
          
          msgbuf[0] = 0;
          
          /* wait to get it back ... */
          if ((msgsize = (f_recv)(recv_idx, msgbuf, ECHO_BUF_SIZE)) != op_size) {
            th_log_error("Task id %d received wrong message size: %d\n",
                         task_id, msgsize);
            status = TASK_ERROR;  
          } else {
            
            /* check the received msg */
            if (strcmp(msgbuf, expected_value) != 0) {
              th_log_error("Task id %d received corrupted msg: %s, expected %s\n",
                           task_id, msgbuf, expected_value);
              status = TASK_ERROR;  
            }
            
            th_log_info("[TC_ECHO]    -- Master Node %d received '%s'\n", 
                        task_id, msgbuf);
          }
        }
      } else {
        
        msgbuf[0] = 0;
        
        /* wait for a msg and fwd it */
        th_log_info("[TC_ECHO]    -- Slave Recv, task=%d, recv_idx=%d \n", 
                    task_id, recv_idx);
        
        if ((msgsize = (f_recv)(recv_idx, msgbuf, ECHO_BUF_SIZE)) != op_size) {
          th_log_error("Task id %d failed to receive message\n",
                       task_id);
          status = TASK_ERROR;  
        } else {
          
          th_log_info("[TC_ECHO]    -- Slave Node %d received '%s'\n", task_id ,msgbuf);
          
          if (op_size > 0) {
            msgbuf[0]++;
          }
          
          th_log_info("[TC_ECHO]    -- Slave Sending: '%s' from node %d\n", 
                      msgbuf, task_id);
          
          if ((f_send)(send_idx, msgbuf, op_size) != op_size) {
            th_log_error("Task id %d failed to send msg\n", task_id);
            status = TASK_ERROR;  
          }
          
          th_log_info("[TC_ECHO]    -- Slave, Send done for node %d\n", task_id);
         }
      }
    }
    
    if (task_id == 0) {
      /* end timer and calculate avg roundtrip time */
      delta = th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        status = TASK_ERROR;  
      } else {

        /* quantify_stop_recording_data(); */
        th_log("Messages sent:\t%d\tAverage Time (in seconds):\t%Lf\n"
               "Total Time (in seconds):\t%Lf\n",
               op_count, delta / op_count, delta);

        /* TODO - how to handle files for bare metal */
        if (filename) {
          fprintf(outfile, "%s\t%s\t%d\t%d\t%d\t%Lf\t%Lf\n", th_get_impl_string(),
                  op_type_string(), num_tasks - 1, op_size, op_count, delta / op_count,delta);
        }
      }
    }
    
    /* call cleanup function */
    if (f_cleanup) {
      if ((*f_cleanup)(task_id) != 0) {
        th_log_error("Task id %d failed to cleanup properly\n", task_id);
        status = TASK_ERROR;  
      }
    }
  }

  th_task_exit(task_id, NULL, 0, status);
}

/***********************************************/
/*         M C A P I   M E S S A G E S         */
/***********************************************/

/*****************************************************************************
 *  task_body_msg_blocking
 *****************************************************************************/
void task_body_msg_blocking(int task_id, 
                            void* input_data, 
                            int input_data_size) 
{
  echo_driver(task_id, 
              setup_echo_endpoints, 
              write_mcapi_msg, 
              read_mcapi_msg, 
              finalize_mcapi);
}

/*****************************************************************************
 *  task_body_msg_non_blocking
 *****************************************************************************/
void task_body_msg_non_blocking(int task_id, 
                                void* input_data, 
                                int input_data_size) 
{
  echo_driver(task_id, 
              setup_echo_endpoints, 
              write_mcapi_msg_a, 
              read_mcapi_msg_a, 
              finalize_mcapi);
}

/*****************************************************************************
 *  read_mcapi_msg
 *****************************************************************************/
int read_mcapi_msg(int index, char* buf, int buf_size)
{
  mcapi_status_t status;
  size_t msgsize = 0;

  mcapi_msg_recv(local_endpts[index].endpts[READ_IDX], 
                 (void *) buf, buf_size, &msgsize, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv msg: %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  
  return (int) msgsize;
}

/*****************************************************************************
 *  write_mcapi_msg
 *****************************************************************************/
int write_mcapi_msg(int index, char* buf, int write_size)
{
  mcapi_status_t status;
  mcapi_priority_t priority = 1;
  mcapi_msg_send(local_endpts[index].endpts[WRITE_IDX],
                 remote_endpts[index], buf, write_size, priority,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send msg: %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return write_size;
}

/*****************************************************************************
 *  read_mcapi_msg_a
 *****************************************************************************/
int read_mcapi_msg_a(int index, char* buf, int buf_size)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t msgsize = 0;

  mcapi_msg_recv_i(local_endpts[index].endpts[READ_IDX], 
                   (void *) buf, buf_size, &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv msg (recv): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  
  mcapi_wait(&request, &msgsize, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv msg (wait): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return (int) msgsize;
}

/*****************************************************************************
 *  write_mcapi_msg_a
 *****************************************************************************/
int write_mcapi_msg_a(int index, char* buf, int write_size)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t msgsize = 0;
  mcapi_priority_t priority = 1;

  mcapi_msg_send_i(local_endpts[index].endpts[WRITE_IDX],
                 remote_endpts[index], buf, write_size, priority,&request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send msg (send): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  mcapi_wait(&request, &msgsize, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send msg (wait): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return (int) msgsize;
}


/*********************************************/
/*         M C A P I   P A C K E T S         */
/*********************************************/

/*****************************************************************************
 *  task_body_pkt_blocking
 *****************************************************************************/
void task_body_pkt_blocking(int task_id, 
                            void* input_data, 
                            int input_data_size) 
{
  echo_driver(task_id, 
              setup_pkt_channels, 
              write_mcapi_pkt, 
              read_mcapi_pkt, 
              finalize_mcapi);
}

/*****************************************************************************
 *  task_body_pkt_non_blocking
 *****************************************************************************/
void task_body_pkt_non_blocking(int task_id, 
                                void* input_data, 
                                int input_data_size)
{
  echo_driver(task_id, 
              setup_pkt_channels, 
              write_mcapi_pkt_a, 
              read_mcapi_pkt_a, 
              finalize_mcapi);
}

/*****************************************************************************
 *  setup_pkt_channels
 *****************************************************************************/
int setup_pkt_channels(int task_id)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t dummy;
  

  /* create/ get endpoints */
  if (setup_echo_endpoints(task_id) != 0) {
    return -1;
  }


  /* connect the write channel only */
  mcapi_pktchan_connect_i(local_endpts[task_id].endpts[WRITE_IDX],
                          remote_endpts[task_id], 
                          &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (1): %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  mcapi_wait(&request, &dummy, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (2): %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* open the write channel */
  mcapi_pktchan_send_open_i(&(pkt_hndls[task_id].send),
                            local_endpts[task_id].endpts[WRITE_IDX], 
                            &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (3) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  mcapi_wait(&request, &dummy, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (4) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* open the read channel */
  /* the read channel may not yet be connected */
  mcapi_pktchan_recv_open_i(&(pkt_hndls[task_id].recv),
                              local_endpts[task_id].endpts[READ_IDX], 
                              &request, &status);
  mcapi_wait(&request, &dummy, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (5) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return 0;
}

/*****************************************************************************
 *  read_mcapi_pkt
 *****************************************************************************/
int read_mcapi_pkt(int index, char* buf, int buf_size)
{
  mcapi_status_t status;
  size_t pktsize = 0;
  char* tmpbuf;

  mcapi_pktchan_recv(pkt_hndls[index].recv, (void **) ((void*)&tmpbuf), &pktsize, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv packet: %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  } else {
    strncpy(buf, tmpbuf, pktsize);
    mcapi_pktchan_release((void *)tmpbuf, &status);
    if (status != MCAPI_SUCCESS){
      th_log_error("Task id %d failed to free a buffer: %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } 
  }
  
  return (int) pktsize;
}

/*****************************************************************************
 *  write_mcapi_pkt
 *****************************************************************************/
int write_mcapi_pkt(int index, char* buf, int write_size)
{
  mcapi_status_t status;

  mcapi_pktchan_send(pkt_hndls[index].send, buf, write_size, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send MCAPI packet : %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return write_size;
}

/*****************************************************************************
 *  read_mcapi_pkt_a
 *****************************************************************************/
int read_mcapi_pkt_a(int index, char* buf, int buf_size)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t pktsize = 0;
  char* tmpbuf;

  mcapi_pktchan_recv_i(pkt_hndls[index].recv, (void**)((void*)&tmpbuf), &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv pkt (recv): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  
  mcapi_wait(&request, &pktsize, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv pkt (wait): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  } else {
    strncpy(buf, tmpbuf, pktsize);
    mcapi_pktchan_release((void *)tmpbuf, &status);
    if (status != MCAPI_SUCCESS){
      th_log_error("Task id %d failed to free a buffer: %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } 
  }

  return (int) pktsize;
}

/*****************************************************************************
 *  write_mcapi_pkt_a
 *****************************************************************************/
int write_mcapi_pkt_a(int index, char* buf, int write_size)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t pktsize = 0;

  mcapi_pktchan_send_i(pkt_hndls[index].send, buf, write_size, &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send pkt (send): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  mcapi_wait(&request, &pktsize, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send pkt (wait): %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return (int) pktsize;
}


/*********************************************/
/*         M C A P I   S C A L A R S         */
/*********************************************/

/*****************************************************************************
 *  task_body_scalar_blocking
 *****************************************************************************/
void task_body_scalar_blocking(int task_id, 
                               void* input_data, 
                               int input_data_size) 
{
  echo_driver(task_id, 
              setup_scalar_channels, 
              write_mcapi_scalar, 
              read_mcapi_scalar, 
              finalize_mcapi);
}



/*****************************************************************************
 *  setup_scalar_channels
 *****************************************************************************/
int setup_scalar_channels(int task_id)
{
  mcapi_status_t status;
  mcapi_request_t request;
  size_t dummy;
  
  /* create/ get endpoints */
  if (setup_echo_endpoints(task_id) != 0) {
    return -1;
  }

  /* connect the write channel only */
  mcapi_sclchan_connect_i(local_endpts[task_id].endpts[WRITE_IDX],
                          remote_endpts[task_id], 
                          &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (1): %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  mcapi_wait(&request, &dummy, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (2): %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* open the write channel */
  mcapi_sclchan_send_open_i(&(scl_hndls[task_id].send),
                            local_endpts[task_id].endpts[WRITE_IDX], 
                            &request, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (3) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  mcapi_wait(&request, &dummy, TIMEOUT, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (4) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* open the read channel */
  /* the read channel may not yet be connected */
  mcapi_sclchan_recv_open_i(&(scl_hndls[task_id].recv),
                              local_endpts[task_id].endpts[READ_IDX], 
                              &request, &status);
  mcapi_wait(&request, &dummy, TIMEOUT, &status); 
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize (5) : %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return 0;
}

/*****************************************************************************
 *  read_mcapi_scalar
 *****************************************************************************/
int read_mcapi_scalar(int index, char* buf, int buf_size)
{
  mcapi_status_t status;

  switch (op_size) {
  case 1:
    *buf = mcapi_sclchan_recv_uint8(scl_hndls[index].recv, &status);
    break;
  case 2:
    *buf = mcapi_sclchan_recv_uint16(scl_hndls[index].recv, &status);
    break;
  case 4:
    *buf = mcapi_sclchan_recv_uint32(scl_hndls[index].recv, &status);
    break;
  case 8:
    *buf = mcapi_sclchan_recv_uint64(scl_hndls[index].recv, &status);
    break;
  default:
    th_log_error("Invalid scalar recv size requested: %d\n", op_size);
    break;
  }

  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to recv MCAPI scalar : %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  return op_size;
}

/*****************************************************************************
 *  write_mcapi_scalar
 *****************************************************************************/
int write_mcapi_scalar(int index, char* buf, int write_size)
{
  mcapi_status_t status;

  switch (op_size) {
  case 1:
    mcapi_sclchan_send_uint8(scl_hndls[index].send, (uint8_t) *buf, &status);
    break;
  case 2:
    mcapi_sclchan_send_uint16(scl_hndls[index].send, (uint16_t) *buf, &status);
    break;
  case 4:
    mcapi_sclchan_send_uint32(scl_hndls[index].send, (uint32_t) *buf, &status);
    break;
  case 8:
    mcapi_sclchan_send_uint64(scl_hndls[index].send, (uint64_t) *buf, &status);
    break;
  default:
    th_log_error("Invalid scalar send size requested: %d\n", op_size);
    return -1;
  }

  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to send MCAPI scalar : %s\n",
                 index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return write_size;
}



/**********************************************/
/*        M C A P I   G E N E R A L           */
/**********************************************/

/*****************************************************************************
 *  setup_echo_endpoints
 *****************************************************************************/
int setup_echo_endpoints(int task_id)
{
  mcapi_status_t status;
  int local_send_port = 0;
  int local_recv_port = 1;  
  int next_node = (task_id == num_tasks - 1) ? 
    0 + th_get_node_offset() : 
    task_id + 1 + th_get_node_offset();
  int next_recv_port = 1;
  mcapi_info_t version;
  mcapi_param_t parms;

  /* TODO -- this needs to be cleaned up */
  /* figure out how many cores we have, then split the tasks up evenly among the cores */
#if (WITH_AFFINITY != 0)
  int i;
  int num_cores = 2;
  int tasks_per_core = num_tasks/num_cores;
  int core = 0;
  for (i = 0; i < num_tasks; i+=tasks_per_core) {
    if ((i <= task_id) && ((i + tasks_per_core) >= task_id )) {
      th_log("[TC_ECHO]    -- setting task_id(%i) affinity to core(%i)\n",task_id,core);
      th_set_affinity(core);
      break;
    }
   core++;
  }
#endif
 
  /* initialize */
  //mca_set_debug_level(1); 
  mcapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* create local_send endpoint */
  th_log_info("[TC_ECHO]    -- creating local send endpoint node=%d, port=%d\n", 
              task_id + th_get_node_offset(), local_send_port);

  local_endpts[task_id].endpts[WRITE_IDX] = 
    mcapi_endpoint_create(local_send_port, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to create local send endpoint: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* create local_recv endpoint */
  th_log_info("[TC_ECHO]    -- creating local recv endpoint node=%d, port=%d\n", 
              task_id + th_get_node_offset(), local_recv_port);

  local_endpts[task_id].endpts[READ_IDX] = 
    mcapi_endpoint_create(local_recv_port, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to create local receive endpoint: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* get the endpoint to send to */
  th_log_info("[TC_ECHO]    -- get next recv endpt, node=%d, next_node=%d, next_recv_port=%d\n", 
              task_id, next_node, next_recv_port);

  remote_endpts[task_id] = 
    mcapi_endpoint_get(CONKER_TESTCASE_DOMAIN, next_node, next_recv_port, TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to get next recv endpoint: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  return 0;
}


/*****************************************************************************
 *  finalize_mcapi
 *****************************************************************************/
int finalize_mcapi(int task_id)
{
  mcapi_status_t status;

  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to finalize %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }
  return 0;
}


/***********************************************/
/*                P  I  P  E  S                */
/***********************************************/

/*****************************************************************************
 *  setup_pipes
 *****************************************************************************/
void setup_pipes(void) 
{
  int i = 0;
  int recv_idx = -1;

  for (i = 0; i < num_tasks; i++) {
    if (pipe(fds_array[i].fds) < 0) {
      th_log_error("Failure setting up pipe for task id %d!\n", i);
      exit(1);
    }
    if (using_pthreads) {
      pthread_rdwr_init(&(locks[i]));
    }
  }

  for (i = 0; i < num_tasks; i++) {
    recv_idx = (i == 0) ? num_tasks - 1 : i - 1;
    remote_fds[i] = fds_array[recv_idx].fds[READ_IDX];
  }
}

/*****************************************************************************
 *  task_body_pipes_blocking
 *****************************************************************************/
void task_body_pipes_blocking(int task_id, 
                              void* input_data, 
                              int input_data_size) 
{
  echo_driver(task_id, 
              NULL, 
              write_fds, 
              read_fds, 
              NULL);
}

/***********************************************/
/*                S O C K E T S                */
/***********************************************/

/*****************************************************************************
 *  setup_sockets
 *****************************************************************************/
void setup_sockets(void) 
{
  int i = 0;
  int recv_idx = -1;

  for (i = 0; i < num_tasks; i++) {
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds_array[i].fds) < 0) {
      th_log_error("Failure setting up socket for task id %d!\n", i);
      exit(1);
    }
    if (using_pthreads) {
      pthread_rdwr_init(&(locks[i]));
    }
  }

  for (i = 0; i < num_tasks; i++) {
    recv_idx = (i == 0) ? num_tasks - 1 : i - 1;
    remote_fds[i] = fds_array[recv_idx].fds[READ_IDX];
  }
}

/*****************************************************************************
 *  task_body_sockets_blocking
 *****************************************************************************/
void task_body_sockets_blocking(int task_id, 
                                void* input_data, 
                                int input_data_size) 
{
  echo_driver(task_id, 
              NULL, 
              write_fds, 
              read_fds, 
              NULL);
}


/*********************************************************/
/* R E A D / W R I T E   F I L E   D E S C R I P T O R S */
/*********************************************************/

/*****************************************************************************
 *  write_fds
 *****************************************************************************/
int write_fds(int index, char* buf, int write_size) 
{
  int msgsize = 0;

  if (using_pthreads) {
    if (pthread_rdwr_wlock(&(locks[index])) != 0) {
      return -1;
    }
  }

  msgsize = write(fds_array[index].fds[WRITE_IDX], buf, write_size);

  if (using_pthreads) {
    if (pthread_rdwr_wunlock(&(locks[index])) != 0) {
      return -1;
    }
  }

  return msgsize;
}

/*****************************************************************************
 *  read_fds
 *****************************************************************************/
int read_fds(int index, char* buf, int buf_size)
{
  int msgsize = 0;
  int done = 0;
  struct pollfd poll_descr;

  if (using_pthreads) {

    poll_descr.fd = remote_fds[index];
    poll_descr.events = POLLIN;

    while (!done) {   
      if (pthread_rdwr_rlock(&(locks[index])) != 0) {
        return -1;
      }

      poll_descr.revents = 0;      
      poll(&poll_descr, 1, 0);
      if (poll_descr.revents & POLLIN) {
        msgsize = read(remote_fds[index], buf, buf_size);
        done = 1;
      }

      if (pthread_rdwr_runlock(&(locks[index])) != 0) {
        return -1;
      }
    }
  } else {
    msgsize = read(remote_fds[index], buf, buf_size);
  }

  return msgsize;
}


/**************************************************************/
/*           R E A D E R / W R I T E R   L O C K S            */
/**************************************************************/

/*****************************************************************************
 * pthread_rdwr_init
 *****************************************************************************/
int pthread_rdwr_init(pthread_rdwr_t* rdwrp)
{
  rdwrp->readers_reading = 0;
  rdwrp->writer_writing = 0;

  if (pthread_mutex_init(&(rdwrp->mutex), NULL) != 0) {
    return -1;
  }

  if (pthread_cond_init(&(rdwrp->lock_free), NULL) != 0) {
    return -1;
  }

  return 0;
}

/*****************************************************************************
 * pthread_rdwr_rlock
 *****************************************************************************/
int pthread_rdwr_rlock(pthread_rdwr_t* rdwrp)
{
  if (pthread_mutex_lock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  while (rdwrp->writer_writing) {
    if (pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex)) != 0) {
      return -1;
    }
    sched_yield();
  }

  rdwrp->readers_reading++;

  if (pthread_mutex_unlock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  sched_yield();

  return 0;
}

/*****************************************************************************
 * pthread_rdwr_wlock
 *****************************************************************************/
int pthread_rdwr_wlock(pthread_rdwr_t* rdwrp)
{
  if (pthread_mutex_lock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  while (rdwrp->writer_writing || rdwrp->readers_reading) {
    if (pthread_cond_wait(&(rdwrp->lock_free), &(rdwrp->mutex)) != 0) {
      return -1;
    }
    sched_yield();
  }

  rdwrp->writer_writing++;

  if (pthread_mutex_unlock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  sched_yield();

  return 0;
}

/*****************************************************************************
 * pthread_rdwr_runlock
 *****************************************************************************/
int pthread_rdwr_runlock(pthread_rdwr_t* rdwrp)
{
  if (pthread_mutex_lock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  if (rdwrp->readers_reading == 0) {
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  } else {
    rdwrp->readers_reading--;

    if (rdwrp->readers_reading == 0) {
      if (pthread_cond_signal(&(rdwrp->lock_free)) != 0) {
        return -1;
      }
    }

    if (pthread_mutex_unlock(&(rdwrp->mutex)) != 0) {
      return -1;
    }

    sched_yield();
  }

  return 0;
}

/*****************************************************************************
 * pthread_rdwr_wunlock
 *****************************************************************************/
int pthread_rdwr_wunlock(pthread_rdwr_t* rdwrp)
{
  if (pthread_mutex_lock(&(rdwrp->mutex)) != 0) {
    return -1;
  }

  if (rdwrp->writer_writing == 0) {
    pthread_mutex_unlock(&(rdwrp->mutex));
    return -1;
  } else {
    rdwrp->writer_writing = 0;

    if (pthread_cond_broadcast(&(rdwrp->lock_free)) != 0) {
      return -1;
    }

    if (pthread_mutex_unlock(&(rdwrp->mutex)) != 0) {
      return -1;
    }
  }

  return 0;
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
    case USE_MESSAGES:
      if (use_async) {
        return "async_msg";
      } else {
        return "message";
      }
      break;
    case USE_PACKETS:
      if (use_async) {
        return "async_pkt";
      } else {
        return "packet"; 
      }
      break;
    case USE_SCALARS:
      return "scalar";
      break;
    case USE_PIPES:
      return "pipe";
      break;
    case USE_SOCKETS:
      return "socket";
      break;
    default:
      return "FIXME";
      break;
    }
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
