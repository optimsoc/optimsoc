#include "utils.h"
#include "board.h"
#include "uart.h"

const int UART_BASE_ADR[1] = {UART0_BASE};
const int UART_BAUDS[1] = {UART0_BAUD_RATE};

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

#define WAIT_FOR_XMITR(core)			\
        do { \
                lsr = REG8(UART_BASE_ADR[core] + UART_LSR); \
        } while ((lsr & BOTH_EMPTY) != BOTH_EMPTY)

#define WAIT_FOR_THRE(core)			\
        do { \
                lsr = REG8(UART_BASE_ADR[core] + UART_LSR); \
        } while ((lsr & UART_LSR_THRE) != UART_LSR_THRE)

#define CHECK_FOR_CHAR(core) (REG8(UART_BASE_ADR[core] + UART_LSR) & UART_LSR_DR)

#define WAIT_FOR_CHAR(core)			\
         do { \
                lsr = REG8(UART_BASE_ADR[core] + UART_LSR); \
         } while ((lsr & UART_LSR_DR) != UART_LSR_DR)

#define UART_TX_BUFF_LEN 32
#define UART_TX_BUFF_MASK (UART_TX_BUFF_LEN -1)

char tx_buff[UART_TX_BUFF_LEN];
volatile int tx_level, rx_level;

void uart_init(int core)
{
        int divisor;
	float float_divisor;
        /* Reset receiver and transmiter */
        REG8(UART_BASE_ADR[core] + UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;

        /* Disable all interrupts */
        REG8(UART_BASE_ADR[core] + UART_IER) = 0x00;

        /* Set 8 bit char, 1 stop bit, no parity */
        REG8(UART_BASE_ADR[core] + UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

        /* Set baud rate */
	float_divisor = (float) IN_CLK/(16 * UART_BAUDS[core]);
	float_divisor += 0.50f; // Ensure round up
        divisor = (int) float_divisor;

        REG8(UART_BASE_ADR[core] + UART_LCR) |= UART_LCR_DLAB;
        REG8(UART_BASE_ADR[core] + UART_DLL) = divisor & 0x000000ff;
        REG8(UART_BASE_ADR[core] + UART_DLM) = (divisor >> 8) & 0x000000ff;
        REG8(UART_BASE_ADR[core] + UART_LCR) &= ~(UART_LCR_DLAB);

        return;
}

void uart_putc(int core, char c)
{
        unsigned char lsr;

        WAIT_FOR_THRE(core);
        REG8(UART_BASE_ADR[core] + UART_TX) = c;
        if(c == '\n') {
          WAIT_FOR_THRE(core);
          REG8(UART_BASE_ADR[core] + UART_TX) = '\r';
        }
        WAIT_FOR_XMITR(core);
}

// Only used when we know THRE is empty, typically in interrupt
void uart_putc_noblock(int core, char c)
{
  REG8(UART_BASE_ADR[core] + UART_TX) = c;
}


char uart_getc(int core)
{
        unsigned char lsr;
        char c;

        WAIT_FOR_CHAR(core);
        c = REG8(UART_BASE_ADR[core] + UART_RX);
        return c;
}

int uart_check_for_char(int core)
{
  return CHECK_FOR_CHAR(core);
}

void uart_rxint_enable(int core)
{
  REG8(UART_BASE_ADR[core] + UART_IER) |= UART_IER_RDI;
}

void uart_rxint_disable(int core)
{
  REG8(UART_BASE_ADR[core] + UART_IER) &= ~(UART_IER_RDI);
}

void uart_txint_enable(int core)
{
  REG8(UART_BASE_ADR[core] + UART_IER) |= UART_IER_THRI;
}

void uart_txint_disable(int core)
{
  REG8(UART_BASE_ADR[core] + UART_IER) &= ~(UART_IER_THRI);
}

char uart_get_iir(int core)
{
  return REG8(UART_BASE_ADR[core] + UART_IIR);
}


char uart_get_lsr(int core)
{
  return REG8(UART_BASE_ADR[core] + UART_LSR);
}


char uart_get_msr(int core)
{
  return REG8(UART_BASE_ADR[core] + UART_MSR);
}
