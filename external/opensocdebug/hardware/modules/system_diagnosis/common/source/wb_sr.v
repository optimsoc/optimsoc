/* Copyright (c) 2016 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * wrapper module for delay shiftregister of write back signals */

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
