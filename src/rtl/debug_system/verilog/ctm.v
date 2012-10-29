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
 * Cross-Trigger Matrix (CTM)
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module ctm(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   trigger_from_ctm,
   // Inputs
   clk, rst, dbgnoc_out_ready, dbgnoc_in_flit, dbgnoc_in_valid,
   trigger_to_ctm
   );

   // parameters for the Debug NoC interface
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;
   parameter DBG_NOC_VCHANNELS = 'hx;

   // number of debug module
   parameter DEBUG_MODULE_COUNT = 'hx;

   // module description
   localparam MODULE_TYPE_CTM = 8'h01;
   localparam MODULE_VERSION_CTM = 8'h00;

   // number of configuration registers required to hold all cross-trigger
   // configuration entries (one bit per input trigger)
   // ceil(DEBUG_MODULE_COUNT/16) with integers
   localparam TRIGGER_CONF_REGISTERS = 1 + ((DEBUG_MODULE_COUNT - 1) / 16);

   localparam CONF_MEM_SIZE = 1 + TRIGGER_CONF_REGISTERS;

   input clk;
   input rst;

   // Debug NoC interface (IN = NoC -> CTM; OUT = CTM -> NoC)
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // trigger interface
   output [DEBUG_MODULE_COUNT-1:0] trigger_from_ctm;
   input [DEBUG_MODULE_COUNT-1:0] trigger_to_ctm;

   wire [DEBUG_MODULE_COUNT-1:0] conf_triggers_enabled;

   // Right now all trigger outputs get the same trigger signal.
   // For eventual forward compatibility we leave the multiple trigger outputs
   // in place.
   wire trigger_from_ctm_single;
   assign trigger_from_ctm = {DEBUG_MODULE_COUNT{trigger_from_ctm_single}};

   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_in;
   reg [CONF_MEM_SIZE-1:0] conf_mem_flat_in_valid;
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;

   // un-flatten conf_mem_in to conf_mem_flat_in
   reg [15:0] conf_mem_in [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_in
         assign conf_mem_flat_in[((i+1)*16)-1:i*16] = conf_mem_in[i];
      end
   endgenerate

   // un-flatten conf_mem_flat_out to conf_mem_out
   wire [15:0] conf_mem_out [CONF_MEM_SIZE-1:0];
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_out
         assign conf_mem_out[i] = conf_mem_flat_out[((i+1)*16)-1:i*16];
      end
   endgenerate

   /* dbgnoc_conf_if AUTO_TEMPLATE(
      .dbgnoc_out_rts(),
      .conf_mem_flat_in_ack(),
      .\(.*\)(\1), // suppress explict port widths
    ); */
   dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(/*AUTOINST*/
                       // Outputs
                       .dbgnoc_out_flit (dbgnoc_out_flit),       // Templated
                       .dbgnoc_out_valid(dbgnoc_out_valid),      // Templated
                       .dbgnoc_in_ready (dbgnoc_in_ready),       // Templated
                       .dbgnoc_out_rts  (),                      // Templated
                       .conf_mem_flat_out(conf_mem_flat_out),    // Templated
                       .conf_mem_flat_in_ack(),                  // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .dbgnoc_out_ready(dbgnoc_out_ready),      // Templated
                       .dbgnoc_in_flit  (dbgnoc_in_flit),        // Templated
                       .dbgnoc_in_valid (dbgnoc_in_valid),       // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated

   // configuration register initialization
   always @ (posedge clk) begin
      if (rst) begin
         // initialize configuration registers
         conf_mem_in[0] = {MODULE_TYPE_CTM, MODULE_VERSION_CTM};
         // all other registers are set to 0 in the generate block below
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};
      end else begin
         conf_mem_flat_in_valid <= 0;
      end
   end

   generate
      for (i = 1; i < 1 + TRIGGER_CONF_REGISTERS; i = i + 1) begin
         always @ (posedge clk) begin
            if (rst) begin
               conf_mem_in[i] <= 0; // disable all cross-triggers by default
            end
         end
      end
   endgenerate

   // build a conf_triggers_enabled signal out of the individual configuration
   // registers
   genvar j;
   generate
      for (i = 1; i < 1 + TRIGGER_CONF_REGISTERS; i = i + 1) begin
         for (j = 0; j < 16; j = j + 1) begin
            if ((i-1)*16 + j < DEBUG_MODULE_COUNT) begin
               assign conf_triggers_enabled[(i-1)*16 + j] = conf_mem_out[i][j];
            end
         end
      end
   endgenerate

   // triggering
   assign trigger_from_ctm_single =| (conf_triggers_enabled & trigger_to_ctm);

endmodule
