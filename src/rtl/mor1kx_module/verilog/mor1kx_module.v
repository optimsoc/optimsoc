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

module mor1kx_module (
   /*AUTOARG*/
   // Outputs
   dbg_lss_o, dbg_is_o, dbg_wp_o, dbg_bp_o, dbg_dat_o, dbg_ack_o,
   iwb_cyc_o, iwb_adr_o, iwb_stb_o, iwb_we_o, iwb_sel_o, iwb_dat_o,
   iwb_bte_o, iwb_cti_o, dwb_cyc_o, dwb_adr_o, dwb_stb_o, dwb_we_o,
   dwb_sel_o, dwb_dat_o, dwb_bte_o, dwb_cti_o, trace,
   // Inputs
   clk_i, bus_clk_i, rst_i, bus_rst_i, dbg_stall_i, dbg_ewt_i,
   dbg_stb_i, dbg_we_i, dbg_adr_i, dbg_dat_i, pic_ints_i, iwb_ack_i,
   iwb_err_i, iwb_rty_i, iwb_dat_i, dwb_ack_i, dwb_err_i, dwb_rty_i,
   dwb_dat_i, snoop_enable_i, snoop_adr_i
   );

   parameter ID = 0;
   parameter NUMCORES = 1;
   
   parameter CPU_IMPLEMENTATION = "CAPPUCCINO";

   input          clk_i;
   input          bus_clk_i;
   input          rst_i;
   input          bus_rst_i;

   input          dbg_stall_i;  // External Stall Input
   input          dbg_ewt_i;    // External Watchpoint Trigger Input
   output [3:0]   dbg_lss_o;    // External Load/Store Unit Status
   output [1:0]   dbg_is_o;     // External Insn Fetch Status
   output [10:0]  dbg_wp_o;     // Watchpoints Outputs
   output         dbg_bp_o;     // Breakpoint Output
   input          dbg_stb_i;     // External Address/Data Strobe
   input          dbg_we_i;      // External Write Enable
   input [31:0]   dbg_adr_i;    // External Address Input
   input [31:0]   dbg_dat_i;    // External Data Input
   output [31:0]  dbg_dat_o;    // External Data Output
   output         dbg_ack_o;    // External Data Acknowledge (not WB compatible)

   input [31:0]   pic_ints_i;

   //
   // Instruction WISHBONE interface
   //
   input           iwb_ack_i;   // normal termination
   input           iwb_err_i;   // termination w/ error
   input           iwb_rty_i;   // termination w/ retry
   input [31:0]    iwb_dat_i;   // input data bus
   output          iwb_cyc_o;   // cycle valid output
   output [31:0]   iwb_adr_o;   // address bus outputs
   output          iwb_stb_o;   // strobe output
   output          iwb_we_o;    // indicates write transfer
   output [3:0]    iwb_sel_o;   // byte select outputs
   output [31:0]   iwb_dat_o;   // output data bus
   output [1:0]    iwb_bte_o;
   output [2:0]    iwb_cti_o;

   //
   // Data WISHBONE interface
   //
   input           dwb_ack_i;   // normal termination
   input           dwb_err_i;   // termination w/ error
   input           dwb_rty_i;   // termination w/ retry
   input [31:0]    dwb_dat_i;   // input data bus
   output          dwb_cyc_o;   // cycle valid output
   output [31:0]   dwb_adr_o;   // address bus outputs
   output          dwb_stb_o;   // strobe output
   output          dwb_we_o;    // indicates write transfer
   output [3:0]    dwb_sel_o;   // byte select outputs
   output [31:0]   dwb_dat_o;   // output data bus
   output [1:0]    dwb_bte_o;
   output [2:0]    dwb_cti_o;

   input           snoop_enable_i;
   input [31:0]    snoop_adr_i;

   output [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace;

   wire [31:0]                          traceport_exec_insn_o;// From u_cpu of mor1kx.v
   wire [31:0]                          traceport_exec_pc_o;    // From u_cpu of mor1kx.v
   wire                                 traceport_exec_valid_o; // From u_cpu of mor1kx.v
   wire [31:0]                          traceport_exec_wbdata_o;// From u_cpu of mor1kx.v
   wire                                 traceport_exec_wben_o;  // From u_cpu of mor1kx.v
   wire [4:0]                           traceport_exec_wbreg_o;// From u_cpu of mor1kx.v

   assign trace[`DEBUG_TRACE_EXEC_ENABLE_MSB]                              = traceport_exec_valid_o;
   assign trace[`DEBUG_TRACE_EXEC_PC_MSB:`DEBUG_TRACE_EXEC_PC_LSB]         = traceport_exec_pc_o;
   assign trace[`DEBUG_TRACE_EXEC_INSN_MSB:`DEBUG_TRACE_EXEC_INSN_LSB]     = traceport_exec_insn_o;
   assign trace[`DEBUG_TRACE_EXEC_WBEN_MSB]                                = traceport_exec_wben_o;
   assign trace[`DEBUG_TRACE_EXEC_WBREG_MSB:`DEBUG_TRACE_EXEC_WBREG_LSB]   = traceport_exec_wbreg_o;
   assign trace[`DEBUG_TRACE_EXEC_WBDATA_MSB:`DEBUG_TRACE_EXEC_WBDATA_LSB] = traceport_exec_wbdata_o;

   /* mor1kx AUTO_TEMPLATE(
    .clk (clk_i),
    .rst (rst_i),
    .dwbm_\(.*\)      (dwb_\1[]),
    .iwbm_\(.*\)      (iwb_\1[]),

    .du_addr_i (dbg_adr_i[15:0]),
    .du_\(.*\) (dbg_\1[]),

    .irq_i                      (pic_ints_i),
    
    .avm_.*_o (),
    .avm_d_readdata_i (32'h0),
    .avm_i_readdata_i (32'h0),
    .avm_.*_i (1'h0),
    
    .multicore_coreid_i (ID),
    .multicore_numcores_i (NUMCORES),
    .traceport_\(.*\) (traceport_\1),
    
    .snoop_en_i (snoop_enable_i),

    ); */

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
           .iwbm_adr_o                  (iwb_adr_o[31:0]),       // Templated
           .iwbm_stb_o                  (iwb_stb_o),             // Templated
           .iwbm_cyc_o                  (iwb_cyc_o),             // Templated
           .iwbm_sel_o                  (iwb_sel_o[3:0]),        // Templated
           .iwbm_we_o                   (iwb_we_o),              // Templated
           .iwbm_cti_o                  (iwb_cti_o[2:0]),        // Templated
           .iwbm_bte_o                  (iwb_bte_o[1:0]),        // Templated
           .iwbm_dat_o                  (iwb_dat_o[31:0]),       // Templated
           .dwbm_adr_o                  (dwb_adr_o[31:0]),       // Templated
           .dwbm_stb_o                  (dwb_stb_o),             // Templated
           .dwbm_cyc_o                  (dwb_cyc_o),             // Templated
           .dwbm_sel_o                  (dwb_sel_o[3:0]),        // Templated
           .dwbm_we_o                   (dwb_we_o),              // Templated
           .dwbm_cti_o                  (dwb_cti_o[2:0]),        // Templated
           .dwbm_bte_o                  (dwb_bte_o[1:0]),        // Templated
           .dwbm_dat_o                  (dwb_dat_o[31:0]),       // Templated
           .avm_d_address_o             (),                      // Templated
           .avm_d_byteenable_o          (),                      // Templated
           .avm_d_read_o                (),                      // Templated
           .avm_d_burstcount_o          (),                      // Templated
           .avm_d_write_o               (),                      // Templated
           .avm_d_writedata_o           (),                      // Templated
           .avm_i_address_o             (),                      // Templated
           .avm_i_byteenable_o          (),                      // Templated
           .avm_i_read_o                (),                      // Templated
           .avm_i_burstcount_o          (),                      // Templated
           .du_dat_o                    (dbg_dat_o[31:0]),       // Templated
           .du_ack_o                    (dbg_ack_o),             // Templated
           .du_stall_o                  (dbg_stall_o),           // Templated
           .traceport_exec_valid_o      (traceport_exec_valid_o), // Templated
           .traceport_exec_pc_o         (traceport_exec_pc_o),   // Templated
           .traceport_exec_insn_o       (traceport_exec_insn_o), // Templated
           .traceport_exec_wbdata_o     (traceport_exec_wbdata_o), // Templated
           .traceport_exec_wbreg_o      (traceport_exec_wbreg_o), // Templated
           .traceport_exec_wben_o       (traceport_exec_wben_o), // Templated
           // Inputs
           .clk                         (clk_i),                 // Templated
           .rst                         (rst_i),                 // Templated
           .iwbm_err_i                  (iwb_err_i),             // Templated
           .iwbm_ack_i                  (iwb_ack_i),             // Templated
           .iwbm_dat_i                  (iwb_dat_i[31:0]),       // Templated
           .iwbm_rty_i                  (iwb_rty_i),             // Templated
           .dwbm_err_i                  (dwb_err_i),             // Templated
           .dwbm_ack_i                  (dwb_ack_i),             // Templated
           .dwbm_dat_i                  (dwb_dat_i[31:0]),       // Templated
           .dwbm_rty_i                  (dwb_rty_i),             // Templated
           .avm_d_readdata_i            (32'h0),                 // Templated
           .avm_d_waitrequest_i         (1'h0),                  // Templated
           .avm_d_readdatavalid_i       (1'h0),                  // Templated
           .avm_i_readdata_i            (32'h0),                 // Templated
           .avm_i_waitrequest_i         (1'h0),                  // Templated
           .avm_i_readdatavalid_i       (1'h0),                  // Templated
           .irq_i                       (pic_ints_i),            // Templated
           .du_addr_i                   (dbg_adr_i[15:0]),       // Templated
           .du_stb_i                    (dbg_stb_i),             // Templated
           .du_dat_i                    (dbg_dat_i[31:0]),       // Templated
           .du_we_i                     (dbg_we_i),              // Templated
           .du_stall_i                  (dbg_stall_i),           // Templated
           .multicore_coreid_i          (ID),                    // Templated
           .multicore_numcores_i        (NUMCORES),              // Templated
           .snoop_adr_i                 (snoop_adr_i[31:0]),
           .snoop_en_i                  (snoop_enable_i));       // Templated

endmodule // mor1kx_module

// Local Variables:
// verilog-library-directories:("." "../../*/verilog/")
// verilog-auto-inst-param-value: t
// End:
