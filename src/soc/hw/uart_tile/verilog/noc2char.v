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

`include "uart_defines.v"

`define IDLE            2'b00
`define READY           2'b10
 
module noc2char(/*AUTOARG*/
   // Outputs
   noc_ready, char_valid, char_data,
   // Inputs
   clk, rst, noc_flit, noc_valid, char_ready
   );
  
   input clk;
   input rst;
   
   input [33:0] noc_flit;
   input        noc_valid;
   output reg   noc_ready;
   
   input        char_ready;
   output reg   char_valid;
   output [7:0] char_data;
   
   reg [1:0]    state;
   reg [1:0]    nxt_state;  
   reg [7:0]    character;
   reg [7:0]   nxt_character;
   
   assign char_data = character;
   
   always @(posedge clk) begin
      if(rst) begin
         character <= 'bx;  
         state <= `IDLE;
      end else begin
         character <= nxt_character;
         state <= nxt_state;
      end
   end
   
   always @(*) begin        
      nxt_state= state;     
      noc_ready = 1'b0;
      char_valid = 1'b0;
      nxt_character = character;
      
      case(state)
        `IDLE: begin 
           noc_ready = 1'b1;
           if (noc_valid) begin
              if(noc_flit[33:32]==2'b11) begin
                 nxt_state = `READY;
                 nxt_character = noc_flit[7:0];
              end else begin  
                 nxt_state = `IDLE;                          
              end
           end else begin
              nxt_state = `IDLE;                        
           end
        end
        
        `READY :  begin         
           char_valid = 1'b1;
           if (char_ready) begin
              nxt_state = `IDLE;                               
           end else begin
              nxt_state = `READY;  
           end
        end     
      endcase
   end
 endmodule
