/* Copyright (c) 2013 by the author(s)
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
 * =================================================================
 *
 * Simple setup for ethernet tile.
 *
 * Author(s):
 *   Holm Rauchfuss <holm.rauchfuss@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <stdio.h>
#include <optimsoc.h>
#include <malloc.h>
#include <dma.h>

void main() {
  optimsoc_init(0);
  if (optimsoc_get_tileid() == 0) {
    printf("TileID %lx \n", optimsoc_get_tileid());
    printf("Tile 0\n");
    dma_init();

    typedef struct {
      unsigned long rx_buf_addr : 32;
      unsigned long tx_buf_addr : 32;
      unsigned long tx_len_addr : 32;
      unsigned long rx_ntf_addr : 32;
      unsigned long tx_ntf_addr : 32;

      char rx_buf_tid : 8;
      char tx_buf_len_tid : 8;
      char rx_ntf_tid : 8;
      char tx_ntf_tid : 8;

      char prio    : 8;
      char rx_flag : 8;
      char tx_flag : 8;
      char vif_id  : 8;
    } setup_n;
  
    unsigned int *TxFrame1, *TxFrame2, *TxFrame3, *TxFrame4, *TxFrame5, *TxFrame6, *TxFrame7, *TxFrame8;
    unsigned int *TxLength;
    unsigned int *Tx_BV;
    unsigned int *notification;

    dma_transfer_handle_t h_small;
    dma_alloc(&h_small);

    TxFrame1 = malloc(20); // Tx Frame for slot 1
    TxFrame2 = malloc(20); // Tx Frame for slot 2
    TxFrame3 = malloc(20); // Tx Frame for slot 3
    TxFrame4 = malloc(20); // Tx Frame for slot 4
    TxFrame5 = malloc(20); // Tx Frame for slot 5
    TxFrame6 = malloc(20); // Tx Frame for slot 6
    TxFrame7 = malloc(20); // Tx Frame for slot 7
    TxFrame8 = malloc(20); // Tx Frame for slot 8

    TxLength = malloc(20); // Lengths of Tx Frames

    Tx_BV = malloc(4); // Tx Bitvector
    
    notification = malloc(4);      
    *notification = 0;
  
    setup_n *s = (setup_n *) malloc(28);

    // sending first setup packet to ET
    
    s -> rx_buf_addr = 0x0000;
    s -> tx_buf_addr = 0x2100;		// Tx MT address
    s -> tx_len_addr = 0x2300;
    s -> rx_ntf_addr = 0x0000;
    s -> tx_ntf_addr = 0x2400;

    s -> rx_buf_tid = 0x00;
    s -> tx_buf_len_tid = 0x02;
    s -> rx_ntf_tid = 0x00;
    s -> tx_ntf_tid = 0x02;

    s -> prio = 0x01;
    s -> rx_flag = 0x00;
    s -> tx_flag = 0x01;
    s -> vif_id = 0x03;
	
    printf("Sync local notification with notification in memory\n");
    dma_transfer((void*)notification,2,(void*)0x2400,1,LOCAL2REMOTE,h_small);		
    dma_wait(h_small);

    printf("Sending Setup to Ethernet Tile\n");
    dma_transfer((void*)s,3,(void*)0x000,7,LOCAL2REMOTE,h_small);		
    dma_wait(h_small);

    // sending second setup packet to ET
    
    s -> rx_buf_addr = 0x0000;
    s -> tx_buf_addr = 0x2500;		// Tx MT address
    s -> tx_len_addr = 0x2700;
    s -> rx_ntf_addr = 0x0000;
    s -> tx_ntf_addr = 0x2800;

    s -> rx_buf_tid = 0x00;
    s -> tx_buf_len_tid = 0x02;
    s -> rx_ntf_tid = 0x00;
    s -> tx_ntf_tid = 0x02;

    s -> prio = 0x00;
    s -> rx_flag = 0x00;
    s -> tx_flag = 0x01;
    s -> vif_id = 0x02;
	
  /*  printf("Sync local notification with notification in memory\n");
    dma_transfer((void*)notification,2,(void*)0x2400,1,LOCAL2REMOTE,h_small);		
    dma_wait(h_small);
*/
    printf("Sending Setup to Ethernet Tile\n");
    dma_transfer((void*)s,3,(void*)0x000,7,LOCAL2REMOTE,h_small);		
    dma_wait(h_small);
    

    // sending Tx Frame 1 to mem tile

    *TxFrame1 = 0xa1a1a1a1;
    //*(TxFrame1+1) = 0xb1b1b1b1;
    //*(TxFrame1+2) = 0xc1c1c1c1;
    //*(TxFrame1+3) = 0xd1d1d1d1;
    //*(TxFrame1+4) = 0xe1e1e1e1;

    printf("Sending Tx Frame 1 to mem tile\n");				
    dma_transfer((void*)TxFrame1,2,(void*)0x2100,1,LOCAL2REMOTE,h_small);
    dma_wait(h_small);
	
    // sending Tx Frame 2 to mem tile        

    *TxFrame2 = 0xa2a2a2a2;
    *(TxFrame2+1) = 0xb2b2b2b2;
    //*(TxFrame2+2) = 0xc2c2c2c2;
    //*(TxFrame2+3) = 0xd2d2d2d2;
    //*(TxFrame2+4) = 0xe2e2e2e2;

    printf("Sending Tx Frame 2 to mem tile\n");				
    dma_transfer((void*)TxFrame2,2,(void*)0x26f8,2,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Frame 3 to mem tile        

    *TxFrame3 = 0xa3a3a3a3;
    *(TxFrame3+1) = 0xb3b3b3b3;
    *(TxFrame3+2) = 0xc3c3c3c3;
    //*(TxFrame3+3) = 0xd3d3d3d3;
    //*(TxFrame3+4) = 0xe3e3e3e3;

    printf("Sending Tx Frame 3 to mem tile\n");				
    dma_transfer((void*)TxFrame3,2,(void*)0x2cf0,3,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Frame 4 to mem tile        

    *TxFrame4 = 0xa4a4a4a4;
    *(TxFrame4+1) = 0xb4b4b4b4;
    *(TxFrame4+2) = 0xc4c4c4c4;
    *(TxFrame4+3) = 0xd4d4d4d4;
    //*(TxFrame4+4) = 0xe4e4e4e4;

    printf("Sending Tx Frame 4 to mem tile\n");				
    dma_transfer((void*)TxFrame4,2,(void*)0x32e8,4,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Frame 5 to mem tile        

    *TxFrame5 = 0xa5a5a5a5;
    *(TxFrame5+1) = 0xb5b5b5b5;
    *(TxFrame5+2) = 0xc5c5c5c5;
    *(TxFrame5+3) = 0xd5d5d5d5;
    *(TxFrame5+4) = 0xe5e5e5e5;

    printf("Sending Tx Frame 5 to mem tile\n");				
    dma_transfer((void*)TxFrame5,2,(void*)0x38e0,5,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Frame 6 to mem tile        

    *TxFrame6 = 0xa6a6a6a6;
    //*(TxFrame6+1) = 0xb6b6b6b6;
    //*(TxFrame6+2) = 0xc6c6c6c6;
    //*(TxFrame6+3) = 0xd6d6d6d6;
    //*(TxFrame6+4) = 0xe6e6e6e6;

    printf("Sending Tx Frame 6 to mem tile\n");				
    dma_transfer((void*)TxFrame6,2,(void*)0x3ed8,1,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Frame 7 to mem tile        

    *TxFrame7 = 0xa7a7a7a7;
    *(TxFrame7+1) = 0xb7b7b7b7;
    //*(TxFrame7+2) = 0xc7c7c7c7;
    //*(TxFrame7+3) = 0xd7d7d7d7;
    //*(TxFrame7+4) = 0xe7e7e7e7;

    printf("Sending Tx Frame 7 to mem tile\n");				
    dma_transfer((void*)TxFrame7,2,(void*)0x44d0,2,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);


    // sending Tx Frame 8 to mem tile        

    *TxFrame8 = 0xa8a8a8a8;
    *(TxFrame8+1) = 0xb8b8b8b8;
    *(TxFrame8+2) = 0xc8c8c8c8;
    //*(TxFrame8+3) = 0xd8d8d8d8;
    //*(TxFrame8+4) = 0xe8e8e8e8;

    printf("Sending Tx Frame 8 to mem tile\n");				
    dma_transfer((void*)TxFrame8,2,(void*)0x4ac8,3,LOCAL2REMOTE,h_small);	
    dma_wait(h_small);

    // sending Tx Length info to mem tile 

    *TxLength = 0x00040008; // Tx Frame 1 => 4 byte, Tx Frame 2 => 8 byte
    *(TxLength+1) = 0x000c0010; // ...
    *(TxLength+2) = 0x00140004; // ...
    *(TxLength+3) = 0x0008000c; // ...
    *(TxLength+4) = 0x00000003; // vif_id == 3

    printf("Sending lengths to memory\n");
    dma_transfer((void*)TxLength,2,(void*)0x2300,4,LOCAL2REMOTE,h_small);
    dma_wait(h_small);
 
    // sending Tx bit vector info to ethernet tile

    *Tx_BV = 0xff03; // ff = 8 slots filled, vif_id == 3
    printf("Sending bitvector to ethernet tile\n");
    dma_transfer((void*)Tx_BV,3,(void*)0x200,1,LOCAL2REMOTE,h_small);
    dma_wait(h_small);
        
    while (*notification == 0) { 
      dma_transfer((void*)notification,2,(void*)0x2400,1,REMOTE2LOCAL,h_small);
      dma_wait(h_small);
    }
    printf("\n Data recieved from ethernet L2R transfer(TX Bit Vector): %lx\n", *notification); 
    

    __asm__ volatile("l.nop");
  }
  else {
    printf("Not Tile 0\n");
    while(1) {
      __asm__ volatile("l.nop");
    }
  } 
}
