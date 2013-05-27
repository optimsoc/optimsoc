/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Simple setup for ethernet tile.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 *    Holm Rauchfuss, holm.rauchfuss@tum.de
 */

#include <stdio.h>
#include <optimsoc.h>
#include <malloc.h>
#include <dma.h>

void setup_generation (unsigned int * setup,unsigned int priority,unsigned int ct_index, unsigned int ct_addr, unsigned int mt_index, unsigned int mt_addr, unsigned int hash);

unsigned int byte2word (unsigned int x);

void main() {
    optimsoc_init(0);
    if (optimsoc_get_tileid() == 0) {
      dma_init();
  
      unsigned int *buffer_small;
      unsigned int *buffer_large;
      unsigned int *setup;
      unsigned int drop = 0;
      unsigned int length = 0;
      unsigned int *notification1;
      unsigned int *notification2;
      unsigned int *notification3;
      unsigned int queue = 0;
      unsigned int m;
      unsigned int i;
      unsigned int j;

      dma_transfer_handle_t h_small;
      dma_transfer_handle_t h_large;

      printf("Tile 0 started\n");
      notification1 = malloc(4);     
      notification2 = malloc(4);      
      notification3 = malloc(4);
      
      // Allocate memory for buffers
      buffer_small = malloc(36);
      buffer_large = malloc(4096);

      // Allocate slot in DMA controller
      dma_alloc(&h_small);
      dma_alloc(&h_large);

      /*printf("Start test transfers\n");
      
      // Setup a transfer of a few words from 0x100
      dma_transfer((void*)buffer_small,2,0x100,9,REMOTE2LOCAL,h_small);
      // Setup a transfer of whole page from 0x2000 from main memory
      dma_transfer((void*)buffer_large,2,0x2000,1024,REMOTE2LOCAL,h_large);

      printf("Waiting transfers\n");


      // Wait until both complete
      dma_wait(h_small);
      dma_wait(h_large);

      printf("Finished test transfers\n");
      
      // Check whether both are same
      for (int i=0;i<9;i++) {
	  if (buffer_small[i] != *((unsigned int*) 0x100 + i)) {
	      printf("ERROR: mismatch in small transfer, index %d In ROM is %lx Buffer_small is %lx \n",i,*((unsigned int*) 0x100 + i),buffer_small[i]);
	      return 1;
	  }
      }
      printf("Small transfer test successfull\n");
      
      for (int i=0;i<9;i++) {
	  if (buffer_large[i] != *((unsigned int*) 0x2000 + i)) {
	      printf("ERROR: mismatch in large transfer, index %d In ROM is %lx Buffer_large is %lx \n",i,*((unsigned int*) 0x2000 + i),buffer_large[i]);
	      return 1;
	  }
      }

      printf("Large transfer test successfull\n");
    
      printf("Testing finished\n");
      
      
      printf("Finished test transfer\n");
      */
      //==============================================================
      // whole system with Ethernet tile testing
      //==============================================================


      //printf("Address of length is %lx\n",&length);
      *notification1 = 0;
      *notification2 = 0;
      *notification3 = 0;
      
      setup = malloc(12);
      printf("Setup begins\n");
      //printf("Notification1 is %lx \n", notification1);
      setup_generation(setup,1,0,notification1,2,0x2000,0);
      //printf("Setup is %lx\n",setup);
      printf("Setup info: %lx , %lx , %lx\n",*setup,*(setup+1),*(setup+2));
      dma_transfer((void*)setup,3,0x10000000,3,LOCAL2REMOTE,h_small);
      printf("Transfer waiting\n");
      dma_wait(h_small);
      printf("First setup finished\n");

      setup_generation(setup,0,0,notification2,2,0x3000,1);
      printf("Setup info: %lx , %lx , %lx\n",*setup,*(setup+1),*(setup+2));
      dma_transfer((void*)setup,3,0x10000000,3,LOCAL2REMOTE,h_small);
      printf("Transfer waiting\n");
      dma_wait(h_small);
      printf("Second setup finished\n");

      setup_generation(setup,1,0,notification3,2,0x4000,2);
      printf("Setup info: %lx , %lx , %lx\n",*setup,*(setup+1),*(setup+2));
      dma_transfer((void*)setup,3,0x10000000,3,LOCAL2REMOTE,h_small);
      printf("Transfer waiting\n");
      dma_wait(h_small); 
      printf("Third setup finished\n"); 
      while (1){
	  printf("notification1 is %lx \n",*notification1);
	  //if (*notification1 != 0 || *notification2 != 0 || *notification3 != 0){
	      if (*notification1 != 0) {
		  queue = (*notification1) >> 4;
		  *notification1 = 0;
		  while (queue != 0) {
		      for (i = 0;i<8;i++){
			if ((1 << i) & queue) {
			  m = i;
			  dma_transfer((void*)&length,2,(0x2000 + m*1546),1,REMOTE2LOCAL,h_small);
			  dma_wait(h_small); 		  
			  printf("Length is %d bytes and %d words\n",length,byte2word(length));
			  dma_transfer((void*)buffer_large,2,(0x2000 + m*1546 + 4),byte2word(length),REMOTE2LOCAL,h_small);
			  printf("Frame transfer waiting\n");
			  dma_wait(h_small); 	
			  printf("Frame transfer finishes\n");
			  for (j=0;j<byte2word(length);j++){
			    printf("Frame %d th word is %lx\n",j,buffer_large[j]);
			  }
			  //drop
			  queue = ((1 << m) ^ queue);
			  drop = 2 + ((1 << m)<<4);
			  dma_transfer((void*)&drop,3,0x10000000,1,LOCAL2REMOTE,h_small);
			  dma_wait(h_small); 
			  printf("Drop done\n");
			}
		      }  
		  }	  
	      }
	      else {
		dma_transfer((void*)&length,2,(0x2000 + 0*1546),1,REMOTE2LOCAL,h_small);
		dma_wait(h_small); 		  
		printf("Length is %d bytes and %d words\n",length,byte2word(length));
	      }
	  }       
      //}
    }
    else {
      printf("Not Tile 0\n");
      while(1) {
	__asm__ volatile("l.nop");
      }
    }
}

void setup_generation (unsigned int * setup,unsigned int priority,unsigned int ct_index, unsigned int ct_addr, unsigned int mt_index, unsigned int mt_addr, unsigned int hash){
  *setup = 0x1 + (priority << 4) + (ct_index << 7) + (ct_addr << 13);
  *(setup + 1) = (ct_addr >> 19) + (mt_index << 13) + (mt_addr << 19);
  *(setup + 2) = (mt_addr >> 13) + (hash << 19);
}

unsigned int byte2word (unsigned int x){
  unsigned int y;
  if (x << 30 == 0)
    y = x >> 2;
  else
    y = x >> 2 + 1;
  return y;
}
