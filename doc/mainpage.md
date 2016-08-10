GLIP {#mainpage}
===

Introduction
------------

GLIP, the "Generic Logic Interfacing Project", is a solution for transferring
data through FIFOs between a host, usually a PC, and a target, usually a
hardware component such as an FPGA or a microcontroller. The actual data
transport can happen through various interfaces, such as USB 2.0, JTAG or TCP.
GLIP encapsulates all low-level details of the data transfers and provides on
the host side an easy to use C library, and on the target side ready to use
interfaces (e.g. in Verilog) to quickly setup a working communication.

![GLIP Overview](glip-overview.svg "GLIP Overview")

Feature Overview
----------------
 - Easy FIFO-based communication, abstracting away all low-level details
 - Support for different communication channels through
   [backends](@ref backend).
 - Side-channel communication, e.g. reset signals
 - Extensive documentation
 - [MIT](http://opensource.org/licenses/MIT) licensed
 - Developed on and for Linux

Target Side
-----------
On the target side, we provide a common FIFO interface, which we call the "GLIP
Logic Interface". The actual implementation is specific to the target and the
used communication protocol, but tries to adhere to a common interface. This
interface is documented on the [GLIP Logic Interface](@ref logicif) page.

Host Side
---------
On the host we provide a C library, `libglip`, which encapsulates all
communication tasks.

The library is split up into several parts, which are described on individual
pages. Look there for detailled instructions on the individual topics.
- @ref library-init-deinit
- @ref connection-handling
- @ref communication
- @ref log
- @ref utilities
- @ref backend

Getting Started
---------------

Using GLIP is easy, but depends a bit on the selected target. Some
[demonstration examples](@ref examples) to get a quick start on some
selected platforms.

To get an impression of how to use GLIP on the host side, the
following example shows a simple example code. Essentially, you need
to initialize the library, connect to a target, read and write data
from it, close the connection when you're done and free all allocated
resources.

@note The example below shows the error handling once to make the example
  easier to read. (Almost) all functions in the GLIP API use the same kind of
  error reporting: a return value of <code>0</code> indicates everything was
  OK, any other return value marks an error.
  Make sure to check <b>all</b> return codes in your production code.
  The only notable exception to this convention are the getter and setter
  functions, i.e. functions starting with <code>glip_get_</code>,
  <code>glip_set_</code> or <code>glip_is_</code>. Those functions return
  nothing (setters) or the requested value (setters).

@include glip-simple.c
