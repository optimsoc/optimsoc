"""
    test_scm
    ~~~~~~~~

    Cocotb-based unit test for the System Control Module (SCM)

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import RegAccess, DiPacket
from osdtestlib.exceptions import *

import random

# DI address of the tested STM module
MODULE_DI_ADDRESS = 1

# DI address of the sending module
SENDER_DI_ADDRESS = 0


@cocotb.coroutine
def _init_dut(dut):

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Dump design parameters for debugging
    dut._log.info("PARAMETER: SYSTEM_VENDOR_ID is %d" %
                  dut.SYSTEM_VENDOR_ID.value.integer)
    dut._log.info("PARAMETER: SYSTEM_DEVICE_ID is %d" %
                  dut.SYSTEM_DEVICE_ID.value.integer)
    dut._log.info("PARAMETER: NUM_MOD is %d" %
                  dut.NUM_MOD.value.integer)
    dut._log.info("PARAMETER: MAX_PKT_LEN is %d" %
                  dut.MAX_PKT_LEN.value.integer)

    # Reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    dut.debug_out_ready <= 1

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.coroutine
def _cpu_reset(dut):
    """
    The CPU reset value will be set and reset. The corresponding signal will
    be observed.
    """

    access = RegAccess(dut)

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.SCM_REG.SYSRST.value,
                                value=2)

    if dut.cpu_rst != 1:
        raise TestFailure("CPU reset signal could not be set!")

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.SCM_REG.SYSRST.value,
                                value=0)

    if dut.cpu_rst != 0:
        raise TestFailure("CPU reset signal could not be reset!")

@cocotb.coroutine
def _sys_reset(dut):
    """
    The system reset value will be set and reset. The corresponding signal
    will be observed.
    """

    access = RegAccess(dut)

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.SCM_REG.SYSRST.value,
                                value=1)
    if dut.sys_rst != 1:
        raise TestFailure("System reset signal could not be set!")

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.SCM_REG.SYSRST.value,
                                value=0)
    if dut.sys_rst != 0:
        raise TestFailure("System reset signal could not be reset!")

@cocotb.test()
def test_scm_baseregisters(dut):
    """
    Read the 5 additional registers of the SCM and compares the response
    with the desired value
    """
    access = RegAccess(dut)

    yield _init_dut(dut)

    yield access.test_base_registers(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                     mod_vendor=1, mod_type=1, mod_version=0,
                                     can_stall=False)

@cocotb.test()
def test_scm_extended(dut):
    """
    Read the 5 additional registers of the SCM and compares the response
    with the desired value
    """
    access = RegAccess(dut)

    yield _init_dut(dut)

    dut._log.info("Check contents of SYSTEM_VENDOR_ID")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.SCM_REG.SYSTEM_VENDOR_ID.value, 16, 1)

    dut._log.info("Check contents of SYSTEM_DEVICE_ID")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.SCM_REG.SYSTEM_DEVICE_ID.value, 16, 1)

    dut._log.info("Check contents of NUM_MOD")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.SCM_REG.NUM_MOD.value, 16, 1)

    dut._log.info("Check contents of MAX_PKT_LEN")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.SCM_REG.MAX_PKT_LEN.value, 16, 12)

    dut._log.info("Check contents of SYSRST")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.SCM_REG.SYSRST.value, 16, 0)


@cocotb.test()
def test_scm_reset(dut):
    """
    Activates and deactivates the CPU and system reset by writing into the
    system reset register. The sys_rst and cpu_rst ports are observed to
    verify the correct reaction.
    """

    yield _init_dut(dut)


    yield _cpu_reset(dut)

    yield _sys_reset(dut)

