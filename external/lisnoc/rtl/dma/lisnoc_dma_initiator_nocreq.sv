/* Copyright (c) 2015 by the author(s)
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
 * This modules generates the requests for DMA transfers.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

`include "lisnoc_def.vh"
`include "lisnoc_dma_def.vh"

module lisnoc_dma_initiator_nocreq (/*AUTOARG*/
   // Outputs
   noc_out_flit, noc_out_valid, ctrl_read_pos, req_start, req_laddr,
   req_data_ready, req_is_l2r, req_size,
   // Inputs
   clk, rst, noc_out_ready, ctrl_read_req, valid, ctrl_done_pos,
   ctrl_done_en, req_data_valid, req_data
   );

   parameter table_entries = 4;
   localparam table_entries_ptrwidth = $clog2(table_entries);

   parameter tileid = 0;

   parameter noc_packet_size = 16; // flits per packet

   input clk, rst;

   // NOC-Interface
   output reg [`FLIT_WIDTH-1:0]            noc_out_flit;
   output reg                              noc_out_valid;
   input                                   noc_out_ready;

   // Control read (request) interface
   output reg [table_entries_ptrwidth-1:0] ctrl_read_pos;
   input [`DMA_REQUEST_WIDTH-1:0]          ctrl_read_req;

   input [table_entries-1:0]               valid;

   // Feedback from response path
   input [table_entries_ptrwidth-1:0]      ctrl_done_pos;
   input                                   ctrl_done_en;


   // Interface to wishbone request
   output reg                              req_start;
   output [31:0]                           req_laddr;
   input                                   req_data_valid;
   output reg                              req_data_ready;
   input [31:0]                            req_data;
   output                                  req_is_l2r;
   output [`DMA_REQFIELD_SIZE_WIDTH-3:0]   req_size;

   //
   //  NOC request
   //
   localparam NOC_REQ_WIDTH = 4;
   localparam NOC_REQ_IDLE         = 4'b0000;
   localparam NOC_REQ_L2R_GENHDR   = 4'b0001;
   localparam NOC_REQ_L2R_GENADDR  = 4'b0010;
   localparam NOC_REQ_L2R_DATA     = 4'b0011;
   localparam NOC_REQ_L2R_WAITDATA = 4'b0100;
   localparam NOC_REQ_R2L_GENHDR   = 4'b0101;
   localparam NOC_REQ_R2L_GENSIZE  = 4'b1000;

   localparam NOC_REQ_R2L_GENRADDR = 4'b0110;
   localparam NOC_REQ_R2L_GENLADDR = 4'b0111;

   // State logic
   reg [NOC_REQ_WIDTH-1:0]                 noc_req_state;
   reg [NOC_REQ_WIDTH-1:0]                 nxt_noc_req_state;

   // Counter for payload flits/words in request
   reg [`DMA_REQFIELD_SIZE_WIDTH-1:0] noc_req_counter;
   reg [`DMA_REQFIELD_SIZE_WIDTH-1:0] nxt_noc_req_counter;

   // Current packet payload flit/word counter
   reg [4:0]             noc_req_packet_count;
   reg [4:0]             nxt_noc_req_packet_count;

   // Current packet total number of flits/words
   reg [4:0]             noc_req_packet_size;
   reg [4:0]             nxt_noc_req_packet_size;

   /*
    * Table entry selection logic
    *
    * The request table signals all open requests on the 'valid' bit vector.
    * The selection logic arbitrates among those entries to determine the
    * request to be handled next.
    *
    * The arbitration is not done for all entries marked as valid but only
    * for those, that are additionally not marked in the open_responses
    * bit vector.
    *
    * The selection signals only change after a transfer is started.
    */

   // Selects the next entry from the table
   reg [table_entries-1:0]            select;     // current grant of arbiter
   wire [table_entries-1:0]           nxt_select; // next grant of arbiter

   // Store open responses: table entry valid is not sufficient, as
   // current requests would be selected
   reg [table_entries-1:0] open_responses;
   reg [table_entries-1:0] nxt_open_responses;

   wire [table_entries-1:0] requests;
   assign requests = valid & ~open_responses & {table_entries{(noc_req_state == NOC_REQ_IDLE)}};

   /* lisnoc_arb_rr AUTO_TEMPLATE(
    .nxt_gnt (nxt_select),
    .gnt     (select),
    .req     (requests),
    ); */

   // Round robin (rr) arbiter
   lisnoc_arb_rr
     #(.N(table_entries))
   u_select(/*AUTOINST*/
            // Outputs
            .nxt_gnt                    (nxt_select),            // Templated
            // Inputs
            .req                        (requests),              // Templated
            .gnt                        (select));               // Templated

   // register next select to select
   always @(posedge clk) begin
      if (rst) begin
         select <= 0;
      end else begin
         select <= nxt_select;
      end
   end

   // Convert (one hot) select bit vector to binary
   always @(*) begin : readpos_onehottobinary
      integer d;
      ctrl_read_pos = 0;
      for (d=0;d<table_entries;d=d+1)
        if (select[d])
          ctrl_read_pos = ctrl_read_pos | d;
   end

   /*
    *
    * Request generation
    *
    */

   wire       nxt_req_start;
   // This is a pulse that signals the start of a request to the wishbone and noc
   // part of the request generation.
   assign nxt_req_start = // start when any is valid and not already in progress
                          (|(valid & ~open_responses) &
                           // and we are not currently generating a request (pulse)
                           (noc_req_state == NOC_REQ_IDLE));

   // Convenience wires
   wire [`DMA_REQFIELD_RTILE_WIDTH-1:0] req_rtile;
   wire [31:0]                          req_raddr;
   assign req_is_l2r = (ctrl_read_req[`DMA_REQFIELD_DIR] == `DMA_REQUEST_L2R);
   assign req_laddr  = ctrl_read_req[`DMA_REQFIELD_LADDR_MSB:`DMA_REQFIELD_LADDR_LSB];
   assign req_size   = ctrl_read_req[`DMA_REQFIELD_SIZE_MSB:`DMA_REQFIELD_SIZE_LSB];
   assign req_rtile  = ctrl_read_req[`DMA_REQFIELD_RTILE_MSB:`DMA_REQFIELD_RTILE_LSB];
   assign req_raddr  = ctrl_read_req[`DMA_REQFIELD_RADDR_MSB:`DMA_REQFIELD_RADDR_LSB];

   //
   // NoC side request generation
   //

   // next state logic, counters, control signals
   always @(*) begin
      // Default is not generating flits
      noc_out_valid = 1'b0;
      noc_out_flit = 34'h0;

      // Only pop when successfull transfer
      req_data_ready = 1'b0;

      // Counters stay old value
      nxt_noc_req_counter = noc_req_counter;
      nxt_noc_req_packet_count = noc_req_packet_count;
      nxt_noc_req_packet_size  = noc_req_packet_size;

      // Open response only changes when request generated
      nxt_open_responses = open_responses;

      case(noc_req_state)
        NOC_REQ_IDLE: begin
           // Idle'ing
           if (req_start)
             // A valid request exists, that is not open
             if (req_is_l2r)
               // L2R
               nxt_noc_req_state = NOC_REQ_L2R_GENHDR;
             else
               // R2L
               nxt_noc_req_state = NOC_REQ_R2L_GENHDR;
           else
             // wait for request
             nxt_noc_req_state = NOC_REQ_IDLE;

           // Reset counter
           nxt_noc_req_counter = 0;
        end
        NOC_REQ_L2R_GENHDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]       = `FLIT_TYPE_HEADER;
           noc_out_flit[`FLIT_DEST_MSB:`FLIT_DEST_LSB]       = req_rtile;
           noc_out_flit[`PACKET_CLASS_MSB:`PACKET_CLASS_LSB] = `PACKET_CLASS_DMA;
           noc_out_flit[`PACKET_ID_MSB:`PACKET_ID_LSB]       = ctrl_read_pos;
           noc_out_flit[`SOURCE_MSB:`SOURCE_LSB]             = tileid;
           noc_out_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB]   = `PACKET_TYPE_L2R_REQ;

           if ((noc_req_counter + (noc_packet_size-2)) < req_size) begin
              // This is not the last packet in the request (noc_packet_size-2)
              noc_out_flit[`SIZE_MSB:`SIZE_LSB] = noc_packet_size-2;
              noc_out_flit[`PACKET_REQ_LAST]    = 1'b0;
              nxt_noc_req_packet_size  = noc_packet_size-2;
              // count is the current transfer number
              nxt_noc_req_packet_count = 5'd1;
           end else begin
              // This is the last packet in the request
              noc_out_flit[`SIZE_MSB:`SIZE_LSB] = req_size - noc_req_counter;
              noc_out_flit[`PACKET_REQ_LAST] = 1'b1;
              nxt_noc_req_packet_size = req_size - noc_req_counter;
              // count is the current transfer number
              nxt_noc_req_packet_count = 5'd1;
           end // else: !if((noc_req_counter + (noc_packet_size-2)) < req_size)

           // change to next state if successful
           if (noc_out_ready)
             nxt_noc_req_state = NOC_REQ_L2R_GENADDR;
           else
             nxt_noc_req_state = NOC_REQ_L2R_GENHDR;

        end // case: NOC_REQ_GENHDR
        NOC_REQ_L2R_GENADDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_PAYLOAD;
           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = req_raddr + (noc_req_counter << 2);

           if (noc_out_ready) begin
             nxt_noc_req_state = NOC_REQ_L2R_DATA;
           end else begin
             nxt_noc_req_state = NOC_REQ_L2R_GENADDR;
           end
        end
        NOC_REQ_L2R_DATA: begin
           // transfer data to noc if available
           noc_out_valid = req_data_valid;

           // Signal last flit for this transfer
           if (noc_req_packet_count==noc_req_packet_size) begin
             noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_LAST;
           end else begin
             noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_PAYLOAD;
           end

           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = req_data;

           if (noc_out_ready & noc_out_valid) begin
              // transfer was successful

              // signal to data fifo
              req_data_ready = 1'b1;

              // increment the counter for this packet
              nxt_noc_req_packet_count = noc_req_packet_count + 1;

              if (noc_req_packet_count == noc_req_packet_size) begin
                 // This was the last flit in this packet
                 if (noc_req_packet_count+noc_req_counter == req_size) begin
                    // .. and the last flit for the request

                    // keep open_responses and "add" currently selected request to it
                    nxt_open_responses = open_responses | select;
                    // back to IDLE
                    nxt_noc_req_state = NOC_REQ_IDLE;
                 end else begin
                    // .. and other packets to transfer

                    // Start with next header
                    nxt_noc_req_state = NOC_REQ_L2R_GENHDR;

                    // add the current counter to overall counter
                    nxt_noc_req_counter = noc_req_counter + noc_req_packet_count;
                 end
              end else begin // if (noc_req_packet_count == noc_req_packet_size)
                 // we transfered a flit inside the packet
                 nxt_noc_req_state = NOC_REQ_L2R_DATA;
              end
           end else begin // if (noc_out_ready & noc_out_valid)
              // no success
              nxt_noc_req_state = NOC_REQ_L2R_DATA;
           end
        end // case: NOC_REQ_L2R_DATA
        NOC_REQ_R2L_GENHDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]       = `FLIT_TYPE_HEADER;
           noc_out_flit[`FLIT_DEST_MSB:`FLIT_DEST_LSB]       = req_rtile;
           noc_out_flit[`PACKET_CLASS_MSB:`PACKET_CLASS_LSB] = `PACKET_CLASS_DMA;
           noc_out_flit[`PACKET_ID_MSB:`PACKET_ID_LSB]       = ctrl_read_pos;
           noc_out_flit[`SOURCE_MSB:`SOURCE_LSB]             = tileid;
           noc_out_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB]   = `PACKET_TYPE_R2L_REQ;

           noc_out_flit[11:0] = 0;

           // There's only one packet needed for the request
           noc_out_flit[`PACKET_REQ_LAST] = 1'b1;

           // change to next state if successful
           if (noc_out_ready)
              nxt_noc_req_state = NOC_REQ_R2L_GENSIZE;
           else
              nxt_noc_req_state = NOC_REQ_R2L_GENHDR;

        end // case: NOC_REQ_GENHDR

        NOC_REQ_R2L_GENSIZE: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`SIZE_MSB:`SIZE_LSB] = req_size;

           // change to next state if successful
           if (noc_out_ready)
              nxt_noc_req_state = NOC_REQ_R2L_GENRADDR;
           else
              nxt_noc_req_state = NOC_REQ_R2L_GENSIZE;
        end // case: NOC_REQ_R2L_GENSIZE

        NOC_REQ_R2L_GENRADDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_PAYLOAD;
           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = ctrl_read_req[`DMA_REQFIELD_RADDR_MSB:`DMA_REQFIELD_RADDR_LSB];

           if (noc_out_ready) begin
          // keep open_responses and "add" currently selected request to it
             nxt_noc_req_state = NOC_REQ_R2L_GENLADDR;
           end else begin
             nxt_noc_req_state = NOC_REQ_R2L_GENRADDR;
           end
        end // case: NOC_REQ_R2L_GENRADDR

        NOC_REQ_R2L_GENLADDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_LAST;
           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = ctrl_read_req[`DMA_REQFIELD_LADDR_MSB:`DMA_REQFIELD_LADDR_LSB];

           if (noc_out_ready) begin
          // keep open_responses and "add" currently selected request to it
             nxt_open_responses = open_responses | select;
             nxt_noc_req_state = NOC_REQ_IDLE;
           end else begin
             nxt_noc_req_state = NOC_REQ_R2L_GENLADDR;
           end
        end // case: NOC_REQ_R2L_GENLADDR

        default: begin
           nxt_noc_req_state = NOC_REQ_IDLE;
        end
      endcase // case (noc_req_state)

      // Process done information from response
      if (ctrl_done_en) begin
         nxt_open_responses[ctrl_done_pos] = 1'b0;
      end
   end

   // sequential part of NoC interface
   always @(posedge clk) begin
      if (rst) begin
         noc_req_state <= NOC_REQ_IDLE;
         noc_req_counter <= 0;
         noc_req_packet_size <= 5'h0;
         noc_req_packet_count <= 5'h0;
         open_responses <= 0;
         req_start <= 1'b0;
      end else begin
         noc_req_counter <= nxt_noc_req_counter;
         noc_req_packet_size <= nxt_noc_req_packet_size;
         noc_req_packet_count <= nxt_noc_req_packet_count;
         noc_req_state <= nxt_noc_req_state;
         open_responses <= nxt_open_responses;
         req_start <= nxt_req_start;
      end
   end

endmodule // lisnoc_dma_initiator_noc_req

`include "lisnoc_undef.vh"
`include "lisnoc_dma_undef.vh"

// Local Variables:
// verilog-library-directories:("." "../" "../infrastructure")
// verilog-auto-inst-param-value: t
// End:
