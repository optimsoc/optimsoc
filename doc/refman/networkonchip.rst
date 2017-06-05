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
