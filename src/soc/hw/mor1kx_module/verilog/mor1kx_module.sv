/* Copyright (c) 2013 by the author(s)
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
 * =============================================================================
 *
 * This is a wrapper module for the OpenRISC processor
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

import opensocdebug::mor1kx_trace_exec;

module mor1kx_module
  #(parameter ID = 0,
    parameter NUMCORES = 1,
    parameter CPU_IMPLEMENTATION = "CAPPUCCINO"
    )
   (input          clk_i,
    input         bus_clk_i,
    input         rst_i,
    input         bus_rst_i,

    input         dbg_stall_i, // External Stall Input
    input         dbg_ewt_i, // External Watchpoint Trigger Input
    output [3:0]  dbg_lss_o, // External Load/Store Unit Status
    output [1:0]  dbg_is_o, // External Insn Fetch Status
    output [10:0] dbg_wp_o, // Watchpoints Outputs
    output        dbg_bp_o, // Breakpoint Output
    input         dbg_stb_i, // External Address/Data Strobe
    input         dbg_we_i, // External Write Enable
    input [31:0]  dbg_adr_i, // External Address Input
    input [31:0]  dbg_dat_i, // External Data Input
    output [31:0] dbg_dat_o, // External Data Output
    output        dbg_ack_o, // External Data Acknowledge (not WB compatible)

    input [31:0]  pic_ints_i,

   //
   // Instruction WISHBONE interface
   //
    input         iwb_ack_i, // normal termination
    input         iwb_err_i, // termination w/ error
    input         iwb_rty_i, // termination w/ retry
    input [31:0]  iwb_dat_i, // input data bus
    output        iwb_cyc_o, // cycle valid output
    output [31:0] iwb_adr_o, // address bus outputs
    output        iwb_stb_o, // strobe output
    output        iwb_we_o, // indicates write transfer
    output [3:0]  iwb_sel_o, // byte select outputs
    output [31:0] iwb_dat_o, // output data bus
    output [1:0]  iwb_bte_o,
    output [2:0]  iwb_cti_o,

   //
   // Data WISHBONE interface
   //
    input         dwb_ack_i, // normal termination
    input         dwb_err_i, // termination w/ error
    input         dwb_rty_i, // termination w/ retry
    input [31:0]  dwb_dat_i, // input data bus
    output        dwb_cyc_o, // cycle valid output
    output [31:0] dwb_adr_o, // address bus outputs
    output        dwb_stb_o, // strobe output
    output        dwb_we_o, // indicates write transfer
    output [3:0]  dwb_sel_o, // byte select outputs
    output [31:0] dwb_dat_o, // output data bus
    output [1:0]  dwb_bte_o,
    output [2:0]  dwb_cti_o,

    input         snoop_enable_i,
    input [31:0]  snoop_adr_i,

    output mor1kx_trace_exec trace_exec
    );

   wire dbg_stall_o;

   mor1kx
     #(.FEATURE_DATACACHE               ("NONE"),
       .OPTION_DCACHE_LIMIT_WIDTH       (31),
       .FEATURE_INSTRUCTIONCACHE        ("ENABLED"),
       .OPTION_DCACHE_WAYS              (2),
       .OPTION_DCACHE_SET_WIDTH         (8),
       .OPTION_DCACHE_SNOOP             ("ENABLED"),
       .OPTION_ICACHE_WAYS              (2),
       .OPTION_ICACHE_SET_WIDTH         (8),
       .FEATURE_DMMU                    ("ENABLED"),
       .FEATURE_IMMU                    ("ENABLED"),
       .IBUS_WB_TYPE                    ("B3_REGISTERED_FEEDBACK"),
       .DBUS_WB_TYPE                    ("B3_REGISTERED_FEEDBACK"),
       .FEATURE_MULTICORE               ("ENABLED"),
       .FEATURE_TRACEPORT_EXEC          ("ENABLED"),
       .OPTION_OPERAND_WIDTH            (32),
       .OPTION_RF_NUM_SHADOW_GPR        (1))
     u_cpu(/*AUTOINST*/
           // Outputs
           .iwbm_adr_o                  (iwb_adr_o[31:0]),
           .iwbm_stb_o                  (iwb_stb_o),
           .iwbm_cyc_o                  (iwb_cyc_o),
           .iwbm_sel_o                  (iwb_sel_o[3:0]),
           .iwbm_we_o                   (iwb_we_o),
           .iwbm_cti_o                  (iwb_cti_o[2:0]),
           .iwbm_bte_o                  (iwb_bte_o[1:0]),
           .iwbm_dat_o                  (iwb_dat_o[31:0]),
           .dwbm_adr_o                  (dwb_adr_o[31:0]),
           .dwbm_stb_o                  (dwb_stb_o),
           .dwbm_cyc_o                  (dwb_cyc_o),
           .dwbm_sel_o                  (dwb_sel_o[3:0]),
           .dwbm_we_o                   (dwb_we_o),
           .dwbm_cti_o                  (dwb_cti_o[2:0]),
           .dwbm_bte_o                  (dwb_bte_o[1:0]),
           .dwbm_dat_o                  (dwb_dat_o[31:0]),
           .du_dat_o                    (dbg_dat_o[31:0]),
           .du_ack_o                    (dbg_ack_o),
           .du_stall_o                  (dbg_stall_o),
           .traceport_exec_valid_o      (trace_exec.valid),
           .traceport_exec_pc_o         (trace_exec.pc),
           .traceport_exec_jb_o         (trace_exec.jb),
           .traceport_exec_jal_o        (trace_exec.jal),
           .traceport_exec_jr_o         (trace_exec.jr),
           .traceport_exec_jbtarget_o   (trace_exec.jbtarget),
           .traceport_exec_insn_o       (trace_exec.insn),
           .traceport_exec_wbdata_o     (trace_exec.wbdata),
           .traceport_exec_wbreg_o      (trace_exec.wbreg),
           .traceport_exec_wben_o       (trace_exec.wben),
           // Inputs
           .clk                         (clk_i),
           .rst                         (rst_i),
           .iwbm_err_i                  (iwb_err_i),
           .iwbm_ack_i                  (iwb_ack_i),
           .iwbm_dat_i                  (iwb_dat_i[31:0]),
           .iwbm_rty_i                  (iwb_rty_i),
           .dwbm_err_i                  (dwb_err_i),
           .dwbm_ack_i                  (dwb_ack_i),
           .dwbm_dat_i                  (dwb_dat_i[31:0]),
           .dwbm_rty_i                  (dwb_rty_i),
           .irq_i                       (pic_ints_i),
           .du_addr_i                   (dbg_adr_i[15:0]),
           .du_stb_i                    (dbg_stb_i),
           .du_dat_i                    (dbg_dat_i[31:0]),
           .du_we_i                     (dbg_we_i),
           .du_stall_i                  (dbg_stall_i),
           .multicore_coreid_i          (ID),
           .multicore_numcores_i        (NUMCORES),
           .snoop_adr_i                 (snoop_adr_i[31:0]),
           .snoop_en_i                  (snoop_enable_i));

endmodule // mor1kx_module

// Local Variables:
// verilog-library-directories:("." "../../*/verilog/")
// verilog-auto-inst-param-value: t
// End:
