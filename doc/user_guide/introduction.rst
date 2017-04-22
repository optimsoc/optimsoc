************
Introduction
************

Open Tiled Manycore System-on-Chip (OpTiMSoC) is library-based framework that allows you to build you own Manycore.
So called "Tiled Manycores" are based on a regular Network-on-Chip to which different tiles are connected.
Such tiles can contain processing elements, memory, I/O devices etc.

OpTiMSoC is based on LISNoC, an open source Network-on-Chip and other open source hardware components.
In future, this set of components will be continuously extended and you can easily integrate your own components.
A variety of target platforms, such as FPGA boards, emulation platforms and simulations will be supported.

For a general introduction about the project goals and elements of OpTiMSoC please refer to our permanently updated preprint on ArXiv.org_.

This document documents the projects from a user point of view.
Starting with the description of how to get and install OpTiMSoC it describes the different kinds of simulation or syntheses systems currently supported by OpTiMSoC in the style of step-by-step tutorials.

In the following, we will give you a short overview of the different components that are part of OpTiMSoC or other dependencies.

OpTiMSoC Toolchain
==================

The toolchain of OpTiMSoC currently consists of the standard OpenRISC crosscompiler for baremetal systems (newlib libc). Additionally, we have few small utility programs and scripts.

You will need to have it installed in most cases as it is necessary to build all the software running on the OpTiMSoC systems.

SoC Libraries
=============

The system libraries provide all functionalities of the OpTiMSoC platforms to the applications.
This includes hardware drivers, runtime support, communication APIs and task management APIs.

RTL Simulation
==============

In case you are developing hardware in OpTiMSoC, RTL simulation is the starting point of development.
Beside this, RTL simulation is used to execute small pieces of software for testing or development of drivers and the runtime system.

Unfortunately, we do not know of any high-quality open source RTL simulation tool, so that we here rely on commercial EDA tools.
At the moment this is Mentor's Modelsim.

Verilator
=========

Verilator is an open source tool that compiles Verilog code to SystemC and allows users without a commercial RTL simulator license to run system simulations.

Synthesis
=========

Currently we focus on FPGA synthesis for Xilinx FPGA.
There will always be a supported board that can be used with the free Xilinx WebPack, but in general you will of course need a Xilinx license for this.

Beside Xilinx tools, we also support Synopsys Synplify as this a more sophisticated synthesis tool.

Host Software
=============

Host software is needed for control and diagnosis/debug of the running systems.

.. _ArXiv.org: http://arxiv.org/abs/1304.5081
