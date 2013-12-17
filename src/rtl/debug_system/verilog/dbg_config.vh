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
 * Global defines for the whole debug system
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

/*
 * Width of a timestamp provided by the Global Timestamp Provider (GTP)
 * The timestamp increments every clock cycle of the fastest clock in the system
 * and should be big enough to be unique until the data reaches the host PC.
 * Note: Changing this parameter requires changes in the flit packaging
 *       logic as well (e.g. in itm_dbgnoc_if.v)!
 * TODO: We still need to find a proper width for this timestamp
 */
`define DBG_TIMESTAMP_WIDTH 32

/*
 * Delay in clock cycles between a local (outgoing) and a remote (incoming)
 * trigger event.
 *
 * This delays all trace data for the given number of cycles before deciding
 * if it should be used or not, e.g. if the trigger matches or not.
 * Consider a simple example with one register between CTM and the debug module
 * on both directions:
 *   ITM generates trigger event -> 1 T delay -> CTM (comb.) -> 1 T delay ->
 *   ITM registers incoming trigger event
 * This means only after two cycles it's clear if the captured data should be
 * used. So we delay the capatured data for two cycles by using a shift register
 * and make sure to delay all trigger signals by this same (fixed) time as well
 * before we decide to use the data or not.
 *
 * Make this buffer as big as the longest latency between the CTM and a debug
 * module, and make sure to delay all internally generated triggers by this
 * time as well.
 */
`define DBG_TRIGGER_DELAY 2

/*
 * Router addresses of predefined modules
 */
// Address of the external interface, connecting the Debug NoC to the host PC
`define DBG_NOC_ADDR_EXTERNALIF 5'd0
// Address of the Trace Controller Module (TCM)
`define DBG_NOC_ADDR_TCM 5'd1
// Begin of the dynamic address range
`define DBG_NOC_ADDR_DYN_START 5'd2
// note: the max. address is 2^DBG_NOC_PH_DEST_WIDTH-1, currently set to 31

// register read request
`define DBG_NOC_CLASS_REG_READ_REQ 3'b000

// register read response
`define DBG_NOC_CLASS_REG_READ_RESP 3'b001

// register write request
`define DBG_NOC_CLASS_REG_WRITE_REQ 3'b010

// software trace data (from STM)
`define DBG_NOC_CLASS_SOFT_TRACE_DATA 3'b011

// instruction trace data (from ITM)
`define DBG_NOC_CLASS_TRACE_DATA 3'b100

// network router statistics (from NRM)
`define DBG_NOC_CLASS_NRM_DATA 3'b101

// encapsulated lisnoc32 packets
`define DBG_NOC_CLASS_NCM 3'b111

/*
 * Maximum length of packets (number of flits, including the header flit) that
 * can be sent from the Debug NoC to the USB interface.
 *
 * The currently used protocol requires a limit like this (even though the
 * size is arbitrary, it only defines a FIFO depth).
 * XXX: Enhance the Debug Noc -> USB protocol to remove this restriction.
 */
`define MAX_DBGNOC_TO_USB_PACKET_LENGTH 32

/*
 * +-----------+-------+
 * | wb_freeze | wb_pc |
 * +-----------+-------+
 *      32      31    0
 */
`define DEBUG_ITM_PORTWIDTH 33

/*
 * +----------+----------+-------+-----------+---------+
 * | rf_addrw | rf_dataw | rf_we | wb_freeze | wb_insn |
 * +----------+----------+-------+-----------+---------+
 *  70      66 65      34   33        32      31      0
 */
`define DEBUG_STM_PORTWIDTH 71
`define DEBUG_STM_RF_ADDRW_MSB  70
`define DEBUG_STM_RF_ADDRW_LSB  66
`define DEBUG_STM_RF_DATAW_MSB  65
`define DEBUG_STM_RF_DATAW_LSB  34
`define DEBUG_STM_RF_WE_MSB     33
`define DEBUG_STM_RF_WE_LSB     33
`define DEBUG_STM_WB_FREEZE_MSB 32
`define DEBUG_STM_WB_FREEZE_LSB 32
`define DEBUG_STM_WB_INSN_MSB   31
`define DEBUG_STM_WB_INSN_LSB    0
