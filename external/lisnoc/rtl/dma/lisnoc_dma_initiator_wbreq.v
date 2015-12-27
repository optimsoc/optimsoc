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
 * The wishbone master of the initiator.
 *
 * (c) 2011-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

`include "lisnoc_def.vh"
`include "lisnoc_dma_def.vh"


module lisnoc_dma_initiator_wbreq(/*AUTOARG*/
   // Outputs
   wb_req_cyc_o, wb_req_stb_o, wb_req_we_o, wb_req_dat_o,
   wb_req_adr_o, wb_req_cti_o, wb_req_bte_o, wb_req_sel_o,
   req_data_valid, req_data,
   // Inputs
   clk, rst, wb_req_ack_i, wb_req_dat_i, req_start, req_is_l2r,
   req_size, req_laddr, req_data_ready
   );

   input clk, rst;

   input             wb_req_ack_i;
   output reg        wb_req_cyc_o, wb_req_stb_o;
   output            wb_req_we_o;
   input [31:0]      wb_req_dat_i;
   output [31:0]     wb_req_dat_o;
   output reg [31:0] wb_req_adr_o;
   output reg [2:0]  wb_req_cti_o;
   output [1:0]      wb_req_bte_o;
   output [3:0]      wb_req_sel_o;

   input                                req_start;
   input                                req_is_l2r;
   input [`DMA_REQFIELD_SIZE_WIDTH-3:0] req_size;
   input [31:0]                         req_laddr;
   output                               req_data_valid;
   output [31:0]                        req_data;
   input                                req_data_ready;

   //
   // Wishbone state machine
   //
`define WB_REQ_WIDTH 2
`define WB_REQ_IDLE  2'b00
`define WB_REQ_DATA  2'b01
`define WB_REQ_WAIT  2'b10

   // State logic
   reg [`WB_REQ_WIDTH-1:0] wb_req_state;
   reg [`WB_REQ_WIDTH-1:0] nxt_wb_req_state;

   // Counter for the state machine for loaded words
   reg [`DMA_REQFIELD_SIZE_WIDTH-3:0] wb_req_count;
   reg [`DMA_REQFIELD_SIZE_WIDTH-3:0] nxt_wb_req_count;

   /*
    * The wishbone data fetch and the NoC interface are seperated by a FIFO.
    * This FIFO relaxes problems with bursts and their termination and decouples
    * the timing of the NoC side and the wishbone side (in terms of termination).
    */

   // The intermediate store a FIFO of three elements
   //
   // There should be no combinatorial path from input to output, so
   // that it takes one cycle before the wishbone interface knows
   // about back pressure from the NoC. Additionally, the wishbone
   // interface needs one extra cycle for burst termination. The data
   // should be stored and not discarded. Finally, there is one
   // element in the FIFO that is the normal timing decoupling.

   reg [31:0]                          data_fifo [0:2]; // data storage
   wire                                data_fifo_pop;   // NoC pops
   reg                                 data_fifo_push;  // WB pushes

   wire [31:0]                         data_fifo_out; // Current first element
   wire [31:0]                         data_fifo_in;  // Push element
    // Shift register for current position (4th bit is full mark)
   reg [3:0]                           data_fifo_pos;

   wire        data_fifo_empty; // FIFO empty
   wire        data_fifo_ready; // FIFO accepts new elements

   // Connect the fifo signals to the ports
   assign data_fifo_pop = req_data_ready;
   assign req_data_valid = ~data_fifo_empty;
   assign req_data = data_fifo_out;

   assign data_fifo_empty = data_fifo_pos[0]; // Empty when pushing to first one
   assign data_fifo_out = data_fifo[0]; // First element is out

   // FIFO is not ready when back-pressure would result in discarded data,
   // that is, we need one store element (high-water).
   assign data_fifo_ready = ~|data_fifo_pos[3:2];

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

   //
   // Wishbone interface logic
   //

   // Statically zero (this interface reads)
   assign wb_req_dat_o = 32'h0000_0000;
   assign wb_req_we_o = 1'b0;

   // We only support full (aligned) word transfers
   assign wb_req_sel_o = 4'b1111;

   // We only need linear bursts
   assign wb_req_bte_o = 2'b00;

   // The input to the fifo is the data from the bus
   assign data_fifo_in = wb_req_dat_i;

   // Next state, wishbone combinatorial signals and counting
   always @(*) begin
      // Signal defaults
      nxt_wb_req_count = wb_req_count;
      wb_req_stb_o     = 1'b0;
      wb_req_cyc_o     = 1'b0;
      data_fifo_push   = 1'b0;

      wb_req_adr_o = 32'hx;
      wb_req_cti_o = 3'b000;

      case (wb_req_state)
        `WB_REQ_IDLE: begin
           // We are idle'ing

           // Always reset counter
           nxt_wb_req_count = 0;

           if (req_start & req_is_l2r)
             // start when new request is handled and it is a L2R
             // request. Direct transition to data fetching from bus,
             // as the FIFO is always empty at this point.
             nxt_wb_req_state = `WB_REQ_DATA;
           else
             // otherwise keep idle'ing
             nxt_wb_req_state = `WB_REQ_IDLE;
        end
        `WB_REQ_DATA: begin
           // We get data from the bus

           // Signal cycle and strobe. We do bursts, but don't insert
           // wait states, so both of them are always equal.
           wb_req_stb_o = 1'b1;
           wb_req_cyc_o = 1'b1;

           // The address is the base address plus the counter
           // Counter counts words, not bytes
           wb_req_adr_o = req_laddr + (wb_req_count << 2);

           if (~data_fifo_ready | (wb_req_count==req_size-1)) begin
              // If fifo gets full next cycle, do cycle termination
              wb_req_cti_o = 3'b111;
           end else begin
              // As long as we can also store the _next_ element in
              // the fifo, signal we are in an incrementing burst
              wb_req_cti_o = 3'b010;
           end

           if (wb_req_ack_i) begin
              // When this request was successfull..

              // increment word counter
              nxt_wb_req_count = wb_req_count + 1;
              // signal push to data fifo
              data_fifo_push = 1'b1;

              if (wb_req_count==req_size-1)
                // This was the last word
                nxt_wb_req_state = `WB_REQ_IDLE;
              else if (data_fifo_ready)
                // when FIFO can still get data, we stay here
                nxt_wb_req_state = `WB_REQ_DATA;
              else
                // .. otherwise we wait for FIFO to become ready
                nxt_wb_req_state = `WB_REQ_WAIT;
           end else begin // if (wb_req_ack_i)
              // ..otherwise we still wait for the acknowledgement
              nxt_wb_req_state = `WB_REQ_DATA;
           end
        end
        `WB_REQ_WAIT: begin
           // Waiting for FIFO to accept new data
           if (data_fifo_ready)
             // FIFO ready, restart burst
             nxt_wb_req_state = `WB_REQ_DATA;
           else
             // wait
             nxt_wb_req_state = `WB_REQ_WAIT;
        end
        default: begin
           nxt_wb_req_state = `WB_REQ_IDLE;
        end
      endcase
   end

   // Sequential part of the state machine
   always @(posedge clk) begin
      if (rst) begin
         wb_req_state <= `WB_REQ_IDLE;
         wb_req_count <= 0;
      end else begin
         wb_req_state <= nxt_wb_req_state;
         wb_req_count <= nxt_wb_req_count;
      end
   end


endmodule // lisnoc_dma_initiator_wbreq

`include "lisnoc_dma_undef.vh"

// Local Variables:
// verilog-library-directories:("../" "../infrastructure")
// verilog-auto-inst-param-value: t
// End:
