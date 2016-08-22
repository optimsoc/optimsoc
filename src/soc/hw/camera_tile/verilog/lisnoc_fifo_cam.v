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
 * This is the FIFO used in the input and output port.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_fifo_cam(/*AUTOARG*/
   // Outputs
   in_ready, out_flit, out_valid, out_first_word,
   // Inputs
   clk, rst, in_flit, in_valid, out_ready, irq_new_frame
   );

   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   parameter packet_length   = 0;

   localparam flit_width = flit_data_width+flit_type_width;

   parameter LENGTH = 16;

   input  clk;
   input  rst;

   // FIFO input side
   //input  [flit_width-1:0] in_flit;   // input
   input  [flit_data_width-1:0] in_flit; // input
   input  in_valid;                   // write_enable
   output in_ready;                   // accepting new data

   //FIFO output side
   //output [flit_width-1:0] out_flit;   // data_out
   output [flit_data_width-1:0] out_flit;
   output out_valid;                   // data available
   input  out_ready;                   // read request

   // Signals for fifo
   reg [flit_width-1:0] fifo_data [0:LENGTH-1]; //actual fifo
   reg [flit_width-1:0] nxt_fifo_data [0:LENGTH-1];

   reg [LENGTH:0]         fifo_write_ptr;

   wire                   pop;
   wire                   push;


   // IRQ FSM
   input irq_new_frame;
   parameter IDLE = 1'b0, WAIT_FOR_FIRST_FRAME_WORD = 1'b1;
   reg state = 0;
   wire nxt_state;
   wire [1:0] first_word;

   // output logic
   always @(*) begin
   	case (state)
		IDLE: first_word = 2'b00;
		WAIT_FOR_FIRST_FRAME_WORD: first_word = 2'b11;
		default: first_word = 2'b00;
   	endcase
   end

   // next state logic
   always @(*) begin
   	case (state)
   		IDLE: begin
			nxt_state = IDLE;
        	if (irq_new_frame == 1) begin
				nxt_state = WAIT_FOR_FIRST_FRAME_WORD;
			end
		end
		WAIT_FOR_FIRST_FRAME_WORD: begin
			nxt_state = WAIT_FOR_FIRST_FRAME_WORD;
			if (in_valid == 1) begin
				nxt_state = IDLE;
			end
		end
		default: nxt_state = IDLE;
   	endcase
   end

   // state transition
   always @ (posedge clk)
   begin
		if (rst == 1) begin
			state <= IDLE;		
		end else begin
			state <= nxt_state;
		end
   end
					
   // end IRQ FSM

   output [1:0] out_first_word;
   assign out_first_word = fifo_data[0][33:32];

   //assign pop = out_valid & out_ready;
   //assign push = in_valid & in_ready;

   //enable overwriting of old values if FIFO is full and
   //no data is popped
   assign pop = (!in_ready & in_valid) | (out_valid & out_ready);
   assign push = in_valid;
   
   assign out_flit = fifo_data[0][31:0];
   assign out_valid = !fifo_write_ptr[0];

   assign in_ready = !fifo_write_ptr[LENGTH];

   always @(posedge clk) begin
      if (rst) begin
         fifo_write_ptr <= {{LENGTH{1'b0}},1'b1};
      end else if (push & !pop) begin
         fifo_write_ptr <= fifo_write_ptr << 1;
      end else if (!push & pop) begin
         fifo_write_ptr <= fifo_write_ptr >> 1;
      end
   end

   always @(*) begin : shift_register_comb
      integer i;
      for (i=0;i<LENGTH;i=i+1) begin
         if (pop) begin
            if (push & fifo_write_ptr[i+1]) begin
               nxt_fifo_data[i] = {first_word, in_flit};
            end else if (i<LENGTH-1) begin
               nxt_fifo_data[i] = fifo_data[i+1];
            end else begin
               nxt_fifo_data[i] = fifo_data[i];
            end
         end else if (push & fifo_write_ptr[i]) begin
            nxt_fifo_data[i] = {first_word, in_flit};
         end else begin
            nxt_fifo_data[i] = fifo_data[i];
         end
      end
   end

   always @(posedge clk) begin : shift_register_seq
      integer i;
      for (i=0;i<LENGTH;i=i+1) begin
        fifo_data[i] <= nxt_fifo_data[i];
      end
   end

endmodule // noc_fifo_cam

`include "lisnoc_undef.vh"
