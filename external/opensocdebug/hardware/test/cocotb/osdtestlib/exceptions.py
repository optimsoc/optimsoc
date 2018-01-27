"""
    osdtestlib.exceptions
    ~~~~~~~~~~~~~~~~~~~~~

    Exception definitions

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""


class Error(Exception):
    """Base class for exceptions."""
    pass


class RegAccessFailedException(Error):
    """
    Exception raised for errors during the register access process.

    Attributes:
        message:            explanation of the error.
    """

    def __init__(self, message):
        self.message = message


class DebugEventFailedException(Error):
    """
    Exception raised for errors during the evaluation of a debug packet.

    Attributes:
        message:            explanation of the error.
    """

    def __init__(self, message):
        self.message = message


class ResetFailedException(Error):
    """
    Exception raised for errors during the reset

    Attributes:
        message:            explanation of the error.
    """

    def __init__(self, message):
        self.message = message
