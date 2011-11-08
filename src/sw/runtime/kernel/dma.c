#include "vmm.h"

#include "dma.h"
#include "thread.h"
#include "scheduler.h"

struct {
	thread_t *waiting;
	void* address;
} dma_state;

struct dma_transfer {
  struct dma_transfer *next;
  struct dma_transfer *prev;

};

struct dma_transfer *dma_transfer_list;

void dma_init() {
  dma_transfer_list = NULL;
  pagetable_addmapping(active_thread[coreid()]->process->page_dir,(void*)0xe0000000,(void*)0xe0000000);
}

unsigned int dma_transfer_l2r(void* local,unsigned int remote_tile, void* remote, unsigned int size, dma_transfer_id *id) {
  *((volatile void**) 0xe0001000) = local;
  *((volatile unsigned int*) 0xe0001004) = size;
  *((volatile unsigned int*) 0xe0001008) = remote_tile;
  *((volatile void**) 0xe000100c) = remote;
  *((volatile unsigned int*) 0xe0001010) = 0;

  *((volatile unsigned int*) 0xe0001014) = 1;

  return 0;
}

// old:

void* dma_transfer(void* local,void* remote, unsigned int size) {
	pagetable_addmapping(active_thread[coreid()]->process->page_dir,(void*)0xe0000000,(void*)0x80002000);
	(*(volatile unsigned int*) 0x80002000) = size;
	return (void*) 0xe0000000;
}

unsigned int dma_poll(void* vaddr) {
	void *addr;
	pagetable_virt2phys(active_thread[coreid()]->process->page_dir,vaddr,&addr);
	return *((volatile unsigned int*) addr);
}

void svc_dma_handler(unsigned int req,arch_thread_ctx_t *ctx) {
	switch (req) {
		case 0:
				arch_context_set_return(ctx,dma_transfer(NULL,NULL,(unsigned int) arch_context_extract_parameter(ctx,4)));
			break;
		case 1:
				arch_context_set_return(ctx,dma_poll(arch_context_extract_parameter(ctx,2)));
			break;
		case 2:
				dma_state.waiting = active_thread[coreid()];
				pagetable_virt2phys(active_thread[coreid()]->process->page_dir, (void*) arch_context_extract_parameter(ctx,2), &(dma_state.address));
				scheduler_suspend(ctx);
			break;
		default:
			break;
	}
}

void dma_interrupt_handler() {
	volatile unsigned int trigger;
	trigger = *((volatile unsigned int*) dma_state.address);
	scheduler_resume(dma_state.waiting->id);
}
