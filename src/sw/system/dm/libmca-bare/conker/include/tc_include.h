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
 *  Header: tc_include.h
 *
 *  Description: include this file in your testcase, it defines functions
 *               required by the test harness.  Note that the test harness
 *               defines 'main'.  The test harness will call each of the
 *               testcase functions declared below, and in the order they
 *               are declared in this header file
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _H_TC_INCLUDE_
#define _H_TC_INCLUDE_

#ifndef CONKER_TESTCASE_DOMAIN
#define CONKER_TESTCASE_DOMAIN 1
#endif

/***************************************************/
/* Error conditions returned by the tc_* functions */
/***************************************************/

typedef enum _tc_status_code { 
  TC_OK , 
  TC_ERROR 
} tc_status;


/***************************************************/
/* The function pointer for task bodies            */
/***************************************************/

typedef void (*tc_task_body_fptr_t) (int task_id, 
                                     void* input_data, 
                                     int input_data_size);


/*************************************************************/
/* Definitions for required functions provided by a testcase */
/*************************************************************/

extern tc_task_body_fptr_t tc_get_task_fptr(int task_id);

extern char* tc_get_testcase_name(void);

extern void tc_print_arg_usage(void);

extern int tc_get_num_tasks(void);

extern tc_status tc_setup(void);

extern tc_status tc_initialize(void); 

extern tc_status tc_run(void); 

extern tc_status tc_report(void); 

extern tc_status tc_finalize(void); 

#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
