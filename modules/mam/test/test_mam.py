"""
    test_mam
    ~~~~~~~~

    Cocotb-based unit test for the Memory Access Module (MAM)

    :copyright: Copyright 2017-2018 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import RegAccess
from osdtestlib.exceptions import *

from mamdriver import *

import random

# DI address of the tested module
MODULE_DI_ADDRESS = 10

# DI address of the sending module
SENDER_DI_ADDRESS = 3

# Parameters read from the DUT
dutparams = {}


@cocotb.coroutine
def _init_dut(dut):

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Store and dump design parameters
    dutparams['DATA_WIDTH'] = dut.DATA_WIDTH.value.integer
    dut._log.info("PARAMETER: DATA_WIDTH is %d" % dutparams['DATA_WIDTH'])
    if dutparams['DATA_WIDTH'] % 8 != 0:
        raise TestFailure("Parameter DATA_WIDTH must be a multiple of 8.")

    dutparams['ADDR_WIDTH'] = dut.ADDR_WIDTH.value.integer
    dut._log.info("PARAMETER: ADDR_WIDTH is %d" % dutparams['ADDR_WIDTH'])
    if dutparams['ADDR_WIDTH'] % 8 != 0:
        raise TestFailure("Parameter ADDR_WIDTH must be a multiple of 8.")

    dutparams['MAX_PKT_LEN'] = dut.MAX_PKT_LEN.value.integer
    dut._log.info("PARAMETER: MAX_PKT_LEN is %d" % dutparams['MAX_PKT_LEN'])

    dut._log.info("PARAMETER: REGIONS is %d" % dut.REGIONS.value.integer)
    dutparams['regions'] = []
    for region in range(0, dut.REGIONS.value.integer):
        baseaddr = getattr(dut, "BASE_ADDR%d" % region).value.integer
        memsize = getattr(dut, "MEM_SIZE%d" % region).value.integer
        dutparams['regions'].append({'baseaddr': baseaddr, 'memsize': memsize})

        dut._log.info("PARAMETER: MEM_SIZE%d is %d" % (region, memsize))
        dut._log.info("PARAMETER: BASE_ADDR%d is %d" % (region, baseaddr))

    # Reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    dut.debug_out_ready <= 0

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.test()
def test_mam_base_registers(dut):
    """
    Check if the base configuration registers have the expected values
    """

    access = RegAccess(dut)

    yield _init_dut(dut)
    yield access.test_base_registers(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                     mod_vendor=1, mod_type=3, mod_version=0,
                                     can_stall=False)

@cocotb.test()
def test_mam_extended_registers(dut):
    """
    Check if the extended configuration registers have the expected values
    """

    access = RegAccess(dut)

    yield _init_dut(dut)
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.MAM_REG.AW.value, 16,
                                  dut.ADDR_WIDTH.value.integer)
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.MAM_REG.DW.value, 16,
                                  dut.DATA_WIDTH.value.integer)
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.MAM_REG.REGIONS.value, 16,
                                  dut.REGIONS.value.integer)

    for region in range(0, dut.REGIONS.value.integer):
        dut._log.info("Checking conf registers for region "+ str(region))
        region_basereg = 0x280 + region * 16
        region_baseaddr_basereg = region_basereg
        region_memsize_basereg = region_basereg + 4

        baseaddr_exp = getattr(dut, "BASE_ADDR%d" % region).value.integer
        memsize_exp = getattr(dut, "MEM_SIZE%d" % region).value.integer

        # REGION*_BASEADDR_*
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_baseaddr_basereg, 16,
                                      baseaddr_exp & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_baseaddr_basereg + 1, 16,
                                      (baseaddr_exp >> 16) & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_baseaddr_basereg + 2, 16,
                                      (baseaddr_exp >> 32) & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_baseaddr_basereg + 3, 16,
                                      (baseaddr_exp >> 48) & 0xFFFF)

        # REGION*_MEMSIZE_*
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_memsize_basereg, 16,
                                      memsize_exp & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_memsize_basereg + 1, 16,
                                      (memsize_exp >> 16) & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_memsize_basereg + 2, 16,
                                      (memsize_exp >> 32) & 0xFFFF)
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      region_memsize_basereg + 3, 16,
                                      (memsize_exp >> 48) & 0xFFFF)


@cocotb.coroutine
def _perform_memory_transfer(dut, mem_transfer):
    di_driver = MamDiDriver(entity=dut, clock=dut.clk,
                            MAX_PKT_LEN=dutparams['MAX_PKT_LEN'],
                            SENDER_DI_ADDRESS=SENDER_DI_ADDRESS,
                            MODULE_DI_ADDRESS=MODULE_DI_ADDRESS)
    dut._log.debug("Performing memory transfer: " + str(mem_transfer))

    sysif_driver = MamSysifDriver(entity=dut, clock=dut.clk)

    di_thread = cocotb.fork(di_driver.drive(mem_transfer))
    sysif_thread = cocotb.fork(sysif_driver.drive(mem_transfer))

    yield di_thread.join()
    yield sysif_thread.join()

@cocotb.test()
def test_mam_memory_write_single(dut):
    """Perform a single-word write"""

    yield _init_dut(dut)

    data = bytes([0x12, 0x34, 0x56, 0x78])
    mem_transfer = MemoryTransfer(AW=dutparams['ADDR_WIDTH'],
                                  DW=dutparams['DATA_WIDTH'])
    mem_transfer.addr = 0x20
    mem_transfer.burst = False
    mem_transfer.operation = 'write'
    mem_transfer.data = data

    yield _perform_memory_transfer(dut, mem_transfer)


@cocotb.test()
def test_mam_memory_write_single_sync(dut):
    """Perform a synchronous (acknowledged) single-word write"""

    yield _init_dut(dut)

    data = bytes([0x12, 0x34, 0x56, 0x78])
    mem_transfer = MemoryTransfer(AW=dutparams['ADDR_WIDTH'],
                                  DW=dutparams['DATA_WIDTH'])
    mem_transfer.addr = 0x20
    mem_transfer.burst = False
    mem_transfer.sync = True
    mem_transfer.operation = 'write'
    mem_transfer.data = data

    yield _perform_memory_transfer(dut, mem_transfer)


@cocotb.test()
def test_mam_memory_write_bulk(dut):
    """Perform a bulk write"""

    yield _init_dut(dut)

    mem_transfer = MemoryTransfer(AW=dutparams['ADDR_WIDTH'],
                                  DW=dutparams['DATA_WIDTH'])

    numbytes = int(32 * (mem_transfer.DW / 8))
    data = bytes([random.randint(0, 0xFF) for _ in range(0, numbytes)])

    mem_transfer.addr = 0x80
    mem_transfer.sync = False
    mem_transfer.burst = True
    mem_transfer.operation = 'write'
    mem_transfer.data = data

    yield _perform_memory_transfer(dut, mem_transfer)


@cocotb.test()
def test_mam_memory_read_single(dut):
    """Perform a single-word read"""

    yield _init_dut(dut)

    mem_transfer = MemoryTransfer(AW=dutparams['ADDR_WIDTH'],
                                  DW=dutparams['DATA_WIDTH'])

    numbytes = int(mem_transfer.DW / 8)
    data = bytes([random.randint(0, 0xFF) for _ in range(0, numbytes)])

    mem_transfer.addr = 0x80
    mem_transfer.burst = False
    mem_transfer.operation = 'read'
    mem_transfer.data = data

    yield _perform_memory_transfer(dut, mem_transfer)


@cocotb.test()
def test_mam_memory_read_bulk(dut):
    """Perform a bulk read"""

    yield _init_dut(dut)

    mem_transfer = MemoryTransfer(AW=dutparams['ADDR_WIDTH'],
                                  DW=dutparams['DATA_WIDTH'])

    numbytes = int(32 * (mem_transfer.DW / 8))
    data = bytes([random.randint(0, 0xFF) for _ in range(0, numbytes)])

    mem_transfer.addr = 0x80
    mem_transfer.burst = True
    mem_transfer.operation = 'read'
    mem_transfer.data = data

    yield _perform_memory_transfer(dut, mem_transfer)
