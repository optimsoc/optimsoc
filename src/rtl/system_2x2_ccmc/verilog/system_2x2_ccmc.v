`include "lisnoc_def.vh"


module system_2x2_ccmc(/*AUTOARG*/
   // Inputs
   clk, rst
   );

   input clk, rst;

   localparam vchannels = 3;
   
   // Flits from NoC->tiles
   wire [`FLIT_WIDTH-1:0] link_in_flit[0:3];
   wire [vchannels-1:0] link_in_valid[0:3];
   wire [vchannels-1:0] link_in_ready[0:3];

   // Flits from tiles->NoC
   wire [`FLIT_WIDTH-1:0] link_out_flit[0:3];
   wire [vchannels-1:0] link_out_valid[0:3];
   wire [vchannels-1:0] link_out_ready[0:3];

   /* lisnoc_mesh2x2 AUTO_TEMPLATE(
    .link\(.*\)_in_\(.*\)_.* (link_out_\2[\1][]),
    .link\(.*\)_out_\(.*\)_.* (link_in_\2[\1][]),
    ); */
   lisnoc_mesh2x2
     #(.vchannels(vchannels))
   u_mesh(/*AUTOINST*/
	  // Outputs
	  .link0_in_ready_o		(link_out_ready[0][(vchannels)-1:0]), // Templated
	  .link0_out_flit_o		(link_in_flit[0][`FLIT_WIDTH-1:0]), // Templated
	  .link0_out_valid_o		(link_in_valid[0][(vchannels)-1:0]), // Templated
	  .link1_in_ready_o		(link_out_ready[1][(vchannels)-1:0]), // Templated
	  .link1_out_flit_o		(link_in_flit[1][`FLIT_WIDTH-1:0]), // Templated
	  .link1_out_valid_o		(link_in_valid[1][(vchannels)-1:0]), // Templated
	  .link2_in_ready_o		(link_out_ready[2][(vchannels)-1:0]), // Templated
	  .link2_out_flit_o		(link_in_flit[2][`FLIT_WIDTH-1:0]), // Templated
	  .link2_out_valid_o		(link_in_valid[2][(vchannels)-1:0]), // Templated
	  .link3_in_ready_o		(link_out_ready[3][(vchannels)-1:0]), // Templated
	  .link3_out_flit_o		(link_in_flit[3][`FLIT_WIDTH-1:0]), // Templated
	  .link3_out_valid_o		(link_in_valid[3][(vchannels)-1:0]), // Templated
	  // Inputs
	  .clk				(clk),
	  .rst				(rst),
	  .link0_in_flit_i		(link_out_flit[0][`FLIT_WIDTH-1:0]), // Templated
	  .link0_in_valid_i		(link_out_valid[0][(vchannels)-1:0]), // Templated
	  .link0_out_ready_i		(link_in_ready[0][(vchannels)-1:0]), // Templated
	  .link1_in_flit_i		(link_out_flit[1][`FLIT_WIDTH-1:0]), // Templated
	  .link1_in_valid_i		(link_out_valid[1][(vchannels)-1:0]), // Templated
	  .link1_out_ready_i		(link_in_ready[1][(vchannels)-1:0]), // Templated
	  .link2_in_flit_i		(link_out_flit[2][`FLIT_WIDTH-1:0]), // Templated
	  .link2_in_valid_i		(link_out_valid[2][(vchannels)-1:0]), // Templated
	  .link2_out_ready_i		(link_in_ready[2][(vchannels)-1:0]), // Templated
	  .link3_in_flit_i		(link_out_flit[3][`FLIT_WIDTH-1:0]), // Templated
	  .link3_in_valid_i		(link_out_valid[3][(vchannels)-1:0]), // Templated
	  .link3_out_ready_i		(link_in_ready[3][(vchannels)-1:0])); // Templated

   /* compute_tile AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    );*/

   /* memory_tile AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    );*/
   
   compute_tile
     #(.id(0))
   u_ct0(/*AUTOINST*/
	 // Outputs
	 .noc_in_ready			(link_in_ready[0][vchannels-1:0]), // Templated
	 .noc_out_flit			(link_out_flit[0][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_valid			(link_out_valid[0][vchannels-1:0]), // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .noc_in_flit			(link_in_flit[0][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_valid			(link_in_valid[0][vchannels-1:0]), // Templated
	 .noc_out_ready			(link_out_ready[0][vchannels-1:0])); // Templated
   
   compute_tile
     #(.id(1))
   u_ct1(/*AUTOINST*/
	 // Outputs
	 .noc_in_ready			(link_in_ready[1][vchannels-1:0]), // Templated
	 .noc_out_flit			(link_out_flit[1][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_valid			(link_out_valid[1][vchannels-1:0]), // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .noc_in_flit			(link_in_flit[1][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_valid			(link_in_valid[1][vchannels-1:0]), // Templated
	 .noc_out_ready			(link_out_ready[1][vchannels-1:0])); // Templated

   memory_tile
     #(.id(2))
   u_mt2(/*AUTOINST*/
	 // Outputs
	 .noc_in_ready			(link_in_ready[2][vchannels-1:0]), // Templated
	 .noc_out_flit			(link_out_flit[2][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_valid			(link_out_valid[2][vchannels-1:0]), // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .noc_in_flit			(link_in_flit[2][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_valid			(link_in_valid[2][vchannels-1:0]), // Templated
	 .noc_out_ready			(link_out_ready[2][vchannels-1:0])); // Templated
   
   
   compute_tile
     #(.id(3))
   u_ct3(/*AUTOINST*/
	 // Outputs
	 .noc_in_ready			(link_in_ready[3][vchannels-1:0]), // Templated
	 .noc_out_flit			(link_out_flit[3][`FLIT_WIDTH-1:0]), // Templated
	 .noc_out_valid			(link_out_valid[3][vchannels-1:0]), // Templated
	 // Inputs
	 .clk				(clk),
	 .rst				(rst),
	 .noc_in_flit			(link_in_flit[3][`FLIT_WIDTH-1:0]), // Templated
	 .noc_in_valid			(link_in_valid[3][vchannels-1:0]), // Templated
	 .noc_out_ready			(link_out_ready[3][vchannels-1:0])); // Templated
   
endmodule // system_2x2_ccmc

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../compute_tile/verilog" "../../memory_tile/verilog")
// verilog-auto-inst-param-value: t
// End:
