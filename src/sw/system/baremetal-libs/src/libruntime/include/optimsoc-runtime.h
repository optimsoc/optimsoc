/* Copyright (c) 2012-2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#ifndef __OPTIMSOC_RUNTIME_H__
#define __OPTIMSOC_RUNTIME_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "optimsoc-baremetal.h"

/**
 * \defgroup libruntime Runtime library
 */

/**
 * \defgroup initconfig Initialization and Configuration
 * \ingroup libruntime
 */

/**
 * Boot into the runtime system
 *
 * From your main program you can boot into the runtime system using this
 * function. It initializes the system and then replaces the current program
 * with the scheduler which takes over then. You need to define an init()
 * function as this is the first (kernel) thread that is started then.
 */
void optimsoc_runtime_boot(void);

/**
 * \defgroup paging Page Handling
 * \ingroup libruntime
 * @{
 */

/**
 * Page directory
 *
 * The page directory is the data structure for the memory lookup from virtual
 * addresses to physical addresses. It can be shared by multiple threads and
 * is assigned to a thread by calling optimsoc_thread_set_pagedir.
 *
 * It should not be manipulated directly, but only by the functions provided.
 * Mapping pages is thread-safe, but generally caution is necessary when
 * calling the functions on two different cores concurrently when accessing
 * the same virtual pages.
 */
typedef uint32_t* optimsoc_page_dir_t;

/**
 * Create a new page directory
 *
 * This function allocates a new page directory. You can set it for a thread by
 * calling optimsoc_thread_set_pagedir.
 *
 * @return Empty page dir
 */
optimsoc_page_dir_t optimsoc_vmm_create_page_dir(void);

/**
 * Map virtual page to physical page
 *
 * This functions adds the entry of the virtual page of vaddr to the physical
 * page of paddr.
 *
 * @param directory Directory to add page mapping to
 * @param vaddr Virtual address of the page to map
 * @param paddr Physical address of the page to map
 * @return Success of operation
 */
int optimsoc_vmm_map(optimsoc_page_dir_t directory, uint32_t vaddr,
                     uint32_t paddr);

/**
 * Remove a page from directory
 *
 * Remove an entry from the page directory.
 *
 * @param directory Directory to remove page from
 * @param vaddr Virtual address of page to map
 * @return Success of operation
 */
int optimsoc_vmm_unmap(optimsoc_page_dir_t directory, uint32_t vaddr);

/**
 * Virtual to physical mapping
 *
 * Map a virtual address to the physical address.
 *
 * @param directory Directory for lookup
 * @param vaddr Virtual address
 * @param[out] paddr Physical address
 * @return Success of operation
 */
int optimsoc_vmm_virt2phys(optimsoc_page_dir_t directory, uint32_t vaddr,
                           uint32_t *paddr);

/**
 * Physical to virtual mapping
 *
 * Find virtual address for a physical address. This is a very time consuming
 * operation, especially if the virtual address space is fully or sparsely
 * populated. It also only finds the first mapping, but may be usefull to
 * find and delete incorrect mappings.
 *
 * @param directory Directory for lookup
 * @param paddr Physical address
 * @param[out] vaddr Virtual address
 * @return Success of operation
 */
int optimsoc_vmm_phys2virt(optimsoc_page_dir_t directory, uint32_t paddr,
                           uint32_t *vaddr);

/**
 * Page fault handler type
 *
 * The function pointer is used to define callback function for page faults
 * when a page cannot be found in the page directory. The user code has to
 * take appropriate actions then.
 *
 * @param vaddr The virtual address of the fault
 */
typedef void (*optimsoc_pfault_handler_fptr) (uint32_t vaddr);

/**
 * Set handler for data page faults
 *
 * Register the handler for fault exceptions from the data MMU.
 *
 * @param handler Handler to register
 */
void optimsoc_vmm_set_dfault_handler(optimsoc_pfault_handler_fptr handler);

/**
 * Set handler for instruction page faults
 *
 * Register the handler for fault exceptions from the instructions MMU.
 *
 * @param handler Handler to register
 */
void optimsoc_vmm_set_ifault_handler(optimsoc_pfault_handler_fptr handler);

/**
 * Destroy a page directory
 *
 * Destroy an entire page directory and free the memory.
 *
 * @param dir Directory to destroy
 */
void optimsoc_vmm_destroy_page_dir(optimsoc_page_dir_t dir);

typedef void* (*page_alloc_fptr)();

optimsoc_page_dir_t optimsoc_vmm_dir_copy(uint32_t remote_tile,
                                          void *remote_addr,
                                          page_alloc_fptr page_alloc_fnc);

/**
 * @}
 */


/**
 * \defgroup thread Thread Management
 * \ingroup libruntime
 * @{
 */

/* Todo Doxygen*/

/* FLAGS for thread creation */
#define OPTIMSOC_THREAD_FLAG_IDLE_THREAD      0x80000000
#define OPTIMSOC_THREAD_FLAG_PIN              0x00000100
#define OPTIMSOC_THREAD_FLAG_CORE_MASK        0x000000FF
#define OPTIMSOC_THREAD_FLAG_FORCEID          0x40000000
#define OPTIMSOC_THREAD_FLAG_KERNEL           0x20000000
#define OPTIMSOC_THREAD_FLAG_CREATE_SUSPENDED 0x10000000

struct optimsoc_thread_attr {
    void *args;
    uint32_t flags;
    uint32_t force_id;
    char *identifier;
    void *extra_data;
};

void optimsoc_thread_attr_init(struct optimsoc_thread_attr *attr);

/**
 * Thread identifier.
 *
 * The internals are not exposed.
 */

typedef struct optimsoc_thread* optimsoc_thread_t;

/**
 * Create a new thread.
 *
 * The function allocates a thread data structure in the runtime system
 * and stores the pointer to it at the given address. The function
 * specified by start is called and the argument arg given to it.
 * The function is automatically added to the ready queue.
 */
int optimsoc_thread_create(optimsoc_thread_t *thread,
                           void (*start)(void*),
                           struct optimsoc_thread_attr *attr);

/**
 * Identify the thread currently running.
 */
optimsoc_thread_t optimsoc_thread_current();

/**
 * Yield thread.
 *
 * The specified thread is put at the end of the schedule. If the thread is the
 * current thread (what makes most sense), the thread yields its remaining
 * quantum and is scheduled to be executed later again (e.g., when waiting for
 * I/O).
 */
void optimsoc_thread_yield(optimsoc_thread_t thread);

/**
 * Suspend thread.
 *
 * The specified thread is suspended until it gets reactivated.
 */
void optimsoc_thread_suspend(optimsoc_thread_t thread);

/**
 * Exit current thread's execution.
 */
void optimsoc_thread_exit();

/**
 * Resume a suspended thread.
 */
void optimsoc_thread_resume(optimsoc_thread_t thread);

/**
 * Insert a thread in the system.
 */
void optimsoc_thread_add(optimsoc_thread_t thread);

/**
 * Remove a thread from the system.
 */
void optimsoc_thread_remove(optimsoc_thread_t thread);

/**
 * Copy a remote thread with DMA
 */
optimsoc_thread_t optimsoc_thread_dma_copy(uint32_t remote_tile,
                                           void *remote_addr);

/**
 * Wait for a thread until it exits.
 *
 * If the thread is still running, the function blocks and waits for
 * the finish of the thread. If it is already finished, the function
 * returns immediately.
 */
int optimsoc_thread_join(optimsoc_thread_t thread,
                         optimsoc_thread_t waitforthread);

/**
 * Extra data associated with a thread
 *
 * The user or runtime system can set extra data with the thread.This can be
 * a pointer to a process structure or similar.
 *
 * @param thread Thread to assign the data pointer to
 * @param extra_data Pointer to the data
 */
void optimsoc_thread_set_extra_data(optimsoc_thread_t thread,
                                    void* extra_data);

/**
 * Get extra data pointer
 *
 * Retrieve the pointer to the extra data assigned to a thread.
 *
 * @param thread Thread to get data pointer from
 * @return Associated data pointer
 */
void* optimsoc_thread_get_extra_data(optimsoc_thread_t thread);

/**
 * Set page directory for thread
 *
 * Assign a page directory to a thread.
 *
 * @param thread Thread to assign page directory to
 * @param dir Page directory to assign
 */
void optimsoc_thread_set_pagedir(optimsoc_thread_t thread,
                                 optimsoc_page_dir_t dir);

/**
 * Retrieve page directory of thread
 *
 * Get the page directory associated with a thread
 *
 * @param thread Thread to get page directory of
 * @return Page directory
 */
optimsoc_page_dir_t optimsoc_thread_get_pagedir(optimsoc_thread_t thread);

/**
 * @}
 */

/**
 * \defgroup syscall Syscall Handling
 * \ingroup libruntime
 * @{
 */

/**
 * Syscall data
 *
 * The syscall data is an identifier, up to six parameters and a return value.
 */
struct optimsoc_syscall {
    uint32_t id; /*!< Identifier of the system call */
    uint32_t output; /*!< Output/return value */
    uint32_t param[6]; /*!< Six parameters to the system call */
};

/**
 * Syscall handler function
 *
 * The user or the runtime system registers the syscall handler function that is
 * called with a pointer to the syscall data.
 *
 * @param[inout] sysc The syscall data
 */
typedef void (*optimsoc_syscall_handler_fptr) (struct optimsoc_syscall *sysc);

/**
 * Register a syscall handler
 *
 * @param handler The syscall handler function to call on syscall exceptions.
 */
void optimsoc_syscall_handler_set(optimsoc_syscall_handler_fptr handler);

/**
 * @}
 */

extern void runtime_config_set_numticks(unsigned int ticks);

extern void runtime_config_set_use_globalids(unsigned int v);

struct optimsoc_list_entry_t {
    void* data;
    struct optimsoc_list_entry_t* prev;
    struct optimsoc_list_entry_t* next;
};

struct optimsoc_list_t {
    struct optimsoc_list_entry_t* head;
    struct optimsoc_list_entry_t* tail;
};

typedef struct optimsoc_list_entry_t* optimsoc_list_iterator_t;

struct optimsoc_list_t* optimsoc_list_init(void* data);
void optimsoc_list_add_tail(struct optimsoc_list_t* l, void* data);
void optimsoc_list_add_head(struct optimsoc_list_t* l, void* data);
void* optimsoc_list_remove_tail(struct optimsoc_list_t* l);
void* optimsoc_list_remove_head(struct optimsoc_list_t* l);
int optimsoc_list_remove(struct optimsoc_list_t* l, void* data);
int optimsoc_list_contains(struct optimsoc_list_t *l, void* data);
size_t optimsoc_list_length(struct optimsoc_list_t* l);

void *optimsoc_list_find_content_header(struct optimsoc_list_t *l,
                                        uint32_t content);

void* optimsoc_list_first_element(struct optimsoc_list_t* l,
                                  optimsoc_list_iterator_t *list_iter);
void* optimsoc_list_next_element(struct optimsoc_list_t* l,
                                 optimsoc_list_iterator_t *list_iter);

struct optimsoc_list_t *optimsoc_list_dma_copy(uint32_t remote_tile,
                                               void *remote_list_addr,
                                               size_t data_size);

/**
 * Suspend the current thread for a number of timer ticks
 */
void optimsoc_timer_wait_ticks(uint32_t ticks);

/**
 * Initiate a DMA transfer
 */
void optimsoc_dma_transfer(void *local,
                           uint32_t remote_tile,
                           void *remote,
                           size_t size,
                           dma_direction_t dir);
#endif /* OPTIMSOC_RUNTIME_H_ */
