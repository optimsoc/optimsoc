@defgroup backend_jtag JTAG
@ingroup backend


The JTAG backend (internal name: `jtag`) uses (unsurprisingly) the [JTAG
protocol](http://en.wikipedia.org/wiki/Joint_Test_Action_Group) to exchange
data with a target device. To support a wide variety of adapters, the JTAG
connection and device communication is handled by the
[OpenOCD](http://openocd.org/) tool. The transmission protocol is tuned for high
throughput.

Supported Features
==================
- Number of channels: 1 (fixed)
- FIFO width: configurable, 16 bit by default


Components
==========

To make use of this backend, a number of components are required, as shown in the
graphic below.

               Host PC                                                   Target FPGA
    -----------       -----------       ----------------      -----------------------------------
    |         |       |         |       |              |      | --------------   -------------- |
    | libglip |--TCP--| OpenOCD |--???--| JTAG Adapter |-JTAG-| | GLIP logic | - | Your Logic | |
    |         |       |         |       |              |      | --------------   -------------- |
    -----------       -----------       ----------------      -----------------------------------

On the host PC, the `jtag` backend of libglip uses OpenOCD to handle the JTAG
communication. OpenOCD is a standalone application, which can be controlled by
sending TCL commands over a TCP socket.

Since most PCs don't come with a JTAG interface built-in, OpenOCD contains
drivers for many JTAG adapters. Most of are being USB to JTAG adapters with a
FT2232 chip inside. This backend was developed using the
[Olimex ARM-USB-TINY-H](https://www.olimex.com/Products/ARM/JTAG/ARM-USB-TINY-H/)
adapter, but other adapters have been shown to work as well. In general, all
adapters supported by OpenOCD should work.

On the FPGA side, a TAP Controller and logic implementing the GLIP protocol are
required, both of which are encapsulated inside the `glip_toplevel_jtag`
Verilog module, which also provides the GLIP FIFO interface to your custom
logic.

For details and usage instructions for the individual components, see these
pages:
- @ref backend_jtag-sw "libglip Backend"
- @ref backend_jtag-logic "FPGA Logic"


Performance
===========
The performance greatly depends on the used JTAG adapter and the transfer speeds
it supports. Using a OLIMEX ARM-USB-TINY-H adapter, a common variant of FTDI
FT2232-based JTAG adapters and running the JTAG bus at 10 MHz, a data rate of
about 620 KByte/s bidirectional is achieved. (The theoretical maximum without
any overhead would be 1.19 MByte/s at 10 MHz.)

The achievable transfer speed is also influenced by OpenOCD internals and the
individual backend implementations inside OpenOCD. If you find the performance
not satisfactory, try switching to another OpenOCD backend and/or JTAG adapter.


License Notes
=============
This backend is (like all of GLIP) MIT licensed.

OpenOCD is (at the time of
this writing) provided under GPLv2+. GLIP uses OpenOCD unmodified in a
separate process and communicates with it through a TCP socket. Therefore,
GLIP with the JTAG backend can still be used under the terms of the MIT license.
For OpenOCD, the GPL applies as usual, of course.

The HDL logic of the backend uses the "Mohor TAP" implementation, which is
distributed under LGPL. Changes to this TAP need to be made available under the
LGPL as well. All other HDL code is MIT licensed.
