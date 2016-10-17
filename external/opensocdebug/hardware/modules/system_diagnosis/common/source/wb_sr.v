/* wrapper module for delay shiftregister of write back signals */

`include "diagnosis_config.vh"

module wb_sr(/*AUTOARG*/
   // Outputs
   wb_enable_o, wb_reg_o, wb_data_o,
   // Inputs
   clk, rst, wb_enable_i, wb_reg_i, wb_data_i
   );

   input clk, rst;

   input wb_enable_i;
   input [`DIAGNOSIS_WB_REG_WIDTH-1:0] wb_reg_i;
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] wb_data_i;
   output                               wb_enable_o;
   output [`DIAGNOSIS_WB_REG_WIDTH-1:0] wb_reg_o;
   output [`DIAGNOSIS_WB_DATA_WIDTH-1:0] wb_data_o;

   parameter DATA_WIDTH = `DIAGNOSIS_WB_DATA_WIDTH + `DIAGNOSIS_WB_REG_WIDTH + 1;
   
   wire [DATA_WIDTH-1:0] din;                  
   assign din = {wb_enable_i, wb_reg_i, wb_data_i};
   wire [DATA_WIDTH-1:0] dout;
   assign wb_enable_o = dout[DATA_WIDTH-1];
   assign wb_reg_o = dout[`DIAGNOSIS_WB_DATA_WIDTH + `DIAGNOSIS_WB_REG_WIDTH -1 : `DIAGNOSIS_WB_DATA_WIDTH];
   assign wb_data_o = dout[`DIAGNOSIS_WB_DATA_WIDTH-1:0];
   
   stm_data_sr
     #(.DELAY_CYCLES(`DIAGNOSIS_SNAPSHOT_DELAY),
       .DATA_WIDTH(`DIAGNOSIS_WB_DATA_WIDTH + `DIAGNOSIS_WB_REG_WIDTH + 1))
     stm_data_sr(
                 // Outputs
                 .dout                  (dout),
                 // Inputs
                 .clk                   (clk),
                 .rst                   (rst),
                 .din                   (din));

   
endmodule // wb_sr
