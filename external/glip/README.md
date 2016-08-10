GLIP - The Generic Logic Interfacing Project
============================================

GLIP, the "Generic Logic Interfacing Project", is a solution for transferring
data through FIFOs between a host, usually a PC, and a target, usually a
hardware component such as an FPGA or a microcontroller. The actual data
transport can happen through various interfaces, such as USB 2.0, JTAG or TCP.
GLIP encapsulates all low-level details of the data transfers and provides on
the host side an easy to use C library, and on the target side ready to use
interfaces (e.g. in Verilog) to quickly setup a working communication.

For more information, see http://glip.io.
