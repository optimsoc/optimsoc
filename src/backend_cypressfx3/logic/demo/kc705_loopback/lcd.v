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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module lcd(/*AUTOARG*/
   // Outputs
   data, en, rw, rs,
   // Inputs
   clk, rst, in_data, in_enable, in_pos, in_row
   );

   parameter FREQ = 32'hx;
   // Clock divisor down to 657 Hz
   localparam CLOCK_DIVISOR = FREQ / 657;
   
   output reg [3:0] data;
   output reg       en;
   output reg       rw;
   output reg       rs;

   input        clk;
   input        rst;

   input [7:0]  in_data;
   input        in_enable;
   input [3:0]  in_pos;
   input        in_row;
   
   reg [7:0]    mem0 [0:15];
   reg [7:0]    mem1 [0:15];

   always @(posedge clk) begin
      if (in_enable) begin
         if (in_row==0) begin
            mem0[in_pos] <= in_data;
         end else begin
            mem1[in_pos] <= in_data;
         end
      end
   end

/*   initial begin
      mem0[0] = 8'h67; // g
      mem0[1] = 8'h6c; // l
      mem0[2] = 8'h69; // i
      mem0[3] = 8'h70; // p
      mem0[4] = 8'h20; // 
      mem0[5] = 8'h6c; // l
      mem0[6] = 8'h6f; // o
      mem0[7] = 8'h6f; // o
      mem0[8] = 8'h70; // p
      mem0[9] = 8'h62; // b
      mem0[10] = 8'h61; // a
      mem0[11] = 8'h63; // c
      mem0[12] = 8'h6b; // k
      mem0[13] = 8'h20; // 
      mem0[14] = 8'h20; // 
      mem0[15] = 8'h23; // #

      mem1[0] = 8'h20; // 
      mem1[1] = 8'h20; // 
      mem1[2] = 8'h20; // 
      mem1[3] = 8'h2d; // -
      mem1[4] = 8'h2d; // -
      mem1[5] = 8'h2d; // -
      mem1[6] = 8'h2e; // .
      mem1[7] = 8'h2d; // -
      mem1[8] = 8'h2d; // -
      mem1[9] = 8'h2d; // -
      mem1[10] = 8'h20; // 
      mem1[11] = 8'h4d; // M
      mem1[12] = 8'h62; // b
      mem1[13] = 8'h2f; // /
      mem1[14] = 8'h73; // s
      mem1[15] = 8'h20; // 
   end*/
   
   
   reg clk_slow;
   integer clk_slow_counter;
   reg     rst_slow;
   
   
   always @(posedge clk) begin
      if (rst) begin
         clk_slow <= 0;
         clk_slow_counter <= 0;
         rst_slow <= 1;
      end else begin
         if (clk_slow_counter == CLOCK_DIVISOR) begin
            clk_slow <= ~clk_slow;
            clk_slow_counter <= 0;
            if (!clk_slow & rst_slow) begin
               rst_slow <= 1;
            end else begin
               rst_slow <= 0;
            end
         end else begin
            clk_slow <= clk_slow;
            clk_slow_counter <= clk_slow_counter + 1;
            rst_slow <= rst_slow;
         end
      end
   end
   
   reg [4:0] state;
   reg [4:0] nxt_state;

   reg       waitstate;
   reg       nxt_waitstate;
   
   
   localparam STATE_FSET0   = 0;
   localparam STATE_FSET1   = 1;
   localparam STATE_FSET2   = 2;
   localparam STATE_ONH     = 3;
   localparam STATE_ONL     = 4;                      
   localparam STATE_CLEARH  = 5;
   localparam STATE_CLEARL  = 6;
   localparam STATE_MODEH   = 7;
   localparam STATE_MODEL   = 8;
   localparam STATE_UPSETH  = 9;
   localparam STATE_UPSETL  = 10;
   localparam STATE_UPH   = 11;
   localparam STATE_UPL   = 12;
   localparam STATE_DOWNSETH   = 13;
   localparam STATE_DOWNSETL   = 14;
   localparam STATE_DOWNH   = 15;
   localparam STATE_DOWNL   = 16;
   localparam STATE_STOP    = 17;
   
   reg [3:0]         counter;
   reg [3:0]         nxt_counter;

   always @(*) begin
      data = 4'bxxxx;
      en   = 1'b0;
      rw   = 1'bx;
      rs   = 1'bx;
      nxt_waitstate = waitstate;
      nxt_counter = counter;
      
      case(state)
        STATE_FSET0: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_FSET0;
           end else begin
              en = 1'b1;
              data = 4'b0010;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              nxt_state = STATE_FSET1;
           end
        end
        STATE_FSET1: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_FSET1;
           end else begin
              en = 1'b1;
              data = 4'b0010;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              nxt_state = STATE_FSET2;
           end
        end
        STATE_FSET2: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_FSET2;
           end else begin
              en = 1'b1;
              data = 4'b1100;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              nxt_state = STATE_ONH;
           end
        end
        STATE_ONH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 0;
              nxt_state = STATE_ONH;
           end else begin
              en = 1'b1;
              data = 4'b0000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1;
              nxt_state = STATE_ONL;
           end
        end
        STATE_ONL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 0;
              nxt_state = STATE_ONL;
           end else begin
              en = 1'b1;
              data = 4'b1100;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1;
              nxt_state = STATE_CLEARH;
           end
        end        
        STATE_CLEARH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_CLEARH;
           end else begin
              en   = 1'b1;
              data = 4'b0000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_CLEARL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_CLEARL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_CLEARL;
           end else begin
              en   = 1'b1;
              data = 4'b0001;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_MODEH;
              nxt_waitstate = 1'b1;
           end // else: !if(waitstate)
        end // case: STATE_CLEARL
        STATE_MODEH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_MODEH;
           end else begin
              en   = 1'b1;
              data = 4'b0000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_MODEL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_MODEL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_MODEL;
           end else begin
              en   = 1'b1;
              data = 4'b0110;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_UPSETH;
              nxt_waitstate = 1'b1;
           end // else: !if(waitstate)
        end // case: STATE_CLEARL
        STATE_UPSETH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_UPSETH;
           end else begin
              en   = 1'b1;
              data = 4'b1000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_UPSETL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_UPSETL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_UPSETL;
           end else begin
              en   = 1'b1;
              data = 4'b0000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              nxt_state = STATE_UPH;
              nxt_counter = 0;
           end // else: !if(waitstate)
        end // case: STATE_CLEARL
        STATE_UPH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_UPH;
           end else begin
              en   = 1'b1;
              data = mem0[counter][7:4];
              rs   = 1'b1;
              rw   = 1'b0;
              nxt_state = STATE_UPL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_UPL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_UPL;
           end else begin
              en   = 1'b1;
              data = mem0[counter][3:0];
              rs   = 1'b1;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              if (counter==15) begin
                 nxt_state = STATE_DOWNSETH;
              end else begin
                 nxt_state = STATE_UPH;
                 nxt_counter = counter + 1;
              end
           end // else: !if(waitstate)
        end
        STATE_DOWNSETH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_DOWNSETH;
           end else begin
              en   = 1'b1;
              data = 4'b1010;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_state = STATE_DOWNSETL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_DOWNSETL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_DOWNSETL;
           end else begin
              en   = 1'b1;
              data = 4'b1000;
              rs   = 1'b0;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              nxt_state = STATE_DOWNH;
              nxt_counter = 0;
           end // else: !if(waitstate)
        end // case: STATE_CLEARL
        STATE_DOWNH: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_DOWNH;
           end else begin
              en   = 1'b1;
              data = mem1[counter][7:4];
              rs   = 1'b1;
              rw   = 1'b0;
              nxt_state = STATE_DOWNL;
              nxt_waitstate = 1'b1;
           end
        end
        STATE_DOWNL: begin
           if (waitstate) begin
              en = 1'b0;
              nxt_waitstate = 1'b0;
              nxt_state = STATE_DOWNL;
           end else begin
              en   = 1'b1;
              data = mem1[counter][3:0];
              rs   = 1'b1;
              rw   = 1'b0;
              nxt_waitstate = 1'b1;
              if (counter==15) begin
                 nxt_state = STATE_UPSETH;
              end else begin
                 nxt_state = STATE_DOWNH;
                 nxt_counter = counter + 1;
              end
           end // else: !if(waitstate)
        end // case: STATE_CLEARL
        STATE_STOP: begin
           nxt_state = STATE_STOP;
        end
        default: begin
           nxt_state = STATE_FSET0;
        end
      endcase // case (state)
   end

   always @(posedge clk_slow) begin
      if (rst_slow) begin
         state <= STATE_FSET0;
         waitstate <= 1;
         counter <= 0;
      end else begin
         state <= nxt_state;
         waitstate <= nxt_waitstate;
         counter <= nxt_counter;
      end
   end
   
endmodule // ml605_display
