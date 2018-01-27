osd_gateway class
-----------------

This class implements a gateway between two debug subnets, typically between the subnet on the host and the one on a "target device", an OSD enabled chip (as an ASIC, on an FPGA or running in simulation).
On the host side, the gateway registers with the host controller for a given subnet.
It then receives all traffic going to this subnet.
On the device side, the OSD packets are transformed into Data Transport Datagrams (length-value encoded OSD packets).
Standard read()/write() callback function can then be implemented to perform the actual data transfer to the device, possibly using another suitable library (such as GLIP).

Behavior on connection loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^
The connected device can drop the connection at any time.
That is signaled by the device by returning OSD_ERROR_NOT_CONNECTED from its packet_read() and packet_write() callback functions.
When osd_gateway detects this kind of loss in connectivity, it disconnects the gateway from the host controller.

Usage
^^^^^

.. code-block:: c

  #include <osd/osd.h>
  #include <osd/gateway.h>

Public Interface
^^^^^^^^^^^^^^^^

.. doxygenfile:: libosd/include/osd/gateway.h

