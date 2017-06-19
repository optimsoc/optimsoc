***************
Network-on-Chip
***************

In OpTiMSoC we ship our own, minimalistic network-on-chip.
It is a plain, straight-forward implementation with the following features:

* wormhole-based: Packets are preserved in the network
* dimension-routing: Deadlock free XY-routing
* strict ordering: Packets in one stream leave the network in the same order as they entered it
* configurable: Easily create your own topology from the building blocks

In this part of the documentation you can learn more about the network-on-chip.

Transmission & Packet Format
============================

The packet width is parameterized in the network-on-chip, but in OpTiMSoC we currently rely on 32-bit links.
This may change in the future, but is abstracted in the network-on-chip.
The transmission protocol is a very simple handshake signal with the following signals:

* ``valid`` signals from the master that it currently assigns valid data.
* ``ready`` signals from the slave that it can accept a data item in this cycle.
* ``flit`` is the actual data and assigned by the master.
* ``last`` signals from the master that this is the last ``flit``.

Packets are formed by multiple flits and the ``last`` bit marks the end of a packet.
The network-on-chip is agnostic to the packet content, except for the first flit in a packet: the header.
The first 5 bits are the destination of this packet and those bits are used to compute the output port at each hop.

Beside the destination two extra fields are standardized in OpTiMSoC as depicted in the following table:

.. tabularcolumns:: |p{\dimexpr 0.10\linewidth-2\tabcolsep}|p{\dimexpr 0.20\linewidth-2\tabcolsep}|p{\dimexpr 0.70\linewidth-2\tabcolsep}|

.. flat-table::
  :widths: 1 2 7
  :header-rows: 1

  * - Bit(s)
    - Field
    - Description

  * - 31:27
    - ``DEST``
    - **Packet Destination**

      The ID of this packet's destination network-on-chip port.

  * - 26:24
    - ``CLASS``
    - **Packet Class**

      Packet classes are used to differentiate between different services in the network-on-chip.
      Services are commonly associated with a specific module in the network adapter of the tiles.

  * - 23:19
    - ``SRC``
    - **Packet Source**

      The ID of this packet's source network-on-chip port.

  * - 18:0
    - ``...``
    - **Class specific**

      The remaining part of the header is used by the classes.

Network-on-Chip Router
======================

.. figure:: img/noc_router.*
   :alt: The network-on-chip router
   :name: fig:noc_router

   The network-on-chip router    

Figure :numref:`%s<fig:noc_router>` sketches the anatomy of the basic
network-on-chip router: It has a configurable number of input ports
and output ports (that don't have to be the same). An input port
stores incoming flits into a buffer, then a route lookup module
determines the output port based on the destination of a packet and
forwards it to the switch with that routing information. The output
ports get the requests from the input ports and an arbiter decides
which input port to serve, forwarding the packets to an output buffer
then.

Virtual Channels
----------------

Virtual channels are a bit hard to grasp if you are new to
networks-on-chip. First of all, you need to understand the concept of
channels: Channels are independent links between routers in
networks-on-chip. As we use so called strict ordering, the channel
number of the output port link is the same as the input port
link. Hence, this notion does not really exist in our
network-on-chip. Having two channels actually means having two
physical networks.

Channels are useful to avoid so called message-dependent deadlocks and
to divert different traffic classes in general. As the point-to-point
links between routers have historically been more costy in an ASIC,
the idea of virtual channels is to share the links between two routers
virtually. So, instead of having two networks they overlap on the
links. This means that the inner parts of the router are replicated,
but with some extra logic to share the links on input and output.

Configuration
-------------

The router has the following configuration parameters:

.. tabularcolumns:: |p{\dimexpr 0.15\linewidth-2\tabcolsep}|p{\dimexpr 0.30\linewidth-2\tabcolsep}|p{\dimexpr 0.55\linewidth-2\tabcolsep}|
		    
.. flat-table::
  :widths: 1 2 7
  :header-rows: 1

  * - Name
    - Type
    - Description

  * - ``FLIT_WIDTH``
    - integer
    - **Width of the flit data**

      Number of bits in each data item during a network-on-chip
      transfer.

  * - ``VCHANNELS``
    - integer
    - **Number of virtual channels**

  * - ``INPUTS``
    - integer
    - **Number of input ports**
  
  * - ``OUTPUTS``
    - integer
    - **Number of output ports**

  * - ``DESTS``
    - integer
    - **Number of destinations in network**

  * - ``ROUTES``
    - ``[OUTPUTS*DESTS-1:0]`` bits
    - **Output destinations**

      This is a flat field of bitvectors, with one entry for each
      possible destination. Each of the bitvectors is a one-hot
      encoding of the output to route the packet for. Be careful with
      ordering when assigning to it (LSB is index 0).

Instantiating a Network-on-Chip
===============================

Actually, you will rarely build the network-on-chip out of the
routers. Instead you will use a topology toplevel to generate the
router layout for you. The topology we use by default is a mesh.

Mesh
----

The module ``noc_mesh`` is instantiated to build a regular
two-dimensional mesh. It has the following configuration parameters:

.. tabularcolumns:: |p{\dimexpr 0.15\linewidth-2\tabcolsep}|p{\dimexpr 0.30\linewidth-2\tabcolsep}|p{\dimexpr 0.55\linewidth-2\tabcolsep}|
		    
.. flat-table::
  :widths: 1 2 7
  :header-rows: 1

  * - Name
    - Type
    - Description

  * - ``FLIT_WIDTH``
    - integer
    - **Width of the flit data**

      Number of bits in each data item during a network-on-chip
      transfer.

  * - ``CHANNELS``
    - integer
    - **Number of channels**
    
      This is the number of independent channels in the network. The
      parameter ``ENABLE_VCHANNELS`` switches between physical and
      virtual channels.
      
  * - ``ENABLE_VCHANNELS``
    - 1 bit
    - **Activate virtual channels**
    
      Setting this to ``1`` will activate link-sharing between the
      routers in the mesh, while setting it to ``0`` will instantiate
      as many networks as configured by ``CHANNELS``.

  * - ``X``, ``Y``
    - integer
    - **Dimensions of the mesh**
    
    
