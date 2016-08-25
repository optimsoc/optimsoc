#include <stdio.h> // For printf

#include <optimsoc-mp.h>
#include <or1k-support.h>
#include <optimsoc-baremetal.h>

#include <assert.h>

// 640x480 pixels (VGA) = 307200 pixels, with RGB565 = 2 byte/pixel = 0.5 words/pixel
#define FRAME_SIZE_WORDS 153600

uint32_t data[FRAME_SIZE_WORDS];

// This function is called by the driver on receival of a message
void recv(unsigned int *buffer,int len) {
    int source_tile, source_rank;

    //assert(len==1); // This is always one flit

    // Extract tile id from flit
    source_tile = extract_bits(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

    // Calculate rank from tile
    source_rank = optimsoc_get_tilerank(source_tile);

    int i = 0;
    for (i = 0; i < len; i++) {
        printf("received flit #%d: %.8x \n", i, buffer[i]);
    }
}

// The main function
void main() {
    if (or1k_coreid() != 0) {
        return;
    }

    // Initialize optimsoc library
    optimsoc_init(0);
    optimsoc_mp_simple_init();

    // Add handler for received messages (of class 0)
    optimsoc_mp_simple_addhandler(0,&recv);
    or1k_interrupts_enable();

    // Determine tiles rank
    int rank = optimsoc_get_ctrank();


    if (rank == 1) {


        /*// The message is a one flit packet
        uint32_t buffer[2];
        buffer[0] = 0x00000000;
        buffer[1] = 0;
        buffer[1] |= 1 << 0; // update bit
        buffer[1] |= 1 << 1; // IMAGE_MODE = RGB
        buffer[1] |= 0 << 3; // RESOLUTION = QCIF
        buffer[1] |= 1 << 6; // TEST_PATTERN = ON
        buffer[1] |= 1 << 7; // CLKRC = 1
        buffer[1] |= 0 << 13; // clock doublers OFF

        printf("sending configuration: %x\n", buffer[1]);

        // Set destination (tile 3)
        set_bits(&buffer[0],3,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);

        // Set class (0)
        set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);

        // Set sender as my rank
        set_bits(&buffer[0],optimsoc_get_ranktile(rank),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

        // Set write bit
        set_bits(&buffer[0],1,18,18);

        // Send the message 1
        optimsoc_mp_simple_send(2,(uint32_t*) buffer);

        // read access
        set_bits(&buffer[0],0,18,18);

        // Send the message 2
        optimsoc_mp_simple_send(1,(uint32_t*) buffer);


        // SLEEP 50 Million instructions
        printf("sleep, pt 1\n");
        for (unsigned int i = 0; i < 5000000; i++) {
           __asm__ volatile("l.nop");
        }
        printf("sleep done, continue with DMA\n");*/




        printf("DMA transfer\n");
        printf("Initialize data\n");

        for(unsigned i=0; i<FRAME_SIZE_WORDS; i++) {
            data[i] = i | i<<24 | i<<16 | i<<8;
        }

        dma_init();

        dma_transfer_handle_t dma;

        dma_alloc(&dma);

        printf("DMA remote to local\n");

        dma_transfer(&data, 3, &data, FRAME_SIZE_WORDS, REMOTE2LOCAL, dma);

        printf("waiting for response\n");

        dma_wait(dma);
        printf("done waiting\n");

        //dma_transfer(&data, 3, &data, 5, REMOTE2LOCAL, dma);

        //dma_wait(dma);

        // output pixel data to host
        for (unsigned i = 0; i < FRAME_SIZE_WORDS; i++) {
            OPTIMSOC_TRACE(0x200, data[i]);
            // we need to slow down a bit to prevent debug system overflows
            for (int s = 0; s < 3; s++) {
                __asm__ volatile("l.nop");
            }
        }

    }
}
