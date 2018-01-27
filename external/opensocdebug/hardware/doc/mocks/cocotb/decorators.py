import functools


class coroutine(object):
    """Decorator class that allows us to provide common coroutine mechanisms:

        log methods will will log to cocotb.coroutines.name

        join() method returns an event which will fire when the coroutine exits
    """

    def __init__(self, func):
        #self._func = func
        functools.update_wrapper(self, func)

    def __call__(self, *args, **kwargs):
        pass

    def __get__(self, obj, type=None):
        """Permit the decorator to be used on class methods
            and standalone functions"""
        return self.__class__(self._func.__get__(obj, type))
