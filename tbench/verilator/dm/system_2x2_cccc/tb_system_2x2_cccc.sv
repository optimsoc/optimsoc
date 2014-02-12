`include "dbg_config.vh"

module tb_system_2x2_cccc(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu
   );

   input clk;
   input rst_sys;
   input rst_cpu;

   localparam NUMCORES = 4;
   
   wire [`DEBUG_ITM_PORTWIDTH*NUMCORES-1:0] trace_itm;
   wire [`DEBUG_STM_PORTWIDTH*NUMCORES-1:0] trace_stm;

   wire [`DEBUG_STM_PORTWIDTH-1:0] trace_stm_array [0:NUMCORES-1]; 
   wire                            trace_stm_enable [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire [31:0]                     trace_stm_insn [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire                            trace_stm_wben [0:NUMCORES-1];
   wire [4:0]                      trace_stm_wbreg [0:NUMCORES-1];
   wire [31:0]                     trace_stm_wbdata [0:NUMCORES-1];
   wire [31:0]                     trace_stm_r3 [0:NUMCORES-1] /*verilator public_flat_rd*/;
   
   genvar                          i;
   
   generate
      for (i = 0; i < NUMCORES; i++) begin
         assign trace_stm_array[i] = trace_stm[(i+1)*`DEBUG_STM_PORTWIDTH-1:`DEBUG_STM_PORTWIDTH*i];
         assign trace_stm_enable[i] = trace_stm_array[i][`DEBUG_STM_ENABLE_MSB:`DEBUG_STM_ENABLE_LSB];
         assign trace_stm_insn[i] = trace_stm_array[i][`DEBUG_STM_INSN_MSB:`DEBUG_STM_INSN_LSB];
         assign trace_stm_wben[i] = trace_stm_array[i][`DEBUG_STM_WB_MSB:`DEBUG_STM_WB_LSB];
         assign trace_stm_wbreg[i] = trace_stm_array[i][`DEBUG_STM_WBREG_MSB:`DEBUG_STM_WBREG_LSB];
         assign trace_stm_wbdata[i] = trace_stm_array[i][`DEBUG_STM_WBDATA_MSB:`DEBUG_STM_WBDATA_LSB];

         r3_checker
           u_r3_checker(.clk(clk),
                        .valid(trace_stm_enable[i]),
                        .we (trace_stm_wben[i]),
                        .addr (trace_stm_wbreg[i]),
                        .data (trace_stm_wbdata[i]),
                        .r3 (trace_stm_r3[i]));
      end
   endgenerate

   system_2x2_cccc_dm
     u_system(/*AUTOINST*/
              // Inputs
              .clk                      (clk),
              .rst_sys                  (rst_sys),
              .rst_cpu                  (rst_cpu),
              .trace_itm                (trace_itm),
              .trace_stm                (trace_stm));

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
