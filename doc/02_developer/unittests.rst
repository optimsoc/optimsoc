Unit Tests
==========

The OSD software comes with an extensive set of unit tests, written using the `Check framework <https://libcheck.github.io/check/>`_.

You can run the unit tests by calling ``make check`` in the ``build`` directory.

.. code-block:: sh

   make check
  
Check writes two types of log files: one log summary ``tests/unit/test-suite.log`` and an individual log file per test named after the test, e.g. ``tests/unit/check_hostmod.log``.
``printf()`` output from the individual tests is only written to the individual log files if the tests pass.
To get pass/fail remarks per test function set the ``CK_VERBOSITY`` environment variable to ``verbose`` when running a test.

.. note::

   If you don't find expected ``printf()`` output in the test log file try adding a ``fflush(stdout)`` statement after the ``printf()``.

.. code-block:: sh

   # view test summary
   less tests/unit/test-suite.log
   
   # obtain and view verbose logs for check_hostmod
   CK_VERBOSITY=verbose make check
   less tests/unit/check_hostmod.log

Sometimes it's helpful to debug the tests themselves using gdb.
The following command line runs a compiled test under gdb (replace ``check_hostmod`` with your test):

.. code-block:: sh

   make check # build the tests (if not already done)
   CK_FORK=no libtool --mode=execute gdb tests/unit/check_hostmod

Analyzing Code Coverage
-----------------------

To gain insight into which parts of the software are tested you can generate a coverage report.
To build and execute all tests and to finally generate a coverage report as HTML page run

.. code-block:: sh

   make check-code-coverage
   firefox tests/unit/opensocdebug-*-coverage/index.html

Find memory leaks with Valgrind
-------------------------------

To check the code for memory errors you can run the unit test suite under Valgrind with its `memcheck <http://valgrind.org/docs/manual/mc-manual.html>`_ tool.
Valgrind support must be enabled at configure time by passing the options ``--enable-valgrind --disable-asan`` to the ``configure`` script. 

.. code-block:: sh

   # run all tests with Valgrind
   make check-valgrind


Find memory errors with Address Sanitizer (ASan)
------------------------------------------------

`Address Sanitizer <https://github.com/google/sanitizers/wiki/AddressSanitizer>`_ (ASan) is a tool to find common classes of memory errors, such as out of bounds accesses.
It complements Valgrind especially for data on the stack. 

To enable ASan run ``configure`` with ``--disable-valgrind --enable-asan``.
(It's not possible to create a build with ASan and Valgrind enabled at the same time.)
ASan is then built-in and enabled in all produced binaries, including the unit tests.
In case of errors ASan will report them during the program run.

.. code-block:: sh

   # ASan is automatically enabled when running the test suite
   make check
