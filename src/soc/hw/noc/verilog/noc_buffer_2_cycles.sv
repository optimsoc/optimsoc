/* Copyright (c) 2017-2019 by the author(s)
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
 * Buffer for NoC packets
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Wei Song <wsong83@gmail.com>
 *   Max Koenen <max.koenen@tum.de>
 */

/**
 * FIFO buffer for NoC use cases (2 delay cycle version)
 *
 * This FIFO buffer for NoC use cases has 2 clock cycles minimum delay from
 * input to output and is coded according to the "Vivado User Guide UG901",
 * allowing it to be synthesized using LUT RAM.
 * The required logic of this buffer is smaller in comparison to the version
 * with 1 cycle minimum delay.
 * The FIFO provides a feature which will only signal a valid flit at the output
 * if a full packet is stored inside the buffer (i.e. at least one last-flit is
 * stored). If this feature is activated (FULLPACKET = 1) the size of the packet
 * can be read from the 'packet_size' output.
 */
module noc_buffer #(
   parameter FLIT_WIDTH = 32,
   parameter DEPTH = 16,   // must be a power of 2
   parameter FULLPACKET = 0,

   localparam AW = $clog2(DEPTH) // the width of the index
)(
   input clk,
   input rst,

   // FIFO input side
   input [FLIT_WIDTH-1:0]        in_flit,
   input                         in_last,
   input                         in_valid,
   output                        in_ready,

   //FIFO output side
   output reg [FLIT_WIDTH-1:0]   out_flit,
   output reg                    out_last,
   output                        out_valid,
   input                         out_ready,

   output [AW-1:0]               packet_size
);

   // Ensure that parameters are set to allowed values
   initial begin
      if ((1 << $clog2(DEPTH)) != DEPTH) begin
         $fatal("noc_buffer: the DEPTH must be a power of two.");
      end
   end

   reg [AW-1:0]   wr_addr;
   reg [AW-1:0]   rd_addr_reg;
   reg [AW-1:0]   rd_count;
   wire [AW-1:0]  rd_addr;
   wire           fifo_read;
   wire           fifo_write;
   wire           update_out;

   assign in_ready = (rd_count < (DEPTH-1));
   assign rd_addr = fifo_read ? rd_addr_reg + 1'b1 : rd_addr_reg;
   assign fifo_read = out_valid & out_ready;
   assign fifo_write = in_ready & in_valid;
   assign update_out = (rd_count == 1) | fifo_read;

   // Address logic
   always_ff @(posedge clk) begin
      if (rst) begin
         wr_addr <= 'b0;
         rd_addr_reg <= 'b0;
         rd_count <= 'b0;
      end else begin
         if (fifo_write & fifo_read) begin
            wr_addr <= wr_addr + 1'b1;
            rd_addr_reg <= rd_addr_reg + 1'b1;
         end else if (fifo_write) begin
            wr_addr <= wr_addr + 1'b1;
            rd_count <= rd_count + 1'b1;
         end else if (fifo_read) begin
            rd_addr_reg <= rd_addr_reg + 1'b1;
            rd_count <= rd_count - 1'b1;
         end
      end
   end

   // Generic dual-port, single clock memory
   reg [FLIT_WIDTH:0] ram [DEPTH-1:0];

   // Write
   always_ff @(posedge clk) begin
      if (fifo_write) begin
         ram[wr_addr] <= {in_last, in_flit};
      end
   end

   // Read
   always_ff @(posedge clk) begin
      if (update_out) begin
         out_flit <= ram[rd_addr][0 +: FLIT_WIDTH];
         out_last <= ram[rd_addr][FLIT_WIDTH];
      end
   end

   generate
      if (FULLPACKET != 0) begin
         reg [DEPTH-1:0] data_last_buf;
         wire [DEPTH-1:0] data_last_shifted;
         reg nxt_out_valid;

         always @(posedge clk)
            if (rst)
               data_last_buf <= 0;
            else if (fifo_write)
               data_last_buf <= {data_last_buf, in_last};

         // Extra logic to get the packet size in a stable manner
         assign data_last_shifted = data_last_buf << DEPTH - 1 - (rd_count-1);

         function logic [AW:0] find_first_one(input logic [DEPTH-1:0] data);
            automatic int i;
            for (i = DEPTH-1; i >= 0; i--)
               if (data[i]) return i;
                  return DEPTH;
         endfunction // size_count

         // Delay out_valid signal by one cycle to match flit delay
         always @(posedge clk)
            nxt_out_valid <= (rd_count > 1 || (rd_count == 1 && ~fifo_read)) && |data_last_shifted;

         assign out_valid = nxt_out_valid;
         assign packet_size = DEPTH - find_first_one(data_last_shifted);
      end else begin // if (FULLPACKET)
         reg nxt_out_valid;
         // Delay out_valid signal by one cycle to match flit delay
         always @(posedge clk)
            nxt_out_valid <= (rd_count > 1 || (rd_count == 1 && ~fifo_read));
         assign out_valid = nxt_out_valid;
         assign packet_size = 0;
      end
   endgenerate
endmodule // noc_buffer
