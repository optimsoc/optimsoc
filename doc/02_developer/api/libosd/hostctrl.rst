osd_hostctrl class
------------------

This class contains the host controller.
The host controller is the central instance in a host subnet and has the following tasks:

- It serves as central connection point for all host modules (i.e. forming a star topology).
  As such, it handles the setup and teardown of connections, as well as assigning of DI addreses.
- It routes messages between host modules.
- Gateways can connect to the host controller to extend the debug network beyond the host.

Usage
^^^^^

.. code-block:: c

  #include <osd/osd.h>
  #include <osd/hostctrl.h>

Public Interface
^^^^^^^^^^^^^^^^

.. doxygenfile:: libosd/include/osd/hostctrl.h

