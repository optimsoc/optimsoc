# Introduction

This document specifies the implementation of the core debug module
for cores with an advanced debug system interface. It uses the [MMIO
Bridge] module for integration in the Open SoC Debug Interconnect
Infrastructure.

## License

This work is licensed under the Creative Commons
Attribution-ShareAlike 4.0 International License. To view a copy of
this license, visit
[http://creativecommons.org/licenses/by-sa/4.0/](http://creativecommons.org/licenses/by-sa/4.0/)
or send a letter to Creative Commons, PO Box 1866, Mountain View, CA
94042, USA.

You are free to share and adapt this work for any purpose as long as
you follow the following terms: (i) Attribution: You must give
appropriate credit and indicate if changes were made, (ii) ShareAlike:
If you modify or derive from this work, you must distribute it under
the same license as the original.

## Authors

Stefan Wallentowitz

Fill in your name here!

# Core Interface

The core is connected as a slave on this interface:

 Signal | Driver | Width | Description
 ------ | ------ | ----- | -----------
 `stall` | Module | 1 | Stall the core
 `breakpoint` | Core | 1 | Indicates breakpoint
 `strobe` | Module | 1 | Access to the core debug interface
 `ack` | Core | 1 | Complete access to the core
 `adr` | Module | ? | Address of CPU register to access
 `write` | Module | 1 | Write access
 `data_in` | Module | `DATA_WIDTH` | Write data
 `data_out` | Core | `DATA_WIDTH` | Read data

# Memory Map

The following map applies to the interface.

 Address Range | Description
 ------------- | -----------
 (TODO) | (TODO)
 
## General Purpose Registers

## Control- and Status Registers

## Interrupt Cause Register

