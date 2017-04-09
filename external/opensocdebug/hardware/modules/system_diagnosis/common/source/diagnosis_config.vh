/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ============================================================================
 *
 * Global defines for the whole diagnosis system
 *
 * Author(s):
 *   Markus Goehrle <Markus.Goehrle@tum.de>
 */

//*** Event id bit width ********************************************************
`define DIAGNOSIS_EV_ID_WIDTH 6

//*** Bit width of timestamp signal ********************************************
// TODO: find proper width
`define DIAGNOSIS_TIMESTAMP_WIDTH 32



///*** PC Module ***************************************************************
// Maximum number of allowed program counter events
`define DIAGNOSIS_PC_EVENTS_MAX 3

///*** Memory Address Module ***************************************************
// Maximum number of allowed memory address events
`define DIAGNOSIS_MEMADDR_EVENTS_MAX 1

///*** Function return Module *************************************************
// Maximum number of allowed function return events
`define DIAGNOSIS_FCNRET_EVENTS_MAX 2
// Maximum number of return addresses that can be stored.
// This can for example be recursive function calls, all of the same event
`define DIAGNOSIS_R9_ADDRESSES 4

///*** LUT Module **************************************************************
// Total maximum number of simultaneously tracked events in the system
// This is always the sum of the maximum events of each monitor
`define DIAGNOSIS_TOTAL_EVENTS_MAX (`DIAGNOSIS_PC_EVENTS_MAX + `DIAGNOSIS_MEMADDR_EVENTS_MAX + `DIAGNOSIS_FCNRET_EVENTS_MAX)

///*** GPR Module *************************************************************
// Write back interface bit widths
`define DIAGNOSIS_WB_REG_WIDTH 5
`define DIAGNOSIS_WB_DATA_WIDTH 32
// Number of parallel GPR Snapshot sets
`define DIAGNOSIS_parallel_gprshots 2


///*** Stack Module ***********************************************************
// Maximum number of stack arguments that can be accessed
`define DIAGNOSIS_STACKARGS_MAX 6
// Number of stackargument shadow sets, which enable availability despite event bursts
`define DIAGNOSIS_parallel_stackshots 2

//*** Snapshot forwarding: Flit types ******************************************
// Flag coding for data that is forwarded from snapshot modules to packetizer
`define SNAPSHOT_FLIT_TYPE_FIRST 3'b001
`define SNAPSHOT_FLIT_TYPE_MIDDLE 3'b010
`define SNAPSHOT_FLIT_TYPE_LAST 3'b100
`define SNAPSHOT_FLIT_TYPE_SINGLE 3'b000
`define SNAPSHOT_FLIT_TYPE_NONE 3'b111



///*** Packetizer Module ******************************************************
// Input Fifo Lengths
// TODO: Find proper lengths for these fifos
//`define DIAGNOSIS_EVENT_FIFO_LENGTH 4
//`define DIAGNOSIS_GPR_FIFO_LENGTH 10
//`define DIAGNOSIS_STACK_FIFO_LENGTH 10
`define DIAGNOSIS_EVENT_FIFO_LENGTH 30
`define DIAGNOSIS_GPR_FIFO_LENGTH 30
`define DIAGNOSIS_STACK_FIFO_LENGTH 30

//*** Maximum size (i.e. number of 16 bit debug NoC flits) of a snapshot packet
// that is forwarded to the Co-CPU via debug NoC
// [ 1 (header) + 2 (32 bit timestamp) + 2*MAX_GPR + 2* MAX_Stackargs)
`define DIAGNOSIS_MAX_GPR_PER_EVENT 10
// TODO: Maximale Anzahl fuer GPRs setzen, und ggf dann einfach vorher abbrechen
`define DIAGNOSIS_MAX_SNPACKET_SIZE16 (1 + 2 + 2*`DIAGNOSIS_MAX_GPR_PER_EVENT + `DIAGNOSIS_STACKARGS_MAX*2)

///*** NoC Interface **********************************************************
// Virtual channel that is used for debug NoC transmission
`define DIAGNOSIS_VCHANNEL_PACKETS 1
// Number of virtual channels
`define DIAGNOSIS_NR_VCHANNELS 3

///*** Delay *******************************************************************  
// The Snapshot delay is the cycle delay between cpu signals (i.e. writeback)
// and snapshot modules, to assure data consistency due to combinatorial logic
// which delays the resulting event triggers
`define DIAGNOSIS_SNAPSHOT_DELAY  1   

///*** Configuration registers *******************************************************************  
// Defines the number of 16 bit flits that is needed for one configuration entry
// (for each event monitor, as well as the data correlation (LUT) module
`define DIAGNOSIS_CONF_FLITS_PER_ENTRY 3
