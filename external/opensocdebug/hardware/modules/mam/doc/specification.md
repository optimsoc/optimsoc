# Introduction

This document specifies the implementation of the *Memory Access
Module*.

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

Fill in your name here!

# System Interface

There is a generic interface between the MAM and the system:

 Signal        | Direction   | Description
 ------------  | ----------- | -----------
 `req_valid`   | MAM->System | Start a new memory access request
 `req_ready`   | MAM->System | Acknowledge the new memory access request
 `req_rw`      | MAM->System | `0`: Read, `1`: Write
 `req_addr`    | MAM->System | Request base address
 `req_burst`   | MAM->System | `0` for single beat access, `1` for incremental burst
 `req_size`    | MAM->System | Burst size in number of words
 `write_valid` | MAM->System | Next write data is valid
 `write_data`  | MAM->System | Write data
 `write_strb`  | MAM->System | Byte strobe if `req_burst==0`
 `write_ready` | System->MAM | Acknowledge this data item
 `read_valid`  | System->MAM | Next read data is valid
 `read_data`   | System->MAM | Read data
 `read_ready`  | MAM->System | Acknowledge this data item

# Memory Map

 Address Range | Description
 ------------- | -----------
 `0x200`       | Address width in Byte
 `0x201`       | Data width in Byte
 `0x202`       | `1` if unaligned accesses are allowed, `0` otherwise

# Debug Packets

The host sends debug packets of the `PLAIN` type, which means the
content is module-specific. The payload therefore contains the memory
access sequence.

The sequence starts with a common header:

 Payload | Content
 ------- | -------
 0       | `[15]`: R/W, `[14]`: Single/Chunk, `[13:0]`: Strobe/Chunk size
 1       | `Address[WIDTH-1:WIDTH-16]`
 ..      | ..
 N       | `Address[15:0]`

After this sequence the data exchange starts as described below.

A chunk is an incremental burst access starting from the base address.

The bits in the first transfer are defined as:

 Bit(s) | Content
 ------ | -------
 `15`   | R: `0`, W: `1`
 `14`   | Single word access: `0`, Chunk access: `1`
 `13:0` | Chunk: the burst size in number of words,
 `13:0` | Single: byte strobe

TODO: Byte strobe definition

Following this setup information, the actual transfer takes place be
sending the data in a stream, meaning the debug packets are maximally
filled and the data spans multiple packets. It is only possible to
have one read or one write access at a time.

## Write Sequence

The write sequence is the setup header as described before, followed
by the data ordered from MSB to LSB.

An examplary write sequence of a chunk of four data items with data
width 64-bit and address width 32-bit is the sequence:

    0xc004=1100 0000 0000 0100
    Addr[31:16]
	Addr[15:0]
	D0[63:48]
	D0[47:32]
	D0[31:16]
	D0[15:0]
	D1[63:48]
	D1[47:32]
	D1[31:16]
	D1[15:0]
	D2[63:48]
	D2[47:32]
	D2[31:16]
	D2[15:0]
	D3[63:48]
	D3[47:32]
	D3[31:16]
	D3[15:0]

This sequence will write `D0` to `Addr`, `D1` to `Addr+1`, `D2` to
`Addr+2` and `D3` to `Addr+3`.

If the maximum packet size in the debug interconnect is 8, this is the
packet sequence with minimum number of packets:

	(dest=MAM_ID)
	(type=PLAIN,src=0)
	0xc004
	Addr[31:16]
	Addr[15:0]
	D0[63:48]
	D0[47:32]
	D0[31:16]

	(dest=MAM_ID)
	(type=PLAIN,src=0)
	D0[15:0]
	D1[63:48]
	D1[47:32]
	D1[31:16]
	D1[15:0]
	D2[63:48]

	(dest=MAM_ID)
	(type=PLAIN,src=0)
	D2[47:32]
	D2[31:16]
	D2[15:0]
	D3[63:48]
	D3[47:32]
	D3[31:16]

    (dest=MAM_ID)
	(type=PLAIN,src=0)
	D3[15:0]

