"""
    test_regaccess_demux
    ~~~~~~~~~~~~~~~~~~~~

    Cocotb-based unit test for the register access DI demultiplexer

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

import random

from osdtestlib.debug_interconnect import NocDiWriter, NocDiReader, RegAccess, DiPacket

# DI address of the tested module
MODULE_DI_ADDRESS = 1

# DI address of the sending module
SENDER_DI_ADDRESS = 0

STRESS_TEST_PKG_COUNT = 1000

@cocotb.coroutine
def _init_dut(dut):

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0


@cocotb.coroutine
def _assert_signal_stays_low(clock, signal):
    while True:
        yield RisingEdge(clock)
        if signal.value != 0:
            raise TestFailure("Signal %s was %s (expected 0)"
                              % (signal._path, signal.value))

@cocotb.test()
def test_regaccess_pkg(dut):
    """
    Check if a register access packet passes through
    """

    yield _init_dut(dut)

    dut.out_reg_ready <= 1;
    dut.out_bypass_ready <= 1;

    reg_reader_signal_aliases = {'debug_out': 'out_reg',
                                 'debug_out_ready': 'out_reg_ready'}
    reg_reader = NocDiReader(dut, dut.clk, reg_reader_signal_aliases)

    writer_signal_aliases = { 'debug_in': 'in', 'debug_in_ready': 'in_ready'}
    writer = NocDiWriter(dut, dut.clk, writer_signal_aliases)

    reg_pkg = DiPacket()
    reg_pkg.set_contents(dest=1, src=0, type=DiPacket.TYPE.REG.value,
                         type_sub=DiPacket.TYPE_SUB.REQ_WRITE_REG_16.value,
                         payload=[0xdead])

    # send a register write packet to the DUT
    write_thread = cocotb.fork(writer.send_packet(reg_pkg))

    # ensure that the bypass valid signal isn't asserted
    checker_thread = cocotb.fork(_assert_signal_stays_low(dut.clk,
                                                          dut.out_bypass.valid))

    # get packet on reg output
    rcv_pkg = yield reg_reader.receive_packet()

    yield write_thread.join()
    checker_thread.kill()

    if not rcv_pkg:
        raise TestFailure("Register access packet not routed to output.")

    if not rcv_pkg.equal_to(dut, reg_pkg):
        raise TestFailure("Data corruption.")

@cocotb.test()
def test_only_regaccess_ready(dut):
    """
    Check if a register access packet passes through, even though the bypass
    ready signal is tied to 0.
    """

    yield _init_dut(dut)

    dut.out_reg_ready <= 1;
    dut.out_bypass_ready <= 0;

    reg_reader_signal_aliases = {'debug_out': 'out_reg',
                                 'debug_out_ready': 'out_reg_ready'}
    reg_reader = NocDiReader(dut, dut.clk, reg_reader_signal_aliases)

    writer_signal_aliases = { 'debug_in': 'in', 'debug_in_ready': 'in_ready'}
    writer = NocDiWriter(dut, dut.clk, writer_signal_aliases)

    reg_pkg = DiPacket()
    reg_pkg.set_contents(dest=1, src=0, type=DiPacket.TYPE.REG.value,
                         type_sub=DiPacket.TYPE_SUB.REQ_WRITE_REG_16.value,
                         payload=[0xdead])

    # send a register write packet to the DUT
    write_thread = cocotb.fork(writer.send_packet(reg_pkg))

    # ensure that the bypass valid signal isn't asserted
    checker_thread = cocotb.fork(_assert_signal_stays_low(dut.clk,
                                                          dut.out_bypass.valid))

    # get packet on reg output
    rcv_pkg = yield reg_reader.receive_packet()

    yield write_thread.join()
    checker_thread.kill()

    if not rcv_pkg:
        raise TestFailure("Register access packet not routed to output.")

    if not rcv_pkg.equal_to(dut, reg_pkg):
        raise TestFailure("Data corruption.")

@cocotb.test()
def test_event_pkg(dut):
    """
    Check if a register access packet passes through
    """

    yield _init_dut(dut)

    dut.out_reg_ready <= 1;
    dut.out_bypass_ready <= 1;

    bypass_reader_signal_aliases = {'debug_out': 'out_bypass',
                                    'debug_out_ready': 'out_bypass_ready'}
    bypass_reader = NocDiReader(dut, dut.clk, bypass_reader_signal_aliases)

    writer_signal_aliases = { 'debug_in': 'in', 'debug_in_ready': 'in_ready'}
    writer = NocDiWriter(dut, dut.clk, writer_signal_aliases)

    event_pkg = DiPacket()
    event_pkg.set_contents(dest=1, src=0, type=DiPacket.TYPE.EVENT.value,
                           type_sub=0, payload=[0xdead])

    # send an event packet to the DUT
    write_thread = cocotb.fork(writer.send_packet(event_pkg))

    # ensure that the register output valid signal isn't asserted
    checker_thread = cocotb.fork(_assert_signal_stays_low(dut.clk,
                                                          dut.out_reg.valid))

    # get packet on bypass output
    rcv_pkg = yield bypass_reader.receive_packet()

    yield write_thread.join()
    checker_thread.kill()

    if not rcv_pkg:
        raise TestFailure("Event access packet not routed to output.")

    if not rcv_pkg.equal_to(dut, event_pkg):
        raise TestFailure("Data corruption.")

_sent_pkgs = []

@cocotb.coroutine
def _toggle_ready(clock, ready_signal):
    while True:
        ready_signal <= random.randint(0, 1)
        yield RisingEdge(clock)

@cocotb.coroutine
def _create_pkgs(dut):
    writer_signal_aliases = { 'debug_in': 'in', 'debug_in_ready': 'in_ready'}
    writer = NocDiWriter(dut, dut.clk, writer_signal_aliases)

    for i in range(STRESS_TEST_PKG_COUNT):
        is_reg_pkg = random.randint(0, 1)
        pkg = DiPacket()
        if is_reg_pkg:
            pkg.set_contents(dest=1, src=0, type=DiPacket.TYPE.REG.value,
                             type_sub=DiPacket.TYPE_SUB.REQ_WRITE_REG_16.value,
                             payload=[0xdead])
        else:
            payload_len = random.randint(0, 5)
            payload = []
            for _ in range(payload_len):
                payload.append(random.randint(0, 2**16-1))

            pkg.set_contents(dest=1, src=0, type=DiPacket.TYPE.EVENT.value,
                             type_sub=0x3, payload=payload)

        # wait a random number of cycles before sending the packet
        for s in range(random.randint(0, 100)):
            yield RisingEdge(dut.clk)

        _sent_pkgs.append(pkg)
        yield writer.send_packet(pkg)

@cocotb.coroutine
def _check_pkgs(dut):
    bypass_reader_signal_aliases = {'debug_out': 'out_bypass',
                                    'debug_out_ready': 'out_bypass_ready'}
    bypass_reader = NocDiReader(dut, dut.clk, bypass_reader_signal_aliases)

    reg_reader_signal_aliases = {'debug_out': 'out_reg',
                                 'debug_out_ready': 'out_reg_ready'}
    reg_reader = NocDiReader(dut, dut.clk, reg_reader_signal_aliases)

    pkg_rcv_cnt = 0
    while True:
        yield RisingEdge(dut.clk)

        bypass_active = dut.out_bypass.valid.value and dut.out_bypass_ready.value
        reg_active = dut.out_reg.valid.value and dut.out_reg_ready.value

        if not (bypass_active or reg_active):
            continue

        if bypass_active and reg_active:
            raise TestFailure("Both out_*.valid signals are high. Invalid state.")

        if len(_sent_pkgs) == 0:
            raise TestFailure("Got valid signal even though no packet has been sent.");

        sent_pkg = _sent_pkgs.pop(0)


        if bypass_active:
            if sent_pkg.type == DiPacket.TYPE.REG.value:
                raise TestFailure("out_bypass.valid signal is high even though "
                                  "we have sent a REG packet: Sent packet was "
                                  "%s" % str(sent_pkg))
            rcv_pkg = yield bypass_reader.receive_packet()

        if reg_active:
            if not sent_pkg.type == DiPacket.TYPE.REG.value:
                raise TestFailure("out_reg.valid signal is high even though "
                                  "we have *not* sent a REG packet. Sent "
                                  "packet was %s" % str(sent_pkg))
            rcv_pkg = yield reg_reader.receive_packet()

        if not rcv_pkg:
            raise TestFailure("Packet lost.")

        if not rcv_pkg.equal_to(dut, sent_pkg):
            raise TestFailure("Data corruption.")

        pkg_rcv_cnt += 1

        if pkg_rcv_cnt == STRESS_TEST_PKG_COUNT:
            return


@cocotb.test()
def test_stress(dut):
    """
    Stress the implementation with different packet types, delays, and more
    """

    yield _init_dut(dut)

    checker_thread = cocotb.fork(_check_pkgs(dut))

    write_thread = cocotb.fork(_create_pkgs(dut))

    dut.out_reg_ready <= 1
    dut.out_bypass_ready <= 1

    toggle_out_reg_ready_thread = cocotb.fork(_toggle_ready(dut.clk, dut.out_reg_ready))
    toggle_out_bypass_ready_thread = cocotb.fork(_toggle_ready(dut.clk, dut.out_bypass_ready))


    yield write_thread.join()
    yield checker_thread.join()

    toggle_out_reg_ready_thread.kill()
    toggle_out_bypass_ready_thread.kill()


