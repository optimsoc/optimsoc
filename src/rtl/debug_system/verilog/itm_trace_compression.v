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
 * Submodule of the Instruction Trace Module (ITM): the trace compression module
 *
 * TODO: Find a solution for transmitting the timestamp of all instructions,
 * even though the instructions are sequential and removed by compression.
 * This is necessary to allow for cycle-accurate reconstruction of the trace.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

module itm_trace_compression(/*AUTOARG*/
   // Outputs
   trace_out_compressed, trace_out_compressed_valid,
   // Inputs
   rst, clk, trace_in, trace_in_valid
   );

   /*
    * Width of the counter of sequential instructions between two branch
    * instructions.
    * This means 2^INSTR_COUNT_WIDTH instructions between two branch
    * instructions can be compressed as one trace message. If there are more
    * sequential instructions, a new trace message is required.
    * This is a configuration parameter of the trace compression algorithm,
    * a too large value will lead to unnecessary data in each compressed
    * trace message, a too small value causes unnecessary trace messages.
    */
   localparam INSTR_COUNT_WIDTH = 8;

   localparam TRACE_IN_WIDTH = `DBG_TIMESTAMP_WIDTH + 32;
   localparam COMPRESSED_TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH + 32 + INSTR_COUNT_WIDTH;

   input rst;
   input clk;

   input [TRACE_IN_WIDTH-1:0] trace_in;
   input trace_in_valid;
   output reg [COMPRESSED_TRACE_WIDTH-1:0] trace_out_compressed;
   output reg trace_out_compressed_valid;

   reg [31:0] prev_wb_pc;
   reg [INSTR_COUNT_WIDTH-1:0] instr_cnt;

   wire [31:0] trace_in_wb_pc;
   wire [`DBG_TIMESTAMP_WIDTH-1:0] trace_in_ts;

   assign trace_in_wb_pc = trace_in[31:0];
   assign trace_in_ts = trace_in[TRACE_IN_WIDTH-1:TRACE_IN_WIDTH-`DBG_TIMESTAMP_WIDTH];

   always @ (posedge clk) begin
      if (rst) begin
         trace_out_compressed <= 0;
         trace_out_compressed_valid <= 1'b0;
         instr_cnt <= 0;
         prev_wb_pc <= 0;
      end else begin
         if (trace_in_valid && trace_in != 32'b0) begin
            if ((prev_wb_pc + 4 == trace_in_wb_pc) ||
                (prev_wb_pc == trace_in_wb_pc)) begin
               // found a sequential instruction (no branch)
               // FIXME: handle instr_cnt overflow!
               instr_cnt <= instr_cnt + 1;

               trace_out_compressed <= 0;
               trace_out_compressed_valid <= 1'b0;
            end else begin
               // found a branch instruction
               trace_out_compressed <= {trace_in_ts, trace_in_wb_pc, instr_cnt};
               trace_out_compressed_valid <= 1'b1;
               instr_cnt <= 0;
            end

            prev_wb_pc <= trace_in_wb_pc;
         end else begin
            trace_out_compressed <= 0;
            trace_out_compressed_valid <= 1'b0;
         end
      end
   end

endmodule
