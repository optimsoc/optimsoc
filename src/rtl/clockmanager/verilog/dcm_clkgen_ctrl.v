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
 * Xilinx DCM control for the clock manager
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */
module dcm_clkgen_ctrl(/*AUTOARG*/
   // Outputs
   rst_dcm, progen, progdata, rdy_clk,
   // Inputs
   multi, div, clk_in, rst_ctrl, set_clk, locked, progdone, status
   );

   parameter DEFAULT_MULTI = 1;
   parameter DEFAULT_DIV   = 3;

   // inputs
   input wire [7:0] multi;
   input wire [7:0] div;
   input wire       clk_in;
   input wire       rst_ctrl;
   input wire       set_clk;
   input wire       locked;
   input wire       progdone;
   input wire       status;

   // outputs
   output reg       rst_dcm;
   output reg       progen;
   output reg       progdata;
   output reg       rdy_clk;

   reg [1:0]        rstcount;
   reg [1:0]        nxt_rstcount;


   reg [7:0]        multi_reg;
   reg [7:0]        nxt_multi_reg;

   reg [7:0]        div_reg;
   reg [7:0]        nxt_div_reg;

   reg [2:0]        progcount;
   reg [2:0]        nxt_progcount;


   localparam SIZE = 6;
   localparam RESET       = 6'b000000;
   localparam HOLDRST     = 6'b000001;
   localparam INIT        = 6'b000010;
   localparam WAITDCM     = 6'b000011;
   localparam LDD1_1      = 6'b000100;
   localparam LDD1_2      = 6'b000101;
   localparam LOAD_D      = 6'b000110;
   localparam WAIT_2CY_1  = 6'b000111;
   localparam LDM1_1      = 6'b001000;
   localparam LDM1_2      = 6'b001001;
   localparam LOAD_M      = 6'b001010;
   localparam WAIT_2CY_2  = 6'b001011;
   localparam GO          = 6'b001100;
   localparam WAIT_PRDONE = 6'b001101;
   localparam WAIT_LOCKED = 6'b001110;
   localparam RUN         = 6'b001111;

   reg [SIZE-1:0] state;
   reg [SIZE-1:0] nxt_state;

   always @(posedge clk_in) begin
      if (rst_ctrl == 1'b1) begin
         state <= RESET;
         rstcount <= 'b0;
         progcount <= 'bx;
         div_reg <= DEFAULT_DIV;
         multi_reg <= DEFAULT_MULTI;
      end else begin
         state <= nxt_state;
         rstcount <= nxt_rstcount;
         progcount <= nxt_progcount;
         div_reg <= nxt_div_reg;
         multi_reg <= nxt_multi_reg;
      end
   end

   always @(*) begin
      nxt_rstcount = rstcount;

      rst_dcm = 1'b0;
      progen = 1'b0;
      progdata = 1'b0;
      rdy_clk = 1'b0;

      nxt_progcount = progcount;
      nxt_multi_reg = multi_reg;
      nxt_div_reg   = div_reg;

      case(state)
        RESET : begin
           rst_dcm = 1'b1;

           if (rst_ctrl) begin
              nxt_state = RESET;
           end else begin
              nxt_state = HOLDRST;
           end
        end
        HOLDRST: begin
           if (locked) begin
              nxt_state = WAITDCM;
           end else begin
              nxt_state = HOLDRST;
           end
        end
        WAITDCM: begin
           if(progdone == 1'b1) begin // progdone shows DCM is programmable
              nxt_state = RUN;
           end else begin
              nxt_state = WAITDCM;
           end
        end
        INIT: begin
           nxt_state = LDD1_1;
        end
        LDD1_1: begin
           progen = 1'b1;
           progdata = 1'b1;
           nxt_state = LDD1_2;
        end
        LDD1_2: begin
           progen = 1'b1;
           progdata = 1'b0;
           nxt_progcount = 'b0;
           nxt_state = LOAD_D;
        end
        LOAD_D: begin
           progen = 1'b1;
           progdata = div_reg[progcount];
           nxt_progcount = progcount + 1;
           if (progcount == 3'b111) begin
              nxt_state = WAIT_2CY_1;
              nxt_rstcount = 'b0;
           end else begin
              nxt_state = LOAD_D;
           end
        end
        WAIT_2CY_1: begin
           nxt_rstcount = rstcount + 1;
           if (rstcount == 3'b010) begin
              nxt_state = LDM1_1;
           end else begin
              nxt_state = WAIT_2CY_1;
           end
        end
        LDM1_1: begin
           progen = 1'b1;
           progdata = 1'b1;
           nxt_state = LDM1_2;
        end
        LDM1_2: begin
           progen = 1'b1;
           progdata = 1'b1;
           nxt_progcount = 'b0;
           nxt_state = LOAD_M;
        end
        LOAD_M: begin
           progen = 1'b1;
           progdata = multi_reg[progcount];
           nxt_progcount = progcount + 1;
           if (progcount == 3'b111) begin
              nxt_state = WAIT_2CY_2;
              nxt_rstcount = 'b0;
           end else begin
              nxt_state = LOAD_M;
           end
        end
        WAIT_2CY_2: begin
           nxt_rstcount = rstcount + 1;
           if (rstcount == 3'b010) begin
              nxt_state = GO;
           end else begin
              nxt_state = WAIT_2CY_2;
           end
        end
        GO: begin
           progen = 1'b1;
           progdata = 1'b0;
           nxt_state = WAIT_PRDONE;
        end
        WAIT_PRDONE: begin
           if (progdone) begin
              nxt_state = WAIT_LOCKED;
           end else begin
              nxt_state = WAIT_PRDONE;
           end
        end
        WAIT_LOCKED: begin
           if (locked) begin
              nxt_state = RUN;
           end else begin
              nxt_state = WAIT_LOCKED;
           end
        end
        RUN: begin
           rdy_clk = 1'b1;
           if((!locked) && (status == 1'b1)) begin
              nxt_state = RESET;
           end else begin
              if(set_clk == 1'b1) begin
                 nxt_state = INIT;
                 nxt_multi_reg = multi;
                 nxt_div_reg = div;
              end else begin
                 nxt_state = RUN;
              end
           end
        end
        default: begin
           nxt_state = RESET;
        end
      endcase
   end

endmodule // dcm_clkgen_ctrl
