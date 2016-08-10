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
 * The module behaving as target in DMA transfers.
 *
 * (c) 2011-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

`include "lisnoc_def.vh"
`include "lisnoc_dma_def.vh"


module lisnoc_dma_target (/*AUTOARG*/
   // Outputs
   noc_out_flit, noc_out_valid, noc_in_ready, wb_cyc_o, wb_stb_o,
   wb_we_o, wb_dat_o, wb_adr_o, wb_sel_o, wb_cti_o, wb_bte_o,
   // Inputs
   clk, rst, noc_out_ready, noc_in_flit, noc_in_valid, wb_ack_i,
   wb_dat_i
   );

   parameter flit_width = `FLIT_WIDTH;

   localparam STATE_WIDTH = 4;
   localparam STATE_IDLE = 4'b0000;
   localparam STATE_L2R_GETADDR = 4'b0001;
   localparam STATE_L2R_DATA = 4'b0010;
   localparam STATE_L2R_SENDRESP = 4'b0011;


   localparam STATE_R2L_GETLADDR = 4'b0100;
   localparam STATE_R2L_GETRADDR = 4'b0101;
   localparam STATE_R2L_GENHDR = 4'b0110;
   localparam STATE_R2L_GENADDR = 4'b0111;
   localparam STATE_R2L_DATA = 4'b1000;


   //TODO: set nxt_wb_waiting = 1'b0 in certain states like idle, or genheader.
   // Not important since we just loose one cycle in the worst case


   parameter table_entries = 4;
   localparam table_entries_ptrwidth = $clog2(table_entries);

   parameter tileid = 0;

   parameter noc_packet_size = 16;

   input  clk;
   input  rst;

   // NOC-Interface
   output reg [`FLIT_WIDTH-1:0] noc_out_flit;
   output reg                   noc_out_valid;
   input                        noc_out_ready;

   input [`FLIT_WIDTH-1:0]      noc_in_flit;
   input                        noc_in_valid;
   output                       noc_in_ready;

   // Wishbone interface for L2R data store
   input                        wb_ack_i;
   output reg                   wb_cyc_o, wb_stb_o, wb_we_o;
   input [31:0]                 wb_dat_i;
   output [31:0]                wb_dat_o;
   output [31:0]                wb_adr_o;
   output [3:0]                 wb_sel_o;
   output reg [2:0]             wb_cti_o;
   output reg [1:0]             wb_bte_o;

   // There is a buffer between the NoC input and the wishbone
   // handling by the state machine. Those are the connection signals
   // from buffer to wishbone
   wire [`FLIT_WIDTH-1:0]       buf_flit;
   wire                         buf_valid;
   reg                          buf_ready;

   /*
    * One FSM that handles the flow from the input
    * buffer to the wishbone interface
    */

   // FSM state
   reg [STATE_WIDTH-1:0]        state;
   reg [STATE_WIDTH-1:0]        nxt_state;

   //FSM hidden state
   reg                          wb_waiting;
   reg                          nxt_wb_waiting;


   // Store request parameters: address, last packet and source
   reg [31:0]                   src_address;
   reg [31:0]                   nxt_src_address;
   reg [31:0]                   address;
   reg [31:0]                   nxt_address;
   reg                          end_of_request;
   reg                          nxt_end_of_request;
   reg [`SOURCE_WIDTH-1:0]      src_tile;
   reg [`SOURCE_WIDTH-1:0]      nxt_src_tile;
   reg [`PACKET_ID_WIDTH-1:0]   packet_id;
   reg [`PACKET_ID_WIDTH-1:0]   nxt_packet_id;

      // Counter for flits/words in request
   reg [`SIZE_WIDTH-1:0] noc_resp_wcounter;
   reg [`SIZE_WIDTH-1:0] nxt_noc_resp_wcounter;

   // Current packet flit/word counter
   reg [4:0]             noc_resp_packet_wcount;
   reg [4:0]             nxt_noc_resp_packet_wcount;

   // Current packet total number of flits/words
   reg [4:0]             noc_resp_packet_wsize;
   reg [4:0]             nxt_noc_resp_packet_wsize;

   // TODO: correct define!
   reg [`DMA_REQFIELD_SIZE_WIDTH-3:0]   resp_wsize;
   reg [`DMA_REQFIELD_SIZE_WIDTH-3:0]   nxt_resp_wsize;
   reg [`DMA_RESPFIELD_SIZE_WIDTH-3:0]  wb_resp_count;
   reg [`DMA_RESPFIELD_SIZE_WIDTH-3:0]  nxt_wb_resp_count;



   //FIFO-Stuff

   wire                                data_fifo_valid;
   reg [31:0]                          data_fifo [0:2]; // data storage
   reg                                 data_fifo_pop;   // NOC pushes
   reg                                 data_fifo_push;  // WB pops

   wire [31:0]                         data_fifo_out; // Current first element
   wire [31:0]                         data_fifo_in;  // Push element
    // Shift register for current position (4th bit is full mark)
   reg [3:0]                           data_fifo_pos;

   wire        data_fifo_empty; // FIFO empty
   wire        data_fifo_ready; // FIFO accepts new elements



   //
   // Input buffer that stores flits until we have one complete packet
   //

   /* lisnoc_packet_buffer AUTO_TEMPLATE(
    .in_\(.*\) (noc_in_\1[]),
    .out_size  (),
    .out_\(.*\) (buf_\1[]),
    ); */
   lisnoc_packet_buffer
     #(.fifo_depth(noc_packet_size))
   u_buf(/*AUTOINST*/
         // Outputs
         .in_ready                      (noc_in_ready),          // Templated
         .out_flit                      (buf_flit[flit_width-1:0]), // Templated
         .out_valid                     (buf_valid),             // Templated
         .out_size                      (),                      // Templated
         // Inputs
         .clk                           (clk),
         .rst                           (rst),
         .in_flit                       (noc_in_flit[flit_width-1:0]), // Templated
         .in_valid                      (noc_in_valid),          // Templated
         .out_ready                     (buf_ready));            // Templated

   // Is this the last flit of a packet?
   wire buf_last_flit;
   assign buf_last_flit = (buf_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]==`FLIT_TYPE_LAST) |
                          (buf_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]==`FLIT_TYPE_SINGLE);



   // The intermediate store a FIFO of three elements
   //
   // There should be no combinatorial path from input to output, so
   // that it takes one cycle before the wishbone interface knows
   // about back pressure from the NoC. Additionally, the wishbone
   // interface needs one extra cycle for burst termination. The data
   // should be stored and not discarded. Finally, there is one
   // element in the FIFO that is the normal timing decoupling.


   // Connect the fifo signals to the ports
   // assign data_fifo_pop = resp_data_ready;
   assign data_fifo_valid = ~data_fifo_empty;
   assign data_fifo_empty = data_fifo_pos[0]; // Empty when pushing to first one
   assign data_fifo_ready = ~|data_fifo_pos[3:2]; //equal to not full
   assign data_fifo_in = wb_dat_i;
   assign data_fifo_out = data_fifo[0]; // First element is out



    // FIFO position pointer logic
   always @(posedge clk) begin
      if (rst) begin
         data_fifo_pos <= 4'b001;
      end else begin
         if (data_fifo_push & ~data_fifo_pop) begin
            // push and no pop
            data_fifo_pos <= data_fifo_pos << 1;
         end else if (~data_fifo_push & data_fifo_pop) begin
            // pop and no push
            data_fifo_pos <= data_fifo_pos >> 1;
         end else begin
            // * no push or pop or
            // * both push and pop
            data_fifo_pos <= data_fifo_pos;
         end
      end
   end

   // FIFO data shifting logic
   always @(posedge clk) begin : data_fifo_shift
      integer i;
      // Iterate all fifo elements, starting from lowest
      for (i=0;i<3;i=i+1) begin
         if (data_fifo_pop) begin
            // when popping data..
            if (data_fifo_push & data_fifo_pos[i+1])
              // .. and we also push this cycle, we need to check
              // whether the pointer was on the next one
              data_fifo[i] <= data_fifo_in;
           else if (i<2)
              // .. otherwise shift if not last
              data_fifo[i] <= data_fifo[i+1];
            else
              // the last stays static
              data_fifo[i] <= data_fifo[i];
         end else if (data_fifo_push & data_fifo_pos[i]) begin
            // when pushing only and this is the current write
            // position
            data_fifo[i] <= data_fifo_in;
         end else begin
            // else just keep
            data_fifo[i] <= data_fifo[i];
         end
      end
   end



   // Wishbone signal generation

   // We only do word transfers
   assign wb_sel_o = 4'hf;

   // The data of the payload flits
   assign wb_dat_o = buf_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB];

   // Assign stored (and incremented) address to wishbone interface
   assign wb_adr_o = address;



//FSM

   // Next state, counting, control signals
   always @(*) begin
      // Default values are old values
      nxt_address = address;
      nxt_resp_wsize = resp_wsize;
      nxt_end_of_request = end_of_request;
      nxt_src_address = src_address;
      nxt_src_tile = src_tile;
      nxt_end_of_request = end_of_request;
      nxt_packet_id = packet_id;
      nxt_wb_resp_count = wb_resp_count;
      nxt_noc_resp_packet_wcount = noc_resp_packet_wcount;
      nxt_noc_resp_packet_wsize = noc_resp_packet_wsize;
      nxt_wb_waiting = wb_waiting;
      nxt_noc_resp_wcounter = noc_resp_wcounter;

      // Default control signals

      wb_cyc_o = 1'b0;
      wb_stb_o = 1'b0;
      wb_we_o  = 1'b0;
      wb_bte_o = 2'b00;
      wb_cti_o = 3'b000;

      noc_out_valid = 1'b0;
      noc_out_flit  = 34'h0;

      data_fifo_push = 1'b0;
      data_fifo_pop = 1'b0;

      buf_ready = 1'b0;

      case (state)
        STATE_IDLE: begin

           buf_ready= 1'b1;
           nxt_end_of_request = buf_flit[`PACKET_REQ_LAST];
           nxt_src_tile = buf_flit[`SOURCE_MSB:`SOURCE_LSB];
           nxt_resp_wsize = buf_flit[`SIZE_MSB:`SIZE_LSB];
           nxt_packet_id = buf_flit[`PACKET_ID_MSB:`PACKET_ID_LSB];
           nxt_noc_resp_wcounter = 0;

           nxt_wb_resp_count = 1;

           if (buf_valid) begin
              if (buf_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB] == `PACKET_TYPE_L2R_REQ) begin
                nxt_state = STATE_L2R_GETADDR;
              end  else if(buf_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB] == `PACKET_TYPE_R2L_REQ) begin
                 nxt_state = STATE_R2L_GETLADDR;
              end else begin
                // now we have a problem...
                // must not happen
                nxt_state = STATE_IDLE;
              end
           end else begin
              nxt_state = STATE_IDLE;
           end
        end // case: STATE_IDLE


  //L2R-handling


        STATE_L2R_GETADDR: begin
           buf_ready = 1'b1;
           nxt_address = buf_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB];
           if (buf_valid) begin
              nxt_state = STATE_L2R_DATA;
           end else begin
              nxt_state = STATE_L2R_GETADDR;
           end
        end


        STATE_L2R_DATA: begin
           if (buf_last_flit)
             wb_cti_o = 3'b111;
           else
             wb_cti_o = 3'b010;

           wb_cyc_o = 1'b1;
           wb_stb_o = 1'b1;
           wb_we_o = 1'b1;

           if (wb_ack_i) begin
              nxt_address = address + 4;
              buf_ready = 1'b1;
              if (buf_last_flit) begin
                 if (end_of_request) begin
                    nxt_state = STATE_L2R_SENDRESP;
                 end else begin
                    nxt_state = STATE_IDLE;
                 end
              end else begin
                 nxt_state = STATE_L2R_DATA;
              end
           end else begin
              buf_ready = 1'b0;
              nxt_state = STATE_L2R_DATA;
           end
        end // case: STATE_L2R_DATA

        STATE_L2R_SENDRESP: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]       = `FLIT_TYPE_SINGLE;
           noc_out_flit[`FLIT_DEST_MSB:`FLIT_DEST_LSB]       = src_tile;
           noc_out_flit[`PACKET_CLASS_MSB:`PACKET_CLASS_LSB] = `PACKET_CLASS_DMA;
           noc_out_flit[`PACKET_ID_MSB:`PACKET_ID_LSB]       = packet_id;
           noc_out_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB]   = `PACKET_TYPE_L2R_RESP;
           if (noc_out_ready) begin
              nxt_state = STATE_IDLE;
           end else begin
              nxt_state = STATE_L2R_SENDRESP;
           end
         end // case: STATE_L2R_SENDRESP


 //R2L handling

        STATE_R2L_GETLADDR: begin
           buf_ready = 1'b1;
           nxt_address = buf_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB];
           if (buf_valid) begin
              nxt_state = STATE_R2L_GETRADDR;
           end else begin
              nxt_state = STATE_R2L_GETLADDR;
           end
        end


        STATE_R2L_GETRADDR: begin
           buf_ready = 1'b1;
           nxt_src_address = buf_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB];
           if (buf_valid) begin
              nxt_state = STATE_R2L_GENHDR;
           end else begin
              nxt_state = STATE_R2L_GETRADDR;
           end
        end

        STATE_R2L_GENHDR: begin

           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB]       = `FLIT_TYPE_HEADER;
           noc_out_flit[`FLIT_DEST_MSB:`FLIT_DEST_LSB]       = src_tile;
           noc_out_flit[`PACKET_CLASS_MSB:`PACKET_CLASS_LSB] = `PACKET_CLASS_DMA;
           noc_out_flit[`PACKET_ID_MSB:`PACKET_ID_LSB]       = packet_id;
           noc_out_flit[`SOURCE_MSB:`SOURCE_LSB]             = tileid;
           noc_out_flit[`PACKET_TYPE_MSB:`PACKET_TYPE_LSB]   = `PACKET_TYPE_R2L_RESP;

           if ((noc_resp_wcounter + (noc_packet_size -2)) < resp_wsize) begin
              // This is not the last packet in the respuest ((noc_packet_size -2) words*4 bytes=120)
              // Only (noc_packet_size -2) flits are availabel for the payload,
              // because we need a header-flit and an address-flit, too.
              noc_out_flit[`SIZE_MSB:`SIZE_LSB] = 7'd120;
              noc_out_flit[`PACKET_RESP_LAST]     = 1'b0;
              nxt_noc_resp_packet_wsize = noc_packet_size -2;
              // count is the current transfer number
              nxt_noc_resp_packet_wcount = 5'd1;
           end else begin
              // This is the last packet in the respuest
              noc_out_flit[`SIZE_MSB:`SIZE_LSB] = resp_wsize - noc_resp_wcounter;
              noc_out_flit[`PACKET_RESP_LAST] = 1'b1;
              nxt_noc_resp_packet_wsize = resp_wsize - noc_resp_wcounter;
              // count is the current transfer number
              nxt_noc_resp_packet_wcount = 5'd1;
           end // else: !if((noc_resp_wcounter + (noc_packet_size -2)) < resp_wsize)




           // change to next state if successful
           if (noc_out_ready)
             nxt_state = STATE_R2L_GENADDR;
           else
             nxt_state = STATE_R2L_GENHDR;

        end // case: STATE_R2L_GENHDR


        STATE_R2L_GENADDR: begin
           noc_out_valid = 1'b1;
           noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_PAYLOAD;
           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = src_address + (noc_resp_wcounter << 2);


           if (noc_out_ready) begin
             nxt_state = STATE_R2L_DATA;

           end else begin
             nxt_state = STATE_R2L_GENADDR;
           end
        end // case: `NOC_RESP_R2L_GENADDR


        STATE_R2L_DATA: begin

          // NOC-handling

           // transfer data to noc if available
           noc_out_valid = data_fifo_valid;
           noc_out_flit[`FLIT_CONTENT_MSB:`FLIT_CONTENT_LSB] = data_fifo_out;

           //TODO: Rearange ifs
           if (noc_resp_packet_wcount==noc_resp_packet_wsize) begin
             noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_LAST;

              if (noc_out_valid & noc_out_ready) begin
                 data_fifo_pop = 1'b1;

                 if ((noc_resp_wcounter + (noc_packet_size -2)) < resp_wsize) begin
                    // Only (noc_packet_size -2) flits are availabel for the payload,
                    // because we need a header-flit and an address-flit, too.

                    //this was not the last packet of the response
                      nxt_state = STATE_R2L_GENHDR;
                      nxt_noc_resp_wcounter = noc_resp_wcounter + noc_resp_packet_wcount;
                 end else begin
                    //this is the last packet of the response
                      nxt_state = STATE_IDLE;
                 end
              end else begin
                 nxt_state = STATE_R2L_DATA;
              end

           end else begin //not LAST
             noc_out_flit[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] = `FLIT_TYPE_PAYLOAD;
              if (noc_out_valid & noc_out_ready) begin
                 data_fifo_pop = 1'b1;
                 nxt_noc_resp_packet_wcount = noc_resp_packet_wcount + 1;

              end
              nxt_state = STATE_R2L_DATA;
           end



          //FIFO-handling
           if (wb_waiting) begin //hidden state
              //don't get data from the bus
              wb_stb_o     = 1'b0;
              wb_cyc_o     = 1'b0;
              data_fifo_push   = 1'b0;
              if (data_fifo_ready) begin
                 nxt_wb_waiting = 1'b0;
              end else begin
                 nxt_wb_waiting = 1'b1;
              end
           end else begin //not wb_waiting
              // Signal cycle and strobe. We do bursts, but don't insert
              // wait states, so both of them are always equal.
              if ((noc_resp_packet_wcount==noc_resp_packet_wsize) & noc_out_valid & noc_out_ready) begin
                 wb_stb_o = 1'b0;
                 wb_cyc_o = 1'b0;
              end else begin
                 wb_stb_o = 1'b1;
                 wb_cyc_o = 1'b1;
              end
              // TODO: why not generate address from the base address + counter<<2?

              if (~data_fifo_ready | (wb_resp_count==resp_wsize)) begin
                 wb_cti_o = 3'b111;
              end else begin
                 wb_cti_o = 3'b111;
              end

              if (wb_ack_i) begin
                 // When this was successfull..
                 if (~data_fifo_ready | (wb_resp_count==resp_wsize)) begin
                    nxt_wb_waiting = 1'b1;
                 end else begin
                    nxt_wb_waiting = 1'b0;
                 end
                 nxt_wb_resp_count = wb_resp_count + 1;
                 nxt_address = address + 4;
                 data_fifo_push = 1'b1;
              end else begin
                 // ..otherwise we still wait for the acknowledgement
                 nxt_wb_resp_count = wb_resp_count;
                 nxt_address = address;
                 data_fifo_push = 1'b0;
                 nxt_wb_waiting = 1'b0;
              end
           end // else: !if(wb_waiting)
        end // case: STATE_R2L_DATA

        default: begin
           nxt_state = STATE_IDLE;
        end
      endcase // case (state)
   end



   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         address <= 32'h0;

         end_of_request <= 1'b0;
         src_tile <= 0;
         resp_wsize <= 0;
         packet_id <= 0;

         src_address <= 0;
         noc_resp_wcounter <= 0;
         noc_resp_packet_wsize <= 5'h0;
         noc_resp_packet_wcount <= 5'h0;
         noc_resp_packet_wcount <= 0;


         wb_resp_count <= 0;
         wb_waiting <= 0;



      end else begin
         state <= nxt_state;
         address <= nxt_address;

         end_of_request <= nxt_end_of_request;

         src_tile <= nxt_src_tile;
         resp_wsize <= nxt_resp_wsize;
         packet_id <= nxt_packet_id;

         src_address <= nxt_src_address;
         noc_resp_wcounter <= nxt_noc_resp_wcounter;
         noc_resp_packet_wsize <= nxt_noc_resp_packet_wsize;
         noc_resp_packet_wcount <= nxt_noc_resp_packet_wcount;


         wb_resp_count <= nxt_wb_resp_count;
         wb_waiting <= nxt_wb_waiting;


      end
   end




endmodule // lisnoc_dma_target

// Local Variables:
// verilog-library-directories:("../infrastructure")
// verilog-auto-inst-param-value: t
// End:
