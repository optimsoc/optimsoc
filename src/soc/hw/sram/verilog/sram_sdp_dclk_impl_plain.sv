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
 * Simple Dual-Port RAM with Dual Clock implemented as plain registers
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module sram_sdp_dclk_impl_plain
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
   input                   clk_a,     // Clock
   input                   rst_a,     // Reset
   input                   ce_a,   // Chip enable input
   input                   we_a,   // Write enable input
   input [WORD_AW-1:0]     waddr_a, // word address (write port)
   input [DW-1:0]          din_a,  // input data bus
   input [SW-1:0]          sel_a,  // select bytes

   input                   clk_b,     // Clock
   input                   rst_b,     // Reset
   input                   ce_b,   // Chip enable input
   input                   oe_b,   // Output enable input
   input [WORD_AW-1:0]     waddr_b, // word address (read port)
   output logic [DW-1:0]   dout_b // output data bus
);

   reg [DW-1:0] mem [MEM_SIZE_WORDS-1:0];

   // Port A (write port)
   always_ff @(posedge clk_a) begin
      if (we_a) begin
         // memory write
         for (int i = 0; i < SW; i = i + 1) begin
            if (sel_a[i] == 1'b1) begin
               mem[waddr_a][i*8 +: 8] <= din_a[i*8 +: 8];
            end
         end
      end
   end

   // Port B (read port)
   always_ff @(posedge clk_b) begin
      if (oe_b) begin
         dout_b <= mem[waddr_b];
      end
   end
endmodule
