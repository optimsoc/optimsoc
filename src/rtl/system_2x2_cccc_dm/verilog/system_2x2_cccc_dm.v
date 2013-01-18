`include "lisnoc_def.vh"

module system_2x2_cccc_dm(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu
   );

   input clk, rst_sys, rst_cpu;

   parameter noc_data_width = 32;
   parameter noc_type_width = 2;
   localparam noc_flit_width = noc_data_width+noc_type_width;

   localparam flit_width = noc_flit_width;
      
   localparam vchannels = 3;

   parameter mem_size = 128*1024;
   
   // Flits from NoC->tiles
   wire [noc_flit_width-1:0] link_in_flit[0:3];
   wire [vchannels-1:0] link_in_valid[0:3];
   wire [vchannels-1:0] link_in_ready[0:3];

   // Flits from tiles->NoC
   wire [noc_flit_width-1:0] link_out_flit[0:3];
   wire [vchannels-1:0] link_out_valid[0:3];
   wire [vchannels-1:0] link_out_ready[0:3];

   /* lisnoc_mesh2x2 AUTO_TEMPLATE(
    .link\(.*\)_in_\(.*\)_.* (link_out_\2[\1][]),
    .link\(.*\)_out_\(.*\)_.* (link_in_\2[\1][]),
    .clk(clk),
    .rst(rst_sys),
    ); */
   lisnoc_mesh2x2
     #(.vchannels(vchannels))
   u_mesh(/*AUTOINST*/
          // Outputs
          .link0_in_ready_o             (link_out_ready[0][(vchannels)-1:0]), // Templated
          .link0_out_flit_o             (link_in_flit[0][flit_width-1:0]), // Templated
          .link0_out_valid_o            (link_in_valid[0][(vchannels)-1:0]), // Templated
          .link1_in_ready_o             (link_out_ready[1][(vchannels)-1:0]), // Templated
          .link1_out_flit_o             (link_in_flit[1][flit_width-1:0]), // Templated
          .link1_out_valid_o            (link_in_valid[1][(vchannels)-1:0]), // Templated
          .link2_in_ready_o             (link_out_ready[2][(vchannels)-1:0]), // Templated
          .link2_out_flit_o             (link_in_flit[2][flit_width-1:0]), // Templated
          .link2_out_valid_o            (link_in_valid[2][(vchannels)-1:0]), // Templated
          .link3_in_ready_o             (link_out_ready[3][(vchannels)-1:0]), // Templated
          .link3_out_flit_o             (link_in_flit[3][flit_width-1:0]), // Templated
          .link3_out_valid_o            (link_in_valid[3][(vchannels)-1:0]), // Templated
          // Inputs
          .clk                          (clk),                   // Templated
          .rst                          (rst_sys),               // Templated
          .link0_in_flit_i              (link_out_flit[0][flit_width-1:0]), // Templated
          .link0_in_valid_i             (link_out_valid[0][(vchannels)-1:0]), // Templated
          .link0_out_ready_i            (link_in_ready[0][(vchannels)-1:0]), // Templated
          .link1_in_flit_i              (link_out_flit[1][flit_width-1:0]), // Templated
          .link1_in_valid_i             (link_out_valid[1][(vchannels)-1:0]), // Templated
          .link1_out_ready_i            (link_in_ready[1][(vchannels)-1:0]), // Templated
          .link2_in_flit_i              (link_out_flit[2][flit_width-1:0]), // Templated
          .link2_in_valid_i             (link_out_valid[2][(vchannels)-1:0]), // Templated
          .link2_out_ready_i            (link_in_ready[2][(vchannels)-1:0]), // Templated
          .link3_in_flit_i              (link_out_flit[3][flit_width-1:0]), // Templated
          .link3_in_valid_i             (link_out_valid[3][(vchannels)-1:0]), // Templated
          .link3_out_ready_i            (link_in_ready[3][(vchannels)-1:0])); // Templated

   /* compute_tile_dm AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .cpu_stall  (1'b0),
    );*/

   compute_tile_dm
     #(.id(0),.mem_size(mem_size))
   u_ct0(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[0][vchannels-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[0][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[0][vchannels-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[0][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[0][vchannels-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[0][vchannels-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated
   
   compute_tile_dm
     #(.id(1),.mem_size(mem_size))
   u_ct1(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[1][vchannels-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[1][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[1][vchannels-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[1][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[1][vchannels-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[1][vchannels-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated

   compute_tile_dm
     #(.id(2),.mem_size(mem_size))
   u_ct2(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[2][vchannels-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[2][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[2][vchannels-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[2][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[2][vchannels-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[2][vchannels-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated
  
   compute_tile_dm
     #(.id(3),.mem_size(mem_size))
   u_ct3(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[3][vchannels-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[3][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[3][vchannels-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[3][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[3][vchannels-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[3][vchannels-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated
   
endmodule // system_2x2_cccc

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
