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
 * Trace Controller Module (TCM)
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Michael Tempelmeier, michael.tempelmeier@mytum.de
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module tcm(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready, cpu_stall,
   cpu_reset, start_cpu,
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
   output reg cpu_stall;
   output reg cpu_reset;
   output reg start_cpu;

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

         cpu_stall <= 0;
         cpu_reset <= 0;
         start_cpu <= 0;

         halted_sys_clk_counter <= 32'h0;
         sys_clk_counter <= 32'h0;

      end else begin
         // bit 0: stall all CPUs
         if ((conf_mem_out[3] >> 0) & 16'h1) begin
            cpu_stall <= 1;
         end

         // bit 1: un-stall all CPUs
         if ((conf_mem_out[3] >> 1) & 16'h1) begin
            cpu_stall <= 0;
         end

         // bit 2: reset all CPUs
         if ((conf_mem_out[3] >> 2) & 16'h1) begin
            cpu_reset <= 1;
         end else begin
            cpu_reset <= 0;
         end

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

         // bit 4: starts cpu
         if ((conf_mem_out[3] >> 4) & 16'h1) begin
            start_cpu <= 1;
         end else begin
            start_cpu <= 0;
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
