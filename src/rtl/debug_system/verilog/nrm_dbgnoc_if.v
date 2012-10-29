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
 * Submodule of the Network Router Monitor (NRM): the Debug NoC interface
 *
 * TODO: We can probably live without the sending FIFO holding a complete
 *       packet.
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

module nrm_dbgnoc_if(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   sys_clk_disable, conf_mem_flat_out,
   // Inputs
   clk, rst, trace_in, trace_in_valid, dbgnoc_out_ready,
   dbgnoc_in_flit, dbgnoc_in_valid
   );

   parameter MONITORED_LINK_COUNT = 'hx;
   localparam TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH + 8*MONITORED_LINK_COUNT;
   localparam TRACE_WIDTH_PADDED = `DBG_TIMESTAMP_WIDTH + 16*((MONITORED_LINK_COUNT+1)/2);

   // parameters for the Debug NoC interface
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;

   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;

   parameter CONF_MEM_SIZE = 'hx;

   // ID of this router (used in the ID field of the packet header to identify
   // the source)
   parameter ROUTER_ID = 'hx;

   // collect flit statistics every n cycles (default value, can be set on
   // run-time in a configuration register)
   parameter STAT_DEFAULT_SAMPLE_INTERVAL = 16'hx;

   // default setting of the minimum flitcount trigger
   parameter CONF_DEFAULT_MIN_FLITCNT = 8'hx;

   // FSM states: package compressed trace data into individual flits
   localparam STATE_WIDTH = 3; // FSM with 2^STATE_WIDTH = 8 states

   localparam STATE_IDLE = 0;
   localparam STATE_CONF = 1;
   localparam STATE_FLIT_HEADER = 2;
   localparam STATE_FLIT_TS_MSB = 3;
   localparam STATE_FLIT_TS_LSB = 4;
   localparam STATE_FLIT_DATA = 5;
   // note: increase STATE_WIDTH if you have more than 8 FSM states!

   // we send data from 2 links in one flit
   localparam DATA_FLITS_TO_SEND = (MONITORED_LINK_COUNT+1)/2;
   localparam FLITS_TO_SEND = DATA_FLITS_TO_SEND + 2; // TODO: 2 assign static

   // module description
   localparam MODULE_TYPE_NRM = 8'h03;
   localparam MODULE_VERSION_NRM = 8'h00;

   input clk;
   input rst;

   // trace data
   input [TRACE_WIDTH-1:0] trace_in;
   input trace_in_valid;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // system control interface
   output reg sys_clk_disable;

   // configuration memory
   output [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;

   // input for u_noc_out_fifo
   wire [DBG_NOC_FLIT_WIDTH-1:0] to_output_fifo_flit;
   reg to_output_fifo_valid;
   wire to_output_fifo_ready;

   reg [DBG_NOC_FLIT_TYPE_WIDTH-1:0] to_output_fifo_flit_type;
   reg [DBG_NOC_DATA_WIDTH-1:0] to_output_fifo_flit_data;
   assign to_output_fifo_flit = {to_output_fifo_flit_type, to_output_fifo_flit_data};
   reg [DBG_NOC_PH_DEST_WIDTH-1:0] ph_dest;
   reg [DBG_NOC_PH_CLASS_WIDTH-1:0] ph_class;
   reg [DBG_NOC_PH_ID_WIDTH-1:0] ph_id;

   // compressed trace FIFO
   wire trace_fifo_full;
   wire trace_fifo_in_ready;
   assign trace_fifo_full = ~trace_fifo_in_ready;
   wire trace_fifo_empty;
   wire trace_fifo_out_valid;
   assign trace_fifo_empty = ~trace_fifo_out_valid;
   wire [TRACE_WIDTH-1:0] trace_fifo_data_out;
   reg trace_fifo_rd_en;

   // stores a single trace message after being read from trace_data_fifo
   // and before being separated into individual flits
   reg [TRACE_WIDTH-1:0] trace_data_buf;

   // FSM: package compressed trace data into flits
   reg [STATE_WIDTH-1:0] fsm_trace_to_flit_state;
   reg [STATE_WIDTH-1:0] fsm_trace_to_flit_state_next;

   reg start_sending_trace;

   // Debug NoC sending interface multiplexing
   // The configuration interface requests to send via the dbgnoc_conf_out_rts
   // signal, this module grants the request by setting dbgnoc_conf_out_cts.
   // The configuration interface gets the dbgnoc_conf_out_ready signal as
   // dbgnoc_out_ready.
   wire dbgnoc_conf_out_rts;
   reg dbgnoc_conf_out_cts;
   wire dbgnoc_conf_out_ready;
   assign dbgnoc_conf_out_ready = dbgnoc_conf_out_cts & dbgnoc_out_ready[DBG_NOC_CONF_VCHANNEL];

   wire dbgnoc_conf_out_valid;
   wire dbgnoc_trace_out_valid;
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} |
                             (dbgnoc_conf_out_valid << DBG_NOC_CONF_VCHANNEL) |
                             (dbgnoc_trace_out_valid << DBG_NOC_TRACE_VCHANNEL);

   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_conf_out_flit;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_trace_out_flit;
   assign dbgnoc_out_flit = (dbgnoc_conf_out_valid ? dbgnoc_conf_out_flit : dbgnoc_trace_out_flit);

   wire dbgnoc_conf_in_valid;
   assign dbgnoc_conf_in_valid = dbgnoc_in_valid[DBG_NOC_CONF_VCHANNEL];

   wire dbgnoc_conf_in_ready;

   // select DBG_NOC_CONF_VCHANNEL ...
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_conf_mask;
   assign dbgnoc_conf_mask = 1'b1 << DBG_NOC_CONF_VCHANNEL;

   // ... and discard flits on all other vchannels
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_others_in_ready;
   assign dbgnoc_others_in_ready = {DBG_NOC_VCHANNELS{1'b1}} & ~dbgnoc_conf_mask;

   assign dbgnoc_in_ready =  dbgnoc_others_in_ready | (dbgnoc_conf_in_ready << DBG_NOC_CONF_VCHANNEL);

   reg [$clog2(DATA_FLITS_TO_SEND)-1:0] remaining_data_flit_counter;
   reg remaining_data_flit_counter_dec;
   reg remaining_data_flit_counter_res;

   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_in;
   reg [CONF_MEM_SIZE-1:0] conf_mem_flat_in_valid;
   wire conf_mem_flat_in_ack;

   // un-flatten conf_mem_in to conf_mem_flat_in
   reg [15:0] conf_mem_in [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_in
         assign conf_mem_flat_in[((i+1)*16)-1:i*16] = conf_mem_in[i];
      end
   endgenerate

   // configuration interface
   /* dbgnoc_conf_if AUTO_TEMPLATE(
      .dbgnoc_in_ready(dbgnoc_conf_in_ready),
      .dbgnoc_in_valid(dbgnoc_conf_in_valid),
      .\(.*\)(\1), // suppress explict port widths
    ); */
   dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(.dbgnoc_out_ready(dbgnoc_conf_out_ready),
                       .dbgnoc_out_rts  (dbgnoc_conf_out_rts),
                       .dbgnoc_out_valid(dbgnoc_conf_out_valid),
                       .dbgnoc_out_flit (dbgnoc_conf_out_flit[DBG_NOC_FLIT_WIDTH-1:0]),

                       /*AUTOINST*/
                       // Outputs
                       .dbgnoc_in_ready (dbgnoc_conf_in_ready),  // Templated
                       .conf_mem_flat_out(conf_mem_flat_out),    // Templated
                       .conf_mem_flat_in_ack(conf_mem_flat_in_ack), // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .dbgnoc_in_flit  (dbgnoc_in_flit),        // Templated
                       .dbgnoc_in_valid (dbgnoc_conf_in_valid),  // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated


   // FIFO to store trace messages ready to be sent out through the Debug NoC
   lisnoc_fifo
      #(.LENGTH(4),
        .flit_data_width(TRACE_WIDTH),
        .flit_type_width(0))
      u_tfifo (.clk(clk),
               .rst(rst),

               .in_flit(trace_in),
               .in_valid(trace_in_valid),
               .in_ready(trace_fifo_in_ready),

               .out_flit(trace_fifo_data_out),
               .out_ready(trace_fifo_rd_en),
               .out_valid(trace_fifo_out_valid));

   // FIFO to store flits to be sent out on the Debug NoC
   // The FIFO must be big enough to hold a complete packet, that's
   // 1 (for the header) + 2 (for TS) + ceil(MONITORED_LINK_COUNT/2) flits.
   lisnoc_fifo
      #(.LENGTH(3+(MONITORED_LINK_COUNT+1)/2),
        .flit_data_width(DBG_NOC_DATA_WIDTH),
        .flit_type_width(DBG_NOC_FLIT_TYPE_WIDTH))
      u_noc_out_fifo (.clk(clk),
                      .rst(rst),
                      .in_flit(to_output_fifo_flit),
                      .in_ready(to_output_fifo_ready),
                      .in_valid(to_output_fifo_valid),
                      .out_flit(dbgnoc_trace_out_flit),
                      .out_ready(dbgnoc_out_ready[DBG_NOC_TRACE_VCHANNEL]),
                      .out_valid(dbgnoc_trace_out_valid));

   wire noc_out_fifo_empty;
   assign noc_out_fifo_empty = !dbgnoc_trace_out_valid;


   // trace_data_buf, padded with 8 zero bits if MONITORED_LINK_COUNT is odd
   wire [TRACE_WIDTH_PADDED-1:0] trace_data_buf_padded;
   wire [15:0] trace_data_buf_padded_array [0:FLITS_TO_SEND-1];
   generate
      if (MONITORED_LINK_COUNT[0] == 1) begin
         assign trace_data_buf_padded = {trace_data_buf, 8'h0};
      end else begin
         assign trace_data_buf_padded = trace_data_buf;
      end
      for (i=0;i<FLITS_TO_SEND;i=i+1) begin
         assign trace_data_buf_padded_array[i] = trace_data_buf_padded[(i+1)*16-1:i*16];
      end
   endgenerate

   always @ (posedge clk) begin
      if (rst) begin
         conf_mem_in[0] <= {MODULE_TYPE_NRM, MODULE_VERSION_NRM};
         conf_mem_in[1][15:0] <= ROUTER_ID;
         conf_mem_in[2][15:0] <= STAT_DEFAULT_SAMPLE_INTERVAL;
         conf_mem_in[3][15:0] <= {8'h0, CONF_DEFAULT_MIN_FLITCNT[7:0]};
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};


         sys_clk_disable <= 0;
         fsm_trace_to_flit_state <= STATE_IDLE;
         start_sending_trace <= 0;
         trace_data_buf <= 0;
         remaining_data_flit_counter <= 0;
      end else begin
         conf_mem_flat_in_valid <= 0;

         // Stop the whole system if one of the FIFOs is full to prevent data loss.
         if (trace_fifo_full) begin
            // TODO: Use a high water mark here instead of a completely full
            //       buffer to account for register latencies?
            sys_clk_disable <= 1;
         end else begin
            sys_clk_disable <= 0;
         end

         // we send out new packets only if we
         // a) have data to send
         // b) have completed sending the previous packet
         if (~trace_fifo_empty & noc_out_fifo_empty) begin
            start_sending_trace <= 1;
         end else begin
            start_sending_trace <= 0;
         end

         if (remaining_data_flit_counter_res) begin
            remaining_data_flit_counter <= DATA_FLITS_TO_SEND - 1;
         end else if (remaining_data_flit_counter_dec) begin
            remaining_data_flit_counter <= remaining_data_flit_counter - 1;
         end

         if (trace_fifo_rd_en) begin
            trace_data_buf <= trace_fifo_data_out;
         end

         // FSM to send out data through the Debug NoC
         fsm_trace_to_flit_state <= fsm_trace_to_flit_state_next;
      end
   end

   // FSM next state logic: package compressed trace data into individual flits
   always @ (*) begin : FSM_TRACE_TO_FLIT
      to_output_fifo_valid = 0;
      to_output_fifo_flit_data = 'hx;
      to_output_fifo_flit_type = 'hx;
      trace_fifo_rd_en = 0;
      dbgnoc_conf_out_cts = 0;
      remaining_data_flit_counter_res = 0;
      remaining_data_flit_counter_dec = 0;
      fsm_trace_to_flit_state_next = fsm_trace_to_flit_state;

      case (fsm_trace_to_flit_state)
         STATE_IDLE: begin
            to_output_fifo_valid = 1'b0;
            if (dbgnoc_conf_out_rts & noc_out_fifo_empty) begin
               fsm_trace_to_flit_state_next = STATE_CONF;
            end else if (start_sending_trace) begin
               trace_fifo_rd_en = 1'b1;
               fsm_trace_to_flit_state_next = STATE_FLIT_HEADER;
            end else begin
               trace_fifo_rd_en = 1'b0;
               fsm_trace_to_flit_state_next = STATE_IDLE;
            end
         end

         STATE_CONF: begin
            if (dbgnoc_conf_out_rts | dbgnoc_conf_out_valid) begin
               fsm_trace_to_flit_state_next = STATE_CONF;
               dbgnoc_conf_out_cts = 1;
            end else begin
               fsm_trace_to_flit_state_next = STATE_IDLE;
               dbgnoc_conf_out_cts = 0;
            end
         end

         STATE_FLIT_HEADER: begin
            trace_fifo_rd_en = 1'b0;

            to_output_fifo_flit_type = `FLIT_TYPE_HEADER;
            ph_dest = `DBG_NOC_ADDR_EXTERNALIF;
            ph_class = `DBG_NOC_CLASS_NRM_DATA;
            ph_id = {DBG_NOC_PH_ID_WIDTH{ROUTER_ID}};
            to_output_fifo_flit_data = {ph_dest, ph_class, ph_id};
            to_output_fifo_valid = 1'b1;

            remaining_data_flit_counter_res = 1;

            fsm_trace_to_flit_state_next = STATE_FLIT_TS_MSB;
         end

         STATE_FLIT_TS_MSB: begin
            to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
            to_output_fifo_flit_data = trace_data_buf_padded_array[FLITS_TO_SEND-1]; // timestamp (upper two bytes)
            to_output_fifo_valid = 1'b1;

            fsm_trace_to_flit_state_next = STATE_FLIT_TS_LSB;
         end

         STATE_FLIT_TS_LSB: begin
            to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
            to_output_fifo_flit_data = trace_data_buf_padded_array[FLITS_TO_SEND-2]; // timestamp (lower two bytes)
            to_output_fifo_valid = 1'b1;

            fsm_trace_to_flit_state_next = STATE_FLIT_DATA;
         end

         STATE_FLIT_DATA: begin
            if (remaining_data_flit_counter == 0) begin
               to_output_fifo_flit_type = `FLIT_TYPE_LAST;
               fsm_trace_to_flit_state_next = STATE_IDLE;
            end else begin
               to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
               fsm_trace_to_flit_state_next = STATE_FLIT_DATA;
               remaining_data_flit_counter_dec = 1;
            end

            to_output_fifo_flit_data = trace_data_buf_padded_array[remaining_data_flit_counter];
            to_output_fifo_valid = 1'b1;
         end
      endcase
   end

endmodule
