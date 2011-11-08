`include "lisnoc_def.vh"

module networkadapter_ct(/*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid, wbm_adr_o, wbm_cyc_o,
   wbm_dat_o, wbm_sel_o, wbm_stb_o, wbm_we_o, wbm_cab_o, wbm_cti_o,
   wbm_bte_o, wbs_ack_o, wbs_rty_o, wbs_err_o, wbs_dat_o,
   // Inputs
   clk, rst, noc_in_flit, noc_in_valid, noc_out_ready, wbm_ack_i,
   wbm_rty_i, wbm_err_i, wbm_dat_i, wbs_adr_i, wbs_cyc_i, wbs_dat_i,
   wbs_sel_i, wbs_stb_i, wbs_we_i, wbs_cab_i, wbs_cti_i, wbs_bte_i
   );

   parameter vchannels = 3;

   parameter tileid = 0;
   
   input clk, rst;

   input [`FLIT_WIDTH-1:0] noc_in_flit;
   input [vchannels-1:0] noc_in_valid;
   output [vchannels-1:0] noc_in_ready;
   output [`FLIT_WIDTH-1:0] noc_out_flit;
   output [vchannels-1:0] noc_out_valid;
   input [vchannels-1:0] noc_out_ready;

   output [31:0] 	 wbm_adr_o;
   output 		 wbm_cyc_o;
   output [31:0] 	 wbm_dat_o;
   output [3:0] 	 wbm_sel_o;
   output 		 wbm_stb_o;
   output 		 wbm_we_o;
   output 		 wbm_cab_o;
   output [2:0] 	 wbm_cti_o;
   output [1:0] 	 wbm_bte_o;
   input 		 wbm_ack_i;
   input 		 wbm_rty_i;
   input 		 wbm_err_i;
   input [31:0] 	 wbm_dat_i;
   
   input [31:0] 	 wbs_adr_i;
   input 		 wbs_cyc_i;
   input [31:0] 	 wbs_dat_i;
   input [3:0] 		 wbs_sel_i;
   input 		 wbs_stb_i;
   input 		 wbs_we_i;
   input 		 wbs_cab_i;
   input [2:0] 		 wbs_cti_i;
   input [1:0] 		 wbs_bte_i;
   output 		 wbs_ack_o;
   output 		 wbs_rty_o;
   output 		 wbs_err_o;
   output [31:0] 	 wbs_dat_o;

   wire [vchannels-1:0] mod_out_ready;
   wire [vchannels-1:0] mod_out_valid;
   wire [`FLIT_WIDTH-1:0] mod_out_flit[0:vchannels-1];
   wire [vchannels-1:0] mod_in_ready;
   wire [vchannels-1:0] mod_in_valid;
   wire [`FLIT_WIDTH-1:0] mod_in_flit[0:vchannels-1];

   wire [`FLIT_WIDTH*vchannels-1:0] mod_out_flit_flat;

   assign mod_in_valid = noc_in_valid;
   assign noc_in_ready = mod_in_ready;
   generate
      genvar 		  v;
      for (v=0;v<vchannels;v=v+1) begin
	 assign mod_in_flit[v] = noc_in_flit;
	 assign mod_out_flit_flat[(v+1)*`FLIT_WIDTH-1:v*`FLIT_WIDTH] = mod_out_flit[v];
      end
   endgenerate
   
   assign mod_out_valid[`OPTIMSOC_VC_SYNC] = 1'b0;
   assign mod_out_flit[`OPTIMSOC_VC_SYNC] = {`FLIT_WIDTH{1'b0}};
   assign mod_in_ready[`OPTIMSOC_VC_SYNC] = 1'b0; 
   
   /* lisnoc_dma AUTO_TEMPLATE(
    .noc_in_req_ready (mod_in_ready[`OPTIMSOC_VC_DMAREQ]),
    .noc_in_req_valid (mod_in_valid[`OPTIMSOC_VC_DMAREQ]),
    .noc_in_req_flit  (mod_in_flit[`OPTIMSOC_VC_DMAREQ][]),
    .noc_in_resp_ready (mod_in_ready[`OPTIMSOC_VC_DMARESP]),
    .noc_in_resp_valid (mod_in_valid[`OPTIMSOC_VC_DMARESP]),
    .noc_in_resp_flit  (mod_in_flit[`OPTIMSOC_VC_DMARESP][]),
    .noc_out_req_ready (mod_out_ready[`OPTIMSOC_VC_DMAREQ]),
    .noc_out_req_valid (mod_out_valid[`OPTIMSOC_VC_DMAREQ]),
    .noc_out_req_flit  (mod_out_flit[`OPTIMSOC_VC_DMAREQ][]),
    .noc_out_resp_ready (mod_out_ready[`OPTIMSOC_VC_DMARESP]),
    .noc_out_resp_valid (mod_out_valid[`OPTIMSOC_VC_DMARESP]),
    .noc_out_resp_flit  (mod_out_flit[`OPTIMSOC_VC_DMARESP][]),
    .wb_if_\(.*\)   (wbs_\1),
    .wb_\(.*\)      (wbm_\1),
    ); */
   
   lisnoc_dma
     #(.tileid(tileid))
   u_dma(/*AUTOINST*/
	 // Outputs
	 .noc_in_req_ready		(mod_in_ready[`OPTIMSOC_VC_DMAREQ]), // Templated
	 .noc_in_resp_ready		(mod_in_ready[`OPTIMSOC_VC_DMARESP]), // Templated
	 .noc_out_req_flit		(mod_out_flit[`OPTIMSOC_VC_DMAREQ][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_req_valid		(mod_out_valid[`OPTIMSOC_VC_DMAREQ]), // Templated
	 .noc_out_resp_flit		(mod_out_flit[`OPTIMSOC_VC_DMARESP][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_resp_valid		(mod_out_valid[`OPTIMSOC_VC_DMARESP]), // Templated
	 .wb_if_dat_o			(wbs_dat_o),		 // Templated
	 .wb_if_ack_o			(wbs_ack_o),		 // Templated
	 .wb_if_err_o			(wbs_err_o),		 // Templated
	 .wb_if_rty_o			(wbs_rty_o),		 // Templated
	 .wb_adr_o			(wbm_adr_o),		 // Templated
	 .wb_dat_o			(wbm_dat_o),		 // Templated
	 .wb_cyc_o			(wbm_cyc_o),		 // Templated
	 .wb_stb_o			(wbm_stb_o),		 // Templated
	 .wb_sel_o			(wbm_sel_o),		 // Templated
	 .wb_we_o			(wbm_we_o),		 // Templated
	 .wb_cab_o			(wbm_cab_o),		 // Templated
	 .wb_cti_o			(wbm_cti_o),		 // Templated
	 .wb_bte_o			(wbm_bte_o),		 // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .noc_in_req_flit		(mod_in_flit[`OPTIMSOC_VC_DMAREQ][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_req_valid		(mod_in_valid[`OPTIMSOC_VC_DMAREQ]), // Templated
	 .noc_in_resp_flit		(mod_in_flit[`OPTIMSOC_VC_DMARESP][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_resp_valid		(mod_in_valid[`OPTIMSOC_VC_DMARESP]), // Templated
	 .noc_out_req_ready		(mod_out_ready[`OPTIMSOC_VC_DMAREQ]), // Templated
	 .noc_out_resp_ready		(mod_out_ready[`OPTIMSOC_VC_DMARESP]), // Templated
	 .wb_if_adr_i			(wbs_adr_i),		 // Templated
	 .wb_if_dat_i			(wbs_dat_i),		 // Templated
	 .wb_if_cyc_i			(wbs_cyc_i),		 // Templated
	 .wb_if_stb_i			(wbs_stb_i),		 // Templated
	 .wb_if_we_i			(wbs_we_i),		 // Templated
	 .wb_dat_i			(wbm_dat_i),		 // Templated
	 .wb_ack_i			(wbm_ack_i));		 // Templated

   /* lisnoc_router_output_arbiter AUTO_TEMPLATE(
    .fifo_valid_i (mod_out_valid[]),
    .fifo_ready_o (mod_out_ready[]),
    .fifo_flit_i  (mod_out_flit_flat[]),
    .link_valid_o (noc_out_valid[]),
    .link_flit_o (noc_out_flit[]),
    .link_ready_i (noc_out_ready[]),
    ); */
   lisnoc_router_output_arbiter
     #(.vchannels(vchannels))
   u_arb(/*AUTOINST*/
	 // Outputs
	 .fifo_ready_o			(mod_out_ready[(vchannels)-1:0]), // Templated
	 .link_valid_o			(noc_out_valid[(vchannels)-1:0]), // Templated
	 .link_flit_o			(noc_out_flit[`FLIT_WIDTH-1:0]), // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .fifo_valid_i			(mod_out_valid[(vchannels)-1:0]), // Templated
	 .fifo_flit_i			(mod_out_flit_flat[(vchannels)*`FLIT_WIDTH-1:0]), // Templated
	 .link_ready_i			(noc_out_ready[(vchannels)-1:0])); // Templated
   


endmodule // networkadapter_ct

// Local Variables:
// verilog-library-directories:("../../or1200_module/verilog/" "../../compute_tile_bus/verilog" "../../../../../lisnoc/rtl/dma/" "../../../../../lisnoc/rtl/router" "../../wb_ram_b3/verilog")
// verilog-auto-inst-param-value: t
// End:
