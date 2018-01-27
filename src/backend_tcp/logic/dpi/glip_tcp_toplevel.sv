/* Copyright (c) 2016 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Wei Song <wsong83@gmail.com>
 */

module glip_tcp_toplevel
  #(parameter WIDTH = 16,
    parameter PORT = 23000,
    parameter UART_LIKE = 0
    )
   (
    // Clock & Reset
    input                  clk_io,
    input                  clk_logic,
    input                  rst,

    // GLIP FIFO Interface
    glip_channel.slave     fifo_out,
    glip_channel.master    fifo_in,

    // GLIP Control Interface
    output reg             logic_rst,
    output reg             com_rst
    );

   import "DPI-C" function
     chandle glip_tcp_create(input int tcp_port,
                             input int width);

   import "DPI-C" function
     int glip_tcp_reset(input chandle obj);

   import "DPI-C" function
     int glip_tcp_connected(input chandle obj);

   import "DPI-C" function
     int glip_tcp_next_cycle(input chandle obj);

   import "DPI-C" function
     int glip_tcp_control_msg(input chandle obj);

   import "DPI-C" function
     longint glip_tcp_read(input chandle obj);

   import "DPI-C" function
     void glip_tcp_read_ack(input chandle obj);

   import "DPI-C" function
     void glip_tcp_write(input chandle obj, input longint unsigned data);

   chandle obj;

   localparam STATE_MASK_CTRL  = 32'h1;
   localparam STATE_MASK_READ  = 32'h2;
   localparam STATE_MASK_WRITE = 32'h4;
   localparam UART_DELAY = 8;

   logic [63:0] rcnt, wcnt;

   always @(negedge clk_logic) begin
      if (rst) begin
         logic_rst = 0;
         com_rst = 0;
         rcnt = 0;
         wcnt = 0;
      end else begin
         automatic int connected;
         automatic int unsigned state;
         automatic longint data;

         connected = glip_tcp_connected(obj);
         if (connected > 0) begin
            com_rst = 1'b0;
         end else begin
            com_rst = 1'b1;
         end

         state = glip_tcp_next_cycle(obj);

         // Get control message
         if ((state & STATE_MASK_CTRL) != 0) begin
            automatic int data = glip_tcp_control_msg(obj);
            logic_rst = data[0];
         end else begin
            logic_rst = 1'b0;
         end

         // We have new incoming data
         if ((state & STATE_MASK_READ) != 0) begin
            if (UART_LIKE && rcnt == 0) begin
               rcnt = UART_DELAY;
            end

            if (UART_LIKE ? rcnt == 1 : 1) begin
               automatic longint data = glip_tcp_read(obj);
               fifo_in.data = data[WIDTH-1:0];
               fifo_in.valid = 1;
            end else begin
               fifo_in.valid = 0;
            end
         end else begin
            fifo_in.valid = 0;
         end

         // Write outgoing data
         if ((state & STATE_MASK_WRITE) != 0) begin
            if (UART_LIKE && wcnt == 0) begin
               wcnt = UART_DELAY;
            end

            if (UART_LIKE ? wcnt == 1 : 1) begin
               fifo_out.ready = 1;
            end else begin
               fifo_out.ready = 0;
            end
         end else begin
            fifo_out.ready = 0;
         end

         if (rcnt != 0) begin
            rcnt = rcnt - 1;
         end

         if (wcnt != 0) begin
            wcnt = wcnt - 1;
         end
      end
   end

   always @(posedge clk_logic) begin
      if (fifo_in.valid & fifo_in.ready) begin
         glip_tcp_read_ack(obj);
      end
      if (fifo_out.valid & fifo_out.ready) begin
         automatic longint data = 0;
         data[WIDTH-1:0] = fifo_out.data;
         glip_tcp_write(obj, data);
      end
   end

   int port_int;

   initial begin
      obj = glip_tcp_create(PORT, WIDTH);
   end

endmodule
