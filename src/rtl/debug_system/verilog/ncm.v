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
 * This is the NoC Converter Module (NCM): Convert between Debug NoC and NoC
 *
 * The Debug NoC is a 16 bit wide NoC used for debugging only, it's also called
 * "lisnoc16". The "real" NoC is 32 bit wide and connects the system's
 * components. These two networks are connected by this bridge module.
 *
 * TODO: Introduce a configuration register to set NOC_USED_VCHANNEL_NCM to
 *   an arbitrary value. This would enable sending data to other virtual
 *   channels in the NoC in addition to the preconfigured one.
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"
`include "lisnoc16_def.vh"

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
   parameter NOC_USED_VCHANNEL_NCM = 1; // vchannel used on the NoC32 side

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

   // Note: config and data _can_ be sent on the same vchannel, but don't have to
   parameter DBG_NOC_DATA_VCHANNEL = 0; // vchannel for packets from and to the NoC32
   parameter DBG_NOC_CONF_VCHANNEL = 0; // vchannel for config


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

   // During stalled sys_clk-cycles the NoC32 must not accecpt any data.
   // Therefore the ready signal is set to zero. This prevents data loss!
   wire [NOC_VCHANNELS-1:0]     noc32_out_ready_sys_clked;
   assign noc32_out_ready_sys_clked = noc32_out_ready & {NOC_VCHANNELS{!sys_clk_is_halted}};

   // select vchannel on the NOC32 side
   wire                         flit32_out_valid_ser;
   assign noc32_out_valid = (flit32_out_valid_ser << NOC_USED_VCHANNEL_NCM)| {NOC_VCHANNELS{1'b0}};
   wire                         flit32_out_ready_ser;
   assign flit32_out_ready_ser =noc32_out_ready_sys_clked[NOC_USED_VCHANNEL_NCM];



   // Connection between serializer and 32 -> 16 converter
   wire [NOC_FLIT_WIDTH-1:0]    noc32_in_flit_ser;
   wire                         noc32_in_ready_ser;
   wire                         noc32_in_valid_ser;


   // Debug NoC sending interface multiplexing is only need if using the same
   // vchannel for data and configuration.
   // The configuration interface requests to send via the dbgnoc_conf_out_rts
   // signal, this module grants the request by setting dbgnoc_conf_out_cts.
   // The configuration interface gets the dbgnoc_conf_out_ready signal as
   // dbgnoc_out_ready.
   wire dbgnoc_conf_out_rts;
   wire dbgnoc_conf_out_ready;
   wire dbgnoc_data_out_ready;

   generate
      if (DBG_NOC_CONF_VCHANNEL == DBG_NOC_DATA_VCHANNEL) begin: vchannel_mux
         // We use the same vchannel for data and configuration.
         // Therefore we need to multiplex the data.
         // The dbgnoc_conf_if has always priority.
         // The dbgnoc_conf_if module MUST hold dbgnoc_conf_out_rts high during
         // the entire sending process!
         reg dbgnoc_conf_out_cts;
         reg dbgnoc_data_out_cts;
         // select ready signals for conf-if and converter-if
         assign dbgnoc_conf_out_ready = dbgnoc_conf_out_cts & dbgnoc_out_ready[DBG_NOC_CONF_VCHANNEL];
         assign dbgnoc_data_out_ready = dbgnoc_data_out_cts & dbgnoc_out_ready[DBG_NOC_DATA_VCHANNEL];

         always @ (posedge clk) begin
            if (rst) begin
               dbgnoc_conf_out_cts <= 0;
               dbgnoc_data_out_cts <= 1;
            end else begin
               if (dbgnoc_conf_out_rts) begin
                  dbgnoc_conf_out_cts <= 1;
                  dbgnoc_data_out_cts <= 0;
               end else begin
                  dbgnoc_conf_out_cts <= 0;
                  dbgnoc_data_out_cts <= 1;
               end
            end
         end

      end else begin
         // select ready signals for conf-if and converter-if
         assign dbgnoc_conf_out_ready = dbgnoc_out_ready[DBG_NOC_CONF_VCHANNEL];
         assign dbgnoc_data_out_ready = dbgnoc_out_ready[DBG_NOC_DATA_VCHANNEL];
      end

   endgenerate

   // set dbgnoc_out_valid signal
   wire dbgnoc_conf_out_valid;
   wire dbgnoc_data_out_valid;
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} |
                             (dbgnoc_conf_out_valid << DBG_NOC_CONF_VCHANNEL) |
                             (dbgnoc_data_out_valid << DBG_NOC_DATA_VCHANNEL);

   // set dbgnoc_out_flit
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_conf_out_flit;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_data_out_flit;
   assign dbgnoc_out_flit = (dbgnoc_conf_out_valid ? dbgnoc_conf_out_flit : dbgnoc_data_out_flit);

   // select valid signals for conf-if and converter-if
   wire dbgnoc_conf_in_valid;
   wire dbgnoc_data_in_valid;
   assign dbgnoc_conf_in_valid = dbgnoc_in_valid[DBG_NOC_CONF_VCHANNEL];
   assign dbgnoc_data_in_valid = dbgnoc_in_valid[DBG_NOC_DATA_VCHANNEL];

   // set dbgnoc_in_ready-signal
   wire dbgnoc_data_in_ready;
   wire dbgnoc_conf_in_ready;
   // select DBG_NOC_CONF_VCHANNEL
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_conf_mask;
   assign dbgnoc_conf_mask = 1'b1 << DBG_NOC_CONF_VCHANNEL;
   // select DBG_NOC_DATA_VCHANNEL
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_data_mask;
   // ... and discard flits on all other vchannels (=set ready to one)
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_others_in_ready;
   assign dbgnoc_others_in_ready = {DBG_NOC_VCHANNELS{1'b1}} & ~dbgnoc_conf_mask & ~dbgnoc_data_mask;

   generate
      if (DBG_NOC_CONF_VCHANNEL == DBG_NOC_DATA_VCHANNEL) begin: vchannel_mux2
         // If config and data are received on the same vchannel, the
         // demultiplexing is done by the submodules connected to this vchannel.
         // Depending on the PACKET16_CLASS they accept the packet. Therefore
         // both submodules have to be ready to prevent any data loss before
         // setting the ready-signal for this vchannel.
         assign dbgnoc_in_ready = dbgnoc_others_in_ready |
                                  (dbgnoc_data_in_ready & dbgnoc_conf_in_ready << DBG_NOC_CONF_VCHANNEL);
      end else begin
         //we use different vchannels, so no multiplexing is needed.
         assign dbgnoc_in_ready = dbgnoc_others_in_ready |
                                  (dbgnoc_conf_in_ready << DBG_NOC_CONF_VCHANNEL) |
                                  (dbgnoc_data_in_ready << DBG_NOC_DATA_VCHANNEL);
      end
   endgenerate


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

   // initialize configuration memory
   always @ (posedge clk) begin
      if (rst) begin
         conf_mem_in[0] <= {MODULE_TYPE_NCM, MODULE_VERSION_NCM};
         conf_mem_in[1] <= {11'h0, LISNOC32_EXT_IF_ADDR};
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};

      end else begin
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b0}};
      end
   end


   // lisnoc16 -> lisnoc32
   lisnoc16_converter_16to32
      #(.fifo_depth(FIFO_DEPTH_16to32),
        .packet16_class(`DBG_NOC_CLASS_NCM))
      u_16to32 (
         .clk(clk),
         .rst(rst),
         // input side
         .flit16_in(dbgnoc_in_flit),
         .flit16_valid(dbgnoc_data_in_valid),
         .flit16_ready(dbgnoc_data_in_ready),
         // output side
         .flit32_out(noc32_out_flit),
         .flit32_valid(flit32_out_valid_ser),
         .flit32_ready(flit32_out_ready_ser));


   // lisnoc32 -> lisnoc16
   lisnoc16_converter_32to16
      #(.fifo_depth(FIFO_DEPTH_32to16),
        .usb_if_dest(`DBG_NOC_ADDR_EXTERNALIF),
        .vchannels_16(DBG_NOC_VCHANNELS),
        .vchannels_32(NOC_VCHANNELS),
        .packet16_class(`DBG_NOC_CLASS_NCM))
      u_32to16(.clk(clk),
               .rst(rst),
               // input side
               .in_flit32(noc32_in_flit_ser),
               .in_ready32(noc32_in_ready_ser),
               .in_valid32(noc32_in_valid_ser),
               // output side
               .out_flit16(dbgnoc_data_out_flit),
               .out_valid16(dbgnoc_data_out_valid),
               .out_ready16(dbgnoc_data_out_ready));



   // lisnoc32 multiple vchannels -> lisnoc32 one vchannel
   lisnoc_vc_serializer
      #(.vchannels(NOC_VCHANNELS),
        .flit_width(NOC_FLIT_WIDTH))
      vc_ser_32(.clk(clk),
                .rst(rst),
                //ser-side
                .data_ser(noc32_in_flit_ser),
                .valid_ser(noc32_in_valid_ser),
                .ready_ser(noc32_in_ready_ser),

                //mvc-side
                .data_mvc(noc32_in_flit),
                // During stalled sys_clk-cycles the NoC32 does not produce any
                // data. Therefore the valid signal is set to zero. This
                // prevents multiple replication of the same data!
                .valid_mvc(noc32_in_valid & {NOC_VCHANNELS{!sys_clk_is_halted}}),
                .ready_mvc(noc32_in_ready));



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
                       .dbgnoc_in_valid (dbgnoc_conf_in_valid),
                       .dbgnoc_in_flit  (dbgnoc_in_flit),
                       /*AUTOINST*/
                       // Outputs
                       .conf_mem_flat_out(),                     // Templated
                       .conf_mem_flat_in_ack(),                  // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated

endmodule
