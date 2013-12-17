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
 * Submodule of the Software Trace Module (STM): the trace collector
 *
 * This module collects software traces from a single CPU core,
 * assigns the global timestamp to it and saves it into the ringbuffer.
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Michael Tempelmeier <michael.tempelmeier@mytum.de>
 */

`include "dbg_config.vh"

module stm_trace_collector(
`ifdef OPTIMSOC_CLOCKDOMAINS
                           clk_cdc,
`endif
                           /*AUTOARG*/
   // Outputs
   trace_out, trace_enable,
   // Inputs
   clk, rst, rf_addrw, rf_dataw, rf_we, cpu_wb_insn, cpu_wb_freeze,
   timestamp
   );

   localparam R3_ADDR = 3; // R3
   localparam L_NOP = 32'h1500_0000;
   localparam L_NOP_MSB = 16'h1500; // 16 LSB bits are ignored by the CPU
                                    // and can be used as a parameter for debugging
   localparam L_NOP_LSB = 16'h0000; // 16'h0000 indicates a normal NOP without
                                    // any debugging purpose

   localparam TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH+32+16;

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif


   // Control signals from the traced CPU core
   input [4:0] rf_addrw;
   input [31:0] rf_dataw;
   input        rf_we;
   input [31:0] cpu_wb_insn;
   input        cpu_wb_freeze;

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   // to the ring buffer
   output [TRACE_WIDTH-1:0] trace_out;
   output                                  trace_enable;

   wire                                 clk_sample;
`ifdef OPTIMSOC_CLOCKDOMAINS
   assign clk_sample = clk_cdc;
`else
   assign clk_sample = clk;
`endif

   // latch R3
   reg [31:0]                                  r3_data;

   reg [TRACE_WIDTH-1:0] trace_out_cdc;
   reg                                  trace_enable_cdc;

`ifdef OPTIMSOC_CLOCKDOMAINS
   wire                           cdc_fifo_empty;
   assign trace_enable = ~cdc_fifo_empty;

   cdc_fifo
      #(.DW                            (TRACE_WIDTH),
        .ADDRSIZE                      (2))
      u_fifo_in_na(// Outputs
                   .wr_full            (),
                   .rd_empty           (cdc_fifo_empty),
                   .rd_data            (trace_out),
                   // Inputs
                   .wr_clk             (clk_cdc),
                   .rd_clk             (clk),
                   .wr_rst             (~rst),
                   .rd_rst             (~rst), // this works as it is edge triggered (clk_cdc might not be running)
                   .rd_en              (1'b1),
                   .wr_en              (trace_enable_cdc),
                   .wr_data            (trace_out_cdc));
`else
   assign trace_out = trace_out_cdc;
   assign trace_enable = trace_enable_cdc;
`endif

   always @(posedge clk_sample or posedge rst) begin
      if (rst) begin
         r3_data <= 0;
      end else begin
         if (rf_we && (rf_addrw == R3_ADDR)) begin
            // latch new value of R3
            r3_data <= rf_dataw;
         end else begin
            // keep old value of R3
            r3_data <= r3_data;
         end
      end
   end // always @ (posedge clk)

   always @ (posedge clk_sample or posedge rst) begin
      if (rst) begin
         trace_out_cdc <= {TRACE_WIDTH{1'bx}};
         trace_enable_cdc <= 0;
      end else begin
         if ((cpu_wb_insn [31:16] == L_NOP_MSB) && (cpu_wb_insn[15:0]!= L_NOP_LSB) && !cpu_wb_freeze) begin
            // system is running
            // l.nop with debugging information detected
            // trace value of r3 and LSB of l.nop
            trace_out_cdc <={timestamp, r3_data, cpu_wb_insn[15:0]};
            trace_enable_cdc <= 1;
         end else begin
            trace_out_cdc <= {TRACE_WIDTH{1'bx}};
            trace_enable_cdc <= 0;
         end
      end // else: !if(rst)
   end // always @ (posedge clk)

endmodule
