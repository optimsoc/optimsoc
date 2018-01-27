"""
    osdtestlib.asserts
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Assertations for common checking needs

    :copyright: Copyright 2017-2018 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

from cocotb.result import TestFailure

def assert_signal_value(signal, expected_value):
    """Raise a TestFailure if the signal doesn't have an expected value"""
    if signal.value != expected_value:
        raise TestFailure("Expected %s to be 0x%x, got 0x%x" %
                          (signal._name, expected_value, int(signal.value)))
