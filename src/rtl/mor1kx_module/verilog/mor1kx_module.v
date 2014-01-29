/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * This is a wrapper module for the OpenRISC processor that adds
 * the compare-and-swap (CAS) unit on the data port to allow for atomic
 * accesses to data elements.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module mor1kx_module (
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                      trace_itm,
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
                      trace_stm,
`endif
   /*AUTOARG*/
   // Outputs
   dbg_lss_o, dbg_is_o, dbg_wp_o, dbg_bp_o, dbg_dat_o, dbg_ack_o,
   iwb_cyc_o, iwb_adr_o, iwb_stb_o, iwb_we_o, iwb_sel_o, iwb_dat_o,
   iwb_bte_o, iwb_cti_o, dwb_cyc_o, dwb_adr_o, dwb_stb_o, dwb_we_o,
   dwb_sel_o, dwb_dat_o, dwb_bte_o, dwb_cti_o,
   // Inputs
   clk_i, bus_clk_i, rst_i, bus_rst_i, dbg_stall_i, dbg_ewt_i,
   dbg_stb_i, dbg_we_i, dbg_adr_i, dbg_dat_i, pic_ints_i, iwb_ack_i,
   iwb_err_i, iwb_rty_i, iwb_dat_i, dwb_ack_i, dwb_err_i, dwb_rty_i,
   dwb_dat_i, snoop_enable_i, snoop_adr_i
   );

   parameter ID = 0;

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

   input [19:0]   pic_ints_i;

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

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   output [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
`endif

`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   output [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;
`endif

   wire [31:0] traceport_exec_insn_o;// From u_cpu of mor1kx.v
   wire [31:0]		traceport_exec_pc_o;	// From u_cpu of mor1kx.v
   wire			traceport_exec_valid_o;	// From u_cpu of mor1kx.v
   wire [31:0]		traceport_exec_wbdata_o;// From u_cpu of mor1kx.v
   wire			traceport_exec_wben_o;	// From u_cpu of mor1kx.v
   wire [4:0] 		traceport_exec_wbreg_o;// From u_cpu of mor1kx.v

   /*
    * +-------+-----------------+
    * | valid | program counter |
    * +-------+-----------------+
    */
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   assign trace_itm = { traceport_exec_valid_o, traceport_exec_pc_o };
`endif

   /*
    * +-------+-------------+-----------+----------+------+
    * | valid | instruction | writeback | register | data |
    * +-------+-------------+-----------+----------+------+
    */

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   assign trace_stm = { traceport_exec_valid_o, traceport_exec_insn_o, 
			traceport_exec_wben_o, traceport_exec_wbreg_o,
			traceport_exec_wbdata_o };
`endif

   wire            core_ack_i;  // normal termination
   wire            core_err_i;  // termination w/ error
   wire            core_rty_i;  // termination w/ retry
   wire [31:0]     core_dat_i;  // input data bus
   wire            core_cyc_o;  // cycle valid wire
   wire [31:0]     core_adr_o;  // address bus outputs
   wire            core_stb_o;  // strobe output
   wire            core_we_o;   // indicates write transfer
   wire [3:0]      core_sel_o;  // byte select outputs
   wire [31:0]     core_dat_o;  // output data bus
   wire [1:0]      core_bte_o;
   wire [2:0]      core_cti_o;

   wire 	   data_ack;
   wire [31:0] 	   data_dat;

   reg [31:0] 	   scratchmem [0:63];

   always @(posedge clk_i) begin
      if (core_adr_o < 32'h100) begin
	 if (core_cyc_o && core_stb_o && core_we_o) begin
	    scratchmem[core_adr_o[8:2]] <= core_dat_o;
	 end
      end
   end

   assign core_ack_i = (core_adr_o >= 32'h100) ? data_ack : 1'b1;
   assign core_dat_i = (core_adr_o >= 32'h100) ? data_dat : scratchmem[core_adr_o[8:2]];


   /* mor1kx AUTO_TEMPLATE(
    .clk (clk_i),
    .rst (rst_i),
    .dwbm_\(.*\)      (core_\1[]),
    .iwbm_\(.*\)      (iwb_\1[]),

    .du_addr_i (dbg_adr_i),
    .du_\(.*\) (dbg_\1[]),

    .irq_i                      ({11'b0,pic_ints_i}),
    
    .avm_.*_o (),
    .avm_d_readdata_i (32'h0),
    .avm_i_readdata_i (32'h0),
    .avm_.*_i (1'h0),
    
    .multicore_coreid_i (ID),
    .traceport_\(.*\) (traceport_\1),

    ); */

   wire dbg_stall_o;

   mor1kx
     #(.FEATURE_DATACACHE               ("ENABLED"),
       .FEATURE_INSTRUCTIONCACHE        ("ENABLED"),
       .FEATURE_DMMU                    ("ENABLED"),
       .FEATURE_IMMU                    ("ENABLED"),
       .IBUS_WB_TYPE                    ("B3_REGISTERED_FEEDBACK"),
       .DBUS_WB_TYPE                    ("B3_REGISTERED_FEEDBACK"),
       .FEATURE_MULTICORE               ("ENABLED"),
       .FEATURE_TRACEPORT_EXEC          ("ENABLED"),
       .OPTION_OPERAND_WIDTH            (32))
     u_cpu(/*AUTOINST*/
	   // Outputs
	   .iwbm_adr_o			(iwb_adr_o[31:0]),	 // Templated
	   .iwbm_stb_o			(iwb_stb_o),		 // Templated
	   .iwbm_cyc_o			(iwb_cyc_o),		 // Templated
	   .iwbm_sel_o			(iwb_sel_o[3:0]),	 // Templated
	   .iwbm_we_o			(iwb_we_o),		 // Templated
	   .iwbm_cti_o			(iwb_cti_o[2:0]),	 // Templated
	   .iwbm_bte_o			(iwb_bte_o[1:0]),	 // Templated
	   .iwbm_dat_o			(iwb_dat_o[31:0]),	 // Templated
	   .dwbm_adr_o			(core_adr_o[31:0]),	 // Templated
	   .dwbm_stb_o			(core_stb_o),		 // Templated
	   .dwbm_cyc_o			(core_cyc_o),		 // Templated
	   .dwbm_sel_o			(core_sel_o[3:0]),	 // Templated
	   .dwbm_we_o			(core_we_o),		 // Templated
	   .dwbm_cti_o			(core_cti_o[2:0]),	 // Templated
	   .dwbm_bte_o			(core_bte_o[1:0]),	 // Templated
	   .dwbm_dat_o			(core_dat_o[31:0]),	 // Templated
	   .avm_d_address_o		(),			 // Templated
	   .avm_d_byteenable_o		(),			 // Templated
	   .avm_d_read_o		(),			 // Templated
	   .avm_d_burstcount_o		(),			 // Templated
	   .avm_d_write_o		(),			 // Templated
	   .avm_d_writedata_o		(),			 // Templated
	   .avm_i_address_o		(),			 // Templated
	   .avm_i_byteenable_o		(),			 // Templated
	   .avm_i_read_o		(),			 // Templated
	   .avm_i_burstcount_o		(),			 // Templated
	   .du_dat_o			(dbg_dat_o[31:0]),	 // Templated
	   .du_ack_o			(dbg_ack_o),		 // Templated
	   .du_stall_o			(dbg_stall_o),		 // Templated
	   .traceport_exec_valid_o	(traceport_exec_valid_o), // Templated
	   .traceport_exec_pc_o		(traceport_exec_pc_o),	 // Templated
	   .traceport_exec_insn_o	(traceport_exec_insn_o), // Templated
	   .traceport_exec_wbdata_o	(traceport_exec_wbdata_o), // Templated
	   .traceport_exec_wbreg_o	(traceport_exec_wbreg_o), // Templated
	   .traceport_exec_wben_o	(traceport_exec_wben_o), // Templated
	   // Inputs
	   .clk				(clk_i),		 // Templated
	   .rst				(rst_i),		 // Templated
	   .iwbm_err_i			(iwb_err_i),		 // Templated
	   .iwbm_ack_i			(iwb_ack_i),		 // Templated
	   .iwbm_dat_i			(iwb_dat_i[31:0]),	 // Templated
	   .iwbm_rty_i			(iwb_rty_i),		 // Templated
	   .dwbm_err_i			(core_err_i),		 // Templated
	   .dwbm_ack_i			(core_ack_i),		 // Templated
	   .dwbm_dat_i			(core_dat_i[31:0]),	 // Templated
	   .dwbm_rty_i			(core_rty_i),		 // Templated
	   .avm_d_readdata_i		(32'h0),		 // Templated
	   .avm_d_waitrequest_i		(1'h0),			 // Templated
	   .avm_d_readdatavalid_i	(1'h0),			 // Templated
	   .avm_i_readdata_i		(32'h0),		 // Templated
	   .avm_i_waitrequest_i		(1'h0),			 // Templated
	   .avm_i_readdatavalid_i	(1'h0),			 // Templated
	   .irq_i			({11'b0,pic_ints_i}),	 // Templated
	   .du_addr_i			(dbg_adr_i),		 // Templated
	   .du_stb_i			(dbg_stb_i),		 // Templated
	   .du_dat_i			(dbg_dat_i[31:0]),	 // Templated
	   .du_we_i			(dbg_we_i),		 // Templated
	   .du_stall_i			(dbg_stall_i),		 // Templated
	   .multicore_coreid_i		(ID));			 // Templated

   /* wb_cas_unit AUTO_TEMPLATE(
    .clk_i          (bus_clk_i),
    .rst_i          (bus_rst_i),
    .wb_core_cyc_i  (core_cyc_o && (core_adr_o >= 32'h100)),
    .wb_core_stb_i  (core_stb_o && (core_adr_o >= 32'h100)),
    .wb_core_ack_o  (data_ack),
    .wb_core_dat_o  (data_dat),
    .wb_core_\(.*\)_i (core_\1_o[]),
    .wb_core_\(.*\)_o (core_\1_i[]),
    .wb_bus_\(.*\)  (dwb_\1[]),
    ); */
   wb_cas_unit u_cas (/*AUTOINST*/
		      // Outputs
		      .wb_core_dat_o	(data_dat),		 // Templated
		      .wb_core_ack_o	(data_ack),		 // Templated
		      .wb_core_rty_o	(core_rty_i),		 // Templated
		      .wb_core_err_o	(core_err_i),		 // Templated
		      .wb_bus_dat_o	(dwb_dat_o[31:0]),	 // Templated
		      .wb_bus_adr_o	(dwb_adr_o[31:0]),	 // Templated
		      .wb_bus_sel_o	(dwb_sel_o[3:0]),	 // Templated
		      .wb_bus_bte_o	(dwb_bte_o[1:0]),	 // Templated
		      .wb_bus_cti_o	(dwb_cti_o[2:0]),	 // Templated
		      .wb_bus_we_o	(dwb_we_o),		 // Templated
		      .wb_bus_cyc_o	(dwb_cyc_o),		 // Templated
		      .wb_bus_stb_o	(dwb_stb_o),		 // Templated
		      // Inputs
		      .clk_i		(bus_clk_i),		 // Templated
		      .rst_i		(bus_rst_i),		 // Templated
		      .wb_core_dat_i	(core_dat_o[31:0]),	 // Templated
		      .wb_core_adr_i	(core_adr_o[31:0]),	 // Templated
		      .wb_core_sel_i	(core_sel_o[3:0]),	 // Templated
		      .wb_core_bte_i	(core_bte_o[1:0]),	 // Templated
		      .wb_core_cti_i	(core_cti_o[2:0]),	 // Templated
		      .wb_core_we_i	(core_we_o),		 // Templated
		      .wb_core_cyc_i	(core_cyc_o && (core_adr_o >= 32'h100)), // Templated
		      .wb_core_stb_i	(core_stb_o && (core_adr_o >= 32'h100)), // Templated
		      .wb_bus_dat_i	(dwb_dat_i[31:0]),	 // Templated
		      .wb_bus_ack_i	(dwb_ack_i),		 // Templated
		      .wb_bus_rty_i	(dwb_rty_i),		 // Templated
		      .wb_bus_err_i	(dwb_err_i));		 // Templated

endmodule // or1200_module

// Local Variables:
// verilog-library-directories:("." "../../*/verilog/")
// verilog-auto-inst-param-value: t
// End:
