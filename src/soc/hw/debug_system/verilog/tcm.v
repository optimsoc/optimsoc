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
 * Trace Controller Module (TCM)
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Michael Tempelmeier <michael.tempelmeier@mytum.de>
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module tcm(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready, cpu_stall,
   cpu_reset, start_cpu, start_cpu_edge,
   // Inputs
   clk, rst, sys_clk_is_halted, dbgnoc_out_ready, dbgnoc_in_flit,
   dbgnoc_in_valid
   );

   // unique system identifier, used by the host software
   parameter SYSTEM_IDENTIFIER = 16'hx;
   // number of debug modules in this system (apart from this one)
   parameter MODULE_COUNT = 0;

   // parameters for the Debug NoC interface
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;
   parameter DBG_NOC_VCHANNELS = 'hx;
   parameter DBG_NOC_CONF_VCHANNEL = 'hx;

   localparam CONF_MEM_SIZE = 8;

   input clk;
   input rst;
   input sys_clk_is_halted;

   // Debug NoC interface (IN = NoC -> TCM; OUT = TCM -> NoC)
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // CPU control
   output cpu_stall;
   output cpu_reset;
   output start_cpu;
   output start_cpu_edge;

   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_in;
   reg [CONF_MEM_SIZE-1:0] conf_mem_flat_in_valid;
   wire conf_mem_flat_in_ack;

   // un-flatten conf_mem_flat_out to conf_mem_out
   wire [15:0] conf_mem_out [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_out
         assign conf_mem_out[i] = conf_mem_flat_out[((i+1)*16)-1:i*16];
      end
   endgenerate

   // un-flatten conf_mem_in to conf_mem_flat_in
   reg [15:0] conf_mem_in [CONF_MEM_SIZE-1:0];
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_in
         assign conf_mem_flat_in[((i+1)*16)-1:i*16] = conf_mem_in[i];
      end
   endgenerate

   // dbg performance counter
   reg [31:0] halted_sys_clk_counter;
   reg [31:0] sys_clk_counter;

   // vchannel support
   wire       dbgnoc_conf_out_valid;
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} |
                             dbgnoc_conf_out_valid << DBG_NOC_CONF_VCHANNEL;

   wire       dbgnoc_conf_out_ready;
   assign dbgnoc_conf_out_ready = dbgnoc_out_ready[DBG_NOC_CONF_VCHANNEL];

   wire       dbgnoc_conf_in_valid;
   assign dbgnoc_conf_in_valid = dbgnoc_in_valid[DBG_NOC_CONF_VCHANNEL];

   wire       dbgnoc_conf_in_ready;
   // select DBG_NOC_CONF_VCHANNEL ...
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_conf_mask;
   assign dbgnoc_conf_mask = 1'b1 << DBG_NOC_CONF_VCHANNEL;
   // ... and discard flits on all other vchannels
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_others_in_ready;
   assign dbgnoc_others_in_ready = {DBG_NOC_VCHANNELS{1'b1}} & ~dbgnoc_conf_mask;
   assign dbgnoc_in_ready =  dbgnoc_others_in_ready |
                             (dbgnoc_conf_in_ready << DBG_NOC_CONF_VCHANNEL);

   /* dbgnoc_conf_if AUTO_TEMPLATE(
      .dbgnoc_out_valid(dbgnoc_conf_out_valid),
      .dbgnoc_out_ready(dbgnoc_conf_out_ready),
      .dbgnoc_in_valid(dbgnoc_conf_in_valid),
      .dbgnoc_in_ready(dbgnoc_conf_in_ready),
      .\(.*\)(\1), // suppress explict port widths
    ); */
   dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(.dbgnoc_out_rts(),
                       /*AUTOINST*/
                       // Outputs
                       .dbgnoc_out_flit (dbgnoc_out_flit),       // Templated
                       .dbgnoc_out_valid(dbgnoc_conf_out_valid), // Templated
                       .dbgnoc_in_ready (dbgnoc_conf_in_ready),  // Templated
                       .conf_mem_flat_out(conf_mem_flat_out),    // Templated
                       .conf_mem_flat_in_ack(conf_mem_flat_in_ack), // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .dbgnoc_out_ready(dbgnoc_conf_out_ready), // Templated
                       .dbgnoc_in_flit  (dbgnoc_in_flit),        // Templated
                       .dbgnoc_in_valid (dbgnoc_conf_in_valid),  // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated


   // TODO:
   //  * unstall removed, necessary?
   //  * isn't start_cpu = !cpu_reset?
   //  * Review signals, I think the clockmanager assumes the single
   //    cycle signals instead of constant high, nevertheless it is
   //    not important.
   assign cpu_stall = conf_mem_out[3][0];
   assign cpu_reset = conf_mem_out[3][2];
   assign start_cpu = conf_mem_out[3][4];

   // To generate the rising edge of the start cpu signal, we need to
   // register the start cpu signal.
   reg                          start_cpu_r;

   always @(posedge clk) begin
      if (rst) begin
         start_cpu_r <= 1'b0;
      end else begin
         start_cpu_r <= start_cpu;
      end
   end

   // If the start cpu signal is high this cycle and not the previous,
   // this is the edge
   assign start_cpu_edge = start_cpu & ~start_cpu_r;


   always @ (posedge clk) begin
      if (rst) begin
         conf_mem_in[0] <= 16'd0; // version
         conf_mem_in[1] <= SYSTEM_IDENTIFIER;
         conf_mem_in[2] <= MODULE_COUNT;
         conf_mem_in[3] <= 16'h0; // system control register
         conf_mem_in[4] <= 16'h0; // dbg performance: sys_clk_is_halted-counter MSB
         conf_mem_in[5] <= 16'h0; // dbg performance: sys_clk_is_halted-counter LSB
         conf_mem_in[6] <= 16'h0; // dbg performance: sys_clk-counter MSB
         conf_mem_in[7] <= 16'h0; // dbg performance: sys_clk-counter LSB
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};

         halted_sys_clk_counter <= 32'h0;
         sys_clk_counter <= 32'h0;

      end else begin
         // bit 3: request current values of dbg performance counters
         if ((conf_mem_out[3] >> 3) & 16'h1) begin
            // write current values to the configuration register...
            conf_mem_in[4] <= halted_sys_clk_counter[31:16];
            conf_mem_in[5] <= halted_sys_clk_counter[15:0];
            conf_mem_in[6] <= sys_clk_counter[31:16];
            conf_mem_in[7] <= sys_clk_counter[15:0];
            // reset counters
            halted_sys_clk_counter <= 32'h0;
            sys_clk_counter <= 32'h0;
         end else begin
            conf_mem_in[4] <= conf_mem_in[4];
            conf_mem_in[5] <= conf_mem_in[5];
            conf_mem_in[6] <= conf_mem_in[6];
            conf_mem_in[7] <= conf_mem_in[7];

            // check for overflow.
            if ((halted_sys_clk_counter != {32{1'b1}}) && (sys_clk_counter != {32{1'b1}})) begin
               sys_clk_counter <= sys_clk_counter +1 ;
               if (sys_clk_is_halted) begin
                  halted_sys_clk_counter <= halted_sys_clk_counter +1;
               end else begin
                  halted_sys_clk_counter <= halted_sys_clk_counter;
               end
            end else begin
               // overflow! ... keep values
               sys_clk_counter <= sys_clk_counter;
               halted_sys_clk_counter <= halted_sys_clk_counter;
            end

         end

         // reset config register (only writes to this register are possible)
         // we ignore the conf_mem_in_ack signal here since writing the register
         // again through the debug interface should trigger the same action
         // again
         conf_mem_in[3] <= 16'h0;

         conf_mem_flat_in_valid <= 8'b11111000;
      end
   end

endmodule
