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
 *  Source: th_common.c
 *
 *  Description: defines code common to any test harness implementation
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _GNU_SOURCE

#include <th_include.h>
#include <tc_include.h>
#include <mca_config.h>
#include <mca.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
/* #include <quantify.h> */

/** NOTES:
 (1) change use_gettimeofday to a configure option
 (2) move all timing and affinity code to posix_utils (update test cases)
 (3) get rid of node offset concept if baremetal, it's dangerous
 (4) move all logging functions to the utils code
**/

static int use_gettimeofday = 1;
static int node_num = 0;
static int node_offset = 0;

static int _verbose = 0;
#define MAX_LOG_BUF_SIZE 2048
static char log_buf[MAX_LOG_BUF_SIZE];

static th_task_descriptor* tasks[CONKER_MAX_TASKS];


#define MAX_ARGS 256
static arg_table_entry_t* arg_entries[MAX_ARGS];
static char* g_optstring = NULL;

static int num_arg_entries = 0;

void init_arg_entries(void);

th_status add_arg_entry(char optstring, 
                        fp_arg_handler handler);

th_status parse_args(int argc, char* const argv[]);

int th_get_node_num(void) { return node_num; }
int th_get_node_offset(void) { return node_offset; }

void usage(void);

/*****************************************************************************
 *                         M      A     I     N
 *****************************************************************************/
          
int main(int argc, char* const argv[])
{
  int exit_code = 0;
  int i;

#if (WITH_AFFINITY) 
  th_print_affinity();
  g_optstring = malloc(strlen("hvtAN:o:") + 1);
  strcpy(g_optstring,"hvtAN:o:");
#else
  g_optstring = malloc(strlen("hvtN:o:") + 1);
  strcpy(g_optstring,"hvtN:o:");
#endif
  /*   quantify_stop_recording_data(); */

 //mca_set_debug_level(3); 

  for (i = 0; i < CONKER_MAX_TASKS; i++) {
    tasks[i] = NULL;
  }

  /* allow testharness implementation to register any args to be processed */
  if (exit_code == 0 && th_task_setup() != TH_OK) {
    th_log_error("th_task_setup() returned error\n");
    exit_code = 1;
  }

  /* allow testcase to register any args to be processed */
  if (exit_code == 0 && tc_setup() != TC_OK) {
    th_log_error("tc_setup() returned error\n");
    exit_code = 2;
  }

  if (exit_code == 0 && parse_args(argc,argv) != TH_OK) {
    usage();
    th_task_print_arg_usage();
    tc_print_arg_usage();
    exit_code = 3;
  }
  
  /* initialize the testcase, passing any command lines args */
  if (exit_code == 0 && tc_initialize() != TC_OK) {
    th_log_error("tc_initialize() returned error\n");
    exit_code = 4;
  }
  
  /* initialize the task management API */
  if (exit_code == 0 && th_task_initialize() != TH_OK) {
    th_log_error("th_task_initialize() returned error\n");
    exit_code = 5;
  }
  
  /* run the testcase; testcase must do the following in tc_run() 
   * (1) create all tasks
   * (2) collect results from all tasks
   * (3) wait for all tasks to complete
   * (4) return to the test harness
   */
  if (exit_code == 0 && tc_run() != TC_OK) {
    th_log_error("tc_run() returned error\n");
    exit_code = 6;
  }
  
  /* have the testcase report results */
  if (exit_code == 0 && tc_report() != TC_OK) {
    th_log_error("tc_report() returned error\n");
    exit_code = 7;
  }
  
  /* finalize the testcase */
  if (exit_code == 0 && tc_finalize() != TC_OK) {
    th_log_error("tc_finalize() returned error\n");
    exit_code = 8;
  }
  
  /* finalize the task API */
  if (exit_code == 0 && th_task_finalize() != TH_OK) {
    th_log_error("th_task_finalize() returned error\n");
    exit_code = 9;
  }

  th_cleanup();

  exit(exit_code);
}


/*****************************************************************************
 *  th_cleanup -- free all malloc'd memory
 *****************************************************************************/
void th_cleanup(void)
{
  int i;

  /* free the task descriptors */
  for (i = 0; i < CONKER_MAX_TASKS; i++) {
    if (tasks[i] != NULL) {
      free(tasks[i]->task_name);
      free(tasks[i]->input_data);
      free(tasks[i]->return_data);
      free(tasks[i]);
    }
  }

  /* free the argument entries */
  for (i = 0; i < num_arg_entries; i++) {
    free(arg_entries[i]);
  }
  
  /* free the options string */
  free(g_optstring);


}

/*****************************************************************************
 *  th_set_verbose -- zero is off, any other integer is on
 *****************************************************************************/
void th_set_verbose(int verbose)
{
  _verbose = verbose;
}

/*****************************************************************************
 *  th_get_verbose -- zero is off, any other integer is on
 *****************************************************************************/
int th_get_verbose(void)
{
  return _verbose;
}

/*****************************************************************************
 *  th_register_args -- th common parses all args so if another file
 *                      wants access to args then it registers a handler
 *****************************************************************************/
th_status th_register_args(const char* optstring,
                           fp_arg_handler handler)
{
  th_status status = TH_OK;
  char* tempstring;
  int i,optcount = 0;

  assert(handler != NULL);

  tempstring = malloc(strlen(g_optstring) + strlen(optstring) + 1);
  strcpy(tempstring,g_optstring);
  strcat(tempstring,optstring);

  /* go through optstring and build the args table */
  i = 0;
  optcount = 0;
  while (status == TH_OK && optstring[i] != 0) {
    if (optstring[i] != ':') {
      status = add_arg_entry(optstring[i],handler);
      optcount++;
    }
    i++;
  }

  if (status == TH_OK) {
    free(g_optstring);
    g_optstring = tempstring;
  }

  return status;
}

/*****************************************************************************
 *  th_register_task
 *****************************************************************************/
th_status th_register_task(int task_id,
                           char* task_name,
                           tc_task_body_fptr_t impl,
                           void* input_data,
                           size_t input_data_size)
{
  th_status status = TH_OK;

  if (impl == NULL ||
      task_id > CONKER_MAX_TASKS || 
      task_id < 0 ||
      task_name == NULL) {
    status = TH_ERROR;
  }

  tasks[task_id] = (th_task_descriptor *) malloc(sizeof(th_task_descriptor));
  tasks[task_id]->task_id = task_id;
  tasks[task_id]->task_name = malloc(strlen(task_name) + 1);
  tasks[task_id]->task_implementation = impl;
  strcpy(tasks[task_id]->task_name,task_name);
  tasks[task_id]->input_data = (void *) malloc(input_data_size);
  memcpy(tasks[task_id]->input_data, input_data, input_data_size);
  tasks[task_id]->input_data_size = input_data_size;
  tasks[task_id]->return_data = NULL;
  tasks[task_id]->return_data_size = 0;
  tasks[task_id]->status = TC_OK;

  return status;
}

/*****************************************************************************
 *  th_get_task_descriptor
 *****************************************************************************/
th_task_descriptor* th_get_task_descriptor(int task_id)
{
  th_task_descriptor* task = NULL;

  if (task_id >= 0 && task_id < CONKER_MAX_TASKS) {
    task = tasks[task_id];
  }

  return task;
}

/*****************************************************************************
 *  th_get_arg_table -- return a pointer to the args table
 *****************************************************************************/
arg_table_entry_t** th_get_arg_table(void)
{
  return arg_entries;
} 

/* TODO -- option to compile out the impl of this */
/* TODO -- option to compile in a bare metal impl or printf version of this */
/*****************************************************************************
 *  th_log -- log informational messages regardless of verbosity setting
 *****************************************************************************/
void th_log(const char* fmt, ...)
{
  va_list ptr;

  va_start(ptr,fmt);
  vsnprintf(log_buf,MAX_LOG_BUF_SIZE,fmt,ptr);
  printf("%s",log_buf);
}

/* TODO -- option to compile out the impl of this */
/* TODO -- option to compile in a bare metal impl or printf version of this */
/*****************************************************************************
 *  th_log_info -- log informational messages
 *****************************************************************************/
void th_log_info(const char* fmt, ...)
{
  va_list ptr;

  if (th_get_verbose()) {
    va_start(ptr,fmt);
    vsnprintf(log_buf,MAX_LOG_BUF_SIZE,fmt,ptr);
    printf("%s",log_buf);
  }
}

/* TODO -- option to compile out the impl of this */
/* TODO -- option to compile in a bare metal impl or printf version of this */
/*****************************************************************************
 *  th_log_error -- log error messages
 *****************************************************************************/
void th_log_error(const char* fmt, ...)
{
  va_list ptr;

  va_start(ptr,fmt);
  vsnprintf(log_buf,MAX_LOG_BUF_SIZE,fmt,ptr);
  fprintf(stderr,"ERROR: %s",log_buf);
}


/*********************/
/*  TIMING FUNCTIONS */
/*********************/

/* TODO - this needs to be cleaned up; baremetal will use neither */
/* I think we should ifdef one of (times, gettimeofday, _bare_metal_get_time) */
/* no parameter would be passed to the function; bare metal must supply an */
/* implementation of _bare_metal_get_time */
/* includes should be surrounded with ifdefs as well, like with affinity */

/*  gettimeofday */
#include <sys/time.h>
static struct timeval tv_start, tv_end;
/*  times */
#include <sys/param.h>  // for Hz (times)
#include <sys/times.h>  // for times
static time_t      start_sys;
static time_t      start_user;
static time_t      start_sys_c;
static time_t      start_user_c;
static struct tms  tms;


/* TODO -- option to compile in std versions or bare metal versions */
/* TODO -- think about the return value -- is it sufficient?  */
/*****************************************************************************
 *  th_start_timing -- get an initial time value
 *****************************************************************************/
int th_start_timing(void) {  
  int rc = 0;
  if (use_gettimeofday) {
    rc = gettimeofday(&tv_start, NULL);
  } else {
    times(&tms);
    start_user = tms.tms_utime;
    start_sys  = tms.tms_stime;
    start_user_c = tms.tms_cutime;
    start_sys_c  = tms.tms_cstime;
  }
  return rc;
}

/* TODO -- option to compile in std versions or bare metal versions */
/* TODO -- think about the return value -- is it sufficient?  */
/*****************************************************************************
 *  th_end_timing -- get a delta time value
 *****************************************************************************/
long double th_end_timing(void) {  
  int rc = 0;
  long double delta = 0.0;
  
  if (use_gettimeofday) {
    rc = gettimeofday(&tv_end, NULL);
    if (rc >= 0) {
      delta = ((((long double) tv_end.tv_sec) * 1000000.0 + tv_end.tv_usec) - 
               (((long double) tv_start.tv_sec) * 1000000.0 + tv_start.tv_usec)) / 1000000.0;      
    }
  } else {
    static time_t stime;
    static time_t utime;
    static time_t cstime;
    static time_t cutime;
    times(&tms);
    utime = tms.tms_utime - start_user;  
    stime = tms.tms_stime - start_sys;
    cutime = tms.tms_cutime - start_user_c;  
    cstime = tms.tms_cstime - start_sys_c;
    
    th_log_info("User Time  : %d.%.2d Seconds, Hz=%d\n",
                (int)utime / HZ, (int)utime % HZ,(int)HZ );
    
    th_log_info("System Time: %d.%.2d Seconds, Hz=%d\n",
                (int)stime / HZ, (int)stime % HZ,(int)HZ );
    
    th_log_info("User Time children: %d.%.2d Seconds, Hz=%d\n",
                (int)cutime / HZ, (int)cutime % HZ,(int)HZ );
    
    th_log_info("System Time children: %d.%.2d Seconds, Hz=%d\n",
                (int)cstime / HZ, (int)cstime % HZ,(int)HZ );
    
    delta = utime + cutime;
    delta = delta/HZ;
  }
  return delta;
}

/**********************/
/* AFFINITY FUNCTIONS */
/**********************/

#if (WITH_AFFINITY)
#include <sched.h> 

/*
Affinity is the act of binding a thread/process to a given core.  If
the user passes the -A option to the test, then we bind all threads 
and/or processes that we create to a single core, core 0.  Otherwise,
we allow the OS to schedule across cores as it sees fit.  This is 
useful for benchmarking as it allows us to compare MP vs. single-core
performance.

Note: we adjust the affinity if the user has requested it (-A option)
before we fork off any processes or create any threads.  A forked process
inherits it's parents scheduling attributes.  A pthread also inherits it's
parent's scheduling attributes because PTHREAD_INHERIT_SCHED attribute is
set by default, and we don't change it.

Nasty portability issues:
The affinity syscalls were introduced in Linux kernel 2.5.8. The library 
calls were introduced in glibc 2.3, and are still in glibc 2.3.2. Later 
glibc 2.3.2 development versions changed this interface to one without 
the len field, and still later versions reverted again.  Check out the 
PLPA (portable linux processor affinity) project for more info:
  http://jeff.squyres.com/journal/archives/2005/10/linux_processor.html
  http://www.open-mpi.org/projects/plpa/

Because figuring out which versions of set/get_affinity to use is such
a mess, I took the easy way out.  You can try running autogen with
--enable-affinity and we'll compile in the code below which may or 
may not compile.  If it won't, try using the commmented out version
that doesn't pass the length argument.
*/

/*****************************************************************************
 *  th_print_affinity -- print the affinity mask
 *****************************************************************************/
void th_print_affinity(void) {
  cpu_set_t mask;
  size_t len = sizeof(mask);
  if (sched_getaffinity(0, len,&mask) < 0) {
    th_log_error("sched_getaffinity");
    exit(-1);
  }
  th_log_info("my affinity mask is: %08lx\n", *(unsigned long*)&mask);
}

/*****************************************************************************
 *  th_print_affinity -- print the affinity mask
 *****************************************************************************/
void th_set_affinity(int proc) {
  cpu_set_t mask;
  size_t len = sizeof(mask);
  /* set the mask for cpu0 */
  CPU_ZERO(&mask);
  CPU_SET(proc,&mask);
  if (sched_setaffinity(0, len,&mask) < 0) {
    th_log_error("sched_setaffinity");
    exit(-1);
  }
  th_print_affinity();
}
#else
void th_print_affinity(void) {
  th_log_error("get_affinity not defined.  Try re-running autogen and enabling affinity."
               "It may or may not compile, see comment in th_common.c\n");
  assert(0);
}

void th_set_affinity(int proc) {
  th_log_error("get_affinity not defined.  Try re-running autogen and enabling affinity."
               "It may or may not compile, see comment in th_common.c\n");
  assert(0);
}
#endif


/*************************/
/*** PRIVATE FUNCTIONS ***/
/*************************/

/*****************************************************************************
 *  init_arg_entries
 *****************************************************************************/
void init_arg_entries(void)
{
  int i = 0;

  for (i = 0; i < MAX_ARGS; i++) {
    arg_entries[i] = NULL;
  }
}

/*****************************************************************************
 *  add_arg_entry
 *****************************************************************************/
th_status add_arg_entry(char optchar, 
                        fp_arg_handler handler)
{
  th_status status = TH_OK;
  arg_table_entry_t* new_entry;
  arg_table_entry_t* temp;
  int i, j, increment;

  /* 1st ensure we have room, remember array is null terminated */
  if (num_arg_entries == (MAX_ARGS - 1)) {
    /* TODO log error message */
    status = TH_ERROR;
  }
    
  /* now check for a duplicate arg -- avoid bsearch to reduce lib deps */
  for (i = 0; i < num_arg_entries && status == TH_OK; i++) {
    if (optchar == arg_entries[i]->optchar) {
      /* TODO log error message */
      status = TH_ERROR;
    }
  }

  /* now create a new entry for it */
  if (status == TH_OK) {
    new_entry = malloc(sizeof(arg_table_entry_t));
    new_entry->optchar = optchar;
    new_entry->handler = handler;
    new_entry->value = NULL;
    arg_entries[num_arg_entries] = new_entry;
    num_arg_entries++;
  }

  /* now sort them with a shell sort */
  increment = num_arg_entries / 2;
 
  while (increment > 0)
  {
    for (i = increment; i < num_arg_entries; i++) 
    {
      j = i;
      temp = arg_entries[i];
      while ((j >= increment) && 
             (arg_entries[j-increment]->optchar > temp->optchar)) 
      {
        arg_entries[j] = arg_entries[j - increment];
        j = j - increment;
      }
      arg_entries[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (int) (increment / 2.2);
  }

  return status;
}

/*****************************************************************************
 *  parse_args
 *****************************************************************************/
th_status parse_args(int argc, char* const argv[])
{
  th_status status = TH_OK;
  int c;
  int i = 0;
  int done = 0;
  int found = 0;
  int error = 0;

  while (!done && status == TH_OK) {
    c = getopt(argc, argv, g_optstring);
    
    switch (c) {

    case -1:
      done = 1;
      break;

    case 0:
      /* If this option set a flag, do nothing else now. */
      break;

    case 'h':     /* help */
      status = TH_ERROR;
      break;
      
    case 'v':     /* verbose */
      th_set_verbose(1);
      break;
      
    case 't':     /* use gettimeofday */
      use_gettimeofday = 0;
      break;
      
    case 'A':
      th_set_affinity(0);
      printf("[TH_COMMON]    -- all tasks will be bound to processor 0\n");
      break;
      
    case 'N':     /* node number */
      if (optarg) {
        node_num = atoi(optarg);
      } else {
        error = 1;
      }
      break;      
      
    case 'o':     /* node offset */
      if (optarg) {
        node_offset = atoi(optarg);
      } else {
        error = 1;
      }
      break;      
      
    case '?':
      /* getopt already printed an error message. */
      status = TH_ERROR;
      break;

    default:
      
      /* find it in the table, dispatch if there */
      i = 0;
      found = 0;
      while (i < num_arg_entries && !found) {
        if (arg_entries[i]->optchar == c) {
          found = 1;
          if (optarg != NULL) {
            arg_entries[i]->value = malloc(strlen(optarg) + 1);
            strcpy(arg_entries[i]->value,optarg);
          }
          error = arg_entries[i]->handler(c,optarg);
        } else {
          i++;
        }
      }
      if (!found || error) {
        status = TH_ERROR;
      }
      break;
    }
  }

  return status;
}

/* TODO - really only master should print usage ?? */
/*****************************************************************************
 *  usage
 *****************************************************************************/
void usage(void) 
{
  th_log("\nusage: %s_%s <args>\n\n", tc_get_testcase_name(), th_get_impl_string());
  th_log("  -h : print this usage message\n");
  th_log("  -v : run in verbose mode\n");
  th_log("  -t : use times instead of gettimeofday\n");
  th_log("  -N <node_number> : use MCAPI node number [default = 0]\n");
  th_log("  -o <node_offset> : use MCAPI node ids starting at offset [default = 0]\n");
#if (WITH_AFFINITY)
  th_log("  -A : set all tasks to processor affinity 0\n");
#endif
}




#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
