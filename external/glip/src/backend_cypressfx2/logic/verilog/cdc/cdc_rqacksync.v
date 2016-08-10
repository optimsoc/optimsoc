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
 * Safely cross short request pulses through clock domains.
 *
 * Req is set high as long as it is not acknowledged by the read domain
 * wr_reqbusy indicates ongoing signal crossing.
 * If wr_reqbusy is '1' all requests are ignored!
 * To resolve possible metastability req and ack are synchronized by 2-FF
 * synchronizers.
 *
 * Author(s):
 *   Mark Sagi <mark.sagi@mytum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module cdc_rqacksync (/*AUTOARG*/
   // Outputs
   rd_clkreq, wr_reqbusy,
   // Inputs
   wr_clkreq, rd_clk, wr_clk, rst
   );

   input wr_clkreq;
   input rd_clk;
   input wr_clk;
   input rst;

   output rd_clkreq;
   output wr_reqbusy;

   reg     req_reg;
   reg     rd_reqshort;


   wire    ack_reg;
   reg     ack_regshort;

   wire    wr_req_ack;
   wire    req_reg_syn;
   wire    wr_reqbus;
   reg     wr_reqbus_reg;

   assign wr_reqbus = (wr_req_ack ^ req_reg);
   assign wr_reqbusy = wr_reqbus;

   always @(posedge wr_clk or posedge rst) begin
      if (rst == 1'b1) begin
         req_reg <= 1'b0;
      end else begin
         if (wr_reqbus == 1'b0) begin
            req_reg <= wr_clkreq;
         end else begin
            req_reg <= req_reg;
         end
      end
    end

   // change long pulse ack_reg into one clock cycle pulse
   always @(posedge wr_clk or posedge rst) begin
      if (rst == 1'b1) begin
         ack_regshort <= 1'b0;
      end else begin
         if (ack_reg == 1'b1) begin
            ack_regshort <= 1'b1;
         end else begin
            ack_regshort <= 1'b0;
         end
      end
   end
   assign wr_req_ack = !((!ack_reg) || ack_regshort);

   // change long pulse req_reg_syn into one clock cycle pulse
   always @(posedge rd_clk or posedge rst) begin
      if (rst == 1'b1) begin
         rd_reqshort <= 1'b0;
      end else begin
         if (req_reg_syn == 1'b1) begin
            rd_reqshort <= 1'b1;
         end else begin
            rd_reqshort <= 1'b0;
         end
      end
   end
   assign rd_clkreq = !((!req_reg_syn) || rd_reqshort);

   always @(posedge wr_clk or posedge rst) begin
      if (rst == 1'b1) begin
         wr_reqbus_reg <= 1'b0;
      end else begin
         wr_reqbus_reg <= wr_reqbus;
      end
   end

   cdc_sync_2ff
      u_sync_rd2wr(// Outputs
                   .rd_p(req_reg_syn),
                   // Inputs
                   .rd_clk(rd_clk),
                   .wr_p(wr_reqbus_reg),
                   .rst(rst));
   cdc_sync_2ff
      u_sync_wr2rd(// Outputs
                   .rd_p(ack_reg),
                   // Inputs
                   .rd_clk(wr_clk),
                   .wr_p(req_reg_syn),
                   .rst(rst));

endmodule
