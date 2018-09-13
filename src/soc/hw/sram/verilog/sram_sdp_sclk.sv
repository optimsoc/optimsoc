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
 * Simple Dual-Port RAM with Single Clock and Byte Select
 *
 * A simple dual-port RAM has one read and one write port (as opposed to a
 * true dual-port RAM, which can have two write or two read operations at the
 * same time).
 *
 * In this implementation, port A is the write port, and port B is the read
 * port.
 *
 * See the README.md file in this directory for information about the common
 * ports and parameters used in all memory modules.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module sram_sdp_sclk
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

   localparam MEM_SIZE_WORDS = MEM_SIZE_BYTE / SW,

   // Bypass memory when reading and writing from the same address
   parameter ENABLE_BYPASS = 1,

`ifdef OPTIMSOC_DEFAULT_MEM_IMPL_TYPE
   parameter MEM_IMPL_TYPE = `OPTIMSOC_DEFAULT_MEM_IMPL_TYPE
`else
   parameter MEM_IMPL_TYPE = "PLAIN"
`endif
)(
   input                   clk,     // Clock
   input                   rst,     // Reset

   input                   ce_a,   // Chip enable input
   input                   we_a,   // Write enable input
   input [WORD_AW-1:0]     waddr_a, // word address (write port)
   input [DW-1:0]          din_a,  // input data bus
   input [SW-1:0]          sel_a,  // select bytes

   input                   ce_b,   // Chip enable input
   input                   oe_b,   // Output enable input
   input [WORD_AW-1:0]     waddr_b, // word address (read port)
   output [DW-1:0]         dout_b  // output data bus
);

   // validate the memory address (check if it's inside the memory size bounds)
   `ifdef OPTIMSOC_SRAM_VALIDATE_ADDRESS
      logic [AW-1:0] addr_a;
      assign addr_a = {waddr_a, (AW - WORD_AW)'{1'b0}};
      always @(posedge clk_a) begin
         if (addr_a > MEM_SIZE_BYTE) begin
            $display("sram_sdp: access to out-of-bounds memory address detected on port A! Trying to access byte address 0x%x, MEM_SIZE_BYTE is %d bytes.",
                  addr_a, MEM_SIZE_BYTE);
            $stop;
         end
      end

      logic [AW-1:0] addr_b;
      assign addr_b = {waddr_b, (AW - WORD_AW)'{1'b0}};
      always @(posedge clk_a) begin
         if (addr_b > MEM_SIZE_BYTE) begin
            $display("sram_sdp: access to out-of-bounds memory address detected on port B! Trying to access byte address 0x%x, MEM_SIZE_BYTE is %d bytes.",
                  addr_b, MEM_SIZE_BYTE);
            $stop;
         end
      end
   `endif

   // Bypass logic
   logic [DW-1:0] rdata;
   generate
      if (ENABLE_BYPASS) begin : bypass_gen
         reg [DW-1:0] din_r;
         reg          bypass;

         assign dout_b = bypass ? din_r : rdata;

         always_ff @(posedge clk) begin
            if (oe_b) begin
               din_r <= din_a;
            end
         end

         always_ff @(posedge clk) begin
            if (waddr_b == waddr_a && we_a && oe_b) begin
               bypass <= 1;
            end else if (oe_b) begin
               bypass <= 0;
            end
         end
      end else begin
         assign dout_b = rdata;
      end
   endgenerate

   // use dual-clock implemenations
   sram_sdp_dclk
      #(.AW(AW),
        .DW(DW),
        .WORD_AW(WORD_AW),
        .MEM_SIZE_BYTE(MEM_SIZE_BYTE),
        .MEM_IMPL_TYPE(MEM_IMPL_TYPE))
      u_impl(
         .clk_a(clk),
         .rst_a(rst),
         .clk_b(clk),
         .rst_b(rst),

         .ce_a(ce_a),
         .we_a(we_a),
         .waddr_a(waddr_a),
         .din_a(din_a),
         .sel_a(sel_a),

         .ce_b(ce_b),
         .oe_b(oe_b),
         .waddr_b(waddr_b),
         .dout_b(rdata));
endmodule
