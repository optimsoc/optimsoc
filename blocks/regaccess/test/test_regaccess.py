"""
    test_regaccess
    ~~~~~~~~

    Cocotb-based unit test for the regaccess Module

    :copyright: Copyright 2018 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import NocDiWriter, NocDiReader, RegAccess, DiPacket
from osdtestlib.exceptions import *

import random

# DUT parameter
_mod_vendor = 0
_mod_type = 0
_mod_version = 0
_mod_event_dest_default = 0
_can_stall = 0
_max_reg_size = 0

# DI address of the tested STM module
MODULE_DI_ADDRESS = 1

# DI address of the sending module
SENDER_DI_ADDRESS = 0

# expected write value
_write_value = 0

# requested register address and width
_req_addr = 0
_req_width = 0

# requested access mode
_req_write = False

@cocotb.coroutine
def _init_dut(dut):

    global _mod_vendor, _mod_type, _mod_version, _mod_event_dest_default, _can_stall, _max_reg_size

    # Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Dump design parameters for debugging
    _mod_vendor = dut.MOD_VENDOR.value.integer
    _mod_type = dut.MOD_TYPE.value.integer
    _mod_version = dut.MOD_VERSION.value.integer
    _mod_event_dest_default = dut.MOD_EVENT_DEST_DEFAULT.value.integer
    _can_stall = dut.CAN_STALL.value.integer
    _max_reg_size = dut.MAX_REG_SIZE.value.integer
    dut._log.info("PARAMETER: MOD_VENDOR is {}".format(_mod_vendor))
    dut._log.info("PARAMETER: MOD_TYPE is {}".format(_mod_type))
    dut._log.info("PARAMETER: MOD_VERSION is {}".format(_mod_version))
    dut._log.info("PARAMETER: MOD_EVENT_DEST_DEFAULT is {}".format(_mod_event_dest_default))
    dut._log.info("PARAMETER: CAN_STALL is {}".format(_can_stall))
    dut._log.info("PARAMETER: MAX_REG_SIZE is {}".format(_max_reg_size))

    # Reset
    dut._log.info("\nResetting DUT\n")
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0


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


@cocotb.coroutine
def _debug_module_dummy(dut):
    """
    Represents the external registers the osd_regaccess module writes to and
    reads from.
    """

    global _write_value, _req_addr, _req_width, _req_write

    registers = {}

    dut.reg_ack <= 0
    dut.reg_err <= 0
    dut.reg_rdata <= 0

    req_reg_width = [16, 32, 64, 128]

    while True:
        yield RisingEdge(dut.clk)

        if dut.reg_request.value.integer:
            req_addr = dut.reg_addr.value.integer
            req_width = req_reg_width[dut.reg_size.value.integer]
            req_write = dut.reg_write.value

            if req_addr != _req_addr or req_width != _req_width or req_write != _req_write:
                raise TestFailure("Request mismatch!\n" +
                                  "Register address: expected 0x%x, read 0x%x\n" %\
                                  (_req_addr, req_addr) +
                                  "Register width: expected %d, read %d\n" %\
                                  (_req_width, req_width) +
                                  "Write mode: expected %s, read %s" %\
                                  (_req_write, req_write))

            if req_write:
                req_value = dut.reg_wdata.value.integer

                if req_value != _write_value:
                    raise TestFailure("Value mismatch! expected 0x%x, read 0x%x" %\
                                      (_write_value, req_value))

                registers[req_addr] = req_value
                dut.reg_ack <= 1

            else:
                dut.reg_rdata <= registers[req_addr]
                dut.reg_ack <= 1

        else:
            dut.reg_ack <= 0
            dut.reg_rdata <= 0


@cocotb.coroutine
def _trigger_reg_wr_req_err(dut, dest, src, word_width, regaddr, value):
    """
    Sends a register write request to the module and checks if an error response
    gets returned.
    """

    tx_packet = DiPacket()
    rx_packet = DiPacket()
    writer = NocDiWriter(dut, dut.clk)
    reader = NocDiReader(dut, dut.clk)

    if word_width == 16:
        type_sub = DiPacket.TYPE_SUB.REQ_WRITE_REG_16.value
        words = 1
    elif word_width == 32:
        type_sub = DiPacket.TYPE_SUB.REQ_WRITE_REG_32.value
        words = 2
    elif word_width == 64:
        type_sub = DiPacket.TYPE_SUB.REQ_WRITE_REG_64.value
        words = 4
    elif word_width == 128:
        type_sub = DiPacket.TYPE_SUB.REQ_WRITE_REG_128.value
        words = 8
    else:
        raise TestFailure("An invalid register width parameter was chosen! (%d)" %\
                           word_width)

    # Assemble payload of REG debug packet
    payload = [regaddr]
    for w in range(0, words):
        payload.append((value >> ((words - 1 - w) * 16)) & 0xFFFF)

    tx_packet.set_contents(dest=dest, src=src,
                           type=DiPacket.TYPE.REG.value,
                           type_sub=type_sub, payload=payload)

    yield writer.send_packet(tx_packet)

    rx_packet = yield reader.receive_packet(set_ready=True)

    if not rx_packet:
        raise TestFailure("No response packet received!")

    if rx_packet.type_sub != DiPacket.TYPE_SUB.RESP_WRITE_REG_ERROR.value:
        raise TestFailure(
            "Register write did not return RESP_WRITE_REG_ERROR "
            "when writing 0x%x to register 0x%x of module 0x%x."
            % (value, regaddr, dest))


@cocotb.coroutine
def _trigger_reg_rd_req_err(dut, dest, src, word_width, regaddr):
    """
    Sends a register read request to the module and checks if an error response
    gets returned.
    """

    tx_packet = DiPacket()
    rx_packet = DiPacket()
    writer = NocDiWriter(dut, dut.clk)
    reader = NocDiReader(dut, dut.clk)

    if word_width == 16:
        type_sub = DiPacket.TYPE_SUB.REQ_READ_REG_16.value
    elif word_width == 32:
        type_sub = DiPacket.TYPE_SUB.REQ_READ_REG_32.value
    elif word_width == 64:
        type_sub = DiPacket.TYPE_SUB.REQ_READ_REG_64.value
    elif word_width == 128:
        type_sub = DiPacket.TYPE_SUB.REQ_READ_REG_128.value
    else:
        raise TestFailure("An invalid register width parameter was chosen! (%d)" %\
                           word_width)

    tx_packet.set_contents(dest=dest, src=src,
                           type=DiPacket.TYPE.REG.value,
                           type_sub=type_sub, payload=[regaddr])

    yield writer.send_packet(tx_packet)

    rx_packet = yield reader.receive_packet(set_ready=True)

    if not rx_packet:
        raise TestFailure("No response packet received!")

    if rx_packet.type_sub != DiPacket.TYPE_SUB.RESP_READ_REG_ERROR.value:
        raise TestFailure(
            "Register read did not return RESP_READ_REG_ERROR "
            "when reading from register 0x%x of module 0x%x." % (regaddr, dest))


@cocotb.test()
def test_mod_cs_activation(dut):
    """
    Check if regaccess is handling the activation bit correctly
    """
    access = RegAccess(dut)

    yield _init_dut(dut)

    dut._log.info("Check contents of MOD_CS")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.BASE_REG.MOD_CS.value, 16, 0)

    yield _activate_module(dut)

    dut._log.info("Check contents of MOD_CS")
    yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                  DiPacket.BASE_REG.MOD_CS.value, 16, 1)


@cocotb.test()
def test_access_base_registers(dut):
    """
    Check if access to base registers works properly
    """

    global _mod_vendor, _mod_type, _mod_version, _mod_event_dest_default, _can_stall, _max_reg_size

    access = RegAccess(dut)

    yield _init_dut(dut)
    yield access.test_base_registers(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                     mod_vendor=_mod_vendor, mod_type=_mod_type, mod_version=_mod_version,
                                     can_stall=_can_stall)


@cocotb.test()
def test_access_ext_registers(dut):
    """
    Check if access to external registers works properly
    """

    global _max_reg_size, _write_value, _req_addr, _req_width, _req_write

    access = RegAccess(dut)

    yield _init_dut(dut)

    # start thread that works as debug module
    debug_module = cocotb.fork(_debug_module_dummy(dut))

    # different register widths to test
    req_reg_width = [16, 32, 64, 128]

    dut._log.info("Run write/read test of external registers")
    # write and read 2000 random values to random external registers
    for _ in range(200):
        # addresses 0x0 - 0x1ff are handled internally. Test adresses 0x200 - 0xffff
        _req_addr = random.randint(2**9, 2**16-1)
        _req_width = req_reg_width[random.randint(0, 3)]
        # ensure required width is not larger than MAX_REG_SIZE
        _req_width = _req_width if _req_width <= _max_reg_size else _max_reg_size
        _write_value = random.randint(0, 2**_req_width-1)

        _req_write = True

        # write to register
        yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=_req_width,
                                regaddr=_req_addr,
                                value=_write_value)

        # wait a short random time
        wait = random.randint(0, 10)
        for _ in range(wait):
            yield RisingEdge(dut.clk)

        _req_write = False

        # check if value was correctly written to debug module
        yield access.assert_reg_value(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                      _req_addr, _req_width, _write_value)


@cocotb.test()
def test_error_cases(dut):
    """
    Trigger some error cases to check the module's behaviour.
    """

    access = RegAccess(dut)

    # different register widths to test
    req_reg_width = [16, 32, 64, 128]
    # internal address for error test case
    int_reg = 0x100
    # external address for error test case
    ext_reg = 0x200

    yield _init_dut(dut)

    # set reg_err high as the "debug module" returns an error for all requests in this test
    dut.reg_err <= 1

    dut._log.info("Trigger register write response: error...")
    dut._log.debug("by writing to illegal internal register address...")
    for reg_size in req_reg_width:
        if reg_size > _max_reg_size:
            break
        yield _trigger_reg_wr_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, reg_size, int_reg, 0xdead)

    dut._log.debug("by writing to valid internal address with word width > 16 bit...")
    for reg_size in req_reg_width[1:]:
        if reg_size > _max_reg_size:
            break
        yield _trigger_reg_wr_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, reg_size, 4, 0xdead)

    dut._log.debug("by writing to external register address with word width > MAX_REG_SIZE...")
    widths = [w for w in req_reg_width if w > _max_reg_size]
    if len(widths) == 0:
        dut._log.debug("...no tests to be done.")
    else:
        for w in widths:
            yield _trigger_reg_wr_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, w, ext_reg, 0xdead)

    dut._log.debug("by raising reg_err from the debug module side...")
    for reg_size in req_reg_width:
        if reg_size > _max_reg_size:
            break
        yield _trigger_reg_wr_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, reg_size, ext_reg, 0xdead)

    dut._log.info("Trigger register read response: error...")
    dut._log.debug("by reading from illegal internal register address...")
    for reg_size in req_reg_width:
        if reg_size > _max_reg_size:
            break
        yield _trigger_reg_rd_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, reg_size, int_reg)

    dut._log.debug("by reading from external register address with word width > MAX_REG_SIZE...")
    widths = [w for w in req_reg_width if w > _max_reg_size]
    if len(widths) == 0:
        dut._log.debug("...no tests to be done.")
    else:
        for w in widths:
            yield _trigger_reg_rd_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, w, ext_reg)

    dut._log.debug("by raising reg_err from the debug module side...")
    for reg_size in req_reg_width:
        if reg_size > _max_reg_size:
            break
        yield _trigger_reg_rd_req_err(dut, MODULE_DI_ADDRESS, SENDER_DI_ADDRESS, reg_size, ext_reg)
