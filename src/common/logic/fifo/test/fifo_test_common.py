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
def write_fifo(dut, clk, max_delay=5, write_items=10000, write_random_data=False):
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
            yield RisingEdge(clk)

        # make sure that the full signal is stable before checking for it
        yield FallingEdge(clk)

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
            yield RisingEdge(clk)
            dut.wr_en <= 0

            fifo_wrcnt += 1
            dut._log.debug("WRITE: Wrote word %d to FIFO, value 0x%x" % (fifo_wrcnt, data))

        if fifo_wrcnt >= write_items:
            return

@cocotb.coroutine
def write_fifo_prog(dut, clk, prog_full, max_delay=5, write_random_data=False):
    """
    Write to a FIFO to validate the programmable full flag.
    
    Before writing wait between 0 and max_delay cycles.
    """
    fifo_wrcnt = 0
    prog_cnt = 0
    while True:
        # insert random wait before the next write
        wr_delay = random.randint(0, max_delay)
        dut._log.debug("WRITE: Wait for %d clock cycles" % (wr_delay))
        for _ in range(wr_delay):
            yield RisingEdge(clk)

        # make sure that the full signal is stable before checking for it
        yield FallingEdge(clk)

        # return once the FIFO has been filled
        if dut.full.value:
            dut._log.info("WRITE: FIFO full. %d words written. prog_cnt: %d" % (fifo_wrcnt, prog_cnt))
            # check if prog_full works correctly
            if prog_cnt > prog_full:
                raise TestFailure("Number of words written after prog_full: %d, prog_full set to: %d" %
                                  (prog_cnt, prog_full))
            return
        else:
            # generate and write data, keep track of it for checking
            if write_random_data:
                data = random.getrandbits(dut.WIDTH.value.integer)
            else:
                data = fifo_wrcnt % (2 ** dut.WIDTH.value.integer)
            dut.din <= data
            _fifo_data.append(data)

            dut.wr_en <= 1
            
            # count number of words written after prog_full has been raised
            if dut.prog_full.value:
                prog_cnt += 1
                
            yield RisingEdge(clk)
            dut.wr_en <= 0

            fifo_wrcnt += 1
            dut._log.debug("WRITE: Wrote word %d to FIFO, value 0x%x" % (fifo_wrcnt, data))

@cocotb.coroutine
def read_fifo_standard(dut, clk, max_delay=5, read_items=10000):
    """
    Read from a FIFO with standard read semantics and validate the reads.
    
    Before the read starts a delay between 0 and max_delay cycles is
    waited. No read is attempted is the FIFO signals it's empty.
    """
    fifo_rdcnt = 0
    while True:
        # insert random wait before the next read
        rd_delay = random.randint(0, max_delay)
        dut._log.debug("READ: Wait for %d clock cycles" % (rd_delay))
        for _ in range(rd_delay):
            yield RisingEdge(clk)

        if dut.empty.value:
            dut._log.debug("READ: FIFO empty, not reading")
        else:
            # send read request
            dut.rd_en <= 1
            fifo_rdcnt += 1

            yield RisingEdge(clk)

            # output is delayed by one cycle
            # lower read request signal
            dut.rd_en <= 0
            yield RisingEdge(clk)

            data_read = dut.dout.value.integer
            dut._log.debug("READ: Got 0x%x in read %d" % (data_read, fifo_rdcnt))

            data_expected = _fifo_data.pop(0)
            if data_read != data_expected:
                raise TestFailure("READ: Expected 0x%x, got 0x%x at read %d" %
                                  (data_expected, data_read, fifo_rdcnt))

        # done
        if fifo_rdcnt >= read_items:
            # empty list at the end of the test
            del _fifo_data[:]
            return

@cocotb.coroutine
def read_fifo_fwft(dut, clk, max_delay=5, read_items=10000):
    """
    Read from a FIFO with FWFT read semantics and validate the reads.
    
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
                yield RisingEdge(clk)

        # wait until all signals are stable in this cycle before we check
        # its value
        yield FallingEdge(clk)

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
        yield RisingEdge(clk)

        if fifo_rdcnt >= read_items:
            # empty list at the end of the test
            del _fifo_data[:]
            return

@cocotb.coroutine
def read_fifo_fwft_prog(dut, clk, prog_empty, max_delay=5):
    """
    Read from a FIFO with FWFT read semantics and validate the programmable
    full flag.
    
    Before the read starts a delay between 0 and max_delay cycles is
    waited.
    """
    fifo_rdcnt = 0
    prog_cnt = 0
    while True:
        # determine wait cycles before next read
        rd_delay = random.randint(0, max_delay)
        dut._log.debug("READ: Wait for %d clock cycles" % (rd_delay))

        # wait until the next read operation
        if rd_delay != 0:
            # deactivate read request if we have wait cycles
            dut.rd_en <= 0

            for _ in range(rd_delay):
                yield RisingEdge(clk)

        # wait until all signals are stable in this cycle before we check
        # its value
        yield FallingEdge(clk)

        if dut.empty.value:
            dut._log.info("READ: FIFO empty. %d words read. prog_cnt: %d" % (fifo_rdcnt, prog_cnt))
            # check if prog_empty works correctly
            if prog_cnt > prog_empty:
                raise TestFailure("Number of words read after prog_empty: %d, prog_empty set to: %d" %
                                  (prog_cnt, prog_empty))
            # empty list at the end of the test
            del _fifo_data[:]
            return
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
            
            # count number of words read after prog_empty has been raised
            if dut.prog_empty.value:
                prog_cnt += 1

        # done with this cycle, wait for the next one
        yield RisingEdge(clk)
