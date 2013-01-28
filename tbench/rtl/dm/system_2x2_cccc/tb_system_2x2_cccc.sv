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
    // Parameters
    .id (@),
    .enable_trace (0),
    .tracefile_filename (""),
    .stdout_filename ("stdout.@"),
    .term_cross_num (4),
    ); */
   trace_monitor
     #(/*AUTOINSTPARAM*/
       // Parameters
       .id                              (0),                     // Templated
       .enable_trace                    (0),                     // Templated
       .stdout_filename                 ("stdout.0"),            // Templated
       .tracefile_filename              (""),                    // Templated
       .term_cross_num                  (4))                     // Templated
   u_mon0(/*AUTOINST*/
          // Outputs
          .termination                  (termination[0]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct0.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .termination_all              (termination));          // Templated
   
   trace_monitor
     #(/*AUTOINSTPARAM*/
       // Parameters
       .id                              (1),                     // Templated
       .enable_trace                    (0),                     // Templated
       .stdout_filename                 ("stdout.1"),            // Templated
       .tracefile_filename              (""),                    // Templated
       .term_cross_num                  (4))                     // Templated
   u_mon1(/*AUTOINST*/
          // Outputs
          .termination                  (termination[1]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct1.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .termination_all              (termination));          // Templated

   trace_monitor
     #(/*AUTOINSTPARAM*/
       // Parameters
       .id                              (2),                     // Templated
       .enable_trace                    (0),                     // Templated
       .stdout_filename                 ("stdout.2"),            // Templated
       .tracefile_filename              (""),                    // Templated
       .term_cross_num                  (4))                     // Templated
   u_mon2(/*AUTOINST*/
          // Outputs
          .termination                  (termination[2]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct2.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .termination_all              (termination));          // Templated
   
   trace_monitor
     #(/*AUTOINSTPARAM*/
       // Parameters
       .id                              (3),                     // Templated
       .enable_trace                    (0),                     // Templated
       .stdout_filename                 ("stdout.3"),            // Templated
       .tracefile_filename              (""),                    // Templated
       .term_cross_num                  (4))                     // Templated
   u_mon3(/*AUTOINST*/
          // Outputs
          .termination                  (termination[3]),        // Templated
          // Inputs
          .clk                          (clk),
          .enable                       (~u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze), // Templated
          .wb_pc                        (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc), // Templated
          .wb_insn                      (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn), // Templated
          .r3                           (u_system.u_ct3.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]), // Templated
          .termination_all              (termination));          // Templated
   
endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
