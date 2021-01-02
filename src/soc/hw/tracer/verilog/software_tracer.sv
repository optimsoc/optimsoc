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
 * ============================================================================
 *
 * Trace actions on mor1kx CPU cores during simulations
 *
 * - Record and output the printf() calls to STDOUT using the OpTiMSoC-specific
 *   "nop" printf() method.
 * - Record OpTiMSoC trace events (also emitted through the "nop" extensions)
 * - Record exceptions
 * - Terminate the simulation if all programs have terminated.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */
module software_tracer(/*AUTOARG*/
   // Outputs
   termination,
   // Inputs
   clk, enable, wb_pc, wb_insn, r3, termination_all
   );

   parameter ID = 0;
   parameter ENABLE_TRACE = 0;

   parameter STDOUT_FILENAME = "stdout";
   parameter TRACEFILE_FILENAME = "trace";

   // The trace monitor terminates the simulation when a configured
   // number of trace monitors has seen the termination command. The
   // number of trace monitors that track processors is defined by
   // this parameter
   parameter TERM_CROSS_NUM = 1;

   import "DPI-C" function
     void software_tracer_trace(input longint unsigned timestamp,
				input shortint unsigned cpu,
				input shortint unsigned id,
				input int unsigned 	value);

   localparam TRACE_RESET = 15;
   localparam TRACE_EXCEPTION_ENTER = 16;
   localparam TRACE_EXCEPTION_LEAVE = 17;
   
   input clk;
   input enable;
   input [31:0] wb_pc;
   input [31:0] wb_insn;
   input [31:0] r3;

   // This trace monitor wants to terminate
   output reg                   termination;
   // Signals of all termination requests of all monitors
   input [TERM_CROSS_NUM-1:0]   termination_all;

   reg [31:0]   wb_pc_prev;
   integer      count;
   integer      stdout;
   integer      tracefile;
   reg          is_newline;

   initial begin
      wb_pc_prev = 32'h0000_0000;
      count = 0;
      is_newline = 1;

      stdout = $fopen(STDOUT_FILENAME, "w");
      $fwrite(stdout, "# OpTiMSoC trace_monitor stdout file\n");
      $fwrite(stdout, "# [TIME, CORE] MESSAGE\n");

      if (ENABLE_TRACE) begin
         tracefile = $fopen(TRACEFILE_FILENAME, "w");
         $fwrite(tracefile, "# OpTiMSoC trace_monitor trace file\n");
         $fwrite(tracefile, "# [TIME, CORE] COUNT, INSTRUCTION\n");
      end
      termination = 0;
   end

   reg terminated = 0;

   always @(posedge clk) begin
      if (termination & !terminated) begin
         if (&{termination_all}) begin
            if (!terminated) begin
               $finish();
            end
         end
         terminated <= 1;
      end

      if (enable) begin
         wb_pc_prev <= wb_pc;

         if (ENABLE_TRACE) begin
            if ((wb_pc_prev + 4 == wb_pc) || (wb_pc_prev == wb_pc)) begin
               count <= count + 1;
            end 
            else if (count > 0) begin
               $fwrite(tracefile, "[%0t, %0d] %3d, 0x%08x\n", $time, ID, count, wb_pc);
               $fflush(tracefile);
               count <= 0;
            end
         end

         if (wb_insn[31:16] == 16'h1500) begin
            case (wb_insn[15:0])
              16'h0000: begin
                 // ignore..
              end
              16'h0001: begin
                 $display("[%t, %0d] Terminated at address 0x%x (status: %d)", $time, ID, wb_pc, r3);
                 termination <= 1;
              end
              16'h0004: begin
                 // simprint
                 if (is_newline) begin
                    $fwrite(stdout, "[%t, %0d] ", $time, ID);
                 end
                 $fwrite(stdout,"%c",r3);
                 if (r3 == "\n") begin
                    $fflush(stdout);
                    is_newline <= 1;
                 end else begin
                    is_newline <= 0;
                 end
              end // case: 16'h0004
              default: begin
		 software_tracer_trace($time, ID, wb_insn[15:0], r3);
              end
            endcase
         end // if (wb_insn[31:16] == 16'h1500)
         else if ((wb_pc[31:12] == 0) && (wb_pc[7:0] == 0)) begin
            if (wb_pc[11:8] != wb_pc_prev[11:8]) begin
            // record every exception only when it first occurs, not every cycle
	       if (wb_pc[11:8] == 1) begin
		  software_tracer_trace($time, ID, TRACE_RESET, 0);
	       end else begin
		  software_tracer_trace($time, ID, TRACE_EXCEPTION_ENTER, wb_pc[11:8]);
	       end
	    end
         end
         else if (wb_insn[31:0] == 32'h24000000) begin
            software_tracer_trace($time, ID, TRACE_EXCEPTION_LEAVE, 0);
         end
      end
   end
endmodule
