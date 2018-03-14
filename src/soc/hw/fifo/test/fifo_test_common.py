"""
Common functionality to test FIFOs
"""

import random
import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, FallingEdge
from cocotb.result import TestFailure

# Mirroring expected contents of the FIFO
_fifo_data = []

@cocotb.coroutine
def write_fifo(dut, max_delay=5, write_items=10000, write_random_data=False):
    """
    Write to a FIFO
    
    Before writing wait between 0 and max_delay cycles.
    """
    fifo_wrcnt = 0
    while True:
        # insert random wait before the next write
        wr_delay = random.randint(0, max_delay)
        dut._log.debug("WRITE: Wait for %d clock cycles" % (wr_delay))
        for _ in range(wr_delay):
            yield RisingEdge(dut.clk)

        # make sure that the full signal is stable before checking for it
        yield FallingEdge(dut.clk)

        if dut.full.value:
            dut._log.debug("WRITE: FIFO full, not writing")
        else:
            # generate and write data, keep track of it for checking
            if write_random_data:
                data = random.getrandbits(dut.WIDTH.value.integer)
            else:
                data = fifo_wrcnt % (2 ** dut.WIDTH.value.integer)
            dut.din <= data
            _fifo_data.append(data)

            dut.wr_en <= 1
            yield RisingEdge(dut.clk)
            dut.wr_en <= 0

            fifo_wrcnt += 1
            dut._log.debug("WRITE: Wrote word %d to FIFO, value 0x%x" % (fifo_wrcnt, data))

        if fifo_wrcnt >= write_items:
            return

@cocotb.coroutine
def read_fifo_standard(dut, max_delay=5, read_items=10000):
    """
    Read from a FIFO with standard read semantics and validate the reads
    
    Before the read starts a delay between 0 and max_delay cycles is
    waited. No read is attempted is the FIFO signals it's empty.
    """
    fifo_rdcnt = 0
    while True:
        # insert random wait before the next read
        rd_delay = random.randint(0, max_delay)
        dut._log.debug("READ: Wait for %d clock cycles" % (rd_delay))
        for _ in range(rd_delay):
            yield RisingEdge(dut.clk)

        if dut.empty.value:
            dut._log.debug("READ: FIFO empty, not reading")
        else:
            # send read request
            dut.rd_en <= 1
            fifo_rdcnt += 1

            yield RisingEdge(dut.clk)

            # output is delayed by one cycle
            # lower read request signal
            dut.rd_en <= 0
            yield RisingEdge(dut.clk)

            data_read = dut.dout.value.integer
            dut._log.debug("READ: Got 0x%x in read %d" % (data_read, fifo_rdcnt))

            data_expected = _fifo_data.pop(0)
            if data_read != data_expected:
                raise TestFailure("READ: Expected 0x%x, got 0x%x at read %d" %
                                  (data_expected, data_read, fifo_rdcnt))

        # done
        if fifo_rdcnt >= read_items:
            return

@cocotb.coroutine
def read_fifo_fwft(dut, max_delay=5, read_items=10000):
    """
    Read from a FIFO with FWFT read semantics and validate the reads
    
    Before the read starts a delay between 0 and max_delay cycles is
    waited. No read is attempted is the FIFO signals it's empty.
    """
    fifo_rdcnt = 0
    while True:
        # determine wait cycles before next read
        rd_delay = random.randint(0, max_delay)
        dut._log.debug("READ: Wait for %d clock cycles" % (rd_delay))

        # wait until the next read operation
        if rd_delay != 0:
            # deactivate read request if we have wait cycles
            dut.rd_en <= 0

            for _ in range(rd_delay):
                yield RisingEdge(dut.clk)

        # wait until all signals are stable in this cycle before we check
        # its value
        yield FallingEdge(dut.clk)

        if dut.empty.value:
            dut._log.debug("READ: FIFO empty, not reading")
        else:
            # get current data word
            data_read = dut.dout.value.integer
            fifo_rdcnt += 1
            dut._log.debug("READ: Got 0x%x in read %d" % (data_read, fifo_rdcnt))

            # acknowledge read
            dut.rd_en <= 1

            # check read data word
            data_expected = _fifo_data.pop(0)
            if data_read != data_expected:
                raise TestFailure("READ: Expected 0x%x, got 0x%x at read %d" %
                                  (data_expected, data_read, fifo_rdcnt))

        # done with this cycle, wait for the next one
        yield RisingEdge(dut.clk)

        if fifo_rdcnt >= read_items:
            return
