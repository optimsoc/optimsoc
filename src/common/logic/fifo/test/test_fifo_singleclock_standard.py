"""
Test the module fifo_singleclock_standard, a synchronous FIFO with standard
read characteristics
"""

import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, FallingEdge
from cocotb.result import TestFailure

from fifo_test_common import read_fifo_standard, write_fifo
    
@cocotb.test()
def test_fifo_singleclock_standard(dut):
    """
    Test the module fifo_singleclock_standard, a synchronous FIFO with standard
    read characteristics
    """

    # Read the parameters back from the DUT to set up our model
    width = dut.WIDTH.value.integer  # [bit]
    depth = dut.DEPTH.value.integer  # [entries]
    dut._log.info("%d bit wide FIFO with %d entries." % (width, depth))

    # setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    dut.din <= 0
    dut.wr_en <= 0
    dut.rd_en <= 0

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

    # start read and write processes
    write_thread = cocotb.fork(write_fifo(dut, dut.clk))
    read_thread = cocotb.fork(read_fifo_standard(dut, dut.clk))

    # wait for read/write to finish. Read only finishes if all required data
    # has been obtained, i.e. it implicitly waits for write as well.
    yield read_thread.join()

    dut._log.info("All tests done")
