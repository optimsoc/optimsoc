`include "timescale.v"

module tb_compute_tile(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu, cpu_stall
   );

   input clk;
   input rst_sys;
   input rst_cpu;
   input cpu_stall;
   
   parameter noc_flit_data_width = 32;
   parameter noc_flit_type_width = 2;
   localparam noc_flit_width = noc_flit_data_width+noc_flit_type_width;
   parameter vchannels = 3;

   reg [noc_flit_width-1:0] noc_in_flit;
   reg [vchannels-1:0] noc_in_valid;
   wire [vchannels-1:0] noc_in_ready;
   wire [noc_flit_width-1:0] noc_out_flit;
   wire [vchannels-1:0] noc_out_valid;
   reg [vchannels-1:0] noc_out_ready;

   parameter id       = 0;
   parameter cores    = 1;
   /* memory size in bytes */
   parameter mem_size = 1*1024*1024; // 4 MByte
   parameter mem_file = "ct.vmem";

   compute_tile_dm
     #(.id(id),.cores(cores),
       .mem_size(mem_size),.mem_file(mem_file))
     u_compute_tile(/*AUTOINST*/
                    // Outputs
                    .noc_in_ready       (noc_in_ready[vchannels-1:0]),
                    .noc_out_flit       (noc_out_flit[noc_flit_width-1:0]),
                    .noc_out_valid      (noc_out_valid[vchannels-1:0]),
                    // Inputs
                    .clk                (clk),
                    .rst_cpu            (rst_cpu),
                    .rst_sys            (rst_sys),
                    .noc_in_flit        (noc_in_flit[noc_flit_width-1:0]),
                    .noc_in_valid       (noc_in_valid[vchannels-1:0]),
                    .noc_out_ready      (noc_out_ready[vchannels-1:0]),
                    .cpu_stall          (cpu_stall));

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
