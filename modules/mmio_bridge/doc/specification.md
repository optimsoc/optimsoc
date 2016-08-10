# Introduction

This document specifies the interface of the Memory-Mapped I/O Bridge
module. This bridge can be used by debug modules to abstract from the
Debug Packet format and use a simple memory-mapped I/O interface
instead. The debug functionality is then implemented as a slave module
to the bridge. Beside the ease of implementation is also allows the
actual debug functionality to be bridged to some other interconnect.

## License

This work is licensed under the Creative Commons
Attribution-ShareAlike 4.0 International License. To view a copy of
this license, visit
[http://creativecommons.org/licenses/by-sa/4.0/](http://creativecommons.org/licenses/by-sa/4.0/)
or send a letter to Creative Commons, PO Box 1866, Mountain View, CA
94042, USA.

You are free to share and adapt this work for any purpose as long as
you follow the following terms: (i) Attribution: You must give
appropriate credit and indicate if changes were made, (ii) ShareAlike:
If you modify or derive from this work, you must distribute it under
the same license as the original.

## Authors

Stefan Wallentowitz

Andreas Traber

Fill in your name here!

# Memory-Mapped Interface

The memory-mapped interface is a simple interface:

 Signal       | Driver | Width       | Description
 ------------ | ------ | ----------- | -----------
 `valid`      | Master | 1           | Perform a transfer
 `addr`       | Master | 16          | Address of the memory-mapped transfer.
 `write`      | Master | 1           | Signal a write transfer
 `data_write` | Master | `REG_WIDTH` | Data to be written on write transfer
 `ready`      | Slave  | 1           | Acknowledge a transfer
 `data_read`  | Slave  | `REG_WIDTH` | Read data
 `irq`        | Slave  | `IRQ_WIDTH` | Interrupt to start unsolicited transfer

The slave can acknowledge with the `ready` signal in the same cycle to allow
for high-speed interfacing, acknowledge in the next cycle or insert arbitrarily
long wait states. The `valid` signal gets deasserted after a synchronous
`ready`. If the `valid` signal stays asserted after a synchronous `ready`,
another transfer is performed. `valid` must not depend on `ready`
combinationally.

![Simple MMIO transaction](./figures/mmio_if.pdf)

The `irq` signal triggers the MMIO-bridge to read from an address as
described below and send a trace event to the host or configured
destination module. The irq is not a single bit event, but the signal
is set to the size of the trace event to transmit (limited by
`IRQ_WIDTH`). The slave module can determine success of the trace
event with the read sequence but does not have guarantees about the
time between an interrupt and the read sequence. The read sequence can
also contain pause states. Finally, the `irq` signal can be
de-asserted (set to `0x0` during any time between the start of the
first read (`valid=1 && addr=MMI_IRQ_CAUSE`) and the last
acknowledge. After this it must be de-asserted otherwise another trace
event is generated.

# Memory Map

The memory map uses a 16 bit wide address space, roughly divided into 4 bits
for the group and 12 bits for the address inside a group, which results in 16
groups.

 Address Range       | Group | Description
 -------------       | ----- | -----
 `0x0000` - `0x0fff` | GPR   | General-Purpose Register, Floating Point Registers, ...
 `0x1000` - `0x1fff` | CSR   | Control and Status Registers
 `0x2000` - `0x2fff` | DBG   | Debug Registers (Debug Mode, Program Counters, ...)
 `0x3000` - `0x3fff` |       | Reserved for Custom Extensions/Co-Processors
 `0x4000` - `0x41ff` | MMIO  | Control and Status
 `0x4200`            | MMIO  | Interrupt Cause
 `0x4201` - `0x42ff` | MMIO  | Reserved
 `0x4300` - `0xffff` | MMIO  | Slave module address space

## Group "GPR"

The GPR group contains the general-purpose register file and the floating point
register file.

They are mapped as follows:

 Address Range       | Key     | Description
 -------------       | ---     | -----
 `0x0000` - `0x001f` | x0-x31  | General-Purpose Registers
 `0x0020`            | pc      | (Next) Program Counter
 `0x0021`            | ppc     | (Previous) Program Counter
 `0x0100` - `0x001f` | f0-f31  | Floating-Point Registers
 `0x0120`            | fcsr    | Floating-Point Control and Status Register

Address `0x0000 (register x0) is read-only and hard-wired to 0.

It is assumed that the core is halted when a register is read. `pc` then
contains the program counter of the next instruction that will be executed by
the core when it continues operation.
`ppc` contains the program counter of the previously executed instruction which
is important for software breakpoints.

If a register is not present, the MMIO interface returns 0.

## Group "CSR"

The CSR group is directly mapped to the control and status registers specified
in the RISC-V privileged specification.

## Group "DBG"

The DBG group contains registers related to the interaction with a connected
debugger.

Specifically it contains the following registers

 Address Range       | Key     | Description
 -------------       | ---     | -----
 `0x2000`            | DMR     | Debug Mode Register
 `0x2001`            | DSR     | Debug Stop Register
 `0x2002`            | DCR     | Debug Cause Register

The debug mode register controls single-stepping mode, trap on branches and so on

The debug stop register controls trap behaviour when encountering interrupts.

The debug cause register contains information why the core has trapped to the debugger.

TODO: describe those registers in more detail with individual bits

## Control and Status

The slave module must handle the following control and status requests
properly, that are elaborated in the following:

 Address | Key | Description
 ------- | --- | -----------
 `0x0200` | `MMI_IRQ_CAUSE` | Interrupt Cause

### Interrupt Cause

On occurence of an interrupt, a trace event is generated and sent to
the host or configured destination. The value of the trace event is
determined by reading from this 16 bit register and the host or
destination module can take appropriate action.

Depending on the size signaled by the interrupt signal (see above)
`irq` count words are read from the address (same-address burst).

# Parameters

 Key | Width | Description
 --- | ----- | -----------
 `REG_WIDTH` | Integer | Width of registers (16, 32 or 64)
 `MOD_ID` | 16 | Module Identifier
 `MOD_VERSION` | 8 | Module Version
 `MOD_VENDOR` | 16 | Module Vendor (optional)

## Width of Registers (`REG_WIDTH`)

This is the (maximum) width of the internal registers of the target CPU.
It is possible to access smaller registers (by ignoring parts), but only
aligned to `REG_WIDTH` addresses.

## Module Identifier (`MOD_ID`)

The module identifier can take two forms. If bit 15 is `0` it is an
Open SoC Debug module with a unique identifier in the bits `14:0`. If
bit 15 is `1` then the same bits decode a unique module version but of
a module vendor as defined by `MOD_VENDOR`.

## Module Vendor (`MOD_VENDOR`)

The optional vendor identifier (16 bit), that is only used if bit 15
of `MOD_ID` is used. For the use of a valid vendor identifier, please
refer to the Open SoC Debug specification.

## Module Version (`MOD_VERSION`)

An 8 bit version number of this module.
