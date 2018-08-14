"""
    test_osd_event_packetization_fixedwidth
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    :copyright: Copyright 2018 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge
from cocotb.binary import BinaryValue

from osdtestlib.debug_interconnect import NocDiReader, RegAccess, DiPacket

import random

# DI address of the tested module
MODULE_DI_ADDRESS = 1058

# DI address of the sending module
SENDER_DI_ADDRESS = 17


@cocotb.coroutine
def _init_dut(dut):
    # Dump design parameters for debugging
    dut._log.info("PARAMETER: MAX_PKT_LEN is %d" % dut.MAX_PKT_LEN.value.integer)
    dut._log.info("PARAMETER: DATA_WIDTH is %d" % dut.DATA_WIDTH.value.integer)

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Reset
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.test()
def test_fixedwidth(dut):
    """Test the generation of two packets with a byte padding"""
    yield _init_dut(dut)

    
    dut.id <= MODULE_DI_ADDRESS
    dut.dest <= SENDER_DI_ADDRESS
    
    # create data for two packets: one full packet, and one packet with 
    # only a single payload word
    payload_words_per_pkg = dut.MAX_PKT_LEN.value.integer - 3 # 3 header words
    payload_bytes = (payload_words_per_pkg + 1) * 2
    data_bytes = bytearray(random.getrandbits(8) for _ in range(payload_bytes))
    data_int = int.from_bytes(data_bytes, byteorder='little', signed=False)
    dut.data.value = BinaryValue(data_int)

    dut.overflow <= 0
    dut.event_available <= 1


    # packet 1
    exp_pkg1_payload = [data_bytes[i+1] << 8 | data_bytes[i] for i in range(0, payload_words_per_pkg * 2, 2)]
    exp_pkg = DiPacket()
    exp_pkg.set_contents(dest=SENDER_DI_ADDRESS,
                         src=MODULE_DI_ADDRESS,
                         type=DiPacket.TYPE.EVENT,
                         type_sub=1,
                         payload=exp_pkg1_payload)

    reader = NocDiReader(dut, dut.clk)
    rcv_pkg = yield reader.receive_packet(set_ready=True)

    if not rcv_pkg:
        raise TestFailure("No packet 1 generated!")

    if not rcv_pkg.equal_to(dut, exp_pkg):
        raise TestFailure("Received packet 1 doesn't match expected packet. "
                          "Got %s, expected %s" % (str(rcv_pkg), str(exp_pkg)))

    # packet 2
    yield RisingEdge(dut.clk)
    exp_pkg2_payload = [data_bytes[i+1] << 8 | data_bytes[i] for i in range(payload_words_per_pkg * 2, payload_bytes, 2)]
    exp_pkg = DiPacket()
    exp_pkg.set_contents(dest=SENDER_DI_ADDRESS,
                         src=MODULE_DI_ADDRESS,
                         type=DiPacket.TYPE.EVENT,
                         type_sub=0,
                         payload=exp_pkg2_payload)

    reader = NocDiReader(dut, dut.clk)
    rcv_pkg = yield reader.receive_packet(set_ready=True)

    if not rcv_pkg:
        raise TestFailure("No packet 2 generated!")

    if not rcv_pkg.equal_to(dut, exp_pkg):
        raise TestFailure("Received packet 2 doesn't match expected packet. "
                          "Got %s, expected %s" % (str(rcv_pkg), str(exp_pkg)))

    # check if the DUT is ready for the next event transfer
    if not dut.event_consumed.value:
        raise TestFailure("DUT does not indicate that the event has been "
                          "consumed.")
