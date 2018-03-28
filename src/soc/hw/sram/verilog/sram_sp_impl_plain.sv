/* Copyright (c) 2012-2018 by the author(s)
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
 * Single-Port memory implemented as plain registers
 *
 * The main use case for this memory implementation are simulations, but
 * depending on the used Synthesis tool it might also be used in synthesized
 * systems, if the right memory blocks for the target hardware are inferred.
 *
 * The code has been written to follow the inferrence guidelines from Vivado
 * (UG 901, v2017.4) for Single-Port Block RAMs, and tested in Xilinx 7series
 * devices to ensure blockram is inferred.
 *
 * When using Verilator, this memory can be initialized from MEM_FILE by calling
 * the do_readmemh() function. It is also possible to read and write the memory
 * using the get_mem() and set_mem() functions.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module sram_sp_impl_plain(/*AUTOARG*/
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

   // VMEM file used to initialize the memory in simulation
   parameter MEM_FILE = "sram.vmem";


   input                clk;  // Clock
   input                rst;  // Reset
   input                ce;   // Chip enable input
   input                we;   // Write enable input
   input                oe;   // Output enable input
   input [WORD_AW-1:0]  waddr; // word address
   input [DW-1:0]       din;  // input data bus
   input [SW-1:0]       sel;  // select bytes
   output reg [DW-1:0]  dout; // output data bus

   (* ram_style = "block" *) reg [DW-1:0] mem [MEM_SIZE_WORDS-1:0] /*synthesis syn_ramstyle = "block_ram" */;

   always_ff @ (posedge clk) begin
      if (we) begin
         // memory write
         for (int i = 0; i < SW; i = i + 1) begin
            if (sel[i] == 1'b1) begin
               mem[waddr][i*8 +: 8] <= din[i*8 +: 8];
            end
         end
      end
      // memory read
      dout <= mem[waddr];
   end

`ifdef verilator
   export "DPI-C" task do_readmemh;

   task do_readmemh;
      $readmemh(MEM_FILE, mem);
   endtask

   export "DPI-C" task do_readmemh_file;

   task do_readmemh_file;
      input string file;
      $readmemh(file, mem);
   endtask

    // Function to access RAM (for use by Verilator).
   function [DW-1:0] get_mem;
      // verilator public
      input [WORD_AW-1:0] waddr; // word address
      get_mem = mem[waddr];
   endfunction

   // Function to write RAM (for use by Verilator).
   function set_mem;
      // verilator public
      input [WORD_AW-1:0] waddr; // word address
      input [DW-1:0]      data; // data to write
      mem[waddr] = data;
   endfunction // set_mem
`else
   initial
     begin
        $readmemh(MEM_FILE, mem);
     end
`endif
endmodule
