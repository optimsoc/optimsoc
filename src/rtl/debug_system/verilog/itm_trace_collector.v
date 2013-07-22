/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Submodule of the Instruction Trace Module (ITM): the trace collector
 *
 * This module collects uncompressed instruction traces from a single CPU core,
 * assigns the global timestamp to it and saves it into the ringbuffer.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
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
