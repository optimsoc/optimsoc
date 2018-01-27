class Driver(object):
    """

    Class defining the standard interface for a driver within a testbench

    The driver is responsible for serialising transactions onto the physical
    pins of the interface.  This may consume simulation time.
    """
    def __init__(self):
        pass

class BusDriver(Driver):
    """
    Wrapper around common functionality for busses which have:
        a list of _signals (class attribute)
        a list of _optional_signals (class attribute)
        a clock
        a name
        an entity
    """
    _optional_signals = []

    def __init__(self, entity, name, clock):
        pass
