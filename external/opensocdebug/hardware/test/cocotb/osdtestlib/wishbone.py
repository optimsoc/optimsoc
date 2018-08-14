"""
Cocotb is licensed under the Revised BSD License.  Full license text below.

###############################################################################
# Copyright (c) 2013 Potential Ventures Ltd
# Copyright (c) 2013 SolarFlare Communications Inc
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Potential Ventures Ltd,
#       SolarFlare Communications Inc nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL POTENTIAL VENTURES LTD BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###############################################################################
"""
import cocotb

from cocotb.decorators import coroutine
from cocotb.triggers import RisingEdge, Event
from cocotb.drivers import BusDriver
from cocotb.result import ReturnValue, TestFailure
from cocotb.decorators import public

from osdtestlib.debug_interconnect import AliasBusDriver

def is_sequence(arg):
    return (not hasattr(arg, "strip")
            and hasattr(arg, "__getitem__")
            or hasattr(arg, "__iter__"))

class WBAux():
    """
    Wrap meta informations on bus transaction (internal only)
    """
    def __init__(self, sel=0xf, adr=0, datwr=None, waitIdle=0, tsStb=0):
        self.sel      = sel
        self.adr      = adr
        self.datwr    = datwr
        self.waitIdle = waitIdle
        self.ts       = tsStb


@public
class WBOp():
    """
    Represents single operation on a bus, e.g. read/write.
    """
    def __init__(self, adr=0, dat=None, idle=0, sel=0xf):
        self.adr  = adr
        self.dat  = dat
        self.sel  = sel
        self.idle = idle


@public
class WBRes():
    """
    What's happened on the bus plus meta information on timing
    """
    def __init__(self, ack=0, sel=0xf, adr=0, datrd=None, datwr=None, waitIdle=0, waitAck=0):
        self.ack      = ack
        self.sel      = sel
        self.adr      = adr
        self.datrd    = datrd
        self.datwr    = datwr
        self.waitAck  = waitAck
        self.waitIdle = waitIdle


class Wishbone(AliasBusDriver):

    _signals = ["cyc", "stb", "we", "sel", "adr", "datwr", "datrd", "ack"]
    _aliases = {"cyc":"wb_cyc_i", "stb":"wb_stb_i", "we":"wb_we_i",
                "sel":"wb_sel_i", "adr":"wb_adr_i", "datwr":"wb_dat_i",
                "datrd":"wb_dat_o", "ack":"wb_ack_o"}

    def __init__(self, entity, clock, width=32):
        AliasBusDriver.__init__(self, entity, "", clock, self._aliases)

        # Drive some sensible defaults (setimmediatevalue to avoid x asserts)
        self._width = width
        self.bus.cyc.setimmediatevalue(0)
        self.bus.stb.setimmediatevalue(0)
        self.bus.we.setimmediatevalue(0)
        self.bus.adr.setimmediatevalue(0)
        self.bus.datwr.setimmediatevalue(0)

        v = self.bus.sel.value
        v.binstr = "1" * len(self.bus.sel)
        self.bus.sel <= v


class WishboneMaster(Wishbone):

    def __init__(self, entity, clock, timeout=None, width=32):
        sTo = ", no cycle timeout"
        if timeout is not None:
            sTo = ", cycle timeout is %u clockcycles" % timeout
        self.busy_event         = Event("wb_busy")
        self._timeout           = timeout
        self.busy               = False
        self._acked_ops         = 0
        self._res_buf           = []
        self._aux_buf           = []
        self._op_cnt            = 0
        self._clk_cycle_count   = 0
        Wishbone.__init__(self, entity, clock, width)
        self.log.info("Wishbone Master created%s" % sTo)

    @coroutine
    def _clk_cycle_counter(self):
        """
        Cycle counter to time bus operations
        """
        self._clk_cycle_count = 0

        while self.busy:
            yield RisingEdge(self.clock)
            self._clk_cycle_count += 1

    @coroutine
    def _open_cycle(self):

        if self.busy:
            self.log.error("Opening Cycle, but WB Driver is already busy. Someting's wrong")
            yield self.busy_event.wait()
        self.busy_event.clear()
        self.busy       = True

        cocotb.fork(self._read())
        cocotb.fork(self._clk_cycle_counter())

        self.bus.cyc    <= 1
        self._acked_ops = 0
        self._res_buf   = []
        self._aux_buf   = []
        self.log.debug("Opening cycle, %u Ops" % self._op_cnt)

    @coroutine
    def _close_cycle(self):

        count           = 0
        last_acked_ops  = 0

        #Wait for all Operations being acknowledged by the slave before lowering the cycle line
        #This is not mandatory by the bus standard, but a crossbar might send acks to the wrong master
        #if we don't wait. We don't want to risk that, it could hang the bus
        while self._acked_ops < self._op_cnt:
            if last_acked_ops != self._acked_ops:
                self.log.debug("Waiting for missing acks: %u/%u"
                               % (self._acked_ops, self._op_cnt) )
            last_acked_ops = self._acked_ops
            #check for timeout when finishing the cycle
            count += 1
            if (not (self._timeout is None)):
                if (count > self._timeout):
                    raise TestFailure("Timeout of %u clock cycles reached when waiting for reply from slave"
                                      % self._timeout)
            yield RisingEdge(self.clock)

        self.busy = False
        self.busy_event.set()
        self.bus.cyc <= 0
        self.log.debug("Closing cycle")
        yield RisingEdge(self.clock)

    @coroutine
    def _wait_ack(self):

        count = 0

        while not self._get_reply():
            yield RisingEdge(self.clock)
            count += 1
            self.log.debug("Waited %u cycles for ackknowledge" % count)

        raise ReturnValue(count)

    def _get_reply(self):

        tmpAck = int(self.bus.ack.value)
        tmpErr = 0 if not hasattr(self.bus, "err") else int(self.bus.err.value)
        tmpRty = 0 if not hasattr(self.bus, "rty") else int(self.bus.rty.value)

        #check if more than one line was raised
        if ((tmpAck + tmpErr + tmpRty)  > 1):
            raise TestFailure("Slave raised more than one reply line at once! ACK: %u ERR: %u RTY: %u"
                              % (tmpAck, tmpErr, tmpRty))
        #return 0 if no reply, 1 for ACK, 2 for ERR, 3 for RTY. use 'replyTypes' Dict for lookup
        return(tmpAck + 2 * tmpErr + 3 * tmpRty)

    @coroutine
    def _read(self):

        count = 0

        while self.busy:
            reply = self._get_reply()
            # valid reply?
            if(bool(reply)):
                datrd = int(self.bus.datrd.value) # Can't resolve 32 * xxxxx..
                #append reply and meta info to result buffer
                tmpRes = WBRes(ack=reply, sel=None, adr=None, datrd=datrd,
                               datwr=None, waitIdle=None,
                               waitAck=self._clk_cycle_count)
                self._res_buf.append(tmpRes)
                self._acked_ops += 1
            yield RisingEdge(self.clock)
            count += 1

    @coroutine
    def _drive(self, we, adr, datwr, sel, idle):

        if self.busy:
            # insert requested idle cycles
            if idle is not None:
                idlecnt = idle
                while idlecnt > 0:
                    idlecnt -= 1
                    yield RisingEdge(self.clock)

            # drive outputs
            self.bus.stb    <= 1
            self.bus.adr    <= adr
            self.bus.sel    <= sel
            self.bus.datwr  <= datwr
            self.bus.we     <= we

            yield RisingEdge(self.clock)

             #append operation and meta info to auxiliary buffer
            self._aux_buf.append(WBAux(sel, adr, datwr, idle, self._clk_cycle_count))

            yield self._wait_ack()

            #reset strobe and write enable without advancing time
            self.bus.stb    <= 0
            self.bus.we     <= 0            
        else:
            self.log.error("Cannot drive the Wishbone bus outside a cycle!")

    @coroutine
    def send_cycle(self, arg):
        """
        The main sending routine

        Args:
            List of Wishbone operations
        """
        cnt = 0

        yield RisingEdge(self.clock)

        if not is_sequence(arg) or len(arg) < 1 or not all(isinstance(op, WBOp) for op in arg):
            raise TestFailure("arg MUST be a list of WBOp objects only")
            raise ReturnValue(None)

        self._op_cnt = len(arg)
        result = []

        yield self._open_cycle()

        for op in arg:
            if op.dat is not None:
                we  = 1
                dat = op.dat
            else:
                we  = 0
                dat = 0

            yield self._drive(we, op.adr, dat, op.sel, op.idle)
            self.log.debug("#%3u WE: %s ADR: 0x%08x DAT: 0x%08x SEL: 0x%1x IDLE: %3u"
                                   % (cnt, we, op.adr, dat, op.sel, op.idle))
            cnt += 1

        yield self._close_cycle()

        #do pick and mix from result- and auxiliary buffer so we get all operation and meta info
        for res, aux in zip(self._res_buf, self._aux_buf):
            res.datwr       = aux.datwr
            res.sel         = aux.sel
            res.adr         = aux.adr
            res.waitIdle    = aux.waitIdle
            res.waitAck    -= aux.ts
            result.append(res)

        raise ReturnValue(result)
