//////////////////////////////////////////////////////////////////////
////                                                              ////
////  OR1200's configuration                                      ////
////                                                              ////
////  This file is part of the OpenRISC 1200 project              ////
////  http://opencores.org/project,or1k                           ////
////                                                              ////
////  Description                                                 ////
////  Configuration of the OR1200 core                            ////
////                                                              ////
////  Author(s):                                                  ////
////      - Damjan Lampret, lampret@opencores.org                 ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2000 Authors and OPENCORES.ORG                 ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////

//
// Dump VCD
//
//`define OR1200_VCD_DUMP

//
// Generate debug messages during simulation
//
//`define OR1200_VERBOSE

// Enable multiprocessor mode
`define OR1200_MP

//  `define OR1200_ASIC
////////////////////////////////////////////////////////
//
// Typical configuration for an ASIC
//
`ifdef OR1200_ASIC

//
// Target ASIC memories
//
//`define OR1200_ARTISAN_SSP
//`define OR1200_ARTISAN_SDP
//`define OR1200_ARTISAN_STP
`define OR1200_VIRTUALSILICON_SSP
//`define OR1200_VIRTUALSILICON_STP_T1
//`define OR1200_VIRTUALSILICON_STP_T2

//
// Do not implement Data cache
//
//`define OR1200_NO_DC

//
// Do not implement Insn cache
//
//`define OR1200_NO_IC

//
// Do not implement Data MMU
//
//`define OR1200_NO_DMMU

//
// Do not implement Insn MMU
//
//`define OR1200_NO_IMMU

//
// Select between ASIC optimized and generic multiplier
//
//`define OR1200_ASIC_MULTP2_32X32
`define OR1200_GENERIC_MULTP2_32X32

//
// Size/type of insn/data cache if implemented
//
// `define OR1200_IC_1W_512B
// `define OR1200_IC_1W_4KB
`define OR1200_IC_1W_8KB
// `define OR1200_DC_1W_4KB
`define OR1200_DC_1W_8KB

`else


/////////////////////////////////////////////////////////
//
// Typical configuration for an FPGA
//

//
// Target FPGA memories
//
//`define OR1200_ALTERA_LPM
//`define OR1200_XILINX_RAMB16
//`define OR1200_XILINX_RAMB4
`define OR1200_XILINX_RAM32X1D
//`define OR1200_USE_RAM16X1D_FOR_RAM32X1D
//`define OR1200_ACTEL

//
// Do not implement Data cache
//
`define OR1200_NO_DC

//
// Do not implement Insn cache
//
//`define OR1200_NO_IC

//
// Do not implement Data MMU
//
//`define OR1200_NO_DMMU

//
// Do not implement Insn MMU
//
//`define OR1200_NO_IMMU

//
// Select between ASIC and generic multiplier
//
// (Generic seems to trigger a bug in the Cadence Ncsim simulator)
//
//`define OR1200_ASIC_MULTP2_32X32
`define OR1200_GENERIC_MULTP2_32X32

//
// Size/type of insn/data cache if implemented
// (consider available FPGA memory resources)
//
//`define OR1200_IC_1W_512B
//`define OR1200_IC_1W_4KB
`define OR1200_IC_1W_8KB
`define OR1200_DC_1W_4KB
//`define OR1200_DC_1W_8KB

`endif

`ifdef OR1200_MP
// Set the core identifier register value not as parameter
// to the core, but as an input port to the system.
// This can have advantages, when using verilator for example.
//`define OR1200_MP_COREID_AS_PORT
`endif

// Select cache coherence method
//`define OR1200_DC_UPDATE_COHERENCE
//`define OR1200_DC_INVALID_COHERENCE

