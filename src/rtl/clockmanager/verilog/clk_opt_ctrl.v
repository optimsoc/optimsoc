/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Xilinx DCM clock manager option control
 *
 * At least 3 ctrl clk cyles between requests and first request only 4 cycles
 * after global reset
 *
 * TODO: Rewrite FSMs
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Mark Sagi, mark.sagi@mytum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

`include "optimsoc_def.vh"

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

