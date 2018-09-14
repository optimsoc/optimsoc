"""
test_dem_uart_wb

Cocotb-based unit test for UART-Device Emulation Module (DEM UART)
"""

import random
import cocotb

from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, ClockCycles

from osdtestlib.debug_interconnect import RegAccess, DiPacket, NocDiReader, NocDiWriter
from osdtestlib.asserts import *
from osdtestlib.exceptions import DebugEventFailedException
from osdtestlib.wishbone import WishboneMaster, WBOp, WBRes

# Debug Interconnect address for the DEM module
MODULE_DI_ADDRESS = 1

# Debug interconnect address for the testing module
SENDER_DI_ADDRESS = 0

# Mirroring expected transfer through module
_bus_to_di_fifo = []
_di_to_bus_fifo = []


@cocotb.coroutine
def _init_dut(dut):

    #Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Reset DUT
    dut._log.debug("Resetting DUT")
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.coroutine
def _activate_module(dut):
    """
    Set the MOD_CS_ACTIVE bit in the Control and Status register to 1 to
    enable transmission of debug event packets.
    """

    access = RegAccess(dut)

    yield access.write_register(dest=MODULE_DI_ADDRESS,
                                src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.BASE_REG.MOD_CS.value,
                                value=1)

@cocotb.test()
def test_dem_uart_activation(dut):
    """
    Check if DEM_UART is handling the activation bit correctly
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

    _bus_to_di_fifo.clear()

    write_thread = cocotb.fork(_bus_to_di_tx(dut, num_transfers=5))

    read_thread = cocotb.fork(_bus_to_di_rx(dut, num_transfers=5))
    yield read_thread.join()

    yield access.write_register(dest=MODULE_DI_ADDRESS, src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.BASE_REG.MOD_CS.value,
                                value=0)

    write_thread = cocotb.fork(_bus_to_di_tx(dut, num_transfers=5))

    # Wait one cycle to make sure the reader below doesn't accidentally try to
    # receive parts of the RegAccess response packet
    yield RisingEdge(dut.clk)

    packet = yield NocDiReader(dut, dut.clk).receive_packet(set_ready=True)
    if packet:
        raise TestFailure("Received packet while module was deactivated")

    yield access.write_register(dest=MODULE_DI_ADDRESS, src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.BASE_REG.MOD_CS.value,
                                value=1)

    read_thread = cocotb.fork(_bus_to_di_rx(dut, num_transfers=5))
    yield read_thread.join()

@cocotb.coroutine
def _bus_to_di_tx(dut, num_transfers=500, max_delay=100, random_data=False):

    wb_master = WishboneMaster(dut, dut.clk)

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        data = random.randint(0, 255) if random_data else 0x42

        _bus_to_di_fifo.append(data)

        yield wb_master.send_cycle([WBOp(adr=0x0, dat=data, sel=0x8)])

@cocotb.coroutine
def _bus_to_di_rx(dut, num_transfers=500, max_delay=100):

    reader    = NocDiReader(dut, dut.clk);
    ex_packet = DiPacket();

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        rx_packet = yield reader.receive_packet(set_ready=True)

        data = _bus_to_di_fifo.pop(0)

        ex_packet.set_contents(dest=SENDER_DI_ADDRESS, src=MODULE_DI_ADDRESS,
                               type=DiPacket.TYPE.EVENT, type_sub=0,
                               payload=[data])

        if not rx_packet:
            raise TestFailure("receive_packet() timed out")

        if not rx_packet.equal_to(dut, ex_packet, mask=None):
            raise TestFailure("Unexpected content of " + rx_packet.__str__() +
                              "\n Expected " + ex_packet.__str__())

        yield RisingEdge(dut.clk)

@cocotb.test()
def test_bus_to_di(dut):
    """
    Test communication from a WISHBONE bus to the DI
    """

    yield _init_dut(dut)
    yield _activate_module(dut)

    _bus_to_di_fifo.clear()

    write_thread = cocotb.fork(_bus_to_di_tx(dut, num_transfers=1000, max_delay=100, random_data=True))
    read_thread  = cocotb.fork(_bus_to_di_rx(dut, num_transfers=1000, max_delay=100))

    # read_thread implicitly waits for write thread to finish as well
    yield read_thread.join()

@cocotb.coroutine
def _di_to_bus_tx(dut, num_transfers=500, max_delay=100, random_data=False):

    writer = NocDiWriter(dut, dut.clk);
    tx_packet = DiPacket();

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        data = random.randint(0, 255) if random_data else 0x42

        _di_to_bus_fifo.append(data)

        tx_packet.set_contents(dest=MODULE_DI_ADDRESS, src=SENDER_DI_ADDRESS,
                               type=DiPacket.TYPE.EVENT.value, type_sub=0,
                               payload=[data])

        yield writer.send_packet(tx_packet)

        yield RisingEdge(dut.clk)

@cocotb.coroutine
def _di_to_bus_rx(dut, num_transfers=500, max_delay=100):

    wb_master = WishboneMaster(dut, dut.clk)

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        # Only look at the LSB of the 32-bit wide result.
        ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x8)])
        res = ret.pop(0).datrd & 0xFF

        data = _di_to_bus_fifo.pop(0);

        if data != res:
           raise TestFailure("Expected result to be 0x%x, got 0x%x" %
                             (data, res))

        yield RisingEdge(dut.clk)

@cocotb.test()
def test_di_to_bus(dut):
    """
    Test communication from the DI to a WISHBONE bus
    """

    yield _init_dut(dut)
    yield _activate_module(dut)

    _di_to_bus_fifo.clear()

    write_thread = cocotb.fork(_di_to_bus_tx(dut, num_transfers=1000, max_delay=100, random_data=True))
    read_thread  = cocotb.fork(_di_to_bus_rx(dut, num_transfers=1000, max_delay=100))

    yield read_thread.join()

@cocotb.test()
def test_both_directions(dut):
    """
    Randomly alternate between read/write cycles on the WISHBONE bus
    """

    NUM_TRANSFERS = 10000
    MAX_DELAY     = 100
    RANDOM_DATA   = True

    yield _init_dut(dut)
    yield _activate_module(dut)

    _bus_to_di_fifo.clear()
    _di_to_bus_fifo.clear()

    wb_master = WishboneMaster(dut, dut.clk)

    rx_packets = NUM_TRANSFERS
    tx_packets = NUM_TRANSFERS

    write_thread = cocotb.fork(_di_to_bus_tx(dut, num_transfers=NUM_TRANSFERS,
                                             max_delay=MAX_DELAY,
                                             random_data=RANDOM_DATA))
    read_thread = cocotb.fork(_bus_to_di_rx(dut, num_transfers=NUM_TRANSFERS,
                                            max_delay=MAX_DELAY))

    # randomly pick a direction and send/receive any remaining packets
    while tx_packets or rx_packets:
        if random.randint(0,1):
            if not tx_packets:
                continue

            for _ in range(random.randint(0, MAX_DELAY)):
                yield RisingEdge(dut.clk)

            tx_packets -= 1

            data = random.randint(0, 255) if RANDOM_DATA else 0x42
            _bus_to_di_fifo.append(data)

            yield wb_master.send_cycle([WBOp(adr=0x0, dat=data, sel=0x8)])

            yield RisingEdge(dut.clk)
        else:
            if not rx_packets:
                continue

            for _ in range(random.randint(0, MAX_DELAY)):
                yield RisingEdge(dut.clk)

            rx_packets -= 1

            ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x8)])
            res = ret.pop(0).datrd & 0xFF

            data = _di_to_bus_fifo.pop(0)

            if data != res:
                raise TestFailure("Expected result to be 0x%x, got 0x%x"
                                  % (data, res))

            yield RisingEdge(dut.clk)

    # we implicitly wait for the write_thread in the loop above
    yield read_thread.join()

@cocotb.test()
def test_uart_16550_registers(dut):
    """
    Test accessibility and functionality of the implemented UART registers, as
    well as their correct reset values.
    """

    yield _init_dut(dut)
    yield _activate_module(dut)

    wb_master = WishboneMaster(dut, dut.clk)

    # IER, verify reset values and persistence
    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x4)])
    res = ret.pop(0).datrd & 0xFF

    if 0 != res:
        raise TestFailure("IER test failed! Wrong reset values: 0x%x" % res)

    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x3, sel=0x4)])

    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x4)])
    res = ret.pop(0).datrd & 0xFF

    if res != 0x3:
        raise TestFailure("IER test failed! Written value not stored, Wrote: 0x%x, Read: 0x%x"
                          % (0x3, res))

    # IIR (Read Only) & FCR (Write Only), make sure the FIFOs can be enabled
    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x1, sel=0x2)])

    # Only check if the FIFOs have been enabled correctly
    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x2)])
    res = ret.pop(0).datrd & 0xC0

    if res != 0xC0:
        raise TestFailure("FCR test failed! FIFOs not enabled. 0x%x" % res)

    # LCR, make sure DLAB can be set
    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x1)])
    res = ret.pop(0).datrd

    if res != 0x0:
        raise TestFailure("LCR test failed! Wrong reset values: 0x%x" % res)

    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x80, sel=0x1)])

    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x1)])
    res = ret.pop(0).datrd & 0xFF

    if res != 0x80:
        raise TestFailure("LCR test failed! Written value not stored, Wrote: 0x%x, Read: 0x%x"
                          % (0x80, res))

    # LSR (Read Only), verify correct reset value
    ret = yield wb_master.send_cycle([WBOp(adr=0x4, dat=None, sel=0x4)])
    res = ret.pop(0).datrd & 0xFF

    if res != 0x60:
        raise TestFailure("LSR test failed! Wrong reset values: 0x%x" % res)

    # DLM & DLL, verify accessibility
    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x80, sel=0x1)])

    # Set Baudrate to 115200 (LSB first)
    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x1, sel=0x8)])
    yield wb_master.send_cycle([WBOp(adr=0x0, dat=0x0, sel=0x4)])

    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x8)])
    res = ret.pop(0).datrd & 0xFF

    if res != 0x1:
        raise TestFailure("DLL test failed! Wrote: 0x%x, Read: 0x%x"
                          % (0x1, res))

    ret = yield wb_master.send_cycle([WBOp(adr=0x0, dat=None, sel=0x4)])
    res = ret.pop(0).datrd & 0xFF

    if res != 0x0:
        raise TestFailure("DLM test failed! Wrote: 0x%x, Read: 0x%x"
                          % (0x0, res))
