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
 *  Source: th_mtapi.c
 *
 *  Description: defines an mtapi task management implementation for
 *               use by testcases
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <th_include.h>
#include <tc_include.h>

#include <mrapi.h>
#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mtapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARG_STR_LENGTH 1024

/* TODO - control the timeout via cmd line arg ? */
#define TIMEOUT_VALUE 5000000

/* TODO - control the domain  via configure? */
#define TH_MTAPI_RECV_PORT 0
#define TH_MTAPI_SEND_PORT 1


#define CONKER_TESTHARNESS_DOMAIN 1
#define CONKER_MTAPI_DOMAIN 2

static int node_id = -1;

#define MASTER_NODE 0
#define MASTER_IDX  0
static mcapi_endpoint_t recv_endpoint;
static mcapi_endpoint_t send_endpoint;
/* for ease of programming this is set up to be indexed 1..MAX_TASK, index zero is MASTER */
typedef struct remote_node {
  mcapi_endpoint_t endpoint;
  mcapi_request_t request;
  char buffer[MAX_ARG_STR_LENGTH];
} remote_node_t;
static remote_node_t remote_nodes[CONKER_MAX_TASKS];

/* forward declarations */
char* make_arg_string(void);
void mtapi_task_wrapper(mtapi_task_handle_t task, 
                        void* input_data, 
                        int input_data_size);


/*****************************************************************************
 *  th_get_impl_string -- get a string indicating the type of testharness
 *****************************************************************************/
char* th_get_impl_string(void)
{
  static char impl_string[] = "mtapi";
  return impl_string;
}

/*****************************************************************************
 *  th_task_print_arg_usage
 *****************************************************************************/
void th_task_print_arg_usage(void)
{
  /* empty */
}

/*****************************************************************************
 *  arg_handler
 *****************************************************************************/
int th_mtapi_arg_handler (char opt, const char* value)
{
  int error = 0;

  /* empty */

  return error;
}

/*****************************************************************************
 *  th_task_setup -- do any basic setup here, including registering a handler
 *                   for command line args.  Args will be parsed between the 
 *                   call to th_task_setup and the call to th_task_initialize.
 *****************************************************************************/
extern th_status th_task_setup(void)
{
  th_status status = TH_OK;

  /* empty */

  return status;
}

/*****************************************************************************
 *  th_task_initialize -- called to initialize the task API
 *****************************************************************************/
th_status th_task_initialize(void)
{
  int i;
  int slave_error = 0;
  size_t size;
  mcapi_priority_t priority = 1;
  mcapi_status_t mcapi_status;
  mcapi_info_t mcapi_version;
  mcapi_param_t parms;
  mtapi_status_t mtapi_status;
  th_task_descriptor* task_descr = NULL;
  int num_tasks = tc_get_num_tasks();
  char* arg_str = make_arg_string();

  node_id = th_get_node_num();

  th_log_info("[TH_MTAPI] -- th_task_initialize, num tasks = %d, node_id = %d\n", num_tasks, node_id);

  /* initialize MCAPI in the testharness domain */
  //mca_set_debug_level(2); 

  mcapi_initialize(CONKER_TESTHARNESS_DOMAIN, 
                   node_id, 
                   NULL,
                   &parms,
                   &mcapi_version,
                   &mcapi_status);
  
  if (mcapi_status != MCAPI_SUCCESS) {
    th_log_error("MTAPI node id %d failed to initialize %s\n",
                 th_get_node_num(), mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    return TH_ERROR;
  }  
  
  /* create my  endpoints for receive & send */
  recv_endpoint = mcapi_endpoint_create( 
                                        TH_MTAPI_RECV_PORT, 
                                        &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    th_log_error("failed to create receive endpoint for Node id %d : %s\n",
                 node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    return TH_ERROR;
  }
  
  send_endpoint = mcapi_endpoint_create( 
                                        TH_MTAPI_SEND_PORT, 
                                        &mcapi_status);
  if (mcapi_status != MCAPI_SUCCESS) {
    th_log_error("failed to create send endpoint for Node id %d : %s\n",
                 node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
    return TH_ERROR;
  }
  
  if (node_id == 0) {
    /* I am the master -- determine if enough slaves exist and args are same */
        
    /* get endpoints for all slaves -- error if not all gotten within timeout */
    for (i = 1; i < num_tasks; i++) {

        mcapi_endpoint_get_i(CONKER_TESTHARNESS_DOMAIN, i, TH_MTAPI_RECV_PORT, 
                             &remote_nodes[i].endpoint,
                             &remote_nodes[i].request,&mcapi_status);

        if (mcapi_status != MCAPI_SUCCESS) {
          th_log_error("master failed to get remote endpoint for Node id %d: %s\n",
                       node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
          return TH_ERROR;
        }
    }

    for (i = 1; i < num_tasks; i++) {
      if (!mcapi_wait(&remote_nodes[i].request, &size,  TIMEOUT_VALUE,&mcapi_status) || 
          mcapi_status != MCAPI_SUCCESS) {
        th_log_error("master failed to get remote endpoint for Node id %d: %s\n",
                     i, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
        return TH_ERROR;
      }
      th_log_info("[TH_MTAPI] -- th_task_initialize, master got endpoint for node %d \n", i);
    }
    
    /* issue non-blocking receive for each slave */
    for (i = 1; i < num_tasks; i++) {
      mcapi_msg_recv_i(recv_endpoint,  
                       remote_nodes[i].buffer, 
                       MAX_ARG_STR_LENGTH, 
                       &remote_nodes[i].request, 
                       &mcapi_status);
      if (mcapi_status != MCAPI_SUCCESS) {
        th_log_error("master failed to receive args from Node: %s\n",
                     i, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
        return TH_ERROR;
      }
    }
    
    /* for each receive, check args, send slave ACK or NACK */
    for (i = 1; i < num_tasks; i++) {

      if (!mcapi_wait(&remote_nodes[i].request, &size,  TIMEOUT_VALUE,&mcapi_status) || 
          mcapi_status != MCAPI_SUCCESS) {
        th_log_error("master failed to receive args from Node: %s\n",
                     node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
        return TH_ERROR;
      } else if (strcmp(arg_str,remote_nodes[i].buffer) != 0) {
        th_log_error("master: args for Node id %d  do not match: [%s] [%s]\n",
                     node_id, arg_str,remote_nodes[i].buffer);
        slave_error = 1;
      }
      th_log_info("[TH_MTAPI] -- th_task_initialize, master got correct args from node %d \n", i);
    }
   
    /* iff all slaves exist with proper args then OK, else ERR */

    for (i = 1; i < num_tasks; i++) {

      if (slave_error == 1) {

        /* SEND NACK */
        mcapi_msg_send(send_endpoint,
                       remote_nodes[i].endpoint, "NACK", strlen("NACK"), priority, &mcapi_status);
        if (mcapi_status != MCAPI_SUCCESS) {
          th_log_error("master failed to send NACK to Node %d: %s\n",
                       i, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
        }

      } else {

        /* SEND ACK */
        mcapi_msg_send(send_endpoint,
                       remote_nodes[i].endpoint, "ACK", strlen("ACK"), priority, &mcapi_status);
        if (mcapi_status != MCAPI_SUCCESS) {
          th_log_error("master failed to send ACK to Node %d: %s\n",
                       i, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
          return TH_ERROR;
        }
      }
    }

    if (slave_error != 0) {
      return TH_ERROR;
    } else {
      th_log_info("[TH_MTAPI] -- th_task_initialize, master found all slaves with correct args\n");
    }
    
  } else if (node_id > 0) {
    /* I am a slave -- send master my args and wait for ACK */
    
    /* get master endpoint <0,0> */
    
    mcapi_endpoint_get_i(CONKER_TESTHARNESS_DOMAIN, MASTER_NODE, TH_MTAPI_RECV_PORT, 
                         &remote_nodes[MASTER_IDX].endpoint,
                         &remote_nodes[MASTER_IDX].request,&mcapi_status);
    
    if (mcapi_status != MCAPI_SUCCESS) {
      th_log_error("slave node %d failed to get remote endpoint for master: %s\n",
                   node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      return TH_ERROR;
    }

    if (!mcapi_wait(&remote_nodes[MASTER_IDX].request, &size,  TIMEOUT_VALUE,&mcapi_status) || 
        mcapi_status != MCAPI_SUCCESS) {
      th_log_error("slave node %d failed to get remote endpoint for master: %s\n",
                   node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      return TH_ERROR;
    }
    th_log_info("[TH_MTAPI] -- th_task_initialize, slave got endpoint for master \n");

    /* send args to master and wait for ack -- error if timeout */
    
    mcapi_msg_send(send_endpoint, remote_nodes[MASTER_IDX].endpoint, 
                   arg_str, strlen(arg_str), priority, &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      th_log_error("slave %d failed to send args to master: %s\n",
                   node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      return TH_ERROR;
    }

    /* if ack then OK else ERR */
    mcapi_msg_recv_i(recv_endpoint,  
                     remote_nodes[MASTER_IDX].buffer, 
                     MAX_ARG_STR_LENGTH, 
                     &remote_nodes[MASTER_IDX].request, 
                     &mcapi_status);
    if (mcapi_status != MCAPI_SUCCESS) {
      th_log_error("slave %d failed to receive ACK/NACK from master: %s\n",
                   node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      return TH_ERROR;
    }

    if (!mcapi_wait(&remote_nodes[MASTER_IDX].request, &size,  TIMEOUT_VALUE,&mcapi_status) || 
        mcapi_status != MCAPI_SUCCESS) {
      th_log_error("slave node %d failed to receive ACK/NACK from master: %s\n",
                   node_id, mcapi_display_status(mcapi_status,status_buff,sizeof(status_buff)));
      return TH_ERROR;
    }

    if (strncmp("ACK",remote_nodes[MASTER_IDX].buffer,strlen("ACK")) == 0) {
      /* all is well with this slave */
      th_log_info("[TH_MTAPI] -- th_task_initialize, slave %d received ACK from master\n", node_id);
    } else if (strncmp("NACK",remote_nodes[MASTER_IDX].buffer,strlen("NACK")) == 0) {
      th_log_error("slave %d received NACK from master, shutting down\n", node_id);
      return TH_ERROR;
    } else {
      th_log_error("slave %d received garbled reply from master, shutting down\n", node_id);
      return TH_ERROR;
    }
  }
 
 
   /* initialize MTAPI with my node_id  */ 
   mtapi_initialize(CONKER_MTAPI_DOMAIN, 
                   node_id, 
                   &mtapi_status); 
/* fixme: where is this function ??? */
#if 0
  mtapi_initialize_lite(node_id,
                        &mtapi_status);
#endif
  
  if (mtapi_status == MTAPI_ERROR) {
    th_log_error("node %d failed to initialize MTAPI\n", node_id);
    return TH_ERROR;
  }
  
  /* TODO -- this is really not right yet! */

  task_descr = th_get_task_descriptor(node_id);
  if (task_descr == NULL) {
    th_log_error("node %d failed to locate task descriptor\n", node_id);
    return TH_ERROR;
  }

  mtapi_register_impl(task_descr->task_name,
                      mtapi_task_wrapper,
                      &mtapi_status);
  
  if (mtapi_status == MTAPI_ERROR) {
    th_log_error("node %d failed to register task impl\n", node_id);
    return TH_ERROR;
  }
  
  return TH_OK;
} 

/*****************************************************************************
 *  th_task_create -- called to create a new task
 *****************************************************************************/
th_status th_task_create(int task_id)
{
  th_status status = TH_OK;
  th_task_descriptor* task_descr = th_get_task_descriptor(task_id);
  mtapi_task_handle_t the_task;
  mtapi_status_t mtapi_status;

  th_log_info("[TH_MTAPI] -- th_task_create, task id = %d\n", task_id);

  /* TODO -- this is really not right yet */
  if (node_id == 0) {
    /* this test harness only fires up a task from the master node */
    /* mtapi will take care of passing this to remote nodes        */
    the_task = mtapi_create_task(CONKER_MTAPI_DOMAIN,
                                 node_id,
                                 task_descr->task_name,
                                 task_descr->input_data,
                                 task_descr->input_data_size,
                                 &mtapi_status);
    
    if (mtapi_status == MTAPI_ERROR) {
      th_log_error("node %d failed to create task\n", node_id);
      return TH_ERROR;
    }
  }

  return status;
}

/*****************************************************************************
 *  th_task_exit -- called by a task when it exits
 *****************************************************************************/
th_status th_task_exit(int task_id, 
                       void* return_data, 
                       int return_data_size, 
                       task_status_code task_status)
{
  th_status status = TH_OK;
  
  th_log_info("[TH_MTAPI] -- th_task_exit, task id = %d\n", task_id);

  /* TODO */
  if (node_id == 0) {
    /* I am the master -- update my task descriptor */
  } else {
    /* I am a slave -- send msg to master with return_data, size, & status*/
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

  th_log_info("[TH_MTAPI] -- th_task_wait, task id = %d\n", task_id);

  /* TODO */
  if (node_id == 0) {
    /* I am the master */

    /* if task_id == 0 then NOOP */

    /* else receive the exit message from the slave */
  } else {
    /* I am the slave -- NOOP */
  }

  
  return status;
}

/*****************************************************************************
 *  th_task_finalize -- called to finalize the task API
 *****************************************************************************/
th_status th_task_finalize(void)
{
  th_status status = TH_OK;
  mcapi_status_t mcapi_status;

  th_log_info("[TH_MTAPI] -- th_task_finalize\n");

  mcapi_finalize(&mcapi_status);

  if (mcapi_status != MCAPI_SUCCESS) {
    th_log_error("MTAPI node id %d failed to finalize %s\n",
                 th_get_node_num(), mcapi_display_status(status,status_buff,sizeof(status_buff)));
    status = TH_ERROR;
  }

  /* TODO -- finalize MTAPI */

  return status;
}


/******************************/
/* Utility functions          */
/******************************/

/*****************************************************************************
 *  make_arg_string -- make a string of all arg settings whcih can be compared
 *                     between master and slave processes in the harness
 *****************************************************************************/
char* make_arg_string(void)
{
  static char arg_str[MAX_ARG_STR_LENGTH] = "";
  char null_char = 0;
  static char sep_string[] = ":";
  arg_table_entry_t** arg_table = th_get_arg_table(); 
  int i = 0;
  
  while (arg_table[i] != NULL) {
    /* we don't include the task id because it will be different! */
    if (arg_table[i]->optchar != 'T') {
      if (strlen(arg_str) > 0) strcat(arg_str,sep_string);
      strncat(arg_str,(char *) &arg_table[i]->optchar,1);
      strncat(arg_str,(char *) &null_char,1);
      strcat(arg_str,sep_string);
      if (arg_table[i]->value != NULL) {
        strcat(arg_str,arg_table[i]->value);
      } else {
        strcat(arg_str,"NULL");
      }
    }
    i++;
  }

  return arg_str;
}


/*****************************************************************************
 *  mtapi_task_wrapper -- we have to convert the conker task body function
 *                        to an mtapi task impl, so we do that here
 *****************************************************************************/
void mtapi_task_wrapper(mtapi_task_handle_t task, 
                        void* input_data, 
                        int input_data_size)
{
  th_task_descriptor* task_descr = th_get_task_descriptor(task->node);
  
  if (task_descr == NULL) {
    th_log_error("node %d failed to locate task descriptor\n", task->node);
    exit(1);
  }

  task_descr->task_implementation(task->node,input_data,input_data_size);

}

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
