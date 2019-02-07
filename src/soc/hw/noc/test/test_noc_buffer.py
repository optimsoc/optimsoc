# Copyright (c) 2019 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# =============================================================================
#
# Test for the module 'noc_buffer'
#
# Author(s):
#  - Max Koenen <max.koenen@tum.de>
#

import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, FallingEdge
from cocotb.result import TestFailure

# Mirroring expected contents of the FIFO
_buffer_flits = []
_buffer_last = []

@cocotb.test()
def test_noc_buffer_fullpacket(dut):
    """
    Test the module noc_buffer with the fullpacket feature enabled.
    This test is hardcoded and only works with WIDTH = 8, DEPTH = 4 and
    FULLPACKET = 1.
    """

    if (dut.FULLPACKET.value.integer != 1 or dut.FLIT_WIDTH.value.integer != 8 or
        dut.DEPTH.value.integer != 4):
        return

    dut._log.info("NoC buffer for a flit width of 8 bits and a depth of 4. " +
                  "FULLPACKET feature is enabled.")

    yield _init_dut(dut)
    yield _fullpacket_test_case(dut)
    dut._log.info("All tests done")


@cocotb.test()
def test_noc_buffer_no_fullpacket(dut):
    """
    Test the module noc_buffer with the fullpacket feature disabled.
    """

    if dut.FULLPACKET.value.integer == 1:
        return

    width = dut.FLIT_WIDTH.value.integer
    depth = dut.DEPTH.value.integer
    dut._log.info("NoC buffer for a flit width of {} bits and a depth of {}. ".format(width, depth) +
                  "FULLPACKET feature is disabled.")

    yield _init_dut(dut)
    # Start read and write processes
    write_thread = cocotb.fork(_write_buffer(dut, dut.clk, width))
    read_thread = cocotb.fork(_read_buffer(dut, dut.clk))
    # Wait for read/write to finish. Read only finishes if all required data
    # has been obtained, i.e. it implicitly waits for write as well.
    yield read_thread.join()
    dut._log.info("All tests done")


@cocotb.coroutine
def _init_dut(dut):
    # Setup clock
    cocotb.fork(Clock(dut.clk, 10, "ns").start())

    # Reset
    dut._log.info("Resetting DUT")
    dut.rst <= 1

    dut.in_flit <= 0
    dut.in_valid <= 0
    dut.in_last <= 0
    dut.out_ready <= 0

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0


@cocotb.coroutine
def _write_buffer(dut, clk, width, max_delay=5, write_items=1000, write_random_data=True):
    """
    Before writing wait between 0 and max_delay cycles.
    """

    global _buffer_flits, _buffer_last

    wrcnt = 0
    while True:
        # Insert random wait before the next write
        wr_delay = random.randint(0, max_delay)
        dut._log.debug("WRITE: Wait for {} clock cycles".format(wr_delay))
        for _ in range(wr_delay):
            dut.in_valid <= 0
            yield RisingEdge(clk)

        # Generate and write flit, keep track of it for checking
        if write_random_data:
            flit = random.getrandbits(width)
            last = random.getrandbits(1)
        else:
            flit = wrcnt % (2 ** width)
            last = wrcnt % 2
        dut.in_flit <= flit
        dut.in_last <= last
        dut.in_valid <= 1

        yield RisingEdge(clk)

        # Wait until buffer is ready
        if dut.in_ready != 1:
            while dut.in_ready.value.integer != 1:
                yield RisingEdge(clk)

        _buffer_flits.append(flit)
        _buffer_last.append(last)
        wrcnt += 1
        dut._log.debug("WRITE: Wrote word {} to buffer, value {}, last {}".format(wrcnt, hex(flit), last))

        if wrcnt >= write_items:
            dut.in_valid <= 0
            return


@cocotb.coroutine
def _read_buffer(dut, clk, max_delay=5, read_items=1000):
    """
    Before the read starts a delay between 0 and max_delay cycles is
    waited.
    """

    global _buffer_flits, _buffer_last

    rdcnt = 0
    while True:
        # Determine wait cycles before next read
        rd_delay = random.randint(0, max_delay)
        dut._log.debug("READ: Wait for {} clock cycles".format(rd_delay))
        for _ in range(rd_delay):
            yield RisingEdge(clk)
        else:
            dut.out_ready <= 1
            yield RisingEdge(clk)

        while dut.out_valid.value.integer:
            # Get current flit and flag
            flit = dut.out_flit.value.integer
            last = dut.out_last.value.integer
            flit_expected = _buffer_flits.pop(0)
            last_expected = _buffer_last.pop(0)
            rdcnt += 1
            dut._log.debug("READ: Read flit {} in read {}.".format(hex(flit), rdcnt))

            # Check read flit and flag
            if flit_expected != flit or last_expected != last:
                raise TestFailure("READ: Flit expected {}, read {}, ".format(hex(flit_expected), hex(flit)) +\
                                  "last expected {}, read {}, at read {}.".format(last_expected, last, rdcnt))

            yield RisingEdge(clk)

        else:
            dut._log.debug("READ: Buffer empty, not reading")
            dut.out_ready <= 0

        if rdcnt >= read_items:
            return


@cocotb.coroutine
def _fullpacket_test_case(dut):
    """
    Test case to test FULLPACKET behavior.
    This test is hardcoded to the parameters:
        FLIT_WIDTH = 8
        DEPTH = 4
        FULLPACKET = 1
    """

    yield FallingEdge(dut.clk)

    _set_inputs(dut, 0x01, 0, 0, 0)
    yield FallingEdge(dut.clk)

    # Fill buffer with long packet, then add a second, shorter one while draining
    # 1) 5 flits: 0x02 0x04 0x05 0x06 0x07
    # 2) 2 flits: 0x0a 0x0c
    if dut.out_valid.value.integer:
        raise TestFailure("Out valid is '1', expected '0'!")
    _set_inputs(dut, 0x02, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 0, 1, 0)
    _set_inputs(dut, 0x03, 0, 0, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 0, 1, 0)
    _set_inputs(dut, 0x04, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 0, 1, 0)
    _set_inputs(dut, 0x05, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 0, 1, 0)
    _set_inputs(dut, 0x06, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 0, 1, 0)
    _set_inputs(dut, 0x07, 1, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 1, 0, 5)
    _set_inputs(dut, 0x08, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x02, 0, 1, 0, 5)
    _set_inputs(dut, 0x09, 0, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x04, 0, 1, 1, 4)
    _set_inputs(dut, 0x0a, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x04, 0, 1, 0, 4)
    _set_inputs(dut, 0x0b, 0, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x05, 0, 1, 1, 3)
    _set_inputs(dut, 0x0c, 1, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x06, 0, 1, 1, 2)
    _set_inputs(dut, 0x0d, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x07, 1, 1, 1, 1)
    _set_inputs(dut, 0x0e, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x0a, 0, 1, 1, 2)
    _set_inputs(dut, 0x0f, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x0c, 1, 1, 1, 1)
    _set_inputs(dut, 0x10, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x0c, 1, 0, 1, 0)
    _set_inputs(dut, 0x11, 0, 0, 0)
    yield FallingEdge(dut.clk)

    # Insert a series of different sized packets
    # 1) 1 flit: 0x12
    # 2) 3 flits: 0x13 0x14 0x15
    # 3) 1 flit: 0x16
    _assert_outputs(dut, 0x0c, 1, 0, 1, 0)
    _set_inputs(dut, 0x12, 1, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x12, 1, 1, 1, 1)
    _set_inputs(dut, 0x13, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x12, 1, 1, 1, 1)
    _set_inputs(dut, 0x14, 0, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x12, 1, 1, 1, 1)
    _set_inputs(dut, 0x15, 1, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x13, 0, 1, 1, 3)
    _set_inputs(dut, 0x16, 1, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x14, 0, 1, 1, 2)
    _set_inputs(dut, 0x17, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x15, 1, 1, 1, 1)
    _set_inputs(dut, 0x18, 0, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x16, 1, 1, 1, 1)
    _set_inputs(dut, 0x19, 0, 0, 1)
    yield FallingEdge(dut.clk)

    # Insert a series of 3 1-flit packets with 1 idle cycle in between and
    # assure correct timing behavior.
    # 0x1a 0x1c 0x1e
    _assert_outputs(dut, 0x16, 1, 0, 1, 0)
    _set_inputs(dut, 0x1a, 1, 1, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1a, 1, 1, 1, 1)
    _set_inputs(dut, 0x1b, 1, 0, 0)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1a, 1, 1, 1, 1)
    _set_inputs(dut, 0x1c, 1, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1c, 1, 1, 1, 1)
    _set_inputs(dut, 0x1d, 1, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1c, 1, 0, 1, 0)
    _set_inputs(dut, 0x1e, 1, 1, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1e, 1, 1, 1, 1)
    _set_inputs(dut, 0x1f, 1, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1e, 1, 0, 1, 0)
    _set_inputs(dut, 0x1f, 1, 0, 1)
    yield FallingEdge(dut.clk)

    _assert_outputs(dut, 0x1e, 1, 0, 1, 0)
    _set_inputs(dut, 0x1f, 1, 0, 1)
    yield FallingEdge(dut.clk)


def _assert_outputs(dut, out_flit, out_last, out_valid, in_ready, packet_size):
    """
    Checks if the DUT outputs show the expected values.
    """

    error = False

    r_out_flit = dut.out_flit.value.integer
    r_out_last = dut.out_last.value.integer
    r_out_valid = dut.out_valid.value.integer
    r_in_ready = dut.in_ready.value.integer
    r_packet_size = dut.packet_size.value.integer
    if (r_out_flit != out_flit or r_out_last != out_last or
        r_out_valid != out_valid or r_in_ready != in_ready or
        r_packet_size != packet_size):
        error = True

    dut._log.debug("out_flit: {}; out_last: {}; ".format(hex(r_out_flit), r_out_last) +\
                   "out_valid: {}; in_ready: {}\n".format(r_out_valid, r_in_ready) +\
                   "packet_size: {}".format(r_packet_size))
    if error:
        raise TestFailure("Comparison error!\nValues expected:\n" +\
                          "out_flit: {}; out_last: {}; ".format(hex(out_flit), out_last) +\
                          "out_valid: {}; in_ready: {}\n".format(out_valid, in_ready) +\
                          "packet_size: {}\n".format(packet_size) +\
                          "Values read:\n" +\
                          "out_flit: {}; out_last: {}; ".format(hex(r_out_flit), r_out_last) +\
                          "out_valid: {}; in_ready: {}\n".format(r_out_valid, r_in_ready) +\
                          "packet_size: {}\n".format(r_packet_size))


def _set_inputs(dut, in_flit, in_last, in_valid, out_ready):
    """
    Helper method to set the TDM and BE inputs of the DUT.
    """

    dut.in_flit <= in_flit
    dut.in_last <= in_last
    dut.in_valid <= in_valid
    dut.out_ready <= out_ready
