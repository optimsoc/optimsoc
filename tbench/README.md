# Testbenches

This repository contains examplary OpTiMSoC systems for understanding
OpTiMSoC and testing.

## Memory models

The following memory models are available:

 * Distributed Memory (folder `dm`): Each compute tile uses it's own
   local memory. It is also possible to have a global memory
   available, where baremetal applications can access it using DMA and
   the gzll operating system can load the OS or application from there
   and use swapping.

## Supported Simulation tools

Currently verilator is supported as simulation tool. Modelsim and VCS
will be (re-)integrated soonly.