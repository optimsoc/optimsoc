`include "optimsoc_def.vh"

module memory_tile ( /*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid,
   // Inputs
   clk, rst, noc_in_flit, noc_in_valid, noc_out_ready
   );
   
   parameter id = 0;
   parameter mem_file = "sram.vmem";

   parameter vchannels = 3;

   input clk;
   input rst;

   input [`FLIT_WIDTH-1:0] noc_in_flit;
   input [vchannels-1:0] noc_in_valid;
   output [vchannels-1:0] noc_in_ready;
   output [`FLIT_WIDTH-1:0] noc_out_flit;
   output [vchannels-1:0] noc_out_valid;
   input [vchannels-1:0] noc_out_ready;

   assign noc_in_ready[`OPTIMSOC_VC_DMARESP] = 1'b0;
   assign noc_in_ready[`OPTIMSOC_VC_SYNC] = 1'b0;

   assign noc_out_valid[`OPTIMSOC_VC_DMAREQ] = 1'b0;
   assign noc_out_valid[`OPTIMSOC_VC_SYNC] = 1'b0;   
   
   wire [31:0]		wb_adr_o;
   wire			wb_cyc_o;
   wire [31:0]		wb_dat_o;
   wire [3:0]		wb_sel_o;
   wire			wb_stb_o;
   wire			wb_we_o;
   wire			wb_ack_i;
   wire			wb_rty_i;
   wire			wb_err_i;
   wire	[31:0]		wb_dat_i;
   wire [2:0] 		wb_cti_o;
   wire [1:0] 		wb_bte_o;

   /* lisnoc_dma_target AUTO_TEMPLATE(
    .noc_in_valid (noc_in_valid[`OPTIMSOC_VC_DMAREQ]),
    .noc_in_ready (noc_in_ready[`OPTIMSOC_VC_DMAREQ]),
    .noc_out_valid (noc_out_valid[`OPTIMSOC_VC_DMARESP]),
    .noc_out_ready (noc_out_ready[`OPTIMSOC_VC_DMARESP]),
    ); */
   lisnoc_dma_target
     #(.tileid(id))
   u_dma_target(/*AUTOINST*/
		// Outputs
		.noc_out_flit		(noc_out_flit[`FLIT_WIDTH-1:0]),
		.noc_out_valid		(noc_out_valid[`OPTIMSOC_VC_DMARESP]), // Templated
		.noc_in_ready		(noc_in_ready[`OPTIMSOC_VC_DMAREQ]), // Templated
		.wb_cyc_o		(wb_cyc_o),
		.wb_stb_o		(wb_stb_o),
		.wb_we_o		(wb_we_o),
		.wb_dat_o		(wb_dat_o[31:0]),
		.wb_adr_o		(wb_adr_o[31:0]),
		.wb_sel_o		(wb_sel_o[3:0]),
		.wb_cti_o		(wb_cti_o[2:0]),
		.wb_bte_o		(wb_bte_o[1:0]),
		// Inputs
		.clk			(clk),
		.rst			(rst),
		.noc_out_ready		(noc_out_ready[`OPTIMSOC_VC_DMARESP]), // Templated
		.noc_in_flit		(noc_in_flit[`FLIT_WIDTH-1:0]),
		.noc_in_valid		(noc_in_valid[`OPTIMSOC_VC_DMAREQ]), // Templated
		.wb_ack_i		(wb_ack_i),
		.wb_dat_i		(wb_dat_i[31:0]));

   /* wb_ram_b3 AUTO_TEMPLATE(
    .wb_clk_i (clk),
    .wb_rst_i (rst),
    .\(.*\)_i (\1_o[]),
    .\(.*\)_o (\1_i[]),
    ); */
   wb_ram_b3
     #(.mem_size(4194304),.aw(32),.dw(32),.memory_file("mt.vmem"))
   u_ram(/*AUTOINST*/
	 // Outputs
	 .wb_ack_o			(wb_ack_i),		 // Templated
	 .wb_err_o			(wb_err_i),		 // Templated
	 .wb_rty_o			(wb_rty_i),		 // Templated
	 .wb_dat_o			(wb_dat_i[31:0]),	 // Templated
	 // Inputs
	 .wb_adr_i			(wb_adr_o[31:0]),	 // Templated
	 .wb_bte_i			(wb_bte_o[1:0]),	 // Templated
	 .wb_cti_i			(wb_cti_o[2:0]),	 // Templated
	 .wb_cyc_i			(wb_cyc_o),		 // Templated
	 .wb_dat_i			(wb_dat_o[31:0]),	 // Templated
	 .wb_sel_i			(wb_sel_o[3:0]),	 // Templated
	 .wb_stb_i			(wb_stb_o),		 // Templated
	 .wb_we_i			(wb_we_o),		 // Templated
	 .wb_clk_i			(clk),			 // Templated
	 .wb_rst_i			(rst));			 // Templated
   
   
endmodule // memory_tile

// Local Variables:
// verilog-library-directories:("../../wb_ram_b3/verilog" "../../../../../lisnoc/rtl/dma/")
// verilog-auto-inst-param-value: t
// End: