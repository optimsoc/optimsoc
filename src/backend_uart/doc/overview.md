@defgroup backend_uart UART
@ingroup backend

The UART backends (internal name: `uart`) connects to the target via a
UART (aka RS-232 or "serial") device. Such UART devices are usually 
integrated on a board or are otherwise are available as USB adapters. 

GLIP uses a very efficient coding to achieve near-maximum bandwidth.
Still, UART is a rather slow protocol, which is often limited by the 
cabelling and the intermediate adapter (such as USB to UART chips)
to not more than a couple MBaud.

Supported Features
==================
- Number of channels: 1 (fixed)
- FIFO width: 8 or 16 bit
- Logic reset
- Robust end-to-end flow control, even without CTS/RTS

Components
==========

For details and usage instructions for the individual components, see these
pages:
- @ref backend_uart-sw "libglip Backend"
- @ref backend_uart-logic "FPGA Logic"
- @ref backend_uart-protocol "Details of the end-to-end flow control"

Performance
==========

As UART is a relatively slow interface, the backend usually reaches
the maximum throughput. That is 10 symbols (8 bit payload plus start
and stop bit), meaning BAUD/10 symbols Byte/s. For example, With
115,200 Baud you get 11,520 Byte/s and with 3,000,000 Baud you get
300,000 Byte/s. There is a small loss of this theoretical throughput
as we need some flow control messages and mask the byte `0xfe` in the
data stream. With uniformly distributed random data you get
approximately 298,500 Byte/s at 3 MBaud. Hence you generally get
nearly full throughput, with the exception that it drops to roughly
50% if you only send the byte `0xfe`.

