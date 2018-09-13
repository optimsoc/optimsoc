/* Copyright (c) 2018 by the author(s)
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
 * Single-Port memory implemented using the SAED32 primitives
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module sram_sp_impl_saed32(/*AUTOARG*/
   // Outputs
   dout,
   // Inputs
   clk, rst, ce, we, oe, waddr, din, sel
   );

   import optimsoc_functions::*;

   // byte address width
   parameter AW = 32;
   // data width (must be multiple of 8 for byte selects to work)
   parameter DW = 32;

   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;

   // word address width
   parameter WORD_AW = AW - (SW >> 1);

   // size of the memory in bytes
   parameter MEM_SIZE_BYTE = 'hx;

   localparam MEM_SIZE_WORDS = MEM_SIZE_BYTE / SW;

   input                clk;  // Clock
   input                rst;  // Reset
   input                ce;   // Chip enable input
   input                we;   // Write enable input
   input                oe;   // Output enable input
   input [WORD_AW-1:0]  waddr; // word address
   input [DW-1:0]       din;  // input data bus
   input [SW-1:0]       sel;  // select bytes
   output [DW-1:0]      dout; // output data bus

   logic [47:0] dout_int;
   logic [47:0] din_int;

   assign din_int = {{(48 - DW){1'b0}}, din[DW-1:0]};
   assign dout = dout_int[DW-1:0];

   generate
      if (WORD_AW <= 6) begin
         SRAM1RW64x32
            u_ramb(
               .CE(clk),
               .A(waddr),
               .WEB(~we),
               .OEB(1'b0),
               .CSB(1'b0),
               .I(din_int[31:0]),
               .O(dout_int[31:0]));

         if (DW > 32) begin
            initial begin
               $display("DW > 32 not supported.");
               $finish();
            end
         end
      end else if (WORD_AW == 7) begin
         SRAM1RW128x48
            u_ramb(
               .CE(clk),
               .A(waddr),
               .WEB(~we),
               .OEB(1'b0),
               .CSB(1'b0),
               .I(din_int[47:0]),
               .O(dout_int[47:0]));

         if (DW > 48) begin
            initial begin
               $display("DW > 48 not supported.");
               $finish();
            end
         end
      end else if (WORD_AW >= 8 && WORD_AW <= 10) begin
         genvar i;
         for (i = 0; i < DW/8; i = i + 1) begin : gen_sram1024_byte
            SRAM1RW1024x8
               u_ramb(
                  .CE(clk),
                  .A(waddr[9:0]),
                  .WEB(~we),
                  .OEB(1'b0),
                  .CSB(1'b0),
                  .I(din_int[((i+1)*8)-1:i*8]),
                  .O(dout_int[((i+1)*8)-1:i*8]));
         end

         if (DW > 48) begin
            initial begin
               $display("DW > 48 not supported.");
               $finish();
            end
         end
      end else begin
         initial begin
            $display("WORD_AW not supported.");
            $finish();
         end
      end
   endgenerate

endmodule
