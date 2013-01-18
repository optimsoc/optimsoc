`include "timescale.v"

module tb_compute_tile();

   reg clk;
   reg rst_sys;
   reg rst_cpu;
   reg cpu_stall;
   
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
   parameter mem_size = 4*1024*1024; // 4 MByte
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

   initial begin
      clk = 1'b1;
      rst_sys = 1'b1;
      rst_cpu = 1'b1;
      noc_out_ready = {vchannels{1'b1}};
      noc_in_valid = '0;
      cpu_stall = 0;
      #15;
      rst_sys = 1'b0;
      rst_cpu = 1'b0;      
   end

   always clk = #1.25 ~clk;

   wire termination;
   
   /* trace_monitor AUTO_TEMPLATE(
    .enable  (~u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze),
    .wb_pc   (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc),
    .wb_insn (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn),
    .r3      (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]),
    .supv    (u_compute_tile.u_core0.u_cpu.or1200_cpu.supv),
    .if_valid_en ('0),
    .if_valid_pos ('0),
    .ctrl_done_en ('0),
    .ctrl_done_pos ('0),
    .termination  (termination),
    .termination_all (termination),
    ); */
   trace_monitor
     #(.stdout_filename("stdout"),.tracefile_filename("trace"))
   u_mon0(/*AUTOINST*/
          // Outputs
          .termination                  (termination),           // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_compute_tile.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .supv                         (u_compute_tile.u_core0.u_cpu.or1200_cpu.supv), // Templated
          .if_valid_en                  ('0),                    // Templated
          .if_valid_pos                 ('0),                    // Templated
          .ctrl_done_en                 ('0),                    // Templated
          .ctrl_done_pos                ('0),                    // Templated
          .termination_all              (termination));          // Templated
   
endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
