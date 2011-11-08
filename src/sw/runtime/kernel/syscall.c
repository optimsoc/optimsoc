#include "thread.h"
#include "arch.h"
#include ARCH_INCL(printf.h)
#include ARCH_INCL(utils.h)

#include "scheduler.h"
#include "vmm.h"
#include "dma.h"

// System Calls
// 8 Send request to service

#define SYSCALL_MAX	8

// service 0 is always the kernel
#define SVC_NUM 3
const char* services[SVC_NUM] = { "kernel", "dma", "scheduler" };

unsigned int svc_identify(char* identifier) {
	unsigned int i;
	for (i = 0; i < SVC_NUM; i++) {
		if (strcmp(identifier,services[i])==0) {
			return i;
		}
	}
	return 0;
}

void svc_kernel_handler(unsigned int req,arch_thread_ctx_t *ctx) {
	if (req==0) {
		void *vaddr = (void*) arch_context_extract_parameter(ctx,2);
		void *paddr;
		pagetable_virt2phys(active_thread[coreid()]->process->page_dir,vaddr,&paddr);
		arch_context_set_return(ctx,svc_identify((char*)paddr));
	}
}

static void (* const svc_handler_table[])(unsigned int,arch_thread_ctx_t*) = {
		svc_kernel_handler,
		svc_dma_handler,
		svc_scheduler_handler
};

void svc_request(unsigned int service,arch_thread_ctx_t *ctx) {
	unsigned int req = (unsigned int) arch_context_extract_parameter(ctx,1);
	if (service < SVC_NUM)
		svc_handler_table[service](req,ctx);
	else
		printf("Invalid service %d\n", service);
}

void syscall_svc_request(arch_thread_ctx_t *ctx) {
	svc_request((unsigned int) arch_context_extract_parameter(ctx,0),ctx);
}

void syscall_page_alloc_handler(arch_thread_ctx_t *ctx) {
	vmm_page_alloc_handler((void*) arch_context_extract_parameter(ctx,0),(unsigned int) arch_context_extract_parameter(ctx,1));
}

void syscall_write_handler(arch_thread_ctx_t *ctx) {
	void *paddr;
	void *vaddr = arch_context_extract_parameter(ctx,1);
	pagetable_virt2phys(active_thread[coreid()]->process->page_dir,vaddr,&paddr);
	printf( (char*) paddr );
	arch_context_set_return(ctx,arch_context_extract_parameter(ctx,2));
}

static void (* const syscall_table[])(arch_thread_ctx_t*) = {
		0,
		scheduler_yield,
		scheduler_suspend,
		syscall_page_alloc_handler,
		syscall_write_handler,
		process_terminate,
		0,
		syscall_thread_yield,
		syscall_svc_request
};

void syscall(arch_thread_ctx_t *syscall_ctx, unsigned int no) {
/*	printf("Syscall %u occured with ctx: %p\n",no,syscall_ctx);
	context_print(syscall_ctx); */

	if ( no > SYSCALL_MAX || !syscall_table[no] ) {
		printf("Invalid syscall %u\n",no);
		context_print(syscall_ctx);
	} else {
		syscall_table[no](syscall_ctx);
	}
}
