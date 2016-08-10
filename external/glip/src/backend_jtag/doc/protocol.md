@defgroup backend_jtag-protocol JTAG Wire Protocol
@ingroup backend_jtag

@note This section requires some familarity with JTAG and the JTAG state
  machine. If this sounds unfamiliar, please read up on those topics first
  before returning to this documentation.

GLIP uses a custom data transmission protocol on top of JTAG to achieve
reasonable performance. Two main factors limit the throughput over JTAG:

- going through the JTAG state machine between data shifts
- round-trip delays induced by standard JTAG adapters. The common FT2232-based
  JTAG adapters wrap each JTAG command in a USB transmission, which can create
  significant round-trip latencies.

Considering those factors, the central idea of the GLIP JTAG protocol is the
usage of very large transmissions, i.e. a very long shift register to achieve
good performance. All communication is done in the JTAG data shift state
(dshift). To further reduce the overhead introduced by round-trip delays, we
chose a flow control protocol based on the opportunistic sending of data, which
is then acknowledged by the FPGA or (if not acknowledged) retransmitted by the
host.

Three types of communication messages are defined, which are described below.

Configuration Discovery
-----------------------

Before starting the communication between the host and the target, the host
needs to get the values of some configuration parameters, passed to
`glip_jtag_toplevel` at synthesis time.

Therefore, communication starts with a configuration discovery transmission,
which is depicted below:

    word  0        1            2
    bit   15     0 15         0 15        0
         +--------+------------+-----------+
    req  | 0xFFFF |   0x0000   |  0x0000   |
         +--------+------------+-----------+
         +--------+------------+-----------+
    resp | 0x0000 | WORD_WIDTH | NUM_WORDS |
         +--------+------------+-----------+

- `WORD_WIDTH` are the number of bits in a transmitted data word
- `NUM_WORDS` is the log2(size) of the receive buffer in words. The number of
  words is calculated as (1 << NUM_WORDS).


Side Channel Communication
--------------------------
GLIP supports the transmission of side-channel data in addition to the FIFO
payload data. This data is transferred in the same way as the configuration
discovery packet.


    word  0        1            2
    bit   15     0 15         0 15        0
         +--------+------------+-----------+
    req  | 0xFFFF |     SC     |  0x0000   |
         +--------+------------+-----------+
         +--------+------------+-----------+
    resp | 0x0000 |   ignored  |  ignored  |
         +--------+------------+-----------+

`SC` is a bitfield of side-channel data.
- `SC[0]`: logic reset (`ctrl_logic_reset`)



Data Communication
------------------

The basic idea of the data communication is the following.

- The host sends data in words, the target acknowledges the number of words
  written into the output FIFO. Words which are not acknowledged are
  retransmitted by the host.
- The host always guarantees enough buffer space for requested words from the
  FPGA. Therefore, no retransmissions occur on read transfers. (Buffer space
  on the PC is not as expensive as it is on an FPGA.)

The word width (`WW` in the following diagram) is equal to the FIFO width on
the logic side.


    word  0             1             2            n+1          n+2            n+3
    bit   WW-1        0 WW-1        0 WW-1      0  WW-1       0 WW-1         0 WW-1         0
         +-------------+-------------+-----------+/+-----------+--------------+--------------+
    req  | WR_REQ_SIZE | RD_REQ_SIZE | WR_WORD_1 |/| WR_WORD_n |    0x0000    |    0x0000    |
         +-------------+-------------+-----------+/+-----------+--------------+--------------+
         +-------------+-------------+-----------+/+-----------+--------------+--------------+
    resp |    0x0000   |    0x0000   | RD_WORD_1 |/| RD_WORD_n | WR_RESP_SIZE | RD_RESP_SIZE |
         +-------------+-------------+-----------+/+-----------+--------------+--------------+

- `WR_REQ_SIZE`: Number of words that are valid in the request, i.e. that
   should be written into the target-side FIFO. Setting this field to all-one
   (0xFF..FF) is not allowed, since this value has special meaning.
- `RD_REQ_SIZE`: Number of words that are requested by the host for
  transmission. The target is free to send less data.
- `WR_RESP_SIZE`: Number of words that have been successfully written into
  the target's FIFO, i.e. they are acknowledged by the target. All words which
  are not acknowledged need to be resent.
- `RD_RESP_SIZE`: Number of valid words in the response from the target that
   should be written to the host-side FIFO (up to `RD_REQ_SIZE` words).
- `n` is the number of payload words (each being `WW` bit wide). It is equal to
   `MAX(WR_REQ_SIZE, RD_REQ_SIZE)`. `n` can be at most `2*WW - 2`, since
   the 0xFF...FF value inside `WR_REQ_SIZE` is reserved.
- `WR_WORD_x` are the data words sent to the target
- `RD_WORD_x` are the data words received from the target
- The data fields can be empty (but must still be sent, of course). If empty,
  they should be set to 0x0000.
