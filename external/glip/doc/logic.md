@defgroup logicif GLIP Logic Interface

For FPGA targets GLIP provides HDL (usually Verilog) modules as an interface
between the communication interface and your logic. Those logic interfaces share
a common interface if at all possible to make switching between the different
communication methods in hardware as easy as in software. This page describes
this generic interface. For details on the individual backend implementations
refer to the backend documentation.

The toplevel module of the HDL code is commonly called `glip_BACKEND_toplevel`
and contains three four groups of signals: the GLIP FIFO interface, the GLIP
control interface, clock and reset signals and an external communication
interface specific to the backend.

@note The direction in all tables below is seen from the GLIP interface module,
i.e. the `fifo_out_data` port is an input from the GLIP side, thus marked as IN
in the tables below. In your design it's an output.

FIFO Interface
--------------

The FIFO interface is the interface between your logic and GLIP. Whatever you
write into the FIFO in your logic will come out of the FIFO at the host PC, and
the other way around. The signals are named from a target point of view, the
`fifo_out_` signals are the outgoing FIFO from the target to the host PC. The
`fifo_in_` signals are incoming data, data coming from the host PC and being
received at your logic.

Depending on the used backend the width of the FIFO interface (i.e. the number
of parallel data signals) might be configurable, or fixed. The width is always
given in full bytes, i.e. 8 bit, 16 bit, etc. In the table below `FIFO_WIDTH`
describes the width of the FIFO interface.

Some GLIP backends support more than one communication channel. Each channel is
a completely independent FIFO with its own set of data and control signals. All
signals of the same type are combined into one input/output signal, i.e.
`fifo_out_valid[0]` is the `fifo_out_valid` signal for channel 0,
`fifo_out_valid[1]` is the same signal for channel 1. In the table below `NCHAN`
describes the number of supported channels.

| Port Name      | Width            | Direction | Description                            |
|----------------|:----------------:|:---------:|--------------------------------------- |
| fifo_out_data  | NCHAN*FIFO_WIDTH | IN        | the data to write to the host PC       |
| fifo_out_valid | NCHAN            | IN        | the data on `fifo_out_data` is valid   |
| fifo_out_ready | NCHAN            | OUT       | the FIFO is ready to receive new data  |
| fifo_in_data   | NCHAN*FIFO_WIDTH | OUT       | the data received from the host PC     |
| fifo_in_valid  | NCHAN            | OUT       | the data on `fifo_in_data` is valid    |
| fifo_in_ready  | NCHAN            | IN        | the logic is ready to receive new data |

### Byte Ordering ###
On the host side, all reads and writes are done in bytes (see glip_read() and
glip_write() for example). On the logic side the FIFO might be wider than one
byte, e.g. 2 or 4 byte (`FIFO_WIDTH` being 16 or 32 [bit] respectively). This
leaves the question of how the stream of bytes is mapped into wider FIFO
registers. In GLIP, all **FIFOs are Big Endian**. For a 2 byte (16 bit) wide
FIFO on the target this means: the first byte written by glip_write() will be
the most significant byte of the FIFO (i.e. `fifo_out_data[15:8]`), and the
second byte will be the least significant byte (i.e. `fifo_out_data[7:0]`).

### FIFO Timing Behavior ###
All FIFOs are "First Word Fall Through". For reading from the FIFO this means,
as soon as the FIFO is not empty any more, the valid signal is high and data
can be read from the data port. The "ready" signal serves as a acknowledge
signal, i.e. as soon as ready and valid are both high, the data word is
considered read and (if available) the next word can be read from the FIFO.


Control Interface
-----------------

The control interface gives you the possibility to transfer side-channel
information between the host PC and the target device. The signals of the
control interface are not interpreted or used by GLIP itself, from the user
point of view they are just signals which can be triggered on the host PC and
can be used on the logic side for any purpose.

| Port Name       | Width    | Direction | Description                            |
|-----------------|:--------:|:---------:|--------------------------------------- |
| ctrl_logic_rst  | 1        | OUT       | a reset of the user logic is requested |


Clock and Reset
---------------

All signals facing the user logic (the `fifo_` and `ctrl_` signals) are in the
clock domain given by the `clk` signal. Internally, the communication logic provided by
GLIP might operate within a different clock domain, depending on what external
interfaces are being used. In this case, GLIP contains adequate clock-domain
crossing mechanisms to hide this complexity from your logic.

Two reset signals are available: `rst` and `com_rst`. The `rst` should be
connected to the system-wide reset tree also containing the Power On Reset
(POR), possibly an external reset button, etc. It can be used to reset the logic
inside GLIP to be able to receive data from a communication partner.

The `com_rst` is the communication reset. This reset is triggered by the host
(e.g. the PC) when connecting to the target (e.g. the FPGA) to create a common
system state and bring up the communication. The `com_rst` signal can be used by
the logic attached to GLIP to initialize higher-level protocols for example
(i.e. a communication protocol building on top of the GLIP FIFO interface).

| Port Name       | Width    | Direction | Description                                |
|-----------------|:--------:|:---------:|--------------------------------------------|
| clk             | 1        | IN        | user-supplied clock                        |
| rst             | 1        | IN        | user-supplied synchronous reset signal     |
| com_rst         | 1        | OUT       | communication logic reset (host-triggered) |


External Communication Interface
--------------------------------

To do its work GLIP needs to interface with some communication interface on the
FPGA. Depending on the used interface the used signals vary greatly. Thus this
interface is specific to each GLIP backend and not standardized in any way. To
reduce confusion, those signal names never start with `fifo_` or `ctrl_`.
