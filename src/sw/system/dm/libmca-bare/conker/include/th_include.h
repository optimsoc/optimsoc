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
 *  Header: th_include.h
 *
 *  Description: include this file in your testcase, it defines functions
 *               provided by the test harness for use by the testcase
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _H_TH_INCLUDE_
#define _H_TH_INCLUDE_

#include <mca_config.h>

#include <tc_include.h>

#include <sys/types.h>

#define CONKER_MAX_TASKS MTAPI_MAX_TASKS

/**************************************/
/* datatypes used by the test harness */
/**************************************/

typedef enum _th_status_code { 
  TH_OK, 
  TH_ERROR 
} th_status;


typedef int (*fp_arg_handler) (char opt, const char* value);

typedef struct arg_table_entry {
  char            optchar;
  char*           value;
  fp_arg_handler  handler;
} arg_table_entry_t;

typedef enum _task_status_code { 
  TASK_RUNNING,
  TASK_EXITED,
  TASK_ERROR 
} task_status_code;


/* IMPORTANT TODO 
  (1) make tc_example use multiple different tasks impls/names
*/

typedef struct th_task_descriptor {
  unsigned int task_id;
  char* task_name;
  tc_task_body_fptr_t task_implementation;
  void* input_data;
  size_t input_data_size;
  void* return_data;
  size_t return_data_size;
  task_status_code status;
} th_task_descriptor;


/**************************************************/
/* functions provided by common test harness code */
/**************************************************/

extern void th_set_verbose(int verbose);
extern int  th_get_verbose(void);

extern void th_log(const char* fmt, ...);
extern void th_log_info(const char* fmt, ...);
extern void th_log_error(const char* fmt, ...);

extern long double th_end_timing(void);
extern int th_start_timing(void); 

extern void th_print_affinity(void);  
extern void th_set_affinity(int proc);

extern th_status th_register_args(const char* optstring,
                                  fp_arg_handler handler);

extern th_status th_register_task(int task_id,
                                  char* task_name,
                                  tc_task_body_fptr_t impl,
                                  void* input_data,
                                  size_t input_data_size);

extern th_task_descriptor* th_get_task_descriptor(int task_id);

extern arg_table_entry_t** th_get_arg_table(void);

extern int th_get_node_num(void); 

extern int th_get_node_offset(void); 

extern void th_cleanup(void);

/*******************************************************/
/* functions to be provided by concrete test harnesses */
/*******************************************************/

extern char* th_get_impl_string(void);

extern void th_task_print_arg_usage(void);

extern th_status th_task_setup(void);

extern th_status th_task_initialize(void); 

extern th_status th_task_create(int task_id);

extern th_status th_task_exit(int task_id, 
                              void* return_data, 
                              int return_data_size, 
                              task_status_code task_status);

extern th_status th_task_wait(int task_id, 
                              void** return_data, 
                              int* return_data_size, 
                              task_status_code* task_status);

extern th_status th_task_finalize(void);

#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
