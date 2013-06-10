
module tb_system_2x2_cccc(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu
   );

   input clk;
   input rst_sys;
   input rst_cpu;

   system_2x2_cccc_dm
     u_system(/*AUTOINST*/
              // Inputs
              .clk                      (clk),
              .rst_sys                  (rst_sys),
              .rst_cpu                  (rst_cpu));

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
