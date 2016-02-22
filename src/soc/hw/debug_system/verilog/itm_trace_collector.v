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
 * Submodule of the Instruction Trace Module (ITM): the trace collector
 *
 * This module collects uncompressed instruction traces from a single CPU core,
 * assigns the global timestamp to it and saves it into the ringbuffer.
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"

module itm_trace_collector(
`ifdef OPTIMSOC_CLOCKDOMAINS
   clk_cdc,
`endif
   /*AUTOARG*/
   // Outputs
   trace_out,
   // Inputs
   clk, rst, cpu_wb_pc, cpu_wb_freeze, timestamp
   );

   parameter TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH+32;

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif

   // Control signals from the traced CPU core
   input [31:0] cpu_wb_pc;
   input cpu_wb_freeze;

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   // to the ring buffer
   output [TRACE_WIDTH-1:0] trace_out;

   wire clk_sample;
`ifdef OPTIMSOC_CLOCKDOMAINS
   assign clk_sample = clk_cdc;
`else
   assign clk_sample = clk;
`endif

   reg [TRACE_WIDTH-1:0] trace_out_cdc;
   reg trace_out_enable_cdc;

`ifdef OPTIMSOC_CLOCKDOMAINS
   wire cdc_fifo_empty;
   wire [TRACE_WIDTH-1:0] cdc_fifo_out;

   assign trace_out = cdc_fifo_empty ? 'b0 : cdc_fifo_out;

   cdc_fifo
      #(.DW                            (TRACE_WIDTH),
        .ADDRSIZE                      (2))
      u_fifo_in_na(// Outputs
                   .wr_full            (),
                   .rd_empty           (cdc_fifo_empty),
                   .rd_data            (cdc_fifo_out),
                   // Inputs
                   .wr_clk             (clk_cdc),
                   .rd_clk             (clk),
                   .wr_rst             (~rst),
                   .rd_rst             (~rst), // this works as it is edge triggered (clk_cdc might not be running)
                   .rd_en              (1'b1),
                   .wr_en              (trace_out_enable_cdc),
                   .wr_data            (trace_out_cdc));
`else
   assign trace_out = trace_out_cdc;
`endif

   // read trace information
   always @(posedge clk_sample or posedge rst) begin
      if (rst) begin
         trace_out_cdc <= {TRACE_WIDTH{1'b0}};
         trace_out_enable_cdc <= 1'b0;
      end else begin
         if (~cpu_wb_freeze) begin
            trace_out_cdc <= {timestamp, cpu_wb_pc};
            trace_out_enable_cdc <= 1'b1;
         end else begin
            // no message for this clock cycle
            trace_out_cdc <= {TRACE_WIDTH{1'b0}};
            trace_out_enable_cdc <= 1'b0;
         end
      end
   end
endmodule
