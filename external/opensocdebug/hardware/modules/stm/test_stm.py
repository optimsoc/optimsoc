import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

from modtestlib import ModuleSubTest_STM
from modtestlib import init

CONST_BASE_REG_TEST                            = 0



@cocotb.test()
def test_stm(dut):
    """
    Try accessing the design
    """
    
    # initialize the testbench and assign the ID '1' to the device under test
    yield init(dut, 1) 
    
    # conduct a base register test of the debug module
    yield ModuleSubTest_STM(dut, CONST_BASE_REG_TEST)
