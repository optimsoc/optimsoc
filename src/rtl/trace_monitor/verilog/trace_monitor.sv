/**
 * This file is part of OpTiMSoC.
 * 
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
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
 * This is the generic router toplevel.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module trace_monitor(/*AUTOARG*/
   // Inputs
   clk, enable, wb_pc, wb_insn, r3, supv, if_valid_en, if_valid_pos,
   ctrl_done_en, ctrl_done_pos
   );

   parameter string id = "";
   parameter enable_trace=0;

   parameter trans_width = 2;
   parameter stdout_filename = "stdout";
   parameter tracefile_filename = "trace";
   
   input clk;
   input enable;
   input [31:0] wb_pc;
   input [31:0] wb_insn;
   input [31:0] r3;

   input        supv;

   input        if_valid_en;
   input [trans_width-1:0]      if_valid_pos;
   input        ctrl_done_en;
   input [trans_width-1:0]      ctrl_done_pos;

   reg [31:0]   cur_pos;
   integer      count;

   integer          stdout,tracefile;

   reg              is_newline;

   reg [31:0]       schedule_which;
   reg              in_isr;

   integer          trans;
   integer          finished;
   
   
   initial begin
      cur_pos = 32'h0000_0000;
      count = 0;
      is_newline = 1;
      stdout=$fopen(stdout_filename);
      tracefile=$fopen(tracefile_filename);
      in_isr = 0;
      trans = 0;
      finished = 0;      
   end

   always @(posedge clk) begin
      if (enable) begin
         if (enable_trace) begin
            if ( (cur_pos + 4 == wb_pc ) || (cur_pos == wb_pc) ) begin
               count++;
            end else begin
               if (count>0) begin
                  $fwrite(tracefile,"[%0t] %0d, %08x",$time,count,wb_pc);
                  $fflush(tracefile);
                  count <= 0;
               end
            end
            cur_pos <= wb_pc;
         end

         if ( wb_insn[31:16] == 16'h1500) begin
            case (wb_insn[15:0])
              16'h0000: begin
                 // ignore..
              end             
              16'h0001: begin
                 $display("Terminate @%x",wb_pc);
                 $finish();
              end
              16'h0004: begin
                 // simprint
                 if (is_newline) begin
                    $fwrite(stdout,"[%t] ",$time);
                 end
                 $fwrite(stdout,"%c",r3);
                 if (r3=="\n") begin
                    $fflush(stdout);
                    is_newline <= 1;
                 end else begin
                    is_newline <= 0;
                 end
              end // case: 16'h0004
              default: begin
                 $display("[%t] Event %x: %x",$time,wb_insn[15:0],r3);
              end
            endcase
         end
      end // if (enable)


   end
   
endmodule // trace_monitor