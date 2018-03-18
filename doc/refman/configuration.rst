*************
Configuration
*************

A hardware instance is configured using a single configuration
structure. It is passed as a SystemVerilog packed struct through the
system hierarchy. In the following you find the configuration items
with a description and how they become visible in software if
applicable.

There are two types of configuration items: base configuration items
are set by the designer and derived configuration items are derived by
the OpTiMSoC structure.

Configuration Items
===================

System Configuration
--------------------

Number of Tiles (``NUMTILES``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, integer.

**Description** This sets the total number of tiles. It does not
serve as generator input, but is essentially for information purposes.

**Usage** As informational field in the network adapter.

**Software** Accessible via ``num_tiles()``.

Number of Compute Tiles (``NUMCTS``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, integer.

**Description** This sets the number of compute tiles. It does not
serve as generator input, but is essentially for information purposes.

**Usage** As informational field in the network adapter.

**Software** Accessible via ``num_cts()``.

List of Compute Tiles (``CTLIST``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, 1024 bit bitmask

**Description** Provide a map of compute tiles (max. 1024 currently)
with the tileid being the index in the bitmask and the according bit
set when this tile is a compute tile.

**Usage** As informational field in the network adapter.

.. todo:: Software

Cores per tile (``CORES_PER_TILE``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, integer.

**Description** Sets the number of cores per compute tile.

**Usage** As generator for the core instantiations in the tiles and
base core id in each tile. Both can be queried as informational field
in the network adapter.

**Software** Can be queried as ? and influences the values of
``or1k_coreid()`` and ``or1k_numcores()``.

Global memory size (``GMEM_SIZE``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, integer.

**Description** Sets the size of the global memory. Used by PGAS.

**Usage** As configuration value or descriptive at memory tile.

Global memory tile (``GMEM_TILE``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Base item, integer.

**Description** Sets the tile identifier of the global memory
tile. Used by PGAS.

**Usage** Configures the PGAS load-store unit.

Total number of cores (``TOTAL_NUM_CORES``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Type** Derived item, calculated as ``NUMCTS * CORES_PER_TILE``.

Network-on-Chip
---------------

.. todo:: Describe

Compute Tile Configuration
--------------------------

.. todo:: Describe

Network Adapter
---------------

.. todo:: Describe

Debug
-----

.. todo:: Describe

System Configuration
====================

.. todo:: Example of configuration in System Verilog
