`include "timescale.v"

module tb_system_2x2_ccmc();

   reg clk;
   reg rst;

   system_2x2_ccmc
     u_system(/*AUTOINST*/
	      // Inputs
	      .clk		(clk),
	      .rst		(rst));

   initial begin
      clk = 1'b1;
      rst = 1'b1;
      rst = #15 1'b0;
      
   end

   always clk = #5 ~clk;

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("../../src/rtl/system_2x2_ccmc/verilog")
// verilog-auto-inst-param-value: t
// End:
