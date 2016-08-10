@defgroup backend_jtag-logic JTAG FPGA Logic
@ingroup backend_jtag

Usage
=====

The Verilog toplevel module `glip_jtag_toplevel` implements the
[common GLIP logic interface](@ref logicif). The following additional ports are
needed for the JTAG connection.

| Port Name     | Width    | Direction | Description                       |
|---------------|:--------:|:---------:|-----------------------------------|
| tck           | 1        | IN        | JTAG Test Clock                   |
| tms           | 1        | IN        | JTAG Test Mode Select             |
| tdo           | 1        | OUT       | JTAG Test Data Out                |
| tdi           | 1        | IN        | JTAG Test Data In                 |

Note that other JTAG signals are not used, such as TRST, as this functionality
is either a proprietary addition by a vendor, or the functionality can be
achieved in some other way (as with TRST).


Implementation
==============
The hardware logic consists of two main parts: the JTAG TAP, and the GLIP
protocol handling. Both will be described in detail below.

JTAG TAP Controller
-------------------
GLIP does not contain its own JTAG TAP, but uses the
[TAP written by Igor Mohor](http://opencores.org/project,jtag) (commonly
nicknamed "Mohor TAP"), which is also used in many OpenRISC projects. This
TAP is already supported by OpenOCD, reducing the maintenance effort.

Protocol Handling
-----------------
The used protocol is described on the @ref backend_jtag-protocol "protocol page".