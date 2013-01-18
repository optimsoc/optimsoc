`include "timescale.v"

module tb_system_2x2_cccc();

   reg clk;
   reg rst_sys;
   reg rst_cpu;

   system_2x2_cccc_dm
     u_system(/*AUTOINST*/
              // Inputs
              .clk                      (clk),
              .rst_sys                  (rst_sys),
              .rst_cpu                  (rst_cpu));

   initial begin
      clk = 1'b1;
      rst_sys = 1'b1;
      rst_cpu = 1'b1;
      #15;
      rst_sys = 1'b0;
      rst_cpu = 1'b0;      
   end

   always clk = #1.25 ~clk;

   wire [3:0] termination;
   
   /* trace_monitor AUTO_TEMPLATE(
    .enable  (~u_system.u_ct@.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze),
    .wb_pc   (u_system.u_ct@.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc),
    .wb_insn (u_system.u_ct@.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn),
    .r3      (u_system.u_ct@.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]),
    .supv    (u_system.u_ct@.u_core0.u_cpu.or1200_cpu.supv),
    .termination (termination[@]),
    .termination_all (termination),
    .if_valid_en ('0),
    .if_valid_pos ('0),
    .ctrl_done_en ('0),
    .ctrl_done_pos ('0),
    ); */
   trace_monitor
     #(.stdout_filename("stdout.0"),.tracefile_filename("trace.0"),.term_cross_num(4))
   u_mon0(/*AUTOINST*/
          // Outputs
          .termination                  (termination[0]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .supv                         (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.supv), // Templated
          .if_valid_en                  ('0),                    // Templated
          .if_valid_pos                 ('0),                    // Templated
          .ctrl_done_en                 ('0),                    // Templated
          .ctrl_done_pos                ('0),                    // Templated
          .termination_all              (termination));          // Templated
   
   trace_monitor
     #(.stdout_filename("stdout.1"),.tracefile_filename("trace.1"),.term_cross_num(4))
   u_mon1(/*AUTOINST*/
          // Outputs
          .termination                  (termination[1]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .supv                         (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.supv), // Templated
          .if_valid_en                  ('0),                    // Templated
          .if_valid_pos                 ('0),                    // Templated
          .ctrl_done_en                 ('0),                    // Templated
          .ctrl_done_pos                ('0),                    // Templated
          .termination_all              (termination));          // Templated

   trace_monitor
     #(.stdout_filename("stdout.2"),.tracefile_filename("trace.2"),.term_cross_num(4))
   u_mon2(/*AUTOINST*/
          // Outputs
          .termination                  (termination[2]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .supv                         (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.supv), // Templated
          .if_valid_en                  ('0),                    // Templated
          .if_valid_pos                 ('0),                    // Templated
          .ctrl_done_en                 ('0),                    // Templated
          .ctrl_done_pos                ('0),                    // Templated
          .termination_all              (termination));          // Templated
   
   trace_monitor
     #(.stdout_filename("stdout.3"),.tracefile_filename("trace.3"),.term_cross_num(4))
   u_mon3(/*AUTOINST*/
          // Outputs
          .termination                  (termination[3]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .supv                         (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.supv), // Templated
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
