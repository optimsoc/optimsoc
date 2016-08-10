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
 * Author(s):
 *   Andreas Lankes <andreas.lankes@tum.de
 */

`define NORTH 5'b00001
`define EAST  5'b00010
`define SOUTH 5'b00100
`define WEST  5'b01000
`define LOCAL 5'b10000

module mesh(/*AUTOARG*/
   // Outputs
   links_in_ready, links_out_flit, links_out_valid,
   // Inputs
   clk, rst, links_in_flit, links_in_valid, links_out_ready
   );
   parameter flit_type_width = 2;
   parameter flit_data_width = 32;
   parameter vchannels = 1;

   parameter xdim = 2;
   parameter ydim = 2;

   parameter ph_prio_width = 4;    //Defines the lenght of the priority field in the header flit if priority unicast routing is selected
   parameter payload_length = 6;   //Maximum packet lenght if recursive partitioning is selected

   localparam nodes = xdim*ydim;
   localparam dest_width = $clog2(nodes);

   localparam flit_width = flit_data_width + flit_type_width;

   input clk;
   input rst;

   input [flit_width*nodes-1:0] links_in_flit;
   input [vchannels*nodes-1:0]  links_in_valid;
   output [vchannels*nodes-1:0] links_in_ready;
   output [flit_width*nodes-1:0] links_out_flit;
   output [vchannels*nodes-1:0]  links_out_valid;
   input [vchannels*nodes-1:0]   links_out_ready;


   wire [flit_width-1:0] local_in_flit[0:nodes-1];
   wire [vchannels-1:0]  local_in_valid[0:nodes-1];
   wire [vchannels-1:0]  local_in_ready[0:nodes-1];
   wire [flit_width-1:0] local_out_flit[0:nodes-1];
   wire [vchannels-1:0]  local_out_valid[0:nodes-1];
   wire [vchannels-1:0]  local_out_ready[0:nodes-1];
   wire [flit_width-1:0] north_in_flit[0:nodes-1];
   wire [vchannels-1:0]  north_in_valid[0:nodes-1];
   wire [vchannels-1:0]  north_in_ready[0:nodes-1];
   wire [flit_width-1:0] north_out_flit[0:nodes-1];
   wire [vchannels-1:0]  north_out_valid[0:nodes-1];
   wire [vchannels-1:0]  north_out_ready[0:nodes-1];
   wire [flit_width-1:0] east_in_flit[0:nodes-1];
   wire [vchannels-1:0]  east_in_valid[0:nodes-1];
   wire [vchannels-1:0]  east_in_ready[0:nodes-1];
   wire [flit_width-1:0] east_out_flit[0:nodes-1];
   wire [vchannels-1:0]  east_out_valid[0:nodes-1];
   wire [vchannels-1:0]  east_out_ready[0:nodes-1];
   wire [flit_width-1:0] south_in_flit[0:nodes-1];
   wire [vchannels-1:0]  south_in_valid[0:nodes-1];
   wire [vchannels-1:0]  south_in_ready[0:nodes-1];
   wire [flit_width-1:0] south_out_flit[0:nodes-1];
   wire [vchannels-1:0]  south_out_valid[0:nodes-1];
   wire [vchannels-1:0]  south_out_ready[0:nodes-1];
   wire [flit_width-1:0] west_in_flit[0:nodes-1];
   wire [vchannels-1:0]  west_in_valid[0:nodes-1];
   wire [vchannels-1:0]  west_in_ready[0:nodes-1];
   wire [flit_width-1:0] west_out_flit[0:nodes-1];
   wire [vchannels-1:0]  west_out_valid[0:nodes-1];
   wire [vchannels-1:0]  west_out_ready[0:nodes-1];

   genvar x;
   genvar y;

   generate
      for (y=0;y<ydim;y=y+1) begin
         for (x=0;x<xdim;x=x+1) begin
              lisnoc_router_2dgrid #(.vchannels(vchannels),.num_dests(nodes),.lookup(genlookup(x,y)),
                 .flit_data_width(flit_data_width),.flit_type_width(flit_type_width),.ph_dest_width(dest_width),
                 .use_prio(select_router),.ph_prio_width(ph_prio_width))
                  u_router(
                           .clk (clk),
                           .rst (rst),
                           .local_in_valid_i  (local_in_valid[nodenum(x,y)]),
                           .local_in_flit_i   (local_in_flit[nodenum(x,y)]),
                           .local_in_ready_o  (local_in_ready[nodenum(x,y)]),
                           .local_out_valid_o (local_out_valid[nodenum(x,y)]),
                           .local_out_flit_o  (local_out_flit[nodenum(x,y)]),
                           .local_out_ready_i (local_out_ready[nodenum(x,y)]),
                           .north_in_valid_i  (north_in_valid[nodenum(x,y)]),
                           .north_in_flit_i   (north_in_flit[nodenum(x,y)]),
                           .north_in_ready_o  (north_in_ready[nodenum(x,y)]),
                           .north_out_valid_o (north_out_valid[nodenum(x,y)]),
                           .north_out_flit_o  (north_out_flit[nodenum(x,y)]),
                           .north_out_ready_i (north_out_ready[nodenum(x,y)]),
                           .east_in_valid_i   (east_in_valid[nodenum(x,y)]),
                           .east_in_flit_i    (east_in_flit[nodenum(x,y)]),
                           .east_in_ready_o   (east_in_ready[nodenum(x,y)]),
                           .east_out_valid_o  (east_out_valid[nodenum(x,y)]),
                           .east_out_flit_o   (east_out_flit[nodenum(x,y)]),
                           .east_out_ready_i  (east_out_ready[nodenum(x,y)]),
                           .south_in_valid_i  (south_in_valid[nodenum(x,y)]),
                           .south_in_flit_i   (south_in_flit[nodenum(x,y)]),
                           .south_in_ready_o  (south_in_ready[nodenum(x,y)]),
                           .south_out_valid_o (south_out_valid[nodenum(x,y)]),
                           .south_out_flit_o  (south_out_flit[nodenum(x,y)]),
                           .south_out_ready_i (south_out_ready[nodenum(x,y)]),
                           .west_in_valid_i   (west_in_valid[nodenum(x,y)]),
                           .west_in_flit_i    (west_in_flit[nodenum(x,y)]),
                           .west_in_ready_o   (west_in_ready[nodenum(x,y)]),
                           .west_out_valid_o  (west_out_valid[nodenum(x,y)]),
                           .west_out_flit_o   (west_out_flit[nodenum(x,y)]),
                           .west_out_ready_i  (west_out_ready[nodenum(x,y)])
                           );

            // Generate connection in north-south direction
            if (y>0 && y<ydim-1) begin
               // North port connection
               assign north_in_valid[nodenum(x,y)]  = south_out_valid[northof(x,y)];
               assign north_in_flit[nodenum(x,y)]   = south_out_flit[northof(x,y)];
               assign north_out_ready[nodenum(x,y)] = south_in_ready[northof(x,y)];
               assign south_in_valid[northof(x,y)]  = north_out_valid[nodenum(x,y)];
               assign south_in_flit[northof(x,y)]   = north_out_flit[nodenum(x,y)];
               assign south_out_ready[northof(x,y)] = north_in_ready[nodenum(x,y)];

               // South port connection
               assign south_in_valid[nodenum(x,y)]  = north_out_valid[southof(x,y)];
               assign south_in_flit[nodenum(x,y)]   = north_out_flit[southof(x,y)];
               assign south_out_ready[nodenum(x,y)] = north_in_ready[southof(x,y)];
               assign north_out_ready[southof(x,y)] = south_in_ready[nodenum(x,y)];
               assign north_in_valid[southof(x,y)]  = south_out_valid[nodenum(x,y)];
               assign north_in_flit[southof(x,y)]   = south_out_flit[nodenum(x,y)];
            end

            if (y==0) begin
               // North port connection
               assign north_in_valid[nodenum(x,y)]  = south_out_valid[northof(x,y)];
               assign north_in_flit[nodenum(x,y)]   = south_out_flit[northof(x,y)];
               assign north_out_ready[nodenum(x,y)] = south_in_ready[northof(x,y)];
               assign south_in_valid[northof(x,y)]  = north_out_valid[nodenum(x,y)];
               assign south_in_flit[northof(x,y)]   = north_out_flit[nodenum(x,y)];
               assign south_out_ready[northof(x,y)] = north_in_ready[nodenum(x,y)];

               // South border of the mesh
               assign south_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
               assign south_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            if (y==ydim-1) begin
               // South port connection
               assign south_in_valid[nodenum(x,y)]  = north_out_valid[southof(x,y)];
               assign south_in_flit[nodenum(x,y)]   = north_out_flit[southof(x,y)];
               assign south_out_ready[nodenum(x,y)] = north_in_ready[southof(x,y)];
               assign north_out_ready[southof(x,y)] = south_in_ready[nodenum(x,y)];
               assign north_in_valid[southof(x,y)]  = south_out_valid[nodenum(x,y)];
               assign north_in_flit[southof(x,y)]   = south_out_flit[nodenum(x,y)];

               // North border of the mesh
               assign north_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
               assign north_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            // Generate connection in east-west direction
            if (x>0 && x<xdim-1) begin
               // West port connection
               assign west_in_valid[nodenum(x,y)]  = east_out_valid[westof(x,y)];
               assign west_in_flit[nodenum(x,y)]   = east_out_flit[westof(x,y)];
               assign west_out_ready[nodenum(x,y)] = east_in_ready[westof(x,y)];
               assign east_out_ready[westof(x,y)]  = west_in_ready[nodenum(x,y)];
               assign east_in_valid[westof(x,y)]   = west_out_valid[nodenum(x,y)];
               assign east_in_flit[westof(x,y)]    = west_out_flit[nodenum(x,y)];

               // East port connection
               assign east_in_valid[nodenum(x,y)]  = west_out_valid[eastof(x,y)];
               assign east_in_flit[nodenum(x,y)]   = west_out_flit[eastof(x,y)];
               assign west_out_ready[eastof(x,y)]  = east_in_ready[nodenum(x,y)];
               assign west_in_valid[eastof(x,y)]   = east_out_valid[nodenum(x,y)];
               assign west_in_flit[eastof(x,y)]    = east_out_flit[nodenum(x,y)];
               assign east_out_ready[nodenum(x,y)] = west_in_ready[eastof(x,y)];
            end

            if (x==0) begin
              // East port connection
              assign east_in_valid[nodenum(x,y)]  = west_out_valid[eastof(x,y)];
              assign east_in_flit[nodenum(x,y)]   = west_out_flit[eastof(x,y)];
              assign west_out_ready[eastof(x,y)]  = east_in_ready[nodenum(x,y)];
              assign west_in_valid[eastof(x,y)]   = east_out_valid[nodenum(x,y)];
              assign west_in_flit[eastof(x,y)]    = east_out_flit[nodenum(x,y)];
              assign east_out_ready[nodenum(x,y)] = west_in_ready[eastof(x,y)];

              // West border of the mesh
              assign west_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
              assign west_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            if (x==xdim-1) begin
              // West port connection
              assign west_in_valid[nodenum(x,y)]  = east_out_valid[westof(x,y)];
              assign west_in_flit[nodenum(x,y)]   = east_out_flit[westof(x,y)];
              assign west_out_ready[nodenum(x,y)] = east_in_ready[westof(x,y)];
              assign east_out_ready[westof(x,y)]  = west_in_ready[nodenum(x,y)];
              assign east_in_valid[westof(x,y)]   = west_out_valid[nodenum(x,y)];
              assign east_in_flit[westof(x,y)]    = west_out_flit[nodenum(x,y)];

              // East border of the mesh
              assign east_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
              assign east_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end
         end
      end

   endgenerate

   function integer nodenum(input integer x,input integer y);
      nodenum = x+y*xdim;
   endfunction // nodenum

   function integer northof(input integer x,input integer y);
      northof = x+(y+1)*xdim;
   endfunction // northof
   function integer eastof(input integer x,input integer y);
      eastof  = (x+1)+y*xdim;
   endfunction // eastof
   function integer southof(input integer x,input integer y);
      southof = x+(y-1)*xdim;
   endfunction // southof
   function integer westof(input integer x,input integer y);
      westof = (x-1)+y*xdim;
   endfunction // westof

   // This generates the lookup table for each single node
   function [nodes*5-1:0] genlookup(input integer x,input integer y);
      integer yd,xd;
      integer nd;
      reg [4:0] d;

      genlookup = {nodes{5'b00000}};

      for (yd=0;yd<xdim;yd=yd+1) begin
         for (xd=0;xd<xdim;xd=xd+1) begin : inner_loop


            nd = nodenum(xd,yd);
            d = 5'b00000;
            if ((xd==x) && (yd==y)) begin
               d = `LOCAL;
            end else if (xd==x) begin
               if (yd<y) begin
                  d = `SOUTH;
               end else begin
                  d = `NORTH;
               end
            end else begin
               if (xd<x) begin
                  d = `WEST;
               end else begin
                  d = `EAST;
               end
            end // else: !if(xd==x)
            genlookup = genlookup | (d<<((nodes-nd-1)*5));
         end
      end
   endfunction

endmodule // mesh
