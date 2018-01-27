"""
    test_stm
    ~~~~~~~~

    Cocotb-based unit test for the System Trace Module (STM)

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import NocDiReader, RegAccess, DiPacket
from osdtestlib.soc_interface import StmTraceGenerator
from osdtestlib.exceptions import *

import random

# DI address of the tested STM module
MODULE_DI_ADDRESS = 1

# DI address of the sending module
SENDER_DI_ADDRESS = 0

# Number of trace events to generate during the test
TRACE_EVENT_TEST_COUNT = 1000


@cocotb.coroutine
def _init_dut(dut):

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Dump design parameters for debugging
    dut._log.info("PARAMETER: VALWIDTH is %d" % dut.VALWIDTH.value.integer)
    dut._log.info("PARAMETER: REG_ADDR_WIDTH is %d" %
                  dut.REG_ADDR_WIDTH.value.integer)

    # Reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    dut.trace_valid <= 0

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.coroutine
def _assert_trace_event(dut, trace_id, trace_value):
    """
    Stimuli on the trace port will be generated once to trigger the emission
    of a new debug event packet which will be read and evaluated.
    """

    generator = StmTraceGenerator()
    reader = NocDiReader(dut, dut.clk)

    # Build expected packet
    expected_packet = DiPacket()
    exp_payload = [0, 0, trace_id]
    payload_words = int(dut.VALWIDTH.value.integer / 16)
    for w in range(0, payload_words):
        exp_payload.append(trace_value >> (w * 16) & 0xFFFF)

    expected_packet.set_contents(dest=SENDER_DI_ADDRESS,
                                 src=MODULE_DI_ADDRESS,
                                 type=DiPacket.TYPE.EVENT.value,
                                 type_sub=0,
                                 payload=exp_payload)

    # Build comparison mask for expected packet
    # Ignore flits 0 and 1 with timestamp
    exp_payload_mask = [1] * len(exp_payload)
    exp_payload_mask[0] = 0
    exp_payload_mask[1] = 0

    yield generator.trigger_event(dut, trace_id, trace_value)
    rcv_pkg = yield reader.receive_packet(set_ready=True)

    if not rcv_pkg:
        raise TestFailure("No response received!")

    if not rcv_pkg.equal_to(dut, expected_packet, exp_payload_mask):
        raise TestFailure("The STM generated an unexpected debug event packet!")


@cocotb.coroutine
def _activate_module(dut):
    """
    Set the MOD_CS_ACTIVE bit in the Control and Status register to 1 to
    enable emitting debug event packets.
    """

    access = RegAccess(dut)

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.BASE_REG.MOD_CS.value,
                                value=1)


@cocotb.test()
def test_stm_activation(dut):
    """
    Check if STM is handling the activation bit correctly
    """
    access = RegAccess(dut)

    yield _init_dut(dut)

    dut._log.info("Check contents of MOD_CS")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.BASE_REG.MOD_CS.value, 0)

    yield _activate_module(dut)

    dut._log.info("Check contents of MOD_CS")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.BASE_REG.MOD_CS.value, 1)

@cocotb.test()
def test_stm_base_registers(dut):
    """
    Check if STM properly generates trace events
    """

    access = RegAccess(dut)

    yield _init_dut(dut)
    yield access.test_base_registers(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                     mod_vendor=1, mod_type=4, mod_version=0,
                                     can_stall=True)

@cocotb.test()
def test_stm_trace_events(dut):
    """
    Check if STM properly generates trace events
    """

    yield _init_dut(dut)

    yield _activate_module(dut)

    for _ in range(0, TRACE_EVENT_TEST_COUNT):
        # Randomly wait between trace events
        for _ in range(0, random.randint(0, 100)):
            yield RisingEdge(dut.clk)

        trace_id = random.randint(0, 2 ** 16 - 1)
        trace_value = random.randint(0, 2 ** 32 - 1)

        yield _assert_trace_event(dut, trace_id, trace_value)
