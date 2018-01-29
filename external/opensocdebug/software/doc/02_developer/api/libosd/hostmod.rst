osd_hostmod class
-----------------

This class represents a debug module on the host.

``osd_hostmod`` implements functionality common to all host debug modules, and provides extension for individual host module implementations.
Most importantly, ``osd_hostmod`` encapsulates all communication with the host controller.

Usage
^^^^^

.. code-block:: c

  #include <osd/osd.h>
  #include <osd/hostmod.h>

Example
^^^^^^^

.. code-block:: c

  #include <osd/osd.h>
  #include <osd/hostmod.h>

  const char* HOST_CONTROLLER_URL = "tcp://localhost:6666";

  // initialize class
  osd_hostmod_ctx *hostmod_ctx;
  osd_hostmod_new(&hostmod_ctx);

  // connect to host controller
  osd_hostmod_connect(hostmod_ctx, HOST_CONTROLLER_URL);

  // the subnet controller assigns this host module a unique address
  uint16_t addr;
  addr = osd_hostmod_get_diaddr(hostmod_ctx);
  printf("This module got the address %d assigned.\n", addr);

  // read register 0x0000 from module with address 0x0000
  uint16_t result;
  osd_hostmod_reg_read(hostmod_ctx, 0, 0, 16, &result, 0);
  printf("Read returned value %u.\n", result);

  // disconnect from host controller
  osd_hostmod_disconnect(hostmod_ctx);

  // cleanup
  osd_hostmod_free(&hostmod_ctx);


Public Interface
^^^^^^^^^^^^^^^^

.. doxygenfile:: libosd/include/osd/hostmod.h


Internal Architecture
^^^^^^^^^^^^^^^^^^^^^

.. note::

   This section is targeting developers working on the ``osd_hostmod`` module.

.. note::
   TBD
