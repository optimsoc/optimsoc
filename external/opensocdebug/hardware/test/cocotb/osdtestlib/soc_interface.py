"""
    osdtestlib.soc_interface
    ~~~~~~~~~~~~~~~~~~~~~~~~

    Trace generators and other tools to stimulate the SoC as DUT

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from osdtestlib.debug_interconnect import RegAccess


class StmTraceGenerator:
    """
    Mimics CPU signals to generate stimuli for connected debug modules
    """

    @cocotb.coroutine
    def trigger_event(self, dut, trace_id, trace_value):
        """
        Set the trace* signals of the STM so that it generates a debug event 
        packet

        Args:
            dut:              device under test.
            trace_id:         ID of the trace event.
            trace_value:      value of the trace event.
        """

        # Define the content of its components
        dut.trace_id <= trace_id
        dut.trace_value <= trace_value

        # Trigger the generation of a new debug event packet
        dut.trace_valid <= 1

        yield RisingEdge(dut.clk)

        dut.trace_valid <= 0
