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
 * Submodule of the Software Trace Module (STM): the Debug NoC interface
 *
 * TODO: We can probably live without the sending FIFO holding a complete
 *       packet.
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Michael Tempelmeier <michael.tempelmeier@mytum.de>
 */

`include "dbg_config.vh"
`include "diagnosis_config.vh"

//
`include "diagnosis_missing_defines.vh"
//

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module packetizer_dbgnoc_if(/*AUTOARG*/
   // Outputs
   in_data34_ready, dbgnoc_out_flit, dbgnoc_out_valid,
   dbgnoc_in_ready, sys_clk_disable, conf_mem_flat_out,
   // Inputs
   clk, rst, in_data34, in_data34_valid, full_packetizer_fifo,
   dbgnoc_out_ready, dbgnoc_in_flit, dbgnoc_in_valid
   );

   /*
    * Parameters for the trace data (input)
    * Changing these parameters requires changing the flit packaging as well.
    * To prevent accidental changes, these parameters are defined as localparam.
    */

  
   
   parameter MAX_SNAPSHOT_PACKET_SIZE = `DIAGNOSIS_MAX_SNPACKET_SIZE16;


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

   // FSM states: package compressed trace data into individual flits
   localparam STATE_WIDTH = 3; // FSM with 2^STATE_WIDTH = 8 states

   localparam STATE_IDLE = 0;
   localparam STATE_CONF = 1;
   localparam STATE_FLIT_HEADER = 2;
   localparam STATE_FLIT_FINGERPRINT = 3;
   localparam STATE_FLIT_PAYLOAD_LSB = 4;
   localparam STATE_FLIT_PAYLOAD_MSB = 5;
   localparam STATE_FLIT_TAIL_MSB = 6;
   // note: increase STATE_WIDTH if you have more than 8 FSM states!

   // module description
   localparam MODULE_TYPE_PACKETIZER = 8'h10;
   localparam MODULE_VERSION_STM = 8'h00;
   parameter CORE_ID = 16'hx;

   input clk;
   input rst;

   // Snapshot packet data
   input [33:0] in_data34;
   input        in_data34_valid;
   output reg   in_data34_ready;
   wire [31:0]  in_data32;
   wire [1:0]   in_datatype;
   assign in_data32 = in_data34[31:0];
   assign in_datatype = in_data34[33:32];
   // fifo full flag, that signals any full fifo in the packetizer
   // this information is a criteria for system stall
   input        full_packetizer_fifo;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // input for u_noc_out_fifo
   wire [DBG_NOC_FLIT_WIDTH-1:0] to_output_fifo_flit;
   reg to_output_fifo_valid;
   wire to_output_fifo_ready;

   reg [DBG_NOC_FLIT_TYPE_WIDTH-1:0] to_output_fifo_flit_type;
   wire [DBG_NOC_DATA_WIDTH-1:0] to_output_fifo_flit_data;
   assign to_output_fifo_flit = {to_output_fifo_flit_type, to_output_fifo_flit_data};
   reg [DBG_NOC_PH_DEST_WIDTH-1:0] ph_dest;
   reg [DBG_NOC_PH_CLASS_WIDTH-1:0] ph_class;
   //reg [DBG_NOC_PH_ID_WIDTH-1:0] ph_id;

   // system control interface
   output reg sys_clk_disable;

   // configuration memory
   output [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;

   // trace FIFO
   //wire trace_in_ready;
   wire trace_fifo_full;
   assign trace_fifo_full = full_packetizer_fifo; 
   wire trace_fifo_empty;
   
   //reg trace_fifo_rd_en;

   // stores a single trace message after being read from trace_data_fifo
   // and before being separated into individual flits
   //reg [TRACE_WIDTH-1:0] trace_data_buf;
  
   assign trace_fifo_empty = ~in_data34_valid;

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
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} | (dbgnoc_conf_out_valid << DBG_NOC_CONF_VCHANNEL) | 
                             (dbgnoc_trace_out_valid << DBG_NOC_TRACE_VCHANNEL);

   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_conf_out_flit;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_trace_out_flit;
   assign dbgnoc_out_flit = (dbgnoc_conf_out_valid ? dbgnoc_conf_out_flit : dbgnoc_trace_out_flit);

   wire dbgnoc_conf_in_valid;
   assign dbgnoc_conf_in_valid = dbgnoc_in_valid[DBG_NOC_CONF_VCHANNEL];

   wire dbgnoc_conf_in_ready;

   //select DBG_NOC_CONF_VCHANNEL ...
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_conf_mask;
   assign dbgnoc_conf_mask = 1'b1 << DBG_NOC_CONF_VCHANNEL;

   // ... and discard flits on all other vchannels
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_others_in_ready;
   assign dbgnoc_others_in_ready = {DBG_NOC_VCHANNELS{1'b1}} & ~dbgnoc_conf_mask;
   
   assign dbgnoc_in_ready =  dbgnoc_others_in_ready | (dbgnoc_conf_in_ready << DBG_NOC_CONF_VCHANNEL);

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
 
/*  dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(.dbgnoc_out_ready(dbgnoc_conf_out_ready),
                       .dbgnoc_out_rts  (dbgnoc_conf_out_rts),
                       .dbgnoc_out_valid(dbgnoc_conf_out_valid),
                       .dbgnoc_out_flit (dbgnoc_conf_out_flit[DBG_NOC_FLIT_WIDTH-1:0]),

                       
                       // Outputs
                       .dbgnoc_in_ready (dbgnoc_conf_in_ready),
                       .conf_mem_flat_out(conf_mem_flat_out),
                       .conf_mem_flat_in_ack(conf_mem_flat_in_ack),
                       // Inputs
                       .clk             (clk),
                       .rst             (rst),
                       .dbgnoc_in_flit  (dbgnoc_in_flit),
                       .dbgnoc_in_valid (dbgnoc_conf_in_valid),
                       .conf_mem_flat_in(conf_mem_flat_in),
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid));

*/
   // FIFO to store compressed trace messages ready to be sent out through the
   // Debug NoC
   /*
   lisnoc_fifo
      #(.LENGTH(4),
        .flit_data_width(TRACE_WIDTH),
        .flit_type_width(0))
      u_tfifo(.clk(clk),
              .rst(rst),

              .in_flit(trace_in),
              .in_valid(trace_in_valid),
              .in_ready(trace_in_ready),

              .out_ready(trace_fifo_rd_en),
              .out_flit(trace_fifo_data_out),
              .out_valid(trace_fifo_data_out_valid));
*/
   // FIFO to store flits to be sent out on the debug noc
   lisnoc_fifo
      #(.LENGTH(MAX_SNAPSHOT_PACKET_SIZE), // store a full packet
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

   // Multiplexer that forwards the correct 16bit segment to the u_noc_out_fifo
   // Multiplexing and valid signals are controlled by FSM
   wire [15:0] noc_out_mux_signal [0:2];
   reg [1:0] noc_out_mux_sel;
   reg [15:0] header_fsm_to_noc_out;
   localparam MUX_FIRST_DATA16 = 0;
   localparam MUX_SECOND_DATA16 = 1;
   localparam MUX_HEADER = 2;
   assign noc_out_mux_signal[MUX_FIRST_DATA16] = in_data32[15:0];
   assign noc_out_mux_signal[MUX_SECOND_DATA16] = in_data32[31:16];
   assign noc_out_mux_signal[MUX_HEADER] = header_fsm_to_noc_out;
   assign to_output_fifo_flit_data = noc_out_mux_signal[noc_out_mux_sel];
   
   
   always @ (posedge clk) begin
      if (rst) begin
         // Initialize configuration memory
         // module description
         conf_mem_in[0] <= {MODULE_TYPE_PACKETIZER, MODULE_VERSION_STM};
         conf_mem_in[1] <= CORE_ID;

// TODO: Do we need any additional config regs?
//       If so, change CONF_MEM_SIZE as well!

//         // lower-bound $PC trigger
//         conf_mem_in[2] <= 16'h0; // MSB
//         conf_mem_in[3] <= 16'h0; // LSB
//         // upper-bound $PC trigger
//         conf_mem_in[4] <= 16'h0; // MSB
//         conf_mem_in[5] <= 16'h0; // LSB
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};

         sys_clk_disable <= 0;
         fsm_trace_to_flit_state <= STATE_IDLE;
         start_sending_trace <= 0;
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

         /*
         if (trace_fifo_rd_en) begin
            trace_data_buf <= trace_fifo_data_out;
         end
          */
          
         // FSM to send out data through the Debug NoC
         fsm_trace_to_flit_state <= fsm_trace_to_flit_state_next;
      end
   end

   // FSM next state logic: package compressed trace data into individual flits
   always @ (*) begin : FSM_TRACE_TO_FLIT
      to_output_fifo_valid = 1'b0;
      to_output_fifo_flit_type = {DBG_NOC_FLIT_TYPE_WIDTH{1'bx}};
      //trace_fifo_rd_en = 1'b0;
      in_data34_ready = 1'b0;
      dbgnoc_conf_out_cts = 0;
      noc_out_mux_sel = MUX_HEADER;
      header_fsm_to_noc_out = 'bx;
      fsm_trace_to_flit_state_next = STATE_IDLE;

      case (fsm_trace_to_flit_state)
         STATE_IDLE: begin
            to_output_fifo_valid = 1'b0;
            // only start sending (trace or config) if all previous
            // packets have been sent
            if (dbgnoc_conf_out_rts && noc_out_fifo_empty) begin
               fsm_trace_to_flit_state_next = STATE_CONF;
            end else if (start_sending_trace) begin
               fsm_trace_to_flit_state_next = STATE_FLIT_HEADER;
            end else begin
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
            to_output_fifo_flit_type = `FLIT_TYPE_HEADER;
            noc_out_mux_sel = MUX_HEADER;
            to_output_fifo_valid = 1'b1;
            in_data34_ready = 1'b0;
				//ph_dest = `DBG_NOC_ADDR_EXTERNALIF;
            ph_dest = `DBG_NOC_ADDR_DP;
            ph_class = `DBG_NOC_CLASS_DIAG_SNAPSHOT;
            header_fsm_to_noc_out = {ph_dest, ph_class, 8'hx};
            fsm_trace_to_flit_state_next = STATE_FLIT_FINGERPRINT;
         end

        // This state forwards the fingerprint flit (currently only holding the event ID)
        STATE_FLIT_FINGERPRINT: begin 
           to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
           noc_out_mux_sel = MUX_FIRST_DATA16;
           to_output_fifo_valid = 1'b1;
           in_data34_ready = 1'b1; // only take 16 LSB, discard rest
           fsm_trace_to_flit_state_next = STATE_FLIT_PAYLOAD_LSB;
        end
           
         STATE_FLIT_PAYLOAD_LSB: begin
            to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
            noc_out_mux_sel = MUX_FIRST_DATA16;
            to_output_fifo_valid = 1'b1;
            in_data34_ready = 1'b0;
            // if data type of the 32bit input data is 'last flit'
            // then go to TAIL state
            if (in_datatype == 2'b10) begin
               fsm_trace_to_flit_state_next = STATE_FLIT_TAIL_MSB;
            end else begin
               fsm_trace_to_flit_state_next = STATE_FLIT_PAYLOAD_MSB;
            end
         end

         STATE_FLIT_PAYLOAD_MSB: begin
            to_output_fifo_flit_type = `FLIT_TYPE_PAYLOAD;
            noc_out_mux_sel = MUX_SECOND_DATA16;
            to_output_fifo_valid = 1'b1;
            in_data34_ready = 1'b1;
            fsm_trace_to_flit_state_next = STATE_FLIT_PAYLOAD_LSB;
         end

         STATE_FLIT_TAIL_MSB: begin
            to_output_fifo_flit_type = `FLIT_TYPE_LAST;
            noc_out_mux_sel = MUX_SECOND_DATA16;
            to_output_fifo_valid = 1'b1;
            in_data34_ready = 1'b1;
            fsm_trace_to_flit_state_next = STATE_IDLE;
         end
      endcase
   end

endmodule
