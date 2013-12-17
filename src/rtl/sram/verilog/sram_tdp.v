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
 * True Synchronous, Dual-Port RAM with Dual Clock and Byte Select
 *
 * See the README file in this directory for information about the common
 * ports and parameters used in all memory modules.
 *
 * If concurrent write access to the same address happens on both ports,
 * the write from port B will be dropped.
 *
 * Most of the code is taken from the ORPSoCv2 project (or1200_tpram_32x32)
 * of opencores.org.
 *
 * Author(s):
 *    Opencores.org authors of or1200_tpram_32x32
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Markus Goehrle <markus.goehrle@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`define OPTIMSOC_RAM_VALIDATE_ADDRESS

module dp_ram(/*AUTOARG*/
   // Outputs
   dout_a, dout_b,
   // Inputs
   clk_a, rst_a, ce_a, we_a, oe_a, addr_a, din_a, sel_a, clk_b, rst_b,
   ce_b, we_b, oe_b, addr_b, din_b, sel_b
   );

   // address width
   parameter AW = 32;
   // data width (must be multiple of 8 for byte selects to work)
   parameter DW = 32;

   // size of the memory. default is as large as the address space allows for
   parameter MEM_SIZE_BYTES = (1 << AW);

   // byte select width (must be a power of two)
   localparam SW = DW / 8;

   // ensure that parameters are set to allowed values
   // TODO: Check if synthesis tools statically check this statement and remove
   //       it. Otherwise we'll need some defines here.
   initial begin
      if (DW % 8 != 0) begin
         $display("dp_ram: the data port width (parameter DW) must be a multiple of 8");
         $stop;
      end

      if ((1 << clog2(SW)) != SW) begin
         $display("dp_ram: the byte select width (paramter SW = DW/8) must be a power of two");
         $stop;
      end
   end

   // validate the memory address (check if it's inside the memory size bounds)
`ifdef OPTIMSOC_RAM_VALIDATE_ADDRESS
   always @(posedge clk_a) begin
      if (addr_a > MEM_SIZE_BYTES) begin
         $display("dp_ram: access to out-of-bounds memory address detected on port A!");
         $stop;
      end
   end
   always @(posedge clk_b) begin
      if (addr_b > MEM_SIZE_BYTES) begin
         $display("dp_ram: access to out-of-bounds memory address detected on port B!");
         $stop;
      end
   end
`endif

   //
   // Generic synchronous two-port RAM interface
   //
   input                   clk_a;  // Clock
   input                   rst_a;  // Reset
   input                   ce_a;   // Chip enable input
   input                   we_a;   // Write enable input
   input                   oe_a;   // Output enable input
   input [AW-1:0]          addr_a; // address bus inputs
   input [DW-1:0]          din_a;  // input data bus
   input [SW-1:0]          sel_a;  // select bytes
   output [DW-1:0]         dout_a; // output data bus

   input                   clk_b;  // Clock
   input                   rst_b;  // Reset
   input                   ce_b;   // Chip enable input
   input                   we_b;   // Write enable input
   input                   oe_b;   // Output enable input
   input [AW-1:0]          addr_b; // address bus inputs
   input [DW-1:0]          din_b;   // input data bus
   input [SW-1:0]          sel_b;  // select bytes
   output [DW-1:0]         dout_b; // output data bus

   // calculate the word address from the byte address
   wire [AW-clog2(SW)-1:0] addr_word_a;
   wire [AW-clog2(SW)-1:0] addr_word_b;
   assign addr_word_a = addr_a[AW-1:clog2(SW)];
   assign addr_word_b = addr_b[AW-1:clog2(SW)];

   //
   // Generic two-port synchronous RAM model
   //

   //
   // Generic RAM's registers and wires
   //
   reg [DW-1:0]            mem [(MEM_SIZE_BYTES/SW)-1:0];      // RAM content

   //
   // Data output drivers
   //
   assign dout_a = (oe_a) ? mem[addr_word_a] : {DW{1'b0}};
   assign dout_b = (oe_b) ? mem[addr_word_b] : {DW{1'b0}};

   genvar i;

   //
   // RAM write a
   //
   always @ (posedge clk_a) begin
      if (we_a) begin
         for (i = 0; i < SW; i = i + 1) begin
            mem[addr_word_a][((i+1)*8)-1:i*8] <= sel_a[i] ? din_a[((i+1)*8)-1:i*8] : mem[addr_a][((i+1)*8)-1:i*8];
         end
      end
   end

   //
   // RAM write b
   //
   always @ (posedge clk_b) begin
      if (we_b && ((addr_a != addr_b) || ~ we_a)) begin // if write to same address occurs, block b
         for (i = 0; i < SW; i = i + 1) begin
            mem[addr_word_b][((i+1)*8)-1:i*8] <= sel_b[i] ? din_b[((i+1)*8)-1:i*8] : mem[addr_b][((i+1)*8)-1:i*8];
         end
      end
   end

   `include "optimsoc_functions.vh"

endmodule
