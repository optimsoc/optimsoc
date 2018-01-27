OSD Host Communication Protocol
===============================

Message Types
-------------

All data on the host is transferred using ZeroMQ.
The protocol used inside the ZeroMQ messages is described in this section.

All ZeroMQ messages consist of three frames.

.. tabularcolumns:: |p{\dimexpr 0.20\linewidth-2\tabcolsep}|p{\dimexpr 0.20\linewidth-2\tabcolsep}|p{\dimexpr 0.60\linewidth-2\tabcolsep}|
.. flat-table:: ZeroMQ message frame structure
  :widths: 2 2 6
  :header-rows: 1

  * - Frame
    - Name
    - Description

  * - 1
    - ``src``
    - Message source (ZeroMQ identity frame)

  * - 2
    - ``type``
    - Type of the message. Either ``D`` for data messages (encapsulated DI packets), or ``M`` for management messages (host only). 
    
  * - 3
    - ``payload``
    - Payload of the message. Its contents depend on the type. 


Data Messages
^^^^^^^^^^^^^

Data messages must have the ``type`` frame set to ``D``.
The ``payload`` field contains then a full OSD DI packet as an array of :c:type:`uint16_t` words in system-native byte ordering (i.e. usually little endian).


Management Messages
^^^^^^^^^^^^^^^^^^^

Management messages must have the ``type`` frame set to ``M``.
These messages are used as a protocol layer below the DI, e.g. to dynamically aquire and release an address in the debug interconnect (which later can be used when sending data messages).

DIADDR_REQUEST
""""""""""""""

- Source: any host debug module
- Target: host subnet controller

Request a new (previously unused) Debug Interconnect address from the host controller for this subnet.

The subnet controller responds with a management message containing the assigned address as unsigned integer as payload.
If the address assignment failed, a ``NACK`` message is sent instead.
 
DIADDR_RELEASE
""""""""""""""

- Source: any host debug module
- Target: host subnet controller

Release the DI address assigned to the source.

If successsful, the subnet controller responds with an ``ACK`` message.
If not successful, a ``NACK`` message is sent.
 
GW_REGISTER <subnet-addr>
"""""""""""""""""""""""""

- Source: any
- Target: host subnet controller 

Register the source of this message as gateway for all traffic intended for subnet *<subnet-addr>*.
*<subnet-addr>* is given as decimal integer (base 10).

If successsful, the subnet controller responds with an ``ACK`` message.
If not successful, a ``NACK`` message is sent.
 
ACK
"""
- Source: any
- Target: any

Generic acknowledgement "operation successful". 

NACK
""""
- Source: any
- Target: any

Generic error message "operation failed". 

Protocol Flows
--------------

Host Debug Module: Connect
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. seqdiag::

   seqdiag {
     "host module" => "host controller" [label = "DIADDR_REQ", return = "<ADDRESS>"];
   }
  
Host: Disconnect
^^^^^^^^^^^^^^^^
.. seqdiag::

   seqdiag {
     "host module" => "host controller" [label = "DIADDR_RELEASE", return = "ACK or NACK"];
   }

Host: Register as Gateway Device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
