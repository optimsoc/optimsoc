// Copyright 2016 by the authors
//
// Copyright and related rights are licensed under the Solderpad
// Hardware License, Version 0.51 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a
// copy of the License at http://solderpad.org/licenses/SHL-0.51.
// Unless required by applicable law or agreed to in writing,
// software, hardware and materials distributed under this License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the
// License.
//
// Authors:
//    Stefan Wallentowitz <stefan@wallentowitz.de>

import dii_package::dii_flit;

module osd_him
  #(parameter MAX_PKT_LEN = 8)
  (input clk, rst,
   glip_channel.slave glip_in,
   glip_channel.master glip_out,

   output dii_flit dii_out,
   input           dii_out_ready,
   input dii_flit dii_in,
   output         dii_in_ready);

   localparam BUF_SIZE = MAX_PKT_LEN;

   dii_flit dii_ingress;
   logic          dii_ingress_ready;

   logic ingress_active;
   logic [4:0] ingress_size;

   logic [15:0] ingress_data_be;
   assign ingress_data_be[7:0] = glip_in.data[15:8];
   assign ingress_data_be[15:8] = glip_in.data[7:0];

   assign glip_in.ready = !ingress_active | dii_ingress_ready;
   assign dii_ingress.data  = ingress_data_be;
   assign dii_ingress.valid = ingress_active & glip_in.valid;
   assign dii_ingress.last  = ingress_active & (ingress_size == 0);

   always @(posedge clk) begin
      if (rst) begin
         ingress_active <= 0;
      end else begin
         if (!ingress_active) begin
            if (glip_in.valid & glip_in.ready) begin
               ingress_size <= ingress_data_be[4:0] - 1;
               ingress_active <= 1;
            end
         end else begin
            if (glip_in.valid & glip_in.ready) begin
               ingress_size <= ingress_size - 1;
               if (ingress_size == 0) begin
                  ingress_active <= 0;
               end
            end
         end
      end
   end

   dii_buffer
     #(.BUF_SIZE(BUF_SIZE), .FULLPACKET(1))
   u_ingress_buffer(.*,
                   .packet_size (),
                   .flit_in (dii_ingress),
                   .flit_in_ready (dii_ingress_ready),
                   .flit_out (dii_out),
                   .flit_out_ready (dii_out_ready));

   dii_flit dii_egress;
   logic    dii_egress_ready;
   logic [$clog2(BUF_SIZE):0] egress_packet_size;

   logic       egress_active;

   logic [15:0] egress_data_be;

   always @(*) begin
     if (!egress_active) begin
        egress_data_be = 0;
        egress_data_be[$clog2(BUF_SIZE):0] = egress_packet_size;
     end else
       egress_data_be  = dii_egress.data;
   end

   assign glip_out.data = {egress_data_be[7:0], egress_data_be[15:8]};
   assign glip_out.valid = dii_egress.valid;
   assign dii_egress_ready = egress_active & glip_out.ready;

   always @(posedge clk) begin
      if (rst) begin
         egress_active <= 0;
      end else begin
         if (!egress_active) begin
            if (dii_egress.valid & glip_out.ready) begin
               egress_active <= 1;
            end
         end else begin
            if (dii_egress.valid & dii_egress_ready & dii_egress.last) begin
               egress_active <= 0;
            end
         end
      end
   end

   dii_buffer
     #(.BUF_SIZE(BUF_SIZE), .FULLPACKET(1))
   u_egress_buffer(.*,
                   .packet_size (egress_packet_size),
                   .flit_in (dii_in),
                   .flit_in_ready (dii_in_ready),
                   .flit_out (dii_egress),
                   .flit_out_ready (dii_egress_ready));


endmodule // osd_him
