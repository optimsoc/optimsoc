OpTiMSoC Memory Implementation
==============================

This folder contains a common SRAM memory implementation for OpTiMSoC,
consisting of three parts:

* A generic implementation of a single port and a dual port memory
* A Wishbone bus interface

The external interface complies to the Wishbone bus protocol version B.3, 
especially with respect to memory addressing and burst modes.

All SRAM memory modules use word addressing and offer byte granularity
through the use of the "byte select" (`sel`) signal.

Timing
------

All memory implementations (sp_ram_impl_* and dp_ram_impl_*) are expected to
have one cycle latency between setting the address and the data on the output.

~~~
     + cycle 0 + cycle 1 + cycle 2 +
addr |<   A0   ><   A1   >----------
dout |----------< D(A0)  >< D(A1)  >
~~~


Since most signals and parameters are common across the different modules, they
are explained here in one single place for your reference.

Parameters
----------
* `MEMORY_SIZE_BYTE`: Memory size in bytes

* `AW`: Width of the a byte address in bit, defaults to clog2(MEMORY_SIZE)

* `WORD_AW`: Width of a word address in bit, defaults to AW - (SW >> 1)

* `DW`: Width of the data bus in bit. Allowed values: 8; 16; 32

Calculated Parameters
---------------------
* `SW = DW / 8`: Width of the byte select signal.

Signals/Ports
-------------
* `wb_adr_i[AW-1:0]`: 
  Wishbone byte address

* `addr[AW-1:0]`: 
  *Byte* address to read from/write to.

* `waddr[WORD_AW-1:0]`: 
   *Word* address to read from/write to.

* `sel[SW-1:0]`, `wb_sel_i[SW-1:0]`:
   Byte select. Each bit represents one byte in the output data.
