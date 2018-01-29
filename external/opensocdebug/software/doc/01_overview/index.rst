Introduction
============

This is the reference implementation of the Open SoC Debug host software, i.e. the part of Open SoC Debug typically executed on a "standard" PC.

About the code
--------------

A couple of quick facts before we start:

- Most code is written in object-oriented C.
- The code targets POSIX platforms (e.g. Linux). Windows is not supported right now (patches are welcome!).
- ZeroMQ is used heavily for communication between the individual components.
- All code is licensed under the permissive MIT license.

On a high level, the implementation is split into two parts:

- ``libosd``, a reusable software library which does all the heavy lifting and provides an API for debug tools on the host. This code can be found in ``src/libosd``.
- A set of tools which use libosd to fulfill a well-defined task (e.g. interface to a device, or log trace messages). The associated code can be found in ``src/tools``.

The software architecture
-------------------------

The Open SoC Debug host software is composed of a set of standalone tools which each fulfill a single task.
In many cases, a tool is identical to a debug module on the host.

The tools communicate via ZeroMQ sockets.
These sockets provide an abstraction over the actual transport type.
Typically, TCP is chosen (which allows for communication between modules on a single machine, but also on different machines).
Also available are in-process communication via shared memory (useful if two tools are implemented inside a single process), socket communication (a bit faster than TCP when running on a single machine), and others.

.. figure:: img/osd-sw-overview.*
   :alt: An overview of all OSD host software components from a user's perspective
   :name: fig:osd-sw-overview

   An overview of all OSD host software components from a user's perspective
   
:numref:`Figure %s <fig:osd-sw-overview>` presents an overview of the OSD host software architecture.

At the center of the architecture is the osd-host-controller.
The host subnet controller has two jobs:

- It provides a central connection point for all tools and acts as message router.
- It provides low-level communication tasks, i.e. it assigns DI addresses to host debug modules.


From a user's perspective, all interesting functionality is implemented as host debug module.
In many cases, each host debug module is implemented as standalone tool.
For example, the tool osd-systrace-log is a host debug module which writes all STM messages it receives into a file.
Other examples include a gdb server debug module which enables GDB clients to connect to an OSD-enabled system, or a trace logger, which writes instruction traces into a file.

Finally, the osd-device-gateway tool connects the host to a target device, e.g. an FPGA.
It registers itself as gateway with the osd-host-controller, and transparently extends the debug interconnect to the subnet on the target device.
The communication interface between the target device and the osd-device-gateway depends on whatever physical interfaces are available on the target (typically UART or USB).
This device-host communication is encapsulated by GLIP, hence all GLIP-supported communication methods are also supported.

