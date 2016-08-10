@defgroup backend_uart-logic UART FPGA Logic
@ingroup backend_uart

The GLIP logic provides the interface between the UART controller and
the user logic. This is essentially serialization and de-serialization
of the UART data stream.

The Verilog toplevel module `glip_uart_toplevel` implements the
[common GLIP logic interface](@ref logicif).

| Port Name | Width | Direction | Description                        |
|-----------|:-----:|:---------:|------------------------------------|
| uart_rx   | 1     | IN        | RX signal                          |
| uart_tx   | 1     | OUT       | TX signal                          |
| uart_cts  | 1     | IN        | clear to send, flow control for TX |
| uart_rts  | 1     | OUT       | ready to send, flow control for RX |

The following parameters need to be set:

| Name | Description     |
|------|-----------------|
| FREQ | Clock frequency |
| BAUD | Baud rate       |

The backend currently does not support logic reset as there is no
out-of-band signaling. Doing it in-band is a peding feature.


Ressource Utilization
---------------------

The following resource utilization of the hardware logic was measured
on a Xilinx 7 Series device:

| LUTs | Registers | BRAMs |
| 323  | 154       | 2     |
