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
 * True Dual-Port RAM with Dual Clock implemented using the SAED32 tech libs
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module sram_tdp_dclk_impl_saed32
#(
   // address width
   parameter AW = 32,
   // data width (must be multiple of 8 for byte selects to work)
   parameter DW = 32,

   // byte select width (must be a power of two)
   localparam SW = DW / 8,

   // word address width
   parameter WORD_AW = AW - (SW >> 1),

   // size of the memory in bytes. default is as large as the address space
   // allows for
   parameter MEM_SIZE_BYTE = (1 << AW),

   localparam MEM_SIZE_WORDS = MEM_SIZE_BYTE / SW
)(
   input                   clk_a,  // Clock
   input                   rst_a,  // Reset
   input                   ce_a,   // Chip enable input
   input                   we_a,   // Write enable input
   input                   oe_a,   // Output enable input
   input [WORD_AW-1:0]     waddr_a, // word address (port A)
   input [DW-1:0]          din_a,  // input data bus
   input [SW-1:0]          sel_a,  // select bytes
   output logic [DW-1:0]   dout_a, // output data bus

   input                   clk_b,  // Clock
   input                   rst_b,  // Reset
   input                   ce_b,   // Chip enable input
   input                   we_b,   // Write enable input
   input                   oe_b,   // Output enable input
   input [WORD_AW-1:0]     waddr_b, // word address (port B)
   input [DW-1:0]          din_b,  // input data bus
   input [SW-1:0]          sel_b,  // select bytes
   output logic [DW-1:0]   dout_b // output data bus
);


   logic [47:0] dout_a_int;
   logic [47:0] din_a_int;

   logic [47:0] dout_b_int;
   logic [47:0] din_b_int;

   assign din_a_int = {{(48 - DW){1'b0}}, din_a[DW-1:0]};
   assign dout_a = dout_a_int[DW-1:0];

   assign din_b_int = {{(48 - DW){1'b0}}, din_b[DW-1:0]};
   assign dout_b = dout_b_int[DW-1:0];

   generate
      if (WORD_AW <= 6) begin
         SRAM2RW64x32
            u_ramb(
               // port A
               .CE1(clk_a),
               .A1(waddr_a),
               .WEB1(~we_a),
               .OEB1(~oe_a),
               .CSB1(1'b1),
               .I1(din_a_int[31:0]),
               .O1(dout_a_int[31:0]),

               // port B
               .CE2(clk_b),
               .A2(waddr_b),
               .WEB21(~we_b),
               .OEB2(~oe_b),
               .CSB2(1'b1),
               .I2(din_b_int[31:0]),
               .O2(dout_b_int[31:0]));

         if (WORD_AW != 6) begin
            initial begin
               $display("Wasting space due to overly wide SRAM. Improve this module!");
            end
         end
         if (DW < 32) begin
            initial begin
               $display("DW < 32 is inefficient. Improve this module!");
            end
         end else if (DW > 32) begin
            initial begin
               $display("DW > 32 not supported.");
               $finish();
            end
         end
      end else if (WORD_AW == 7) begin
         genvar b;
         for (b = 0; b < DW/8; b = b + 1) begin : gen_sram128_byte
            SRAM2RW128x8
               u_ramb(
                  // port A
                  .CE1(clk_a),
                  .A1(waddr_a),
                  .WEB1(~we_a),
                  .OEB1(~oe_a),
                  .CSB1(1'b1),
                  .I1(din_a_int[((b+1)*8)-1:b*8]),
                  .O1(dout_a_int[((b+1)*8)-1:b*8]),

                  // port B
                  .CE2(clk_b),
                  .A2(waddr_b),
                  .WEB21(~we_b),
                  .OEB2(~oe_b),
                  .CSB2(1'b1),
                  .I2(din_b_int[((b+1)*8)-1:b*8]),
                  .O2(dout_b_int[((b+1)*8)-1:b*8]));
         end
      end else begin
         initial begin
            $display("WORD_AW not supported.");
            $finish();
         end
      end
   endgenerate
endmodule
