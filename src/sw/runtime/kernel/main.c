#include "arch.h"

#include ARCH_INCL(printf.h)
#include <stdlib.h>

#include "thread.h"
#include "scheduler.h"
#include ARCH_INCL(utils.h) // TODO: remove later
#include "process.h"

#include "vmm.h"
#include "barrier.h"
#include "multicore.h"

#include "../appinit.h"

void *kernel_stack[NUMCORES]; // for exception handling
void *kernel_sp[NUMCORES];

int main() {
/*	if ( coreid() == 0) {
		multicore_init_early();

		void *stacks = malloc(0x2000);
		for ( unsigned int c = 0; c < NUMCORES; ++c ) {
			kernel_stack[c] = stacks + 0x500 * c;
			kernel_sp[c]    = stacks + 0x500 * (c+1) - 4;
		}

		scheduler_init();

		vmm_initialize();

		for ( unsigned int i = 0; i < app_init_size; ++i ) {
			void *start = app_init[i].start;
			void *end   = app_init[i].end;

			for ( void* p = start; p <= end; p += PAGESIZE ) {
				vmm_reserve_page((void*)((unsigned int) p & ~PAGEMASK));
			}
		}

		for ( unsigned int i = 0; i < app_init_size; ++i ) {
			void *start = app_init[i].start;
			void *end   = app_init[i].end;

			//			printf("Load application from %p-%p\n",start,end);

			process_create(start,end);
		}

		//		printf("%u pages available\n", vmm_available_pages());

		//		printf("Initialized!\n");

		multicore_init_late();
	}

	boot_barrier();*/

	//	printf("Start %u\n",coreid());
	//scheduler_start();

  vmm_initialize();
  dma_init();

  unsigned int id;
  dma_transfer_l2r(0x00002000,2,0x00000000,8,&id);
  return 2;
}
