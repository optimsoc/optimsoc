Testing
=======

The Open SoC Debug hardware reference implementation comes with a set of tests on different levels of abstraction.

**Cocotb-based unit tests** run in simulation and validate the functionality of smaller parts of the implementation, usually a single debug module or another small components.
The test cases are typically stored along the module source code in a ``test`` subdirectory.
Common functionality in all tests is contained in a software library ``osdtestlib``.

.. toctree::
   :maxdepth: 2
   :caption: Content

   run_cocotb.rst
   osdtestlib/index.rst
