/* Copyright (c) 2013 by the author(s)
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
 * =============================================================================
 *
 * Xilinx DCM clock manager option control
 *
 * At least 3 ctrl clk cyles between requests and first request only 4 cycles
 * after global reset
 *
 * TODO: Rewrite FSMs
 *
 * Author(s):
 *   Mark Sagi <mark.sagi@mytum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module clk_opt_ctrl (/*AUTOARG*/
   // Outputs
   dcm_set, ddc_rst, freq_mode, multi, div,
   // Inputs
   clk, rst_sys, req_clk, f_req
   );

   input clk;
   input rst_sys;
   input req_clk;
   input [2:0] f_req;

   output reg dcm_set;
   output reg ddc_rst;
   output reg freq_mode;
   output reg [7:0] multi;
   output reg [7:0] div;

   // FSM States
   localparam SIZE     = 4;
   localparam RESET    = 4'b0000;
   localparam SET_DFT  = 4'b0001;
   localparam RUN      = 4'b0010;
   localparam SET_WAIT = 4'b0011;
   localparam SET      = 4'b0100;
   localparam INIT     = 4'b0101;

   // Setting->MHz: 0: 32, 1: 28, 2: 24, 3: 20, 4: 16, 5: 12, 6: 10, 7: 10
   // M and D are always plus 1!
   localparam [63:0] MULTI = { 8'd4, 8'd4, 8'd1, 8'd1, 8'd4, 8'd1, 8'd6, 8'd1 };
   localparam [63:0] DIV = { 8'd23, 8'd23, 8'd7, 8'd5, 8'd11, 8'd13, 8'd11, 8'd2 };

   wire [7:0]             MULTI_array [0:7];
   wire [7:0]             DIV_array [0:7];

   genvar  i;
   generate
      for (i=0;i<8;i=i+1) begin
         assign MULTI_array[i] = MULTI[(i+1)*8-1:i*8];
         assign DIV_array[i] = DIV[(i+1)*8-1:i*8];
      end
   endgenerate

   reg [2:0] count;
   reg [SIZE-1:0] state;
   wire [SIZE-1:0] next_state;

   assign next_state = fsm_function(state, rst_sys, req_clk, count);

   function [SIZE-1:0] fsm_function;
      input [SIZE-1:0] state;
      input rst_sys;
      input req_clk;
      input [2:0] count;

      case(state)
         RESET: begin
            if (rst_sys == 1'b0) begin
               fsm_function = INIT;
            end else begin
               fsm_function = RESET;
           end
         end
         INIT: begin
            if (count == 3'b100) begin
               fsm_function = RUN;
            end else begin
               fsm_function = INIT;
            end
         end
         SET_DFT: begin
            fsm_function = RUN;
         end
         RUN: begin
            if (req_clk == 1'b1) begin
               fsm_function = SET;
            end else begin
               fsm_function = RUN;
            end
         end
         SET: begin
            fsm_function = RUN;
         end
         default: begin
            fsm_function = RESET;
         end
      endcase
   endfunction

   always @(posedge clk or posedge rst_sys) begin
      if (rst_sys == 1'b1) begin
         state <= 4'b0000;
      end else begin
         state <= next_state;
      end
   end

   always @(posedge clk) begin
      case (state)
         RESET: begin
            dcm_set = 1'b0;
            ddc_rst = 1'b1;
            freq_mode = 1'b0;
            multi = 8'h00;
            div = 8'h00;
            count = 3'b000;
        end
        INIT: begin
           dcm_set = 1'b0;
           ddc_rst = 1'b0;
           freq_mode = 1'b0;
           multi = 8'h00;
           div = 8'h00;
           count = count + 1;
        end
        SET_DFT: begin
           count = 3'b000;
           dcm_set = 1'b1;
           ddc_rst = 1'b0;
           freq_mode = 1'b0;
           multi = MULTI_array[`OPTIMSOC_CDC_DYN_DEFAULT];
           div   = DIV_array[`OPTIMSOC_CDC_DYN_DEFAULT];
        end
        SET: begin
           count = 3'b000;
           dcm_set = 1'b1;
           ddc_rst = 1'b0;
           freq_mode = 1'b0;
           multi = MULTI_array[f_req];
           div   = DIV_array[f_req];
        end
        RUN: begin
           dcm_set = 1'b0;
           ddc_rst = 1'b0;
           freq_mode = 1'b0;
           multi = 8'h00;
           div = 8'h00;
        end
      endcase
   end

endmodule
