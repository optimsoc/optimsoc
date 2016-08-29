@defgroup backend_uart-logic UART FPGA Logic
@ingroup backend_uart

The GLIP logic provides the interface between the UART controller and
the user logic. This is essentially serialization and de-serialization
of the UART data stream.

Due to the use of Xilinx FIFO primitives, only Xilinx 7-series devices are
supported at the moment. (However, support for other devices should be possible
by exchanging the FIFOs.)

The Verilog toplevel module `glip_uart_toplevel` implements the
[common GLIP logic interface](@ref logicif).

Additionally, the following signals are available in `glip_uart_toplevel`.

| Port Name  | Width | Direction | Description                                    |
|------------|:-----:|:---------:|------------------------------------------------|
| clk_io     | 1     | IN        | I/O clock                                      |
| uart_rx    | 1     | IN        | RX signal                                      |
| uart_tx    | 1     | OUT       | TX signal                                      |
| uart_cts_n | 1     | IN        | clear to send, flow control for TX. active low |
| uart_rts_n | 1     | OUT       | ready to send, flow control for RX. active low |

The following parameters are available.

| Name        | Description                         |
|-------------|-------------------------------------|
| FREQ_CLK_IO | Frequency of the I/O clock `clk_io` |
| BAUD        | Baud rate (default: 115200)         |
| XILINX_TARGET_DEVICE | Xilinx target device. Valid options: `7SERIES` for Xilinx 7 series devices (Virtex, Kintex, Artix). Default: `7SERIES` |
| WIDTH       | Width of the FIFO (`fifo_*`) ports. Supported values: 8 and 16. Default: 8 |
| BUFFER_OUT_DEPTH | Size of the output buffer (i.e. the buffer between the FPGA and the host) in bytes. Default: 4096 bytes |


