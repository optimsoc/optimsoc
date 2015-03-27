/* Copyright (c) 2012-2013 by the author(s)
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

#include <stdint.h>

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

// Page table entry
typedef uint32_t optimsoc_pte_t;

// Page table
typedef optimsoc_pte_t* optimsoc_page_table_t;

// Page directory
typedef optimsoc_pte_t* optimsoc_page_dir_t;

extern optimsoc_page_dir_t optimsoc_vmm_create_page_dir();

extern optimsoc_page_table_t optimsoc_vmm_create_page_table();

extern void optimsoc_vmm_add_page_table(optimsoc_page_dir_t directory,
		uint32_t index, optimsoc_page_table_t);

extern uint32_t optimsoc_vmm_virt2phys(optimsoc_page_dir_t directory,
		uint32_t vaddr, uint32_t *paddr);

extern uint32_t optimsoc_vmm_phys2virt(optimsoc_page_dir_t directory,
		uint32_t paddr, uint32_t *vaddr);

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
#define THREAD_FLAG_NO_FLAGS    0x0
#define THREAD_FLAG_IDLE_THREAD 0x80000000
#define THREAD_FLAG_PIN         0x00000100
#define THREAD_FLAG_CORE_MASK   0x000000FF
#define THREAD_FLAG_FORCEID     0x40000000

struct optimsoc_thread_attr {
    void *args;
    uint32_t flags;
    uint32_t force_id;
    char *identifier;
    void *extra_data;
};

/**
 * Thread identifier.
 *
 * The internals are not exposed.
 */

typedef uint32_t optimsoc_thread_t;

/**
 * Create a new thread.
 *
 * The function allocates a thread data structure in the runtime system
 * and stores the pointer to it at the given address. The function
 * specified by start is called and the argument arg given to it.
 * The function is automatically added to the ready queue.
 */
int optimsocthread_create(optimsoc_thread_t *thread,
		void (*start)(void*), struct optimsoc_thread_attr *attr);

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
 * Wait for a thread until it exits.
 *
 * If the thread is still running, the function blocks and waits for
 * the finish of the thread. If it is already finished, the function
 * returns immediately.
 */
int optimsoc_thread_join(optimsoc_thread_t thread,
		optimsoc_thread_t waitforthread);

/**
 * @}
 */

/**
 * \defgroup syscall Syscall Handling
 * \ingroup libruntime
 * @{
 */

struct optimsoc_syscall {
    uint32_t id; /*!< Identifier of the system call */
    uint32_t output; /*!< Output/return value */
    uint32_t param[6]; /*!< Six parameters to the system call */
};

typedef void (*optimsoc_syscall_handler_fptr) (struct optimsoc_syscall *syscall);

void optimsoc_syscall_handler_set(optimsoc_syscall_handler_fptr handler);

/**
 * @}
 */

extern void runtime_config_set_numticks(unsigned int ticks);

extern void runtime_config_set_use_globalids(unsigned int v);

#endif /* OPTIMSOC_RUNTIME_H_ */
