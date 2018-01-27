Run Cocotb-Based Unit Tests
---------------------------

Prerequisites
~~~~~~~~~~~~~
The unit tests require Synopsys VCS as simulator.
Make sure it's installed.

You *don't* need to install cocotb manually, it's installed when the tests are run.


Run a single test
~~~~~~~~~~~~~~~~~

Tests are executed using the ``test/cocotb/cocotb_testrunner`` tool.
The testrunner reads all ``*.manifest.yaml`` files describing the test setup, and configures cocotb in a way to run the test with the simulator.

.. code-block:: sh

   # All commands in this example expect you're in the top-level directory
   # of the OSD hardware repository.
   cd your_osd_hardware_toplevel_dir

   # Run the tests for the STM in non-interactive mode
   test/cocotb/cocotb_testrunner modules/stm

   # Run tests in cocotb debug mode
   # This is especially useful when tests are failing due to Python coding
   # errors, as it shows backtraces.
   test/cocotb/cocotb_testrunner -l DEBUG modules/stm


Interactively run a test with GUI and waveforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

During test development and bug hunting, you might want to run the unit tests in the VCS GUI to see waveforms and other runtime data.
You can use the ``-g`` (or ``--gui``) switch to show the GUI during the test run.

.. code-block:: sh

   # normal log level
   test/cocotb/cocotb_testrunner -g modules/stm

   # debug log level
   test/cocotb/cocotb_testrunner -l DEBUG -g modules/stm

After the GUI shows up,

- select your top-level DUT in the hierarchical view on the left,
- press CTRL-4 to open the waveform view, and then
- press F5 (or go to Simulator, Start/Continue) to start the simulation.

You can see the outputs of the run in the VCS console on the bottom.
To re-run the simulation, press F5 twice.

.. note::

   Restarting a simulation picks up all changes made to the **Python test code** automatically.
   If you change the **HDL code** or the test manifest files you need to close VCS and call the test runner again to see your changes.

Run all tests
~~~~~~~~~~~~~

In addition to running a single test, ``cocotb_testrunner.py`` can also run multiple tests at once.

.. code-block:: sh

   test/cocotb/cocotb_testrunner
