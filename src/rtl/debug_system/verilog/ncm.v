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
 * This is the NoC Converter Module (NCM): Convert between Debug NoC and NoC
 *
 * The Debug NoC is a 16 bit wide NoC used for debugging only, it's also called
 * "lisnoc16". The "real" NoC is 32 bit wide and connects the system's
 * components. These two networks are connected by this bridge module.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Michael Tempelmeier, michael.tempelmeier@tum.de
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

module ncm(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready, noc32_in_ready,
   noc32_out_flit, noc32_out_valid,
   // Inputs
   clk, rst, sys_clk_is_halted, dbgnoc_out_ready, dbgnoc_in_flit,
   dbgnoc_in_valid, noc32_in_flit, noc32_in_valid, noc32_out_ready
   );

   // NoC Bridge parameters (32 <-> 16)
   // maximum number of lisnoc32 flits a packet can contain
   localparam FIFO_DEPTH_16to32 = 16;
   // maximum number of lisnoc16 flits a packet can contain
   localparam FIFO_DEPTH_32to16 = 32;

   // NoC interface (32 bit flit payload, aka lisnoc)
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter NOC_VCHANNELS = 3;
   parameter NOC_USED_VCHANNEL_NCM = 0;

   // Address of the external lisnoc32 interface that is connected to this
   // module. This is needed by the software to set the correct source when
   // sending lisnco32 packets.
   parameter LISNOC32_EXT_IF_ADDR = 5'hx;

   // Debug NoC interface (16 bit flit payload, aka lisnoc16)
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_USED_VCHANNEL = 0;

   // module description
   localparam MODULE_TYPE_NCM = 8'h04;
   localparam MODULE_VERSION_NCM = 8'h00;

   localparam CONF_MEM_SIZE = 2;

   input clk;
   input rst;
   input sys_clk_is_halted;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // NoC interface. Connect this to an external module of a system.
   input [NOC_FLIT_WIDTH-1:0]   noc32_in_flit;
   input [NOC_VCHANNELS-1:0]    noc32_in_valid;
   output [NOC_VCHANNELS-1:0]   noc32_in_ready;
   output [NOC_FLIT_WIDTH-1:0]  noc32_out_flit;
   output [NOC_VCHANNELS-1:0]   noc32_out_valid;
   input [NOC_VCHANNELS-1:0]    noc32_out_ready;

   // Debug NoC sending interface multiplexing
   // The configuration interface requests to send via the dbgnoc_conf_out_rts
   // signal, this module grants the request by setting dbgnoc_conf_out_cts.
   // The configuration interface gets the dbgnoc_conf_out_ready signal as
   // dbgnoc_out_ready.
   wire dbgnoc_conf_out_rts;
   reg dbgnoc_conf_out_cts;
   reg dbgnoc_dat_out_cts;
   wire dbgnoc_conf_out_ready;
   wire dbgnoc_dat_out_ready;
   assign dbgnoc_conf_out_ready = dbgnoc_conf_out_cts & dbgnoc_out_ready;
   assign dbgnoc_dat_out_ready = dbgnoc_dat_out_cts & dbgnoc_out_ready;

   wire dbgnoc_conf_out_valid;
   wire dbgnoc_dat_out_valid;
   assign dbgnoc_out_valid = dbgnoc_conf_out_valid | dbgnoc_dat_out_valid;

   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_conf_out_flit;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_dat_out_flit;
   assign dbgnoc_out_flit = (dbgnoc_conf_out_valid ? dbgnoc_conf_out_flit : dbgnoc_dat_out_flit);

   wire dbgnoc_dat_in_ready;
   wire dbgnoc_conf_in_ready;
   assign dbgnoc_in_ready = dbgnoc_dat_in_ready & dbgnoc_conf_in_ready;


   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_in;
   reg [CONF_MEM_SIZE-1:0] conf_mem_flat_in_valid;

   // un-flatten conf_mem_in to conf_mem_flat_in
   reg [15:0] conf_mem_in [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_in
         assign conf_mem_flat_in[((i+1)*16)-1:i*16] = conf_mem_in[i];
      end
   endgenerate


   // lisnoc16 -> lisnoc32
   lisnoc16_converter_16to32
      #(.fifo_depth(FIFO_DEPTH_16to32),
        .vchannels_16(DBG_NOC_VCHANNELS),
        .vchannels_32(NOC_VCHANNELS),
        .packet16_class(`DBG_NOC_CLASS_NCM),
        .use_vchannel_32(NOC_USED_VCHANNEL_NCM))
      u_16to32 (
         .clk(clk),
         .rst(rst),
         // input side
         .flit16_in(dbgnoc_in_flit),
         .flit16_valid(dbgnoc_in_valid),
         .flit16_ready(dbgnoc_dat_in_ready),
         // output side
         .flit32_out(noc32_out_flit),
         .flit32_valid(noc32_out_valid),
         // During stalled sys_clk-cycles the NoC32 must not accecpt any data.
         // Therefore the ready signal is set to zero. This prevents data loss!
         .flit32_ready(noc32_out_ready & {NOC_VCHANNELS{!sys_clk_is_halted}}));


   // lisnoc32 -> lisnoc16
   lisnoc16_converter_32to16
      #(.fifo_depth(FIFO_DEPTH_32to16),
        .usb_if_dest(`DBG_NOC_ADDR_EXTERNALIF),
        .vchannels_16(DBG_NOC_VCHANNELS),
        .vchannels_32(NOC_VCHANNELS),
        .packet16_class(`DBG_NOC_CLASS_NCM),
        .use_vchannel_16(DBG_NOC_USED_VCHANNEL))
      u_32to16(.clk(clk),
               .rst(rst),
               // input side
               .in_flit32(noc32_in_flit),
               .in_ready32(noc32_in_ready),
               // During stalled sys_clk-cycles the NoC32 does not produce any data.
               // Therefore the valid signal is set to zero. This prevents multiple
               // replication of the same data!
               .in_valid32(noc32_in_valid & {NOC_VCHANNELS{!sys_clk_is_halted}}),
               // output side
               .out_flit16(dbgnoc_dat_out_flit),
               .out_valid16(dbgnoc_dat_out_valid),
               .out_ready16(dbgnoc_dat_out_ready));

   /* dbgnoc_conf_if AUTO_TEMPLATE(
      .conf_mem_flat_out(),
      .conf_mem_flat_in_ack(),
      .\(.*\)(\1), // suppress explict port widths
    ); */
   dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(.dbgnoc_out_ready(dbgnoc_conf_out_ready),
                       .dbgnoc_out_rts  (dbgnoc_conf_out_rts),
                       .dbgnoc_out_valid(dbgnoc_conf_out_valid),
                       .dbgnoc_out_flit (dbgnoc_conf_out_flit),
                       .dbgnoc_in_ready (dbgnoc_conf_in_ready),
                       /*AUTOINST*/
                       // Outputs
                       .conf_mem_flat_out(),                     // Templated
                       .conf_mem_flat_in_ack(),                  // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .dbgnoc_in_flit  (dbgnoc_in_flit),        // Templated
                       .dbgnoc_in_valid (dbgnoc_in_valid),       // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated

   // lisnoc16 sending multiplexing: configuration has always priority
   always @ (posedge clk) begin
      if (rst) begin
         conf_mem_in[0] <= {MODULE_TYPE_NCM, MODULE_VERSION_NCM};
         conf_mem_in[1] <= {11'h0, LISNOC32_EXT_IF_ADDR};
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};

         dbgnoc_conf_out_cts <= 0;
         dbgnoc_dat_out_cts <= 1;
      end else begin
         conf_mem_flat_in_valid <= 0;

         if (dbgnoc_conf_out_rts) begin
            dbgnoc_conf_out_cts <= 1;
            dbgnoc_dat_out_cts <= 0;
         end else begin
            dbgnoc_conf_out_cts <= 0;
            dbgnoc_dat_out_cts <= 1;
         end
      end
   end

endmodule
