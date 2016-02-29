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
 * Submodule of the NoC Router Monitor (NRM): the data qualificator
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"

module nrm_qualificator(/*AUTOARG*/
   // Outputs
   trace_enable, trigger_out,
   // Inputs
   clk, rst, trace_in, trace_in_valid, trigger_in, conf_mem_flat
   );

   parameter CONF_MEM_SIZE = 'hx;
   parameter MONITORED_LINK_COUNT = 'hx;

   input clk;
   input rst;

   // trace data (uncompressed, before delay through shift register)
   input [`DBG_TIMESTAMP_WIDTH+8*MONITORED_LINK_COUNT-1:0] trace_in;
   input trace_in_valid;

   // enable tracing?
   output trace_enable;

   // trigger interface
   input trigger_in;
   output trigger_out;

   input [16*CONF_MEM_SIZE-1:0] conf_mem_flat;

   // un-flatten conf_mem_flat to conf_mem
   wire [15:0] conf_mem [CONF_MEM_SIZE-1:0];
   generate
      genvar i;
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem
         assign conf_mem[i] = conf_mem_flat[((i+1)*16)-1:i*16];
      end
   endgenerate

   wire trace_enable_undelayed;
   wire internal_trigger_enable;

   debug_data_sr
      #(.DATA_WIDTH(1))
      u_data_sr(.clk(clk),
                .rst(rst),
                .din(trace_enable_undelayed),
                .dout(internal_trigger_enable));

   assign trace_enable = internal_trigger_enable | trigger_in;

   // convenience signals: map config registers to named variables
   wire [7:0] conf_min_flitcnt;
   assign conf_min_flitcnt = conf_mem[3][7:0];

   // trace qualification: we trigger if one counter is >= conf_min_flitcnt
   wire [MONITORED_LINK_COUNT-1:0] trace_enable_undelayed_flat;
   assign trace_enable_undelayed =| {trace_enable_undelayed_flat, (conf_min_flitcnt == 0)};
   generate
      genvar j;
      for (j = 0; j < MONITORED_LINK_COUNT; j = j + 1) begin
         assign trace_enable_undelayed_flat[j] = trace_in[(j+1)*8-1:j*8] >= conf_min_flitcnt;
      end
   endgenerate

   assign trigger_out = trace_enable_undelayed;

endmodule
