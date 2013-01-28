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
documentation and/or other materia]ls provided with the distribution. 

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

#ifndef MRAPI_IMPL_SPEC_H
#define MRAPI_IMPL_SPEC_H



#ifdef MRAPI_HAVE_INTTYPES_H
#include <stdint.h>
#endif

#include <inttypes.h>
   
#include <signal.h>
#include <pthread.h> 
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>   
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <mrapi.h>
#include <mca_config.h>
#include <mca_utils.h>                                                                                                                                                                         
/******************************************************************
           definitions and constants 
 ******************************************************************/
#define MRAPI_MAX_SEMS 128  /* we don't currently support different values for max mutex/sem/rwl */
#define MRAPI_MAX_SHMEMS 10
#define MRAPI_MAX_RMEMS 10
#define MRAPI_MAX_REQUESTS MCA_MAX_REQUESTS 
#define MRAPI_MAX_SHARED_LOCKS 32

#define MRAPI_RMEM_DEFAULT MRAPI_RMEM_DUMMY                                                                                             
                                                  
#define mrapi_dprintf mca_dprintf                                 
/******************************************************************
           datatypes
******************************************************************/    

typedef enum {
  RWL,
  SEM,
  MUTEX,
} lock_type;

/* Metadata resource related structs */
typedef enum {
  CPU,
  CACHE,
  MEM,
  CORE_COMPLEX,
  CROSSBAR,
  SYSTEM,
} mrapi_resource_type;


typedef enum {
  MRAPI_ATTR_STATIC,
  MRAPI_ATTR_DYNAMIC,
} mrapi_attribute_static;


typedef enum {
  MRAPI_EVENT_POWER_MANAGEMENT,
  MRAPI_EVENT_CROSSBAR_BUFFER_UNDER_20PERCENT,
  MRAPI_EVENT_CROSSBAR_BUFFER_OVER_80PERCENT,
} mrapi_event_t;

/* Part of this struct is opaque */
typedef struct mrapi_resource_type {
  char*                         name;
  mrapi_resource_type           resource_type;
  uint32_t                      number_of_children;
  struct mrapi_resource_type  **children;
  uint32_t                      number_of_attributes;
  void                        **attribute_types;
  void                        **attribute_values;
  mrapi_attribute_static      **attribute_static;
  mca_boolean_t               **attribute_started;
} mrapi_resource_t;


typedef struct {
  mca_boolean_t ext_error_checking;
  mca_boolean_t shared_across_domains; 
  mca_boolean_t recursive; /* only applies to mutexes */
  void* mem_addr; 
  uint32_t mem_size;
  mrapi_resource_t resource;
} mrapi_impl_attributes_t;

typedef mrapi_impl_attributes_t mrapi_mutex_attributes_t;
typedef mrapi_impl_attributes_t mrapi_sem_attributes_t;
typedef mrapi_impl_attributes_t mrapi_rwl_attributes_t;
typedef mrapi_impl_attributes_t mrapi_shmem_attributes_t;
typedef mrapi_impl_attributes_t mrapi_rmem_attributes_t;

typedef uint32_t mrapi_mutex_hndl_t;
typedef uint32_t mrapi_sem_hndl_t;
typedef uint32_t mrapi_rwl_hndl_t;
typedef uint32_t mrapi_shmem_hndl_t;
typedef uint32_t mrapi_rmem_hndl_t;

typedef int mrapi_key_t;  /* system created key used for locking/unlocking for recursive mutexes */


#define MRAPI_MAX_NODES MCA_MAX_NODES
#define MRAPI_MAX_DOMAINS MCA_MAX_DOMAINS
#define MRAPI_MAX_CALLBACKS 10


/*-------------------------------------------------------------------
  the mrapi_impl database structure
  -------------------------------------------------------------------*/
/* resource structure */
typedef struct {
  void (*callback_func) (mrapi_event_t the_event);
  mrapi_event_t callback_event;
  unsigned int callback_frequency;
  unsigned int callback_count;
  mrapi_node_t node_id;
} mrapi_callback_t;

typedef struct {
  mrapi_boolean_t valid;
  mrapi_boolean_t locked;
  mrapi_uint8_t lock_holder_nindex; 
  mrapi_uint8_t lock_holder_dindex;
  mrapi_uint32_t lock_key;
} mrapi_lock_t;

/* mutexes, semaphores and reader-writer locks share this data structure */
typedef struct {
  uint32_t handle; /* used for reverse look-up when ext error checking is enabled */
  int32_t num_locks; 
  mrapi_lock_t locks [MRAPI_MAX_SHARED_LOCKS]; 
  int32_t key; /* the shared key passed in on get/create */
  int32_t id; /* the handle returned by the os or whoever creates it */
  int32_t shared_lock_limit;
  lock_type   type;
  mrapi_sem_attributes_t attributes; 
  mrapi_boolean_t valid;
  /* only used when ext error checking is enabled.  Basically protects the 
     entry from ever being overwritten. */
  mrapi_boolean_t deleted; 
  uint8_t refs; /* the number of nodes using the sem (for reference counting) */
} mrapi_sem_t;

/* shared memory */
typedef struct {
  mrapi_boolean_t valid;
  int32_t key; /* the shared key passed in on get/create */
  int32_t id; /* the handle returned by the os or whoever creates it */
  void* addr;
  mrapi_shmem_attributes_t attributes;
  uint8_t refs; /* the number of nodes currently attached (for reference counting) */
} mrapi_shmem_t;

/* remote memory */
typedef struct {
  mrapi_boolean_t valid;
  mrapi_rmem_atype_t access_type;
  int32_t key; /* the shared key passed in on get/create */
  const void* addr;
  size_t size;
  mrapi_rmem_attributes_t attributes;
  uint8_t refs; /* the number of nodes currently attached (for reference counting) */
  uint8_t nodes[MRAPI_MAX_NODES]; /* the list of nodes currently attached */
} mrapi_rmem_t;

typedef struct {
  struct sigaction signals[MCA_MAX_SIGNALS];
  mrapi_boolean_t valid;
  pid_t pid;
  pthread_t tid;
  mrapi_uint_t node_num;
  uint8_t sems [MRAPI_MAX_SEMS]; // list of sems this node is referencing 
  uint8_t shmems [MRAPI_MAX_SHMEMS]; // list of shmems this node is referencing
} mrapi_node_data;

typedef struct{
  mrapi_boolean_t valid;
  mrapi_uint8_t num_nodes;
  mrapi_domain_t domain_id;
  mrapi_node_data nodes [MRAPI_MAX_NODES];
} mrapi_domain_data;

typedef struct {
  mrapi_boolean_t valid;
  mrapi_boolean_t completed;
  mrapi_boolean_t cancelled;
  uint32_t node_num;
  mrapi_domain_t domain_id;
  mrapi_status_t status;
} mrapi_request_data;

struct lock_t {
  volatile uint32_t lock;
};

typedef struct {
  int32_t num_locks;
  struct lock_t locks [MRAPI_MAX_SHARED_LOCKS];
  int32_t key; /* the shared key passed in on get/create */
  mrapi_boolean_t valid;
} mrapi_sys_sem_t; 

typedef struct {
  struct lock_t global_lock;
  mrapi_uint8_t num_shmems;
  mrapi_uint8_t num_sems;
  mrapi_uint8_t num_rmems;
  mrapi_uint8_t num_domains;
  mrapi_shmem_t shmems[MRAPI_MAX_SHMEMS];
  mrapi_sem_t sems[MRAPI_MAX_SEMS];
  mrapi_sys_sem_t sys_sems[MRAPI_MAX_SEMS];
  mrapi_rmem_t rmems [MRAPI_MAX_RMEMS];
  mrapi_domain_data domains [MRAPI_MAX_DOMAINS];
  mrapi_request_data requests [MRAPI_MAX_REQUESTS];
  /* Rollover variables */
  void (*rollover_callbacks_ptr[MRAPI_MAX_CALLBACKS]) (void);
  mrapi_uint16_t rollover_index;
  /* Callback variables */
  mrapi_callback_t callbacks_array[MRAPI_MAX_CALLBACKS];
  mrapi_uint16_t   callback_index;
} mrapi_database;
 
#define TLS __thread
extern mrapi_database* mrapi_db; /* our shared memory addr for our internal database */



/******************************************************************
           Function declarations (the MRAPI impl API)
******************************************************************/
mrapi_boolean_t mrapi_impl_initialize (mrapi_domain_t domain_id, 
                                       mrapi_node_t node_id,
                                       mrapi_status_t* status);

mrapi_boolean_t mrapi_impl_initialized ();
mrapi_boolean_t mrapi_impl_finalize();

mrapi_boolean_t mrapi_impl_valid_status_param (const mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_valid_info_param (const mrapi_info_t* mrapi_info);
mrapi_boolean_t mrapi_impl_valid_parameters_param (mrapi_parameters_t mrapi_parameters);

mrapi_boolean_t mrapi_impl_valid_request_hndl (const mrapi_request_t* request);
mrapi_boolean_t mrapi_impl_canceled_request (const mrapi_request_t* request);

mrapi_boolean_t mrapi_impl_valid_node();
mrapi_boolean_t mrapi_impl_valid_node_num(mrapi_node_t node_num);
mrapi_boolean_t mrapi_impl_get_node_num(mrapi_node_t* node);
mrapi_boolean_t mrapi_impl_get_domain_num(mrapi_domain_t* domain);
mrapi_boolean_t mrapi_impl_valid_domain();
mrapi_boolean_t mrapi_impl_valid_domain_num(mrapi_domain_t domain_num);

mrapi_boolean_t mrapi_impl_get_domain_num(mrapi_domain_t* domain);

mrapi_boolean_t mrapi_impl_test(const mrapi_request_t* request,mrapi_status_t* status);




/* MUTEXES */
mrapi_boolean_t mrapi_impl_valid_mutex_hndl(mrapi_mutex_hndl_t mutex, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_mutex_get (mrapi_mutex_hndl_t* mutex,mrapi_mutex_id_t mutex_id);
mrapi_boolean_t mrapi_impl_mutex_create(mrapi_mutex_hndl_t* mutex,  
                                        mrapi_mutex_id_t mutex_id,
                                        const mrapi_mutex_attributes_t* attributes, 
                                        mrapi_status_t* status);
void mrapi_impl_mutex_init_attributes(mrapi_mutex_attributes_t* attributes);
void mrapi_impl_mutex_set_attribute (mrapi_mutex_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, const void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
void mrapi_impl_mutex_get_attribute (mrapi_mutex_hndl_t mutex, 
                                     mrapi_uint_t attribute_num, void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_mutex_delete(mrapi_mutex_hndl_t mutex);
mrapi_boolean_t mrapi_impl_mutex_lock(mrapi_mutex_hndl_t mutex, 
                                      mrapi_key_t* lock_key,
                                      mrapi_timeout_t timeout,
                                      mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_mutex_unlock(mrapi_mutex_hndl_t mutex,
                                        const mrapi_key_t* lock_key,
                                        mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_mutex_validID(mrapi_mutex_id_t mutex);

/* SEMAPHORES */
mrapi_boolean_t mrapi_impl_valid_sem_hndl (mrapi_sem_hndl_t sem,mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_sem_get (mrapi_sem_hndl_t* sem,
                                    mrapi_sem_id_t sem_id);
mrapi_boolean_t mrapi_impl_sem_create(mrapi_sem_hndl_t* sem, 
                                      mrapi_sem_id_t sem_id,
                                      const mrapi_sem_attributes_t* attributes, 
                                      mrapi_uint32_t shared_lock_limit
                                      ,mrapi_status_t* status);
void mrapi_impl_sem_init_attributes(mrapi_sem_attributes_t* attributes);
void mrapi_impl_sem_set_attribute (mrapi_sem_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, const void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
void mrapi_impl_sem_get_attribute (mrapi_sem_hndl_t sem, 
                                     mrapi_uint_t attribute_num, void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_sem_delete(mrapi_sem_hndl_t sem);
mrapi_boolean_t mrapi_impl_sem_lock(mrapi_sem_hndl_t sem, 
                                    int32_t num_locks,
                                    mrapi_timeout_t timeout,
                                    mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_sem_unlock(mrapi_sem_hndl_t sem,
                                      int32_t num_locks,
                                      mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_sem_validID(mrapi_sem_id_t sem);

/* READER/WRITER LOCKS */
mrapi_boolean_t mrapi_impl_valid_rwl_hndl (mrapi_rwl_hndl_t shmem,mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rwl_get (mrapi_rwl_hndl_t* rwl,mrapi_rwl_id_t rwl_id);
mrapi_boolean_t mrapi_impl_rwl_create(mrapi_rwl_hndl_t* rwl,  
                                      mrapi_rwl_id_t rwl_id,
                                      const mrapi_rwl_attributes_t* attributes, 
                                      mrapi_uint32_t reader_lock_limit,
                                      mrapi_status_t* status);
void mrapi_impl_rwl_init_attributes(mrapi_rwl_attributes_t* attributes);
void mrapi_impl_rwl_set_attribute (mrapi_rwl_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, const void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
void mrapi_impl_rwl_get_attribute (mrapi_rwl_hndl_t rwl, 
                                     mrapi_uint_t attribute_num, void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rwl_delete(mrapi_rwl_hndl_t rwl);
mrapi_boolean_t mrapi_impl_rwl_lock(mrapi_rwl_hndl_t rwl, 
                                    mrapi_rwl_mode_t mode,
                                    mrapi_timeout_t timeout, 
                                    mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rwl_unlock(mrapi_rwl_hndl_t rwl,
                                      mrapi_rwl_mode_t mode,
                                      mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rwl_validID(mrapi_rwl_id_t rwl);

/* SHARED MEMORY */
void mrapi_impl_shmem_create(mrapi_shmem_hndl_t* shm,
                                        uint32_t shmkey,
                                        uint32_t size,
                                        const mrapi_shmem_attributes_t* attributes,
                                        mrapi_status_t* status);

void mrapi_impl_shmem_init_attributes(mrapi_shmem_attributes_t* attributes);
void mrapi_impl_shmem_set_attribute (mrapi_shmem_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, const void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
void mrapi_impl_shmem_get_attribute (mrapi_shmem_hndl_t shmem, 
                                     mrapi_uint_t attribute_num, void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_valid_shmem_hndl (mrapi_shmem_hndl_t shmem);
mrapi_boolean_t mrapi_impl_shmem_get(mrapi_shmem_hndl_t* shm,uint32_t shmkey);
void* mrapi_impl_shmem_attach (mrapi_shmem_hndl_t shm);
mrapi_boolean_t mrapi_impl_shmem_attached (mrapi_shmem_hndl_t shmem);
mrapi_boolean_t mrapi_impl_shmem_exists(uint32_t shmkey);
mrapi_boolean_t mrapi_impl_shmem_delete(mrapi_shmem_hndl_t shm);
mrapi_boolean_t mrapi_impl_shmem_detach(mrapi_shmem_hndl_t shm);
mrapi_boolean_t mrapi_impl_shmem_validID(mrapi_shmem_id_t shmem);

/* REMOTE MEMORY */
mrapi_boolean_t mrapi_impl_valid_rmem_id (mrapi_rmem_id_t rmem_id);
mrapi_boolean_t mrapi_impl_valid_atype (mrapi_rmem_atype_t access_type);
mrapi_boolean_t mrapi_impl_rmem_attached (mrapi_rmem_hndl_t rmem);
mrapi_boolean_t mrapi_impl_rmem_exists (mrapi_rmem_id_t rmem_id);
void mrapi_impl_rmem_create(mrapi_rmem_hndl_t* rmem,
                                       mrapi_rmem_id_t rmem_id,
                                       const void* mem,
                                       mrapi_rmem_atype_t access_type,
                                       const mrapi_rmem_attributes_t* attributes,
                                       mrapi_uint_t size,mrapi_status_t* status);
void mrapi_impl_rmem_init_attributes(mrapi_rmem_attributes_t* attributes);
void mrapi_impl_rmem_set_attribute (mrapi_rmem_attributes_t* attributes, 
                                     mrapi_uint_t attribute_num, const void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
void mrapi_impl_rmem_get_attribute (mrapi_rmem_hndl_t rmem, 
                                     mrapi_uint_t attribute_num, void* attribute,size_t 
                                     attr_size, mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_valid_rmem_hndl(mrapi_rmem_hndl_t rmem);
mrapi_boolean_t mrapi_impl_rmem_get(mrapi_rmem_hndl_t* rmem_hndl,uint32_t rmem_id);
mrapi_boolean_t mrapi_impl_rmem_attach(mrapi_rmem_hndl_t rmem);
mrapi_boolean_t mrapi_impl_rmem_detach(mrapi_rmem_hndl_t rmem);
mrapi_boolean_t mrapi_impl_rmem_delete(mrapi_rmem_hndl_t rmem);
mrapi_boolean_t mrapi_impl_rmem_read( mrapi_rmem_hndl_t rmem,
                                      mrapi_uint32_t rmem_offset,
                                      void* local_buf,
                                      mrapi_uint32_t local_offset,
                                      mrapi_uint32_t bytes_per_access,
                                      mrapi_uint32_t num_strides,
                                      mrapi_uint32_t rmem_stride,
                                      mrapi_uint32_t local_stride,
                                      mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rmem_write( mrapi_rmem_hndl_t rmem,
                                       mrapi_uint32_t rmem_offset,
                                       const void* local_buf,
                                       mrapi_uint32_t local_offset,
                                       mrapi_uint32_t bytes_per_access,
                                       mrapi_uint32_t num_strides,
                                       mrapi_uint32_t rmem_stride,
                                       mrapi_uint32_t local_stride,
                                       mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_rmem_read_i( mrapi_rmem_hndl_t rmem,
                                        mrapi_uint32_t rmem_offset,
                                        void* local_buf,
                                        mrapi_uint32_t local_offset,
                                        mrapi_uint32_t bytes_per_access,
                                        mrapi_uint32_t num_strides,
                                        mrapi_uint32_t rmem_stride,
                                        mrapi_uint32_t local_stride,
                                        mrapi_status_t* status,
                                        mrapi_request_t* request);
mrapi_boolean_t mrapi_impl_rmem_write_i( mrapi_rmem_hndl_t rmem,
                                       mrapi_uint32_t rmem_offset,
                                       const void* local_buf,
                                       mrapi_uint32_t local_offset,
                                       mrapi_uint32_t bytes_per_access,
                                       mrapi_uint32_t num_strides,
                                       mrapi_uint32_t rmem_stride,
                                       mrapi_uint32_t local_stride,
                                       mrapi_status_t* status,
                                       mrapi_request_t* request);
mrapi_boolean_t mrapi_impl_rmem_validID(mrapi_rmem_id_t rmem);

/* RESOURCES */
mrapi_resource_t* mrapi_impl_resources_get(
                                           mrapi_rsrc_filter_t subsystem_filter,
                                           mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_resource_tree_free(
                                   mrapi_resource_t* const * root_ptr,
                                   mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_resource_get_attribute(
                                       const mrapi_resource_t* resource,
                                       mrapi_uint_t attribute_number,
                                       void* attribute_value,
                                       size_t attr_size,
                                       mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_dynamic_attribute_reset(
                                        const mrapi_resource_t *resource,
                                        mrapi_uint_t attribute_number,
                                        mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_dynamic_attribute_start(
                                        const mrapi_resource_t* resource,
                                        mrapi_uint_t attribute_number,
                                        void (*rollover_callback) (void),
                                        mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_dynamic_attribute_stop(
                                       const mrapi_resource_t* resource,
                                       mrapi_uint_t attribute_number,
                                       mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_resource_register_callback(
                                           mrapi_event_t event,
                                           unsigned int frequency,
                                           void (*callback_function) (mrapi_event_t event),
                                           mrapi_status_t* status);
mrapi_boolean_t mrapi_impl_valid_attribute_number(const mrapi_resource_t* resource,
						  const mrapi_uint_t attribute_number);

mrapi_boolean_t mrapi_impl_is_static(const mrapi_resource_t* resource,
				     const mrapi_uint_t attribute_number);

#endif
