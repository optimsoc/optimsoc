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
   clk, rst, trace_port, timestamp, sys_clk_halted
   );

   localparam R3_ADDR = 3; // R3
   localparam L_NOP = 32'h1500_0000;
   localparam L_NOP_MSB = 16'h1500; // 16 LSB bits are ignored by the CPU
                                    // and can be used as a parameter for debugging
   localparam L_NOP_LSB = 16'h0000; // 16'h0000 indicates a normal NOP without
                                    // any debugging purpose

   localparam L_RFE = 32'h24000000;

   localparam STM_INDEX_EXCEPTION = 16'h10;
   localparam STM_INDEX_RFE = 16'h11;

   localparam TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH+32+16;

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif

   input [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace_port;

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   // to the ring buffer
   output [TRACE_WIDTH-1:0]         trace_out;
   output                           trace_enable;

   input                            sys_clk_halted;

   wire                             trace_port_enable;
   wire [31:0]                      trace_port_pc;
   wire [31:0]                      trace_port_insn;
   wire                             trace_port_wb_en;
   wire [4:0]                       trace_port_wb_reg;
   wire [31:0]                      trace_port_wb_data;

   assign trace_port_enable  = trace_port[`DEBUG_TRACE_EXEC_ENABLE_MSB];
   assign trace_port_pc      = trace_port[`DEBUG_TRACE_EXEC_PC_MSB:`DEBUG_TRACE_EXEC_PC_LSB];
   assign trace_port_insn    = trace_port[`DEBUG_TRACE_EXEC_INSN_MSB:`DEBUG_TRACE_EXEC_INSN_LSB];
   assign trace_port_wb_en   = trace_port[`DEBUG_TRACE_EXEC_WBEN_MSB];
   assign trace_port_wb_reg  = trace_port[`DEBUG_TRACE_EXEC_WBREG_MSB:`DEBUG_TRACE_EXEC_WBREG_LSB];
   assign trace_port_wb_data = trace_port[`DEBUG_TRACE_EXEC_WBDATA_MSB:`DEBUG_TRACE_EXEC_WBDATA_LSB];

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
         if (trace_port_enable && trace_port_wb_en && (trace_port_wb_reg == R3_ADDR)) begin
            // latch new value of R3
            r3_data <= trace_port_wb_data;
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
         if (trace_port_enable && !sys_clk_halted) begin
            if ((trace_port_pc[31:12] == 20'h0) &&
                (trace_port_pc[7:0] == 8'h0)) begin
               trace_out_cdc <= {timestamp, {28'h0, trace_port_pc[11:8]}, STM_INDEX_EXCEPTION};
               trace_enable_cdc <= 1;
            end else if ((trace_port_insn[31:16] == L_NOP_MSB) && (trace_port_insn[15:0]!= L_NOP_LSB)) begin
               // system is running
               // l.nop with debugging information detected
               // trace value of r3 and LSB of l.nop
               trace_out_cdc <= {timestamp, r3_data, trace_port_insn[15:0]};
               trace_enable_cdc <= 1;
            end else if (trace_port_insn[31:0] == L_RFE) begin
               trace_out_cdc <= {timestamp, 32'h0, STM_INDEX_RFE};
            end else begin
               trace_out_cdc <= {TRACE_WIDTH{1'bx}};
               trace_enable_cdc <= 0;
            end
         end else begin
            trace_out_cdc <= {TRACE_WIDTH{1'bx}};
            trace_enable_cdc <= 0;
         end
      end // else: !if(rst)
   end // always @ (posedge clk)

endmodule
