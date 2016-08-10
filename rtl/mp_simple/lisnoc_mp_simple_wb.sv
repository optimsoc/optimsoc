/* Copyright (c) 2015 by the author(s)
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
 * The wishbone slave interface to access the simple message passing.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

module lisnoc_mp_simple_wb(/*AUTOARG*/
   // Outputs
   noc_out_flit, noc_out_valid, noc_in_ready, wb_dat_o, wb_ack_o, irq,
   // Inputs
   clk, rst, noc_out_ready, noc_in_flit, noc_in_valid, wb_adr_i,
   wb_we_i, wb_cyc_i, wb_stb_i, wb_dat_i
   );

   parameter noc_data_width = 32;
   parameter noc_type_width = 2;
   localparam noc_flit_width = noc_data_width + noc_type_width;

   parameter  fifo_depth = 16;
   localparam size_width = $clog2(fifo_depth+1);

   input clk;
   input rst;

   // NoC interface
   output [noc_flit_width-1:0] noc_out_flit;
   output                      noc_out_valid;
   input                       noc_out_ready;

   input [noc_flit_width-1:0] noc_in_flit;
   input                      noc_in_valid;
   output                     noc_in_ready;

   input [5:0]                wb_adr_i;
   input                      wb_we_i;
   input                      wb_cyc_i;
   input                      wb_stb_i;
   input [noc_data_width-1:0] wb_dat_i;
   output [noc_data_width-1:0] wb_dat_o;
   output                     wb_ack_o;

   output                     irq;

   // Bus side (generic)
   wire [5:0]                  bus_addr;
   wire                        bus_we;
   wire                        bus_en;
   wire [noc_data_width-1:0]   bus_data_in;
   wire [noc_data_width-1:0]   bus_data_out;
   wire                        bus_ack;

   assign bus_addr    = wb_adr_i;
   assign bus_we      = wb_we_i;
   assign bus_en      = wb_cyc_i & wb_stb_i;
   assign bus_data_in = wb_dat_i;
   assign wb_dat_o    = bus_data_out;
   assign wb_ack_o    = bus_ack;

   lisnoc_mp_simple
     #(.noc_data_width(noc_data_width),
       .noc_type_width(noc_type_width),
       .fifo_depth(fifo_depth))
   u_mp_simple(/*AUTOINST*/
               // Outputs
               .noc_out_flit            (noc_out_flit[noc_flit_width-1:0]),
               .noc_out_valid           (noc_out_valid),
               .noc_in_ready            (noc_in_ready),
               .bus_data_out            (bus_data_out[noc_data_width-1:0]),
               .bus_ack                 (bus_ack),
               .irq                     (irq),
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .noc_out_ready           (noc_out_ready),
               .noc_in_flit             (noc_in_flit[noc_flit_width-1:0]),
               .noc_in_valid            (noc_in_valid),
               .bus_addr                (bus_addr[5:0]),
               .bus_we                  (bus_we),
               .bus_en                  (bus_en),
               .bus_data_in             (bus_data_in[noc_data_width-1:0]));

endmodule // lisnoc_mp_simple_wb
