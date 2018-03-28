/* Copyright (c) 2012-2017 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

module mpbuffer_wb
  import optimsoc_config::*;
  #(parameter config_t CONFIG = 'x,
    parameter SIZE            = 16,
    parameter N               = 1
    )
   (
    input                                clk,
    input                                rst,

    output [N*CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit,
    output [N-1:0]                       noc_out_last,
    output [N-1:0]                       noc_out_valid,
    input [N-1:0]                        noc_out_ready,

    input [N*CONFIG.NOC_FLIT_WIDTH-1:0]  noc_in_flit,
    input [N-1:0]                        noc_in_last,
    input [N-1:0]                        noc_in_valid,
    output [N-1:0]                       noc_in_ready,

    input [31:0]                         wb_adr_i,
    input                                wb_we_i,
    input                                wb_cyc_i,
    input                                wb_stb_i,
    input [31:0]                         wb_dat_i,
    output [31:0]                        wb_dat_o,
    output                               wb_ack_o,
    output                               wb_err_o,

    output                               irq
    );

   // Bus side (generic)
   wire [31:0]                  bus_addr;
   wire                         bus_we;
   wire                         bus_en;
   wire [31:0]                  bus_data_in;
   wire [31:0]                  bus_data_out;
   wire                         bus_ack;
   wire                         bus_err;

   assign bus_addr    = wb_adr_i;
   assign bus_we      = wb_we_i;
   assign bus_en      = wb_cyc_i & wb_stb_i;
   assign bus_data_in = wb_dat_i;
   assign wb_dat_o    = bus_data_out;
   assign wb_ack_o    = bus_ack;
   assign wb_err_o    = bus_err;

   mpbuffer
     #(.CONFIG(CONFIG),
       .SIZE(SIZE), .N(N))
   u_buffer(.*);

endmodule // mpbuffer_wb
