"""
Test the module fifo_dualclock_fwft, a dual clock first-word-fall-through FIFO
with programmable full & flag
"""

import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, FallingEdge
from cocotb.result import TestFailure

from fifo_test_common import read_fifo_fwft, write_fifo, read_fifo_fwft_prog, write_fifo_prog
    

@cocotb.test()
def test_fifo_dualclock_fwft(dut):
    """
    Test the module fifo_dualclock_fwft, a dual clock first-word-fall-through 
    FIFO with programmable full & flag
    """

    # Read the parameters back from the DUT to set up our model
    width = dut.WIDTH.value.integer  # [bit]
    depth = dut.DEPTH.value.integer  # [entries]
    prog_full = dut.PROG_FULL.value.integer
    prog_empty = dut.PROG_EMPTY.value.integer
    dut._log.info("%d bit wide FIFO with %d entries." % (width, depth))
    dut._log.info("PROG_FULL is set to %d, PROG_EMPTY is set to %d." % (prog_full, prog_empty))

    # setup write clock
    cocotb.fork(Clock(dut.wr_clk, 500).start())
    # setup read clock
    cocotb.fork(Clock(dut.rd_clk, 1000).start())
    
    # reset
    dut._log.info("Resetting DUT")
    dut.wr_rst <= 1
    dut.rd_rst <= 1

    dut.din <= 0
    dut.wr_en <= 0
    dut.rd_en <= 0

    for _ in range(2):
        yield RisingEdge(dut.rd_clk)
    dut.wr_rst <= 0
    dut.rd_rst <= 0

    # first test: fill FIFO once and validate the values as well as the full
    # & empty flags
    dut._log.info("Starting first test...");
    
    # start write processes
    write_thread = cocotb.fork(write_fifo_prog(dut, dut.wr_clk, prog_full, 0))
    # wait for write to finish. The FIFO is filled once.
    yield write_thread.join()
    # start read process
    read_thread = cocotb.fork(read_fifo_fwft_prog(dut, dut.rd_clk, prog_empty, 0))
    # wait for read to finish. The FIFO is read until empty.
    yield read_thread.join()
    dut._log.info("first test finished.")
    
    # reset dut again
    dut._log.info("Resetting DUT")
    dut.wr_rst <= 1
    dut.rd_rst <= 1

    dut.din <= 0
    dut.wr_en <= 0
    dut.rd_en <= 0

    for _ in range(2):
        yield RisingEdge(dut.rd_clk)
    dut.wr_rst <= 0
    dut.rd_rst <= 0
    
    # second test: read and write simultaneously and validate read data
    dut._log.info("Starting second test...")
    
    # start read and write processes
    write_thread = cocotb.fork(write_fifo(dut, dut.wr_clk))
    read_thread = cocotb.fork(read_fifo_fwft(dut, dut.rd_clk))

    # wait for read/write to finish. Read only finishes if all required data
    # has been obtained, i.e. it implicitly waits for write as well.
    yield read_thread.join()

    dut._log.info("All tests done")
