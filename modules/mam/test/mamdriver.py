"""
    mamdriver
    ~~~~~~~~~

    Driver for the Memory Access Module (MAM) on the DI and System Interfaces

    :copyright: Copyright 2018 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.result import TestFailure
from cocotb.clock import RisingEdge

from osdtestlib.debug_interconnect import DiPacket, NocDiWriter, NocDiReader
from osdtestlib.asserts import assert_signal_value

from math import ceil


class MemoryTransfer:
    """
    A single memory transfer
    """

    def __init__(self, AW=32, DW=32):
        self.AW = AW
        self.DW = DW

        self.addr = 0x0000
        self.operation = 'write'
        self.burst = False
        self.byteselect = 0
        self.sync = False
        self.data = bytearray()

    def __str__(self):
        return ("AW: %u, DW: %u, operation: %s, addr: %u, burst: %r, "
                "byteselect: 0x%x, sync: %r, data: %s" %
                (self.AW, self.DW, self.operation, self.addr, self.burst,
                 self.byteselect, self.sync, self.data.hex()))


class MamDiDriver:
    """
    MAM memory transfer driver on the Debug Interconnect
    """

    def __init__(self, entity, clock, di_writer=None, di_reader=None,
                 MAX_PKT_LEN=12, MODULE_DI_ADDRESS=1, SENDER_DI_ADDRESS=0):
        self.entity = entity
        self.clock = clock
        self.log = entity._log
        self.MAX_PKT_LEN = MAX_PKT_LEN
        self.MODULE_DI_ADDRESS = MODULE_DI_ADDRESS
        self.SENDER_DI_ADDRESS = SENDER_DI_ADDRESS

        if di_writer:
            self.di_writer = di_writer
        else:
            self.di_writer = NocDiWriter(entity, clock)

        if di_reader:
            self.di_reader = di_reader
        else:
            self.di_reader = NocDiReader(entity, clock)

    def _create_mam_transfer(self, mem_transfer):
        """
        Get MAM transfer bytearray representing the memory transfer 
        """
        mam_transfer = bytearray()

        we = (mem_transfer.operation == 'write')

        selsize = 0
        if mem_transfer.burst:
            selsize = int(len(mem_transfer.data) / (mem_transfer.DW / 8))
        else:
            selsize = mem_transfer.byteselect
        if selsize > 0xFF:
            raise TestFailure("selsize overflow detected")

        hdr0 = 0
        hdr0 |= (we & 0x1) << 7
        hdr0 |= (mem_transfer.burst & 0x1) << 6
        hdr0 |= (mem_transfer.sync & 0x1) << 5

        hdr1 = selsize

        mam_transfer.append(hdr0)
        mam_transfer.append(hdr1)
        mam_transfer.extend(mem_transfer.addr.to_bytes(int(mem_transfer.AW / 8),
                                                       byteorder='big'))

        if mem_transfer.operation == 'write' and mem_transfer.data:
            mam_transfer.extend(mem_transfer.data)

        return mam_transfer

    def _create_di_pkgs(self, mam_transfer):
        """
        Get DI packages representing the memory transfer
        """
        pkgs = []

        max_payload_bytes = (self.MAX_PKT_LEN - 3) * 2
        number_of_pkgs = ceil(len(mam_transfer) / max_payload_bytes)

        b = 0
        for _ in range(number_of_pkgs):
            pkg = DiPacket()
            pkg.dest = self.MODULE_DI_ADDRESS
            pkg.src = self.SENDER_DI_ADDRESS
            pkg.type = DiPacket.TYPE.EVENT.value
            pkg.type_sub = 0

            for i in range(int(max_payload_bytes / 2)):
                payload_word = mam_transfer[b] << 8 | mam_transfer[b + 1]
                pkg.payload.append(payload_word)

                b += 2
                if b >= len(mam_transfer):
                    break

            pkgs.append(pkg)

        return pkgs

    @cocotb.coroutine
    def drive(self, mem_transfer):
        """
        Drive the Debug Interconnect with a memory transfer and check the
        results
        """

        # request
        mam_transfer_request = self._create_mam_transfer(mem_transfer)
        req_pkgs = self._create_di_pkgs(mam_transfer_request)
        for pkg in req_pkgs:
            yield self.di_writer.send_packet(pkg)

        # response
        if mem_transfer.operation == 'read':
            rcv_data = bytearray()

            while len(rcv_data) < len(mem_transfer.data):
                yield RisingEdge(self.clock)
                pkg = yield self.di_reader.receive_packet(set_ready=True)

                self.log.debug("Received memory read response " + str(pkg))
                for payload_word in pkg.payload:
                    rcv_data.extend(payload_word.to_bytes(2, byteorder='big'))

            # check received data
            if rcv_data != mem_transfer.data:
                raise TestFailure("Got invalid data.\nExpected: %s\nReceived: %s" %
                                  (mem_transfer.data.hex(), rcv_data.hex()))

        # for synchronous writes: check if we received a acknowledgement packet
        if mem_transfer.operation == 'write' and mem_transfer.sync:
            pkg = yield self.di_reader.receive_packet(set_ready=True)
            exp_sync_pkg = DiPacket()
            exp_sync_pkg.set_contents(self.SENDER_DI_ADDRESS,
                                      self.MODULE_DI_ADDRESS,
                                      DiPacket.TYPE.EVENT.value, 0, [])
            if not pkg.equal_to(self.entity, exp_sync_pkg):
                raise TestFailure(
                    "Acknowledgement packet for sync write invalid.")


class MamSysifDriver:
    """
    MAM memory transfer driver on the MAM system interface
    """

    def __init__(self, entity, clock):
        self.entity = entity
        self.clock = clock
        self.log = entity._log

    @cocotb.coroutine
    def _check_mam_request(self, mem_transfer):
        self.entity.req_ready <= 1

        while not self.entity.req_valid.value:
            yield RisingEdge(self.clock)

        self.entity.req_ready <= 0

        we = (mem_transfer.operation == 'write')
        assert_signal_value(self.entity.req_we, we)
        assert_signal_value(self.entity.req_addr, mem_transfer.addr)
        assert_signal_value(self.entity.req_burst, mem_transfer.burst)
        assert_signal_value(self.entity.req_sync, mem_transfer.sync)

        exp_data_words = int(len(mem_transfer.data) / (mem_transfer.DW / 8))

        if mem_transfer.burst:
            assert_signal_value(self.entity.req_beats, exp_data_words)

    @cocotb.coroutine
    def _check_mam_write(self, mem_transfer):
        exp_data_words = int(len(mem_transfer.data) / (mem_transfer.DW / 8))
        self.log.debug("Expecting a memory access of %d words." %
                       exp_data_words)

        wr_data = bytearray()

        self.entity.write_ready <= 1

        for _ in range(exp_data_words):
            yield RisingEdge(self.clock)
            while not self.entity.write_valid.value:
                yield RisingEdge(self.clock)

            wr_data_int = self.entity.write_data.value.integer
            wr_data.extend(wr_data_int.to_bytes(int(mem_transfer.DW / 8),
                                                byteorder='big'))

        # check if we're back to initial state, all data has been transferred
        yield RisingEdge(self.clock)
        assert_signal_value(self.entity.write_valid, 0)
        assert_signal_value(self.entity.req_valid, 0)

        self.entity.write_ready <= 0

        # check transferred data
        if wr_data != mem_transfer.data:
            raise TestFailure("Got invalid data.\nSent:     %s\nReceived: %s" %
                              (mem_transfer.data.hex(), wr_data.hex()))

    @cocotb.coroutine
    def _drive_mam_read(self, mem_transfer):
        dw_b = int(mem_transfer.DW / 8)
        numwords = int(len(mem_transfer.data) / dw_b)

        for w in range(numwords):
            # XXX: obey endianness of bus here. currently big endian
            dword = 0
            for b in range(dw_b):
                dword |= mem_transfer.data[w * dw_b + b] << ((dw_b - b - 1) * 8)
            self.entity.read_data <= dword
            self.entity.read_valid <= 1
            yield RisingEdge(self.clock)

            while not self.entity.read_ready.value:
                yield RisingEdge(self.clock)

    @cocotb.coroutine
    def drive(self, mem_transfer):
        yield self._check_mam_request(mem_transfer)

        if mem_transfer.operation == 'write':
            yield self._check_mam_write(mem_transfer)
        else:
            yield self._drive_mam_read(mem_transfer)
