/* Copyright (c) 2012-2017 by the author(s)
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
 */

#ifndef OPTIMSOC_BAREMETAL_H_
#define OPTIMSOC_BAREMETAL_H_

// Internal defines
#define OPTIMSOC_NA_BASE          0xe0000000

// Configuration module
#define OPTIMSOC_NA_REGS       OPTIMSOC_NA_BASE + 0x00000
#define OPTIMSOC_NA_TILEID     OPTIMSOC_NA_REGS + 0x0
#define OPTIMSOC_NA_NUMTILES   OPTIMSOC_NA_REGS + 0x4
#define OPTIMSOC_NA_COREBASE   OPTIMSOC_NA_REGS + 0x10
#define OPTIMSOC_NA_TOTALCORES OPTIMSOC_NA_REGS + 0x18
#define OPTIMSOC_NA_GMEM_SIZE  OPTIMSOC_NA_REGS + 0x1c
#define OPTIMSOC_NA_GMEM_TILE  OPTIMSOC_NA_REGS + 0x20
#define OPTIMSOC_NA_LMEM_SIZE  OPTIMSOC_NA_REGS + 0x24
#define OPTIMSOC_NA_CT_NUM     OPTIMSOC_NA_REGS + 0x28
#define OPTIMSOC_NA_SEED       OPTIMSOC_NA_REGS + 0x2c
#define OPTIMSOC_NA_CT_LIST    (OPTIMSOC_NA_REGS + 0x200)

#define OPTIMSOC_NA_CONF          OPTIMSOC_NA_REGS + 0xc
#define OPTIMSOC_NA_CONF_MPSIMPLE 0x1
#define OPTIMSOC_NA_CONF_DMA      0x2

#define OPTIMSOC_DEST_MSB 31
#define OPTIMSOC_DEST_LSB 27
#define OPTIMSOC_CLASS_MSB 26
#define OPTIMSOC_CLASS_LSB 24
#define OPTIMSOC_CLASS_NUM 8
#define OPTIMSOC_SRC_MSB 23
#define OPTIMSOC_SRC_LSB 19

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <or1k-sprs.h>
#include <or1k-support.h>

/**
 * \defgroup libbaremetal Baremetal library
 *
 * This is the library to build software that runs directly on the hardware.
 * It contains the basic drivers for the hardware, while relying on the
 * standard OpenRISC libc (newlib).
 */

/**
 * \defgroup tracing OpTiMSoC tracing
 *
 * @{
 */

/**
 * The instruction sequence issued as a trace event
 *
 * \param id The identifier of the trace (static value)
 * \param v  The value of the trace event
 */
#define OPTIMSOC_TRACE(id,v)                     \
        asm("l.addi\tr3,%0,0": :"r" (v) : "r3"); \
        asm("l.nop %0": :"K" (id));

/**
 * Define a section for the software tracing
 *
 * This initializes a section with a given id and sets a name, that can be
 * used for display or similar.
 *
 * \param id Section identifier
 * \param name Name of this section
 */
extern void optimsoc_trace_definesection(int id, char* name);

extern void optimsoc_trace_defineglobalsection(int id, char* name);

/**
 * Trace a section enter
 *
 * Enter a section and trace this to the debug system.
 *
 * \param id Section entered
 */
extern void optimsoc_trace_section(int id);

/**
 * Trace a kernel enter
 *
 * On entering the kernel trace this to the debug system.
 */
extern void optimsoc_trace_kernelsection(void);

/**
 * @}
 */

/**
 * \defgroup system OpTiMSoC system functions
 *
 * @{
 */

// TODO: what do we put inside here?
typedef struct optimsoc_conf {

} optimsoc_conf;

extern void optimsoc_init(optimsoc_conf *config);

/**
 * Get the tile identifier
 *
 * \return Identifier of this tile, which is the rank inside all tiles
 */
static inline uint32_t optimsoc_get_tileid(void) {
    return REG32(OPTIMSOC_NA_TILEID);
}

/**
 * Get the total number of tiles
 *
 * \return Number of tiles in this system
 */
static inline uint32_t optimsoc_get_numtiles(void) {
    return REG32(OPTIMSOC_NA_NUMTILES);
}

/**
 * Get the number of compute tiles
 *
 * \ingroup system
 * \return Number of compute tiles
 */
extern uint32_t optimsoc_get_numct(void);

/**
 * Generate rank of this compute tile in all compute tiles
 *
 * \ingroup system
 * This gives the rank in the set of compute tiles.
 * For example in a system where a compute tile is at position 0 and
 * one at position 3, they will get this output
 *  tile 0 -> ctrank 0
 *  tile 3 -> ctrank 1
 *
 * \return rank of this tile
 */
extern int optimsoc_get_ctrank(void);

/**
 * Generate rank of given compute tile in all compute tiles
 *
 * \ingroup system
 * This gives the rank in the set of compute tiles.
 * For example in a system where a compute tile is at position 0 and
 * one at position 3, they will get this output
 *  tile 0 -> ctrank 0
 *  tile 3 -> ctrank 1
 *
 * \param tile Tile to look up
 * \return rank of this tile
 */
extern int optimsoc_get_tilerank(unsigned int tile);

/**
 * Get the tile that has the given rank
 *
 * This is the reverse of optimsoc_tilerank and generates the tile
 * identifier for the given rank.
 *
 * \param rank The rank to lookup
 */
extern int optimsoc_get_ranktile(unsigned int rank);


/**
 * Get the core id, relative in this tile
 *
 * \return relative core identifier
 */
static inline unsigned int optimsoc_get_relcoreid(void) {
    return or1k_mfspr(OR1K_SPR_SYS_COREID_ADDR);
}

/**
 * Get the number of cores in this tile
 *
 * \return Number of cores in this tile
 */
static inline unsigned int optimsoc_get_tilenumcores(void) {
    return or1k_mfspr(OR1K_SPR_SYS_NUMCORES_ADDR);
}

/**
 * Get the absolute core id of this core
 *
 * The absolute core id in the whole system is started from the first compute
 * tile, core 0 and counted up. This may be useful in shared memory systems.
 *
 * \return Absolute core id
 */
static inline unsigned int optimsoc_get_abscoreid(void) {
    return REG32(OPTIMSOC_NA_COREBASE) + optimsoc_get_relcoreid();
}

/**
 * Get the size of global memory (if any)
 *
 * Return the size of globally accessible memory. This memory is not directly
 * accessed, but can only be accessed with DMA transfers. If the system has no
 * global memory, return 0.
 *
 * \return Size of memory (or 0, if no memory)
 */
static inline uint32_t optimsoc_get_mainmem_size() {
    return REG32(OPTIMSOC_NA_GMEM_SIZE);
}

/**
 * Get the tile ID of the global memory
 *
 * If the system has global memory (optimsoc_get_mainmem_size() > 0), this
 * function returns the tile identifier, where the memory is installed.
 *
 * \return Main memory tile identifier
 */
static inline uint32_t optimsoc_get_mainmem_tile() {
    return REG32(OPTIMSOC_NA_GMEM_TILE);
}

/**
 * Get size of local memory
 *
 * Get the size of memory available (and addressed) in this tile.
 *
 * \return Size of local memory
 */
static inline uint32_t optimsoc_get_localmem_size() {
    return REG32(OPTIMSOC_NA_LMEM_SIZE);
}

/**
 * @}
 */

/**
 * \defgroup userio User I/O
 * \ingroup libbaremetal
 * @{
 */

extern int lcd_set(unsigned int row,unsigned int col,char c);
extern void uart_printf(const char *fmt, ...);

/**
 * @}
 */

extern uint32_t optimsoc_noc_maxpacketsize();
extern uint32_t optimsoc_has_hostlink();
extern uint32_t optimsoc_hostlink();
extern uint32_t optimsoc_has_uart();
extern uint32_t optimsoc_uarttile();
extern uint32_t optimsoc_uart_lcd_enable();

/**
 * @}
 */
// TODO: Remove
extern void uart_printf(const char *fmt, ...);

/**
 * \defgroup utility Utility functions
 *
 * Utility functions
 *
 * @{
 */

/**
 * extract bits between MSB and LSB (including)
 *
 *  \code
 *  return x[MSB:LSB];
 *  \endcode
 *
 * \param x Value to extract from
 * \param msb MSB of extraction
 * \param lsb LSB of extraction
 * \return extracted value (aligned to lsb=0)
 *
 */
static inline unsigned int extract_bits(uint32_t x, uint32_t msb,
                                        uint32_t lsb) {
    return ((x>>lsb) & ~(~0 << (msb-lsb+1)));
}

/**
 * Set bits in variable between MSB and LSB (including)

 *  \code
 *  x[MSB:LSB] = v;
 *  \endcode
 *
 * \param x Pointer to value where to set
 * \param v Value to set
 * \param msb MSB of part to set
 * \param lsb LSB of part to set
 */
static inline void set_bits(uint32_t *x, uint32_t v, uint32_t msb,
                            uint32_t lsb) {
    if(msb != 31) {
        *x = (((~0 << (msb+1) | ~(~0 << lsb))&(*x)) | ((v & ~(~0<<(msb-lsb+1))) << lsb));
    } else {
        /* only the last 5 bits from the shift left operand are used -> can not shift 32 bits */
        *x = (((~(~0 << lsb))&(*x)) | ((v & ~(~0<<(msb-lsb+1))) << lsb));
    }
}

/**
 * Get a random seed for randomization
 *
 * This function is used to obtain a random seed from the simulation environment
 * for randomization tests. It has no function when executing in hardware.
 *
 * It is only random when the actual simulation is seeded with a non-static
 * value.
 *
 * \return (random) seed
 */
uint32_t optimsoc_get_seed(void);

/**
 * @}
 */


/**
 * \defgroup sync Support for atomic operations and synchronization
 * \ingroup libbaremetal
 * @{
 */

/**
 * The mutex data type
 *
 * The mutex data type which is actually hidden on purpose.
 */
typedef uint32_t optimsoc_mutex_t;

/**
 * Initialize mutex
 *
 * Initializes a mutex so that you can lock and unlock it later.
 *
 * \param mutex Mutex to initialize
 */
extern void optimsoc_mutex_init(optimsoc_mutex_t *mutex);

/**
 * Lock mutex
 *
 * Lock a mutex
 *
 * \param mutex Mutex to lock
 */
extern void optimsoc_mutex_lock(optimsoc_mutex_t *mutex);

/**
 * Unlock mutex
 *
 * Unlock a mutex
 *
 * \param mutex Mutex to unlock
 */
extern void optimsoc_mutex_unlock(optimsoc_mutex_t *mutex);

/**
 * @}
 */


/**
 * \defgroup dma Direct Memory Access support
 * \ingroup libbaremetal
 * @{
 */

/**
 * OpTiMSoC dma success code
 *
 * The OpTiMSoC dma success code indicated the success of an operation.
 */
typedef enum {
    DMA_SUCCESS = 0,            /*!< Successful operation */
    DMA_ERR_NOTINITIALIZED = 1, /*!< Driver not initialized */
    DMA_ERR_NOSLOT = 2,         /*!< No slot available */
    DMA_ERR_NOTALLOCATED = 3      /*!< Slot not allocated */
} dma_success_t;

/**
 * DMA transfer handle
 *
 * The DMA transfer handle is used when calling the DMA functions
 */
typedef uint32_t dma_transfer_handle_t;

/**
 * The direction of a DMA transfer
 *
 * The direction of a DMA transfer is either from local to a remote tile or
 * vice versa
 */
typedef enum {
    LOCAL2REMOTE=0, /*!< Transfer data from local to remote */
    REMOTE2LOCAL=1  /*!< Transfer data from remore to local */
} dma_direction_t;

/**
 * Initialize DMA driver
 *
 * Initialize the DMA driver. Necessary before calling it the first time
 */
extern void dma_init(void);

/**
 * Allocate a DMA transfer slot and get handle
 *
 * This function allocates a DMA slot. DMA transfers are handled asynchronously
 * and each of the ongoing transfers is controlled by one slot in the DMA
 * controller. You therefore need to allocate a slot before starting transfers.
 *
 * \param[out] id The handle of this slot
 * \return Success code
 */
extern dma_success_t dma_alloc(dma_transfer_handle_t *id);

/**
 * Free a pre-allocated DMA transfer slot
 *
 * \param id the handle to free
 * \return DMA_SUCCESS if successful, any other value indicates an error
 */
extern dma_success_t dma_free(dma_transfer_handle_t id);


/**
 * Initiate a DMA transfer
 *
 * This function initiates a DMA transfer between the local address in this tile
 * to remote_tile address remote of size. The direction of the transfer is
 * determined by dir. The slot id will be used for this transfer.
 *
 * \param local Local address
 * \param remote_tile Remote tile
 * \param remote Remote address
 * \param size Size of the transfer
 * \param dir Direction of the transfer
 * \param id Handle of the slot as allocated by dma_alloc
 * \return Success code
 */
extern dma_success_t dma_transfer(void* local,
                                  uint32_t remote_tile,
                                  void* remote,
                                  size_t size,
                                  dma_direction_t dir,
                                  dma_transfer_handle_t id);

/**
 * Blocking wait for DMA transfer
 *
 * Wait for a DMA transfer to finish and block.
 *
 * \param id The handle of the transfer slot
 * \return Success code
 */
extern dma_success_t dma_wait(dma_transfer_handle_t id);

/**
 * @}
 */

/**
 * \defgroup mpsimple Simple message passing buffers support
 *
 * The simple message passing buffer (mpbuffer) is a hardware device that
 * can be used to communicate among compute tiles. They are useful to build
 * efficient or optimized message passing applications, but in most cases
 * the message passing library is recommended as it first encapsulates the
 * actual message passing protocol (end-to-end flow control control etc.), and
 * second uses the available hardware accelerators which are more performant
 * than the message passing buffers.
 *
 * The message passing buffers are hardware FIFOs and the software directly
 * writes and reads network packets into the FIFOs. There are multiple
 * endpoints to mitigate message-dependent deadlocks. Your software can query
 * the number of available endpoints with optimsoc_mp_simple_num_endpoints().
 *
 * Software can send a packet on an endpoint using optimsoc_mp_simple_send().
 * To receive incoming packets, the software registers a handler for incoming
 * messages using optimsoc_mp_simple_addhandler(). Messages from all endpoints
 * invoke the handler, but you can register different handlers for different
 * message classes.
 *
 * \note
 * Be careful with selecting classes, because they may be occupied by other
 * hardware. If you are not sure, use class 0.
 *
 * \ingroup libbaremetal
 * @{
 */

/**
 * Initialize simple message passing buffers
 */
void optimsoc_mp_simple_init(void);

/**
 * Query the number of endpoints.
 *
 * \return Number of endpoints
 */
uint16_t optimsoc_mp_simple_num_endpoints(void);

/**
 * Enable the endpoint to receive packets
 *
 * The hardware will only be able to receive packets after it was enabled.
 * Hence you need to check the endpoint from the remote using
 * optimsoc_mp_simple_enable() before calling optimsoc_mp_simple_send().
 *
 * \param endpoint Endpoint buffer to enable
 */
void optimsoc_mp_simple_enable(uint16_t endpoint);

/**
 * Check if an endpoint at another compute tile is ready
 *
 * This function must be called before calling optimsoc_mp_simple_send(). The
 * function is non-blocking, meaning that with the first call it will always
 * return 0. It sends a message to the remote to check the state. Further calls
 * will then return immediately if a successfull response was received,
 * otherwise try again.
 *
 * The receiver must enable the endpoint using optimsoc_mp_simple_enable().
 *
 * \param rank Compute tile rank to check
 * \param endpoint Endpoint buffer to check
 * \return Endpoint status (0: not ready, 1: enabled)
 */
int optimsoc_mp_simple_ctready(uint32_t rank, uint16_t endpoint);

/**
 * Send a message
 *
 * Sends a message of size from buf.
 *
 * \param endpoint The endpoint to send the message on
 * \param size Size of the message in (word-sized) flits
 * \param buf Message buffer containint size flits
 */
void optimsoc_mp_simple_send(uint16_t endpoint, size_t size, uint32_t* buf);

/**
 * Add a handler for a class of incoming messages
 *
 * A message header contains a class. This class field can be used to mix
 * different kinds of message services. For each class you are using in your
 * system you need to add a class handler. As there is no default class handler
 * all remaining classes are dropped.
 *
 * \param cls Class to register
 * \param hnd Function pointer to handler for this class
 */
void optimsoc_mp_simple_addhandler(uint8_t cls, void (*hnd)(uint32_t*, size_t));

/**
 * @}
 */

#endif
