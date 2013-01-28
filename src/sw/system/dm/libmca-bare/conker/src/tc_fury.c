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
 *  Source: tc_fury.c
 *
 *  Description: the fury testcase
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];

#include <stdio.h>  /* TODO -- get rid of this when files are n o longer needed */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TIMEOUT MCAPI_TIMEOUT_INFINITE

/* #include <quantify.h> */

/* testcase config options */
typedef enum _op_types {
  USE_UNDEFINED = 0,
  USE_MESSAGES,
  USE_PACKETS,
  USE_SCALARS
} op_types;

static  op_types  op_type = USE_UNDEFINED;
static  int       op_size = 4;
static  int       op_count = 1;
static  int       num_tasks = 2;
static  int       use_async = 0;
static  tc_status final_status = TC_OK;

static char*     filename = NULL;
static  FILE*     outfile;

static  int       num_retries = 0;

#define WRITE_IDX 1
#define READ_IDX  0

#define MAX_FURY_SIZE 1024
#define FURY_BUF_SIZE MAX_FURY_SIZE + 1

/* the actual algorith resides in fury_driver */
typedef int (*td_fptr)   (int task_id);
typedef int (*comm_fptr) (int task_id, char* buf, int buf_size);

void fury_driver(int task_id, 
                 td_fptr f_config, 
                 comm_fptr f_send, 
                 comm_fptr f_recv, 
                 td_fptr f_cleanup);

/* forward declaration of the task_bodies -- they setup the call to fury_driver */
void task_body_msg_blocking(int task_id, void* input_data, int input_data_size);
void task_body_msg_non_blocking(int task_id, void* input_data, int input_data_size);
void task_body_pkt_blocking(int task_id, void* input_data, int input_data_size);
void task_body_pkt_non_blocking(int task_id, void* input_data, int input_data_size);
void task_body_scalar_blocking(int task_id, void* input_data, int input_data_size);

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

int setup_fury_endpoints(int task_id);

int finalize_mcapi(int task_id);

/* other misc functions */
char* op_type_string(void);

char* tc_get_testcase_name(void) { return "tc_fury"; }
int tc_get_num_tasks(void) { return num_tasks; }

/*****************************************************************************
 *  tc_print_arg_usage -- print out the descriptions of arguments
 *****************************************************************************/
void tc_print_arg_usage(void)
{
  th_log("  -m : test using MCAPI messages                           [default]\n");
  th_log("  -p : test using MCAPI packets\n");
  th_log("  -s : test using MCAPI scalars\n");
  th_log("  -a : use non-blocking send/recv\n");
  th_log("  -n : number of sends                                     [default = 1]\n");
  th_log("  -b <size> : number of bytes per send                     [default = 4]\n");
  th_log("  -f <filename> : append stats to file\n");
  th_log("\n  NOTES: \n");
  th_log("    maximum message size = %d\n", MCAPI_MAX_MSG_SIZE);
  th_log("    maximum packet  size = %d\n", MCAPI_MAX_PKT_SIZE);
  th_log("    maximum fury    size = %d\n", MAX_FURY_SIZE);
  th_log("    scalar sizes are {1, 2, 4, 8} bytes only\n");
  th_log("    -m -p -s are mutually exclusive\n");
  th_log("    -a -s are mutually exclusive\n");
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
    
  case 'f':     /* filename */
    if (optarg) {
      filename = malloc(strlen(optarg) + 1);
      strcpy(filename, optarg);
    } else {
      error = 1;
    }
    break;      
    
  case 'n':     /* number of sends */
    if (optarg) {
      op_count = atoi(optarg);
    } else {
      error = 1;
    }
    break;      
    
  case 'b':     /* number of bytes/send */
    if (optarg) {
      op_size = atoi(optarg);
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

  th_register_args("mpsaf:n:b:",arg_handler);

  return status;
}

/*****************************************************************************
 *  tc_initialize  -- called by test harness to initialize the testcase
 *****************************************************************************/
tc_status tc_initialize(void)
{
  tc_status status = TC_OK;
  int i;
  tc_task_body_fptr_t fp = NULL;

  if (op_type == USE_UNDEFINED) {
    op_type = USE_MESSAGES;
  }

  /* check the operation size & async-ness */
  switch (op_type) 
    {
    case USE_MESSAGES:
      if (op_size < 1 || op_size > MCAPI_MAX_MSG_SIZE || op_size > MAX_FURY_SIZE) {
        th_log_error("number of bytes to send is out of range: %d\n",op_size);
        status = TC_ERROR;
      }
      break;
    case USE_PACKETS:
      if (op_size < 1 || op_size > MCAPI_MAX_PKT_SIZE || op_size > MAX_FURY_SIZE) {
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
    default:
      break;
    }
  
  /* check the operation count */
  if (op_count < 1) {
    th_log_error("number of sends must be > 0\n");
    status = TC_ERROR;
  }

  /* if we got here we must have good args ... */
  th_log_info("[TC_FURY]    -- tc_initialize, pid = %d\n", getpid());

  /* print config */
  switch (op_type) {
    case USE_MESSAGES:
      th_log("[TC_FURY]    -- using MCAPI messages\n");
      break;
    case USE_PACKETS:
      th_log("[TC_FURY]    -- using MCAPI packets\n");
      break;
    case USE_SCALARS:
      th_log("[TC_FURY]    -- using MCAPI scalars\n");
      break;
    default:
      break;
  }
  if (use_async) {
    th_log("[TC_FURY]    -- using non-blocking send/recv\n");
  }
  th_log("[TC_FURY]    -- node offset:\t%d\n", th_get_node_offset());
  th_log("[TC_FURY]    -- #sends:\t%d\n", op_count);
  th_log("[TC_FURY]    -- #bytes/send:\t%d\n", op_size);
  
  /* TODO - decide how to handle files in bare metal */
  if (filename) {
    th_log("[TC_FURY]    -- results will be appended to file: %s\n",filename);
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
      default:
        break;
      }

    if (th_register_task(i,"fury__task",fp,NULL,0) != TH_OK) {
      th_log_error("Unable to create task number %d.  Status = %d\n",
                    i, status);
      return TC_ERROR;
    } else {
      th_log_info("[TC_FURY]    -- task registered, id = %d\n", i);
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
      th_log("[TC_FURY] -- task created, id = %d\n", i);
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
          th_log_info("[TC_FURY]    -- return data from task %d = '%s'\n", i, 
                      (char *) (return_data));
        } else {
          th_log_info("[TC_FURY]    -- return data from task %d = <empty>\n", i);
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
    th_log("[TC_FURY]    -- testcase finished successfully, pid = %d\n", getpid());
  } else {
    th_log("[TC_FURY]    -- testcase finished with errors, pid = %d\n", getpid());
  }

  if (filename) {
    fclose(outfile);
    free(filename);
    filename = NULL;
  }

  return TC_OK;
}



/*****************************************************************************
 * fury_driver - thee actual algorithm, with function pointers for different
 *               test types 
 *****************************************************************************/
void fury_driver(int task_id, 
                 td_fptr f_config, 
                 comm_fptr f_send, 
                 comm_fptr f_recv, 
                 td_fptr f_cleanup)
{
  task_status_code status = TASK_RUNNING;
  int send_idx = task_id;
  int recv_idx = task_id;
  char msgbuf[FURY_BUF_SIZE] = "";
  char expected_value[FURY_BUF_SIZE] = "";
  int msgsize = 0;
  int count = 0;
  long double delta;
  int local_op_size = 0;

  /* call configuration function */
  if (f_config) {
    if ((*f_config)(task_id) != 0) {
      th_log_error("Task id %d failed to configure properly\n", task_id);
      status = TASK_ERROR;  
    } 
  }

  if (status != TASK_ERROR) {

    if (task_id == 1) {
      if (op_size > 0) {
        memset(expected_value, 'A', op_size - 1);
        expected_value[op_size - 1] = 0;
      } 
      /* begin timer in CONSUMER only*/
      if (th_start_timing() < 0) {
        th_log_error("Task id %d failed to acquire start time\n", task_id);
        status = TASK_ERROR;  
      }
    } else if (task_id == 0) {
      if (op_size > 0) {
        memset(msgbuf, 'A', op_size - 1);
        msgbuf[op_size - 1] = 0;
      } else {
        msgbuf[0] = 0;
      }
    }
        
    /* testing loop */

    for (count = 0; count < op_count; count++) {
      
      if (task_id == 0) {
               
        /* PRODUCER -- generate a msg, send to CONSUMER */
        th_log_info("[TC_FURY]    -- Producer Sending: '%s', task=%d, send_idx=%d\n", 
                    msgbuf, task_id, send_idx);
               
        if ((f_send)(send_idx, msgbuf, op_size) != op_size) {
          th_log_error("Task id %d failed to send msg\n", task_id);
          status = TASK_ERROR;  
        } else if (op_size > 0) {
          msgbuf[0]++;
        }
       } else {
        
        msgbuf[0] = 0;
        
        /* wait for a msg and check it */
        th_log_info("[TC_FURY]    -- Consumer Recv, task=%d, recv_idx=%d \n", 
                    task_id, recv_idx);
        
        if (0 && strcmp(msgbuf, expected_value) != 0) {
              th_log_error("Task id %d received corrupted msg: %s, expected %s\n",
                           task_id, msgbuf, expected_value);
              status = TASK_ERROR;  
        } else if ((msgsize = (f_recv)(recv_idx, msgbuf, FURY_BUF_SIZE)) != op_size) {
          th_log_error("Task id %d failed to receive message size: found msgsize=%d expected op_size=%d\n",
                       task_id,msgsize,op_size);
          status = TASK_ERROR;
        } else {
          if (op_size > 0) {
            expected_value[0]++;
          }

          th_log_info("[TC_FURY]    -- Consumer Node %d received '%s'\n", task_id ,msgbuf);
        }
      }
    }
    
    if (task_id == 1) {
      /* end timer and calculate total time */
      delta = th_end_timing();
      if (delta < 0) {
        th_log_error("Task id %d failed to acquire end time\n", task_id);
        status = TASK_ERROR;  
      } else {

                
        th_log("Messages sent:\t%d\nTotal Time (in seconds):\t%Lf\nTotal Retries:\t%d\n", 
               op_count, delta, num_retries);
        
        /* TODO - decide how to handle files in bare metal */
        if (filename) {
          fprintf(outfile, "%s\t%s\t%d\t%d\t%d\t%Lf\t%d\n", th_get_impl_string(),
                  op_type_string(), num_tasks - 1, op_size, op_count, delta, num_retries);
        }
      }
    }
    
    /* call cleanup function */
    if (f_cleanup) {
      
      /* send a message that I'm done */
      sprintf(msgbuf,"DONE");
      local_op_size = strlen(msgbuf);
      if ((f_send)(send_idx, msgbuf, local_op_size) != local_op_size) {
        th_log_error("Task id %d failed to send DONE msg\n", task_id);
        status = TASK_ERROR;
      } else {
        th_log_info ("Task id %d sent msg: %s\n",task_id,msgbuf);
      }
      
      /* wait for a message that the other task is done*/
      sprintf(expected_value,"DONE");
      if ((msgsize = (f_recv)(recv_idx, msgbuf, FURY_BUF_SIZE)) != local_op_size) {
        th_log_error("Task id %d failed to receive DONE message\n",
                     task_id);
        status = TASK_ERROR;
      } else if (strcmp(msgbuf, expected_value) != 0) {
        th_log_error("Task id %d received corrupted msg: %s, expected %s\n",
                     task_id, msgbuf, expected_value);
        status = TASK_ERROR;
      } else {
        th_log_info("Task id %d received msg: %s, now calling finalize.\n",task_id,msgbuf);
      }
      
      /* now that both tasks are done, call finalize */
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
  fury_driver(task_id, 
              setup_fury_endpoints, 
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
  fury_driver(task_id, 
              setup_fury_endpoints, 
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
  int done = 0;
  mcapi_priority_t priority = 1;

  while (!done) {
    mcapi_msg_send(local_endpts[index].endpts[WRITE_IDX],
                   remote_endpts[index], buf, write_size, priority, &status);
    if (status == MCAPI_ERR_MEM_LIMIT) {
      /* TODO - this error code changed and so could cause fails; need to check */
      num_retries++;
    } else if (status != MCAPI_SUCCESS) {
      th_log_error("Task id %d failed to send msg: %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } else {
      done = 1;
    }
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
  int done = 0;
  
  while (!done) {
    mcapi_msg_recv_i(local_endpts[index].endpts[READ_IDX], 
                     (void *) buf, buf_size, &request, &status);
    if (status == MCAPI_ERR_REQUEST_LIMIT) {
      num_retries++;
    } else {
      done = 1;
      if (status != MCAPI_SUCCESS) {
        th_log_error("Task id %d failed to recv msg (recv): %s\n",
                     index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
        return -1;
      }
      
      mcapi_wait(&request, &msgsize,  TIMEOUT,&status);
      if (status != MCAPI_SUCCESS) {
        th_log_error("Task id %d failed to recv msg (wait): %s\n",
                     index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
        return -1;
      }
    }
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
  int done = 0;
  int sent = 0;
  mcapi_priority_t priority = 1;

  while (!done) {
    while (!sent) {
      mcapi_msg_send_i(local_endpts[index].endpts[WRITE_IDX],
                       remote_endpts[index], buf, write_size, priority, &request, &status);
      if  (status == MCAPI_ERR_REQUEST_LIMIT) {
        /* TODO - this error code changed and so could cause fails; need to check */
        num_retries++;
      } else if (status != MCAPI_SUCCESS) {
        th_log_error("Task id %d failed to send msg: %s\n",
                     index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
        return -1;
      } else {
        sent = 1;
      }
    }
    
    mcapi_wait(&request, &msgsize,  TIMEOUT, &status);
    if (status == MCAPI_ERR_MEM_LIMIT) {
      sent = 0;
    } else if (status != MCAPI_SUCCESS) {
      th_log_error("Task id %d failed to send msg (wait): %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } else {
      done = 1;
    }
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
  fury_driver(task_id, 
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
  fury_driver(task_id, 
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
  if (setup_fury_endpoints(task_id) != 0) {
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status);
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status);
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status); 
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
  int done = 0;

  while (!done) {
    mcapi_pktchan_send(pkt_hndls[index].send, buf, write_size, &status);
    if (status == MCAPI_ERR_MEM_LIMIT) {
      /* TODO - this error code changed and so could cause fails; need to check */
      num_retries++;
    } else if (status != MCAPI_SUCCESS) {
      th_log_error("Task id %d failed to send msg: %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } else {
      done = 1;
    }
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
  int done = 0;
 
  while (!done) {
    mcapi_pktchan_recv_i(pkt_hndls[index].recv, (void**)((void*)&tmpbuf), &request, &status);
    if (status == MCAPI_ERR_REQUEST_LIMIT) {
      num_retries++;
      printf("send retries %d\n",num_retries);
    } else {  
      done = 1;
      if (status != MCAPI_SUCCESS) {
        th_log_error("Task id %d failed to recv pkt (recv): %s\n",
                     index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
        return -1;
      }   
      mcapi_wait(&request, &pktsize,  TIMEOUT,&status);
      
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
  int done = 0;
  int sent = 0;

  while (!done) {
    while (!sent) {
      mcapi_pktchan_send_i(pkt_hndls[index].send, buf, write_size, &request, &status);
      if (status == MCAPI_ERR_REQUEST_LIMIT) {
        /* TODO - this error code changed and so could cause fails; need to check */
        num_retries++;
        printf("send retries %d\n",num_retries);
      } else if (status != MCAPI_SUCCESS) {
        th_log_error("Task id %d failed to send msg: %s\n",
                     index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
        return -1;
      } else {
        sent = 1;
      }
    }
    
    mcapi_wait(&request, &pktsize,  TIMEOUT,&status);
    if (status == MCAPI_ERR_MEM_LIMIT) {
      sent = 0;
    } else if (status != MCAPI_SUCCESS) {
      th_log_error("Task id %d failed to send pkt (wait): %s req=%x\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)),request);
      return -1;
    } else {
      done = 1;
    }
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
  fury_driver(task_id, 
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
  if (setup_fury_endpoints(task_id) != 0) {
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status);
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status);
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
  mcapi_wait(&request, &dummy,  TIMEOUT,&status); 
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
  int done = 0;

  while (!done) {
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
    if (status == MCAPI_ERR_MEM_LIMIT) {
      /* TODO - this error code changed and so could cause fails; need to check */
      num_retries++;
    } else if (status != MCAPI_SUCCESS) {
      th_log_error("Task id %d failed to send msg: %s\n",
                   index, mcapi_display_status(status,status_buff,sizeof(status_buff)));
      return -1;
    } else {
      done = 1;
    }
  } 

  return write_size;
}



/**********************************************/
/*        M C A P I   G E N E R A L           */
/**********************************************/

/*****************************************************************************
 *  setup_fury_endpoints
 *****************************************************************************/
int setup_fury_endpoints(int task_id)
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

  /* initialize */
  mcapi_initialize(CONKER_TESTCASE_DOMAIN, task_id + th_get_node_offset(), NULL,&parms,&version, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to initialize: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* create local_send endpoint */
  th_log_info("[TC_FURY]    -- creating local send endpoint node=%d, port=%d\n", 
              task_id + th_get_node_offset(), local_send_port);

  local_endpts[task_id].endpts[WRITE_IDX] = 
    mcapi_endpoint_create(local_send_port, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to create local send endpoint: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* create local_recv endpoint */
  th_log_info("[TC_FURY]    -- creating local recv endpoint node=%d, port=%d\n", 
              task_id + th_get_node_offset(), local_recv_port);

  local_endpts[task_id].endpts[READ_IDX] = 
    mcapi_endpoint_create(local_recv_port, &status);
  if (status != MCAPI_SUCCESS) {
    th_log_error("Task id %d failed to create local receive endpoint: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
  }

  /* get the endpoint to send to */
  th_log_info("[TC_FURY]    -- get next recv endpt, node=%d, next_node=%d, next_recv_port=%d\n", 
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
    th_log_error("Task id %d failed to finalize: %s\n",
                 task_id, mcapi_display_status(status,status_buff,sizeof(status_buff)));
    return -1;
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
    default:
      return "FIXME";
      break;
    }
}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
