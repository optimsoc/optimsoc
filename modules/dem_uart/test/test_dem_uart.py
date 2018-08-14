"""
test_dem_uart
    
Cocotb-based unit test for UART-Device Emulation Module (DEM UART)
"""

import random
import cocotb

from cocotb.clock import Clock
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import RegAccess, DiPacket, NocDiReader, NocDiWriter
from osdtestlib.asserts import *
from osdtestlib.exceptions import DebugEventFailedException

# Debug Interconnect address for the DEM module
MODULE_DI_ADDRESS = 1

# Debug interconnect address for the testing module
SENDER_DI_ADDRESS = 0

# Mirroring expected transfer through module
_dem_to_di_fifo = []
_di_to_dem_fifo = []


@cocotb.coroutine
def _init_dut(dut):

    #Setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())

    # Reset DUT
    dut._log.debug("Resetting DUT");
    dut.rst <= 1

    dut.id <= MODULE_DI_ADDRESS

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0

@cocotb.test()
def test_dem_uart_baseregisters(dut):
    """
    Verify that the base configuration registers have the correct values.
    """
    access = RegAccess(dut)

    yield _init_dut(dut)

    yield access.test_base_registers(MODULE_DI_ADDRESS, SENDER_DI_ADDRESS,
                                     mod_vendor=1, mod_type=5, mod_version=0,
                                     can_stall=True)

@cocotb.coroutine
def _activate_module(dut):

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

    _dem_to_di_fifo.clear()

    write_thread = cocotb.fork(_dem_to_di_tx(dut, num_transfers=5))

    read_thread = cocotb.fork(_dem_to_di_rx(dut, num_transfers=5))
    yield read_thread.join()

    yield access.write_register(dest=MODULE_DI_ADDRESS, src=SENDER_DI_ADDRESS,
                                word_width=16,
                                regaddr=DiPacket.BASE_REG.MOD_CS.value,
                                value=0)

    write_thread = cocotb.fork(_dem_to_di_tx(dut, num_transfers=5))

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
    
    read_thread = cocotb.fork(_dem_to_di_rx(dut, num_transfers=5))
    yield read_thread.join()

@cocotb.coroutine
def _dem_to_di_tx(dut, num_transfers=500, max_delay=100, random_data=False):

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        data = random.randint(0, 255) if random_data else 0x42

        _dem_to_di_fifo.append(data)

        dut.out_valid <= 0x1
        dut.out_char <= data

        # out_valid must be asserted for at least one clock cycle
        yield RisingEdge(dut.clk)

        while not dut.out_ready.value:
            yield RisingEdge(dut.clk)

        dut.out_valid <= 0

@cocotb.coroutine
def _dem_to_di_rx(dut, num_transfers=500, max_delay=100):

    reader    = NocDiReader(dut, dut.clk);
    ex_packet = DiPacket();

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        rx_packet = yield reader.receive_packet(set_ready=True)

        data = _dem_to_di_fifo.pop(0)

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
def test_dem_to_di(dut):
    """
    Test the communication from the DEM to the DI
    """
    yield _init_dut(dut)
    yield _activate_module(dut)

    _dem_to_di_fifo.clear()

    write_thread = cocotb.fork(_dem_to_di_tx(dut))
    read_thread  = cocotb.fork(_dem_to_di_rx(dut))

    # read_thread implicitly waits for write thread to finish as well
    yield read_thread.join()

@cocotb.coroutine
def _di_to_dem_tx(dut, num_transfers=100, max_delay=100, random_data=False):

    writer = NocDiWriter(dut, dut.clk);
    tx_packet = DiPacket();

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        data = random.randint(0, 255) if random_data else 0x42

        _di_to_dem_fifo.append(data)

        tx_packet.set_contents(dest=MODULE_DI_ADDRESS, src=SENDER_DI_ADDRESS,
                               type=DiPacket.TYPE.EVENT.value, type_sub=0,
                               payload=[data])

        yield writer.send_packet(tx_packet)
        
        yield RisingEdge(dut.clk)

@cocotb.coroutine
def _di_to_dem_rx(dut, num_transfers=100, max_delay=100):

    for i in range(num_transfers):
        delay = random.randint(0, max_delay)

        for _ in range(delay):
            yield RisingEdge(dut.clk)

        dut.in_ready <= 0x1

        yield RisingEdge(dut.clk)

        while not dut.in_valid.value:
            yield RisingEdge(dut.clk)

        data = _di_to_dem_fifo.pop(0)

        assert_signal_value(dut.in_char, data)

        dut.in_ready <= 0x0

@cocotb.test()
def test_di_to_dem(dut):
    """
    Test the communication from the DI to the DEM
    """
    yield _init_dut(dut)
    yield _activate_module(dut)

    _di_to_dem_fifo.clear()

    write_thread = cocotb.fork(_di_to_dem_tx(dut))
    read_thread  = cocotb.fork(_di_to_dem_rx(dut))

    yield read_thread.join()

@cocotb.test()
def test_both_directions(dut):
    """
    Test communication in both directions concurrently
    """
    yield _init_dut(dut)
    yield _activate_module(dut)

    _dem_to_di_fifo.clear()
    _di_to_dem_fifo.clear()

    di_write_t  = cocotb.fork(_di_to_dem_tx(dut, num_transfers=1000, random_data=True))
    dem_write_t = cocotb.fork(_dem_to_di_tx(dut, num_transfers=1000, random_data=True))
    dem_read_t  = cocotb.fork(_di_to_dem_rx(dut, num_transfers=1000))
    di_read_t   = cocotb.fork(_dem_to_di_rx(dut, num_transfers=1000))

    yield dem_read_t.join()
    yield di_read_t.join()

