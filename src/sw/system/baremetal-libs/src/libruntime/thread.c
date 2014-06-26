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

#include "runtime.h"
#include "thread.h"
#include "task.h"
#include "trace.h"
#include "scheduler.h"
#include "vmm.h"
#include "optimsoc-baremetal.h"

#include <errno.h>
#include <assert.h>
#include <stdio.h>

unsigned int thread_next_id;
volatile unsigned int thread_send_busy;
thread_recv_t thread_recv_data;

void thread_attr_init(thread_attr_t *attr) {
    attr->args = NULL;
    attr->flags = THREAD_FLAG_NO_FLAGS | THREAD_FLAG_KERNEL;
    attr->force_id = 0;
    attr->identifier = NULL;
}


/*
 * Create a new thread with a given starting function.
 */
int thread_create(thread_t* thread, void (*func)(void*), thread_attr_t *attr) {
    thread_t t;

    if (attr == NULL) {
        attr = malloc(sizeof(thread_attr_t));
        assert(attr != NULL);
        thread_attr_init(attr);
    }

    t = malloc(sizeof(struct thread_t));
    assert(t != NULL);
    t->attributes = attr;

    arch_context_create(t, func, t->attributes->args);

    if (t->attributes->flags & THREAD_FLAG_FORCEID) {
        t->id = t->attributes->force_id;
    } else {
        t->id = thread_next_id++;
    }

    if (t->attributes->identifier == NULL) {
        t->name = malloc(64);
        assert(t->name != NULL);
        snprintf(t->name, 64, "thread %d", t->id);
        runtime_trace_createthread(t->name, t->id, thread, func);
    } else {
        t->name = strndup(t->attributes->identifier, 64);
        runtime_trace_createthread(t->attributes->identifier, t->id, thread, func);
    }

    t->joinlist = list_init(0);

    if(attr->flags & THREAD_FLAG_CREATE_SUSPENDED) {
	scheduler_add(t, wait_q);
	t->state = THREAD_SUSPENDED;
    } else {
	scheduler_add(t,ready_q);
    }
    list_add_tail(all_threads,(void*)t);

    *thread = t;

    return t->id;
}


int thread_send(thread_t t, unsigned int dest_tileid){
    uint32_t buffer[4];

    if(thread_send_busy){
	return 1;
    }
    thread_send_busy = 1;

    /* ensure thread not scheduled again */
    list_remove(ready_q, (void*)t);

    runtime_trace_sendthread(t->name, t->id, t, dest_tileid);

    set_bits(&buffer[0], dest_tileid, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&buffer[0], 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&buffer[0], optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);

    set_bits(&buffer[0], TM_MSG_REQ, TM_MSG_ID_MSB, TM_MSG_ID_LSB);
    buffer[1] = (unsigned int)t;
    buffer[2] = list_length(t->task->page_table);

    optimsoc_mp_simple_send(3,(uint32_t*) buffer);

    return 0;
}


void thread_receive(unsigned int *buffer,int len){

    unsigned int src_tile = extract_bits(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
    unsigned int tm_msg_id = extract_bits(buffer[0],TM_MSG_ID_MSB,TM_MSG_ID_LSB);

    unsigned int num_pages,i, success;
    uint32_t resp_buffer[4];
    void *page_addr;
    thread_t remote_thread, local_thread;
    int ret;
    list_entry_i page_table_iterator = NULL;
    page_table_entry_t *entry;

    assert(len>1);

    resp_buffer[0]=0;
    // Prepare response buffer
    set_bits(&resp_buffer[0], src_tile, OPTIMSOC_DEST_MSB, OPTIMSOC_DEST_LSB);
    set_bits(&resp_buffer[0], 0, OPTIMSOC_CLASS_MSB, OPTIMSOC_CLASS_LSB);
    set_bits(&resp_buffer[0], optimsoc_get_tileid(), OPTIMSOC_SRC_MSB, OPTIMSOC_SRC_LSB);
    resp_buffer[1] = buffer[1];

    switch(tm_msg_id){

    case TM_MSG_REQ:
	assert(len==3);

	remote_thread = (thread_t)buffer[1];

	// set default response
	set_bits(&resp_buffer[0], TM_MSG_REQ_NACK, TM_MSG_ID_MSB, TM_MSG_ID_LSB);

	if(thread_recv_data.remote_thread == NULL){
	    /* try to allocate pages */

	    if(thread_recv_data.page_pool == NULL){
		thread_recv_data.page_pool = list_init(0);
	    }

	    assert(list_length(thread_recv_data.page_pool) == 0);

	    num_pages = buffer[2];

	    success = 1;

	    for(i=0; i<num_pages; i++){
		page_addr = list_remove_head(page_pool);
		if(page_addr == NULL){
		    // Undo allocated pages
		    page_addr = list_remove_head(thread_recv_data.page_pool);
		    while(page_addr){
			list_add_tail(page_pool, page_addr);
			page_addr = list_remove_head(thread_recv_data.page_pool);
		    }
		    success = 0;
		    break;
		}
		list_add_tail(thread_recv_data.page_pool, page_addr);
	    }


	    // TODO try to allocate thread structure
	    if(success){
		thread_recv_data.remote_thread = remote_thread;
		set_bits(&resp_buffer[0], TM_MSG_REQ_ACK, TM_MSG_ID_MSB, TM_MSG_ID_LSB);
	    }
	}

	optimsoc_mp_simple_send(2,(uint32_t*) resp_buffer);
	break;

    case TM_MSG_REQ_ACK:
	assert(len == 2);
	local_thread = (thread_t)buffer[1];

	// Check if thread exists
	assert(list_contains(all_threads, (void*)local_thread) == 1);

	// Send page addresses

	entry = list_first_element(local_thread->task->page_table, &page_table_iterator);

	while(entry) {

	    set_bits(&resp_buffer[0], TM_MSG_PAGE, TM_MSG_ID_MSB, TM_MSG_ID_LSB);
	    resp_buffer[2] = (unsigned int)entry->vaddr_base;
	    resp_buffer[3] = (unsigned int)entry->paddr_base;

	    optimsoc_mp_simple_send(4,(uint32_t*) resp_buffer);

	    entry = list_next_element(local_thread->task->page_table, &page_table_iterator);
	}

	set_bits(&resp_buffer[0], TM_MSG_PAGE_END, TM_MSG_ID_MSB, TM_MSG_ID_LSB);

	optimsoc_mp_simple_send(2,(uint32_t*) resp_buffer);

	break;
    case TM_MSG_MIGRATE_ACK:
	assert(len == 2);
	local_thread = (thread_t)buffer[1];

	// Check if thread exists
	assert(list_contains(all_threads, (void*)local_thread) == 1);

	thread_send_busy = 0;
	list_remove(all_threads, (void*)local_thread);
	thread_destroy(local_thread);

	break;

    case TM_MSG_REQ_NACK:
    case TM_MSG_MIGRATE_NACK:
	assert(len == 2);
	local_thread = (thread_t)buffer[1];

	// Check if thread exists
	assert(list_contains(all_threads, (void*)local_thread) == 1);

	thread_send_busy = 0;
	// Could not migrate, reinsert thread into queue
	scheduler_add(local_thread, ready_q);

	break;
    case TM_MSG_PAGE:
	assert(len == 4);

	if(thread_recv_data.remote_page_table == NULL){
	    thread_recv_data.remote_page_table = list_init(0);
	}

        entry = malloc(sizeof(struct page_table_entry_t));
        assert(entry != NULL);
        entry->vaddr_base = (void*)buffer[2];
        entry->paddr_base = (void*)buffer[3];
        list_add_tail(thread_recv_data.remote_page_table, (void*)entry);

	break;
    case TM_MSG_PAGE_END:
	assert(len == 2);

	remote_thread = (thread_t)buffer[1];

	assert(list_length(thread_recv_data.remote_page_table) ==
	       list_length(thread_recv_data.page_pool));

	ret = thread_migrate(src_tile,
		       (void*)remote_thread,
		       thread_recv_data.remote_page_table,
		       thread_recv_data.page_pool);

	if(!ret){
	    // Thread successfully migrated
	    set_bits(&resp_buffer[0], TM_MSG_MIGRATE_ACK, TM_MSG_ID_MSB, TM_MSG_ID_LSB);
	} else {
	    set_bits(&resp_buffer[0], TM_MSG_MIGRATE_NACK, TM_MSG_ID_MSB, TM_MSG_ID_LSB);
	}

	// Migration finished
	thread_recv_data.remote_thread = NULL;

	optimsoc_mp_simple_send(2,(uint32_t*) resp_buffer);

	break;
    default:
	printf("Invalid thread message received. ID = %d\n", tm_msg_id);
	break;
    }
}


int thread_migrate(unsigned int remote_tileid, void* thread_foreign_addr, struct list_t *remote_thread_page_table, struct list_t *thread_page_pool){
    thread_t remote_thread, local_thread;
    struct page_table_entry_t *remote_entry, *local_entry;
    dma_transfer_handle_t dma_handle;

    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    dma_alloc(&dma_handle);

    remote_thread = malloc(sizeof(struct thread_t));
    assert(remote_thread != NULL);

    dma_transfer(remote_thread, remote_tileid, thread_foreign_addr, sizeof(struct thread_t)/4, REMOTE2LOCAL, dma_handle);
    dma_wait(dma_handle);

    local_thread = malloc(sizeof(struct thread_t));
    assert(local_thread != NULL);
    /*tid_t id */
    if (runtime_config_get_use_globalids()) {
        local_thread->id = remote_thread->id;
    } else {
        local_thread->id = thread_next_id++;
    }

    /* struct arch_thread_ctx_t *ctx */
    local_thread->ctx = malloc(sizeof(struct arch_thread_ctx_t));
    assert(local_thread->ctx != NULL);
    dma_transfer(local_thread->ctx, remote_tileid, remote_thread->ctx, sizeof(struct arch_thread_ctx_t)/4, REMOTE2LOCAL, dma_handle);
    dma_wait(dma_handle);


    /* char* name */
    local_thread->name = malloc(64);
    assert(local_thread->name != NULL);
    dma_transfer(local_thread->name, remote_tileid, remote_thread->name, 16, REMOTE2LOCAL, dma_handle);
    dma_wait(dma_handle);

    /* void *stack */
    /* not used in external apps*/
    local_thread->stack = NULL;

    /* enum thread_state state */
    local_thread->state = remote_thread->state;

    /* unsigned int flags */
    local_thread->flags = remote_thread->flags;

    /* unsigned int exit_code */
    local_thread->exit_code = remote_thread->exit_code;

    /* struct list_t *joinlist */
    /* TODO joinlist after thread migration */
    local_thread->joinlist = list_init(0);

    /* struct list_t *page_table */
    local_thread->task->page_table = list_init(0);

    remote_entry = list_remove_head(remote_thread_page_table);

    while(remote_entry){

        local_entry = malloc(sizeof(struct page_table_entry_t));
        local_entry->vaddr_base = remote_entry->vaddr_base;
        local_entry->paddr_base = list_remove_head(thread_page_pool);

        dma_transfer(local_entry->paddr_base, remote_tileid, remote_entry->paddr_base, PAGESIZE/4, REMOTE2LOCAL, dma_handle);
        dma_wait(dma_handle);

        list_add_tail(local_thread->task->page_table, (void*)local_entry);

        free(remote_entry);
        remote_entry = list_remove_head(remote_thread_page_table);
    }


    /* void *paddr_start, *paddr_end */
    /* not needed in this case */
    scheduler_add(local_thread, ready_q);
    list_add_tail(all_threads,(void*)local_thread);

    // Context switch when IDLE
    if (core_ctx->active_thread == core_ctx->idle_thread) {
        schedule();
    }

    // Cleanup
    free(remote_thread);
    return 0;

}


void thread_yield() {
    scheduler_yieldcurrent();
}

void thread_suspend() {
    scheduler_suspendcurrent();
}

void thread_resume(thread_t thread) {
    if (list_remove(wait_q,(void*)thread)) {
        //		runtime_report_scheduler_resume((unsigned int)thread);
        list_add_tail(ready_q,(void*) thread);
    } else {
        // Nothing to resume
    }
}

thread_t thread_self() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    return core_ctx->active_thread;
}

void thread_join(thread_t thread) {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    if (scheduler_thread_exists(thread)) {
        // Add to waiting queue
        list_add_tail(thread->joinlist, (void*) core_ctx->active_thread);
        //		runtime_report_thread_joinwait((unsigned int)thread);

        // Suspend
        thread_suspend();
    }
}

/*
 * Destory a given thread. TODO use list-functions to destroy thread
 */
void thread_destroy(thread_t thread)
{
    runtime_trace_destroythread(thread->id);

    /* Free memory */
    free(thread->ctx);
    free(thread->name);
    if(thread->stack){
	free(thread->stack);
    }

    free(thread);
}

void thread_handle(void (*f)(void*),void *arg) {
    //	runtime_report_thread_started((unsigned int)f);
    f(arg);
    //	runtime_report_thread_finished((unsigned int)f);
    thread_exit();
}

void thread_exit() {
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    //	runtime_report_thread_exit((unsigned int)active_thread);

    thread_t t = (thread_t) list_remove_head(core_ctx->active_thread->joinlist);
    while (t) {
        //  		runtime_report_thread_joinsignal((unsigned int)t);
        thread_resume(t);
        t = list_remove_head(core_ctx->active_thread->joinlist);
    }

    scheduler_thread_exit();
}

/*
 * Create a context for a new thread.
 */
int arch_context_create(thread_t thread, void (*start_routine)(void*), void *arg)
{
    /* Create context and initialize to 0 */
    thread->ctx = calloc(sizeof(struct arch_thread_ctx_t), 1);

    assert(thread->ctx != NULL);

    /* Set stack, arguments, starting routine and thread_handler */
    if (thread->attributes->flags & THREAD_FLAG_KERNEL) {
        thread->stack = malloc(4096);
        assert(thread->stack != NULL);
        thread->ctx->regs[0] = (unsigned int) thread->stack + 4092; /* Stack pointer */
        thread->ctx->regs[2] = (unsigned int) start_routine;
        thread->ctx->regs[3] = (unsigned int) arg;
        thread->ctx->pc      = (unsigned int) &thread_handle;
        thread->ctx->sr      = 0x8017;
        /* - supervisor mode
         * - tick timer enabled
         * - irq enabled
         * - ic enabled
         * - dc disabled
         * - mmu disabled */
    } else {
        /* Set stack, arguments, starting routine and thread_handler */
        thread->stack = NULL;
        thread->ctx->regs[0] = 0xfffffffc;
        /* For the moment only one parameter is supported. */
        thread->ctx->regs[2] = (unsigned int) arg;
        thread->ctx->pc = 0x0;  /* entry point of external app*/
        thread->ctx->sr = 0x8077;
	/* - supervisor mode
         * - tick timer enabled
         * - irq enabled
         * - ic enabled
         * - dc disabled
         * - immu/dmmu enabled */
    }

    return 0;
}

struct syscall_thread_create_args {
    int rv;
    thread_t *thread;
    void *attr;
    void (*start_routine) (void*);
    void *arg;
};

void syscall_thread_create(void* args) {
    struct syscall_thread_create_args *targs;

    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

    targs = vmm_virt2phys(core_ctx->active_thread, args, 4);

    thread_attr_t attr;
    thread_attr_init(&attr);
//    attr.identifier = malloc(256);
    attr.flags &= ~THREAD_FLAG_KERNEL;

    thread_t *thread;
    thread = vmm_virt2phys(core_ctx->active_thread, targs->thread, 4);

    thread_create(thread, targs->start_routine, &attr);

    (*thread)->task = core_ctx->active_thread->task;

    (*thread)->task_local_id = (*thread)->task->next_thread_id++;
    (*thread)->ctx->pc = (unsigned int) targs->start_routine;
    (*thread)->ctx->regs[0] = 0xdffffffc - (0x80000 * (*thread)->task_local_id);
    (*thread)->ctx->regs[2] = (unsigned int) targs->arg;

    targs->rv = (*thread)->task_local_id;
}
