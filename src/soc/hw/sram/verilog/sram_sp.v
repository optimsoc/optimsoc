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
 * Single Port RAM with Byte Select
 *
 * The width of the data and address bus can be configured using the DW and
 * AW parameters. To support byte selects DW must be a multiple of 8.
 *
 * Most of the code is taken from the ORPSoCv2 project (or1200_tpram_32x32)
 * of opencores.org.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Markus Goehrle <markus.goehrle@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

 module sram_sp(/*AUTOARG*/
   // Outputs
   dout,
   // Inputs
   clk, rst, ce, we, oe, addr, din, sel
   );

   parameter MEM_SIZE = 'hx;

   // address width
   parameter AW = 32;

   // data width (word size)
   // Valid values: 32, 16 and 8
   parameter DW = 32;

   // type of the memory implementation
   parameter MEM_IMPL_TYPE = "PLAIN";
   // VMEM memory file to load in simulation
   parameter MEM_FILE = "sram.vmem";

   // byte select width (must be a power of two)
   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;


   // ensure that parameters are set to allowed values
   // TODO: Check if synthesis tools statically check this statement and remove
   //       it. Otherwise we'll need some defines here.
   initial begin
      if (DW % 8 != 0) begin
         $display("sram_sp: the data port width (parameter DW) must be a multiple of 8");
         $stop;
      end

      if ((1 << clog2(SW)) != SW) begin
         $display("sram_sp: the byte select width (paramter SW = DW/8) must be a power of two");
         $stop;
      end
   end

   input           clk;  // Clock
   input           rst;  // Reset
   input           ce;   // Chip enable input
   input           we;   // Write enable input
   input           oe;   // Output enable input
   input [AW-1:0]  addr; // address bus inputs
   input [DW-1:0]  din;  // input data bus
   input [SW-1:0]  sel;  // select bytes
   output [DW-1:0] dout; // output data bus

   // validate the memory address (check if it's inside the memory size bounds)
`ifdef OPTIMSOC_SRAM_VALIDATE_ADDRESS
   always @(posedge clk) begin
      if (addr > MEM_SIZE) begin
         $display("sram_sp: access to out-of-bounds memory address detected! Trying to access byte address 0x%x, MEM_SIZE is %d bytes.",
                  addr, MEM_SIZE);
         $stop;
      end
   end
`endif

   generate
      if (MEM_IMPL_TYPE == "PLAIN") begin : gen_sram_sp_impl
         sram_sp_impl_plain
            #(/*AUTOINSTPARAM*/
              // Parameters
              .AW                       (AW),
              .DW                       (DW),
              .MEM_SIZE                 (MEM_SIZE),
              .MEM_FILE                 (MEM_FILE))
            u_impl(/*AUTOINST*/
                   // Outputs
                   .dout                (dout[DW-1:0]),
                   // Inputs
                   .clk                 (clk),
                   .rst                 (rst),
                   .ce                  (ce),
                   .we                  (we),
                   .oe                  (oe),
                   .addr                (addr[AW-1:0]),
                   .din                 (din[DW-1:0]),
                   .sel                 (sel[SW-1:0]));
      end else if (MEM_IMPL_TYPE == "XILINX_SPARTAN6") begin : gen_sram_sp_impl
         sram_sp_impl_xilinx_spartan6
            #(/*AUTOINSTPARAM*/
              // Parameters
              .MEM_SIZE                 (MEM_SIZE))
            u_impl(/*AUTOINST*/
                   // Outputs
                   .dout                (dout[31:0]),
                   // Inputs
                   .clk                 (clk),
                   .rst                 (rst),
                   .sel                 (sel[3:0]),
                   .addr                (addr[AW-1:0]),
                   .we                  (we),
                   .ce                  (ce),
                   .din                 (din[31:0]));
      end else begin : gen_sram_sp_impl // block: gen_sram_sp_impl
//         $display("Unsupported memory type: ", MEM_IMPL_TYPE);
//         $stop;
      end
   endgenerate

`include "optimsoc_functions.vh"

 endmodule
