#ifndef OPTIMSOC_BAREMETAL_H_
#define OPTIMSOC_BAREMETAL_H_

#include <stdint.h>
#include <stdlib.h>

/**
 * \defgroup libbaremetal Baremetal library
 */

/**
 * \defgroup sync Support for atomic operations and synchronization
 * \ingroup libbaremetal
 * @{
 */

/**
 * Compare-and-swap
 *
 * Read the address and compare the value. If it matches, write the new value
 * and leave unchanged otherwise. Returns the read value, that helps determining
 * whether the CAS-operation was successfull.
 *
 * \param address Address to read
 * \param compare Value to compare
 * \param value Value to write if read value matches compare
 * \return The value read
 */

extern uint32_t cas(void* address, uint32_t compare, uint32_t value);

/**
 * This is a convenience function that disables interrupts
 *
 * The external interrupts and timer interrupts are disabled.
 * optimsoc_critical_end can be used to enable those interrupts again. When
 * calling the latter function the status of the external interrupts and the
 * tick timer before entering this function needs to be restored. Therefore
 * this function returns this value.
 *
 * \code
 * uint32_t restore = optimsoc_critical_begin();
 * // Critical section code
 * optimsoc_critical_end(restore);
 * \endcode
 *
 * \note This only disables the interrupts but does neither manipulate the
 * interrupt mask vector (what you would not expect) and also does not actually
 * stop the timer (what you might expect)
 *
 * \return The current status of enabled interrupts and tick timer
 */
extern uint32_t optimsoc_critical_begin(void);

/**
 * Leave critical section
 *
 * End a critical section and restore interrupt enables. Read the documentation
 * of optimsoc_critical_begin for more details.
 *
 * \param restore Restore vector of interrupt and tick timer enable
 */
extern void optimsoc_critical_end(uint32_t restore);

/**
 * The mutex data type
 *
 * The mutex data type which is actually hidden on purpose.
 */
typedef uint64_t mutex_t;

/**
 * Initialize mutex
 *
 * Initializes a mutex so that you can lock and unlock it later.
 *
 * \param mutex Mutex to initialize
 */
extern void mutex_init(mutex_t *mutex);

/**
 * Lock mutex
 *
 * Lock a mutex
 *
 * \param mutex Mutex to lock
 */
extern void mutex_lock(mutex_t *mutex);

/**
 * Unlock mutex
 *
 * Unlock a mutex
 *
 * \param mutex Mutex to unlock
 */
extern void mutex_unlock(mutex_t *mutex);

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
    DMA_ERR_NOSLOT = 2          /*!< No slot available */
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
 * \param[out] The handle of this slot
 * \return Success code
 */
extern dma_success_t dma_alloc(dma_transfer_handle_t *id);

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
 * \defgroup mp Message passing support
 * \ingroup libbaremetal
 * @{
 */

/**
 * Initialize simple message passing environment
 */
extern void optimsoc_mp_simple_init(void);

/**
 * Send a message
 *
 * Sends a message of size from buf.
 *
 * \param size Size of the message in (word-sized) flits
 * \param buf Message buffer containint size flits
 */
extern void optimsoc_mp_simple_send(unsigned int size, uint32_t* buf);

/**
 * Add a handler for a class of incoming messages
 *
 * A message header contains a class. This class field can be used to mix
 * different kinds of message services. For each class you are using in your
 * system you need to add a class handler. As there is no default class handler
 * all remaining classes are dropped.
 *
 * \param class Class to register
 * \param hnd Function pointer to handler for this class
 */
extern void optimsoc_mp_simple_addhandler(unsigned int class,
                                               void (*hnd)(unsigned int*,int));

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

#endif
