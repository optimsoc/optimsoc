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
 * This is the prio decoder.
 *
 * Author(s):
 *   Manuel Krause <krause.maxen@freenet.de>
 *
 */

`include "lisnoc_def.vh"
module lisnoc_router_arbiter_prio(
                                  // Inputs
                                  clk, rst, flit_i, request_i, ready_i,
                                  // Outputs
                                  flit_o,read_o,valid_o);

   /////////////////////////////////////////////////////////////////////////
   //                             DECLARATIONS                            //
   /////////////////////////////////////////////////////////////////////////
   // Parameter definitions
   parameter  flit_data_width = 32;
   parameter  flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;

   parameter  ph_prio_width = 4;
   parameter  ph_prio_offset = 5;

   parameter  vchannels = 1;
   parameter  ports = 5;
   localparam port_width = $clog2(ports);

   // Clock and Reset declaration
   input clk;
   input rst;

   // I/O declaration
   input ready_i;                                      // fifo ready status
   input [flit_width*ports-1:0] flit_i;                                       // flit data from all input ports
   input [ports-1:0]            request_i;                                    // all incomming request signals from all input arbiters
   output [flit_width-1:0]      flit_o;                                       // output flit
   output reg [ports-1:0]       read_o;                                       // input arbiter select signal
   output reg                   valid_o;                                      // fifo read enable

   // Register declaration
   reg [ph_prio_width-1:0]      flit_prio_field_mem [ports-1:0];              // prio field memory
   reg [ports-1:0]              mem_status;                                   // cell status of the prio field memory
   reg [ports-1:0]              highest_prio_field  [ph_prio_width-1:0];      // rearranged prio.
   reg [ports-1:0]              prio_req_masked;                              // current highest prio jobs or empty
   reg [ph_prio_width-1:0]      nxt_flit_prio_field_mem [ports-1:0];          // look ahead flit prio field
   reg [ports-1:0]              nxt_mem_status;                               // look ahead cell status of prio field memory
   reg [port_width-1:0]         select_port_num;                              // current selected port in unsigned representation
   reg [ports-1:0]              old_port;                                     // last active port in one hot representation
   reg [ports-1:0]              req_buffer;                                   // buffer the request in case of a non-single flit
   reg [ports-1:0]              req_ports;                                    // forward req_buffer content or prio_req_masked

   // Wire declaration
   wire [flit_width-1:0]        flit_array [0:ports-1];                       // flit data in array representation
   wire [ph_prio_width-1:0]     flit_prio_field [ports-1:0];                  // prio field from each filt
   wire [ph_prio_width-1:0]     highest_prio;                                 // highest prio.
   wire [flit_type_width-1:0]   flit_type;                                    // flit type field of current used port
   wire [ports-1:0]             active_port;                                  // current selected port in one hot representation
   wire [ports-1:0]             request;                                      // rename of input request_i
   wire                         ready;                                        // rename of input ready_i

   // Signal assignments for better readability
   assign request = request_i;
   assign ready   = ready_i;

   genvar                       p;
   generate
      for (p=0;p<ports;p=p+1) begin: Nam0
         assign flit_array[p]      = flit_i[flit_width*(p+1)-1:flit_width*p];
         assign flit_prio_field[p] = flit_i[(flit_data_width-ph_prio_offset-1)+flit_width*p:(flit_data_width-ph_prio_offset-ph_prio_width)+flit_width*p];
      end
   endgenerate
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
   //                       FLIT PRIO DECODING LOGIC                      //
   /////////////////////////////////////////////////////////////////////////
   // Rearrange all prioritization bits => switch from n x 4 bit wires to 4 x n bit wires
   always @(*) begin: decoding1
      integer index;
      integer p;
      for (index=0;index<ph_prio_width;index=index+1) begin: Nam1
         for (p=0;p<ports;p=p+1) begin: Nam2
            // If something is stored then we use this
            if (mem_status[p]==1'b1) begin
               if (flit_prio_field_mem[p][ph_prio_width-1]== 1'b1) begin
                  highest_prio_field[index][p] = flit_prio_field_mem[p][index];
               end else begin
                  highest_prio_field[index][p] = 1'b0;
               end

               // otherwise we use the current waiting one which has a request to our output port
            end else begin
               if (flit_prio_field[p][ph_prio_width-1]== 1'b1 && request[p] == 1'b1) begin
                  highest_prio_field[index][p] = flit_prio_field[p][index];
               end else begin
                  highest_prio_field[index][p] = 1'b0;
               end
            end
         end
      end
   end

   // Reduces highest_prioritization_field to highest_prioritization => 4 n bit wires to 4 1 bit wires
   genvar index;
   generate
      for (index=0;index<ph_prio_width;index=index+1) begin: Nam3
         assign highest_prio[index] = | highest_prio_field[index];
      end
   endgenerate

   // Build new request mask from ports with highest prioritization and active prio-bit
   always @(*) begin: decoding2
      integer p;
      for (p=0;p<ports;p=p+1) begin: Nam4
         if (mem_status[p] == 1'b1) begin
            prio_req_masked[p] = (&(highest_prio ~^ flit_prio_field_mem[p])) && request[p];
         end else begin
            // If prio bit false compare with 4'b0000
            if (flit_prio_field[p][ph_prio_width-1] == 1'b0) begin
               prio_req_masked[p] = (&(highest_prio ~^ {ph_prio_width{1'b0}})) && request[p];
               //...otherwise compare with field
            end else begin
               prio_req_masked[p] = (&(highest_prio ~^ flit_prio_field[p]))    && request[p];
            end
         end
      end
   end
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
   //                         LOOK AHEAD LOGIC                            //
   /////////////////////////////////////////////////////////////////////////
   always @(*) begin : store_and_increment
      integer i;
      for (i=0;i<ports;i=i+1) begin

         if (request[i] == 1'b1) begin
            // Check if this field corresponds to the current port
            if (active_port[i] == 1'b1) begin
               nxt_mem_status[i]          = 1'b0;
               nxt_flit_prio_field_mem[i] = {ph_prio_width{1'b0}};

               //...or to a waiting one.
            end else begin
               // If the request is not new then
               if (mem_status[i] == 1'b1) begin

                  nxt_mem_status[i] = 1'b1;
                  // Check if no prio bit is set
                  if (flit_prio_field_mem[i][ph_prio_width-1] == 1'b0) begin
                     nxt_flit_prio_field_mem[i] = {1'b1,{ph_prio_width-1{1'b0}}};
                     //...otherwise manipulate prio.
                  end else begin
                     nxt_flit_prio_field_mem[i] = {1'b1,flit_prio_field_mem[i][ph_prio_width-1:1]};
                  end

                  //...otherwise its a new request.
               end else begin
                  nxt_mem_status[i] = 1'b1;
                  // If no prio then store the lowest one
                  if (flit_prio_field[i][ph_prio_width-1] == 1'b0) begin
                     nxt_flit_prio_field_mem[i] = {ph_prio_width{1'b0}};
                     //...otherwise manipulate prio.
                  end else begin
                     nxt_flit_prio_field_mem[i] = flit_prio_field[i][ph_prio_width-1:0];
                  end

               end
            end

            //...otherwise store zero.
         end else begin
            nxt_mem_status[i]          = 1'b0;
            nxt_flit_prio_field_mem[i] = {ph_prio_width{1'b0}};
         end
      end
   end
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
   //                        ROUND ROBIN ARBITER                          //
   /////////////////////////////////////////////////////////////////////////
   lisnoc_arb_prio_rr #(.N(ports))
   u_arb(  .nxt_gnt(active_port),           // active port
           .req    (req_ports),             // requested ports
           .gnt    (old_port)               // last port
           );
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
     //                           REQUEST MUX                               //
     /////////////////////////////////////////////////////////////////////////
     always @(posedge clk) begin: regBuffer
        if (rst == 1'b1) begin
           req_buffer = {ports{1'b0}};
        end else begin
           if ((flit_type == `FLIT_TYPE_HEADER) && (ready == 1'b1) && (request[select_port_num] == 1'b1)) begin
              req_buffer = prio_req_masked;
           end else if ((flit_type == `FLIT_TYPE_SINGLE || flit_type == `FLIT_TYPE_LAST) && (ready == 1'b1) && (request[select_port_num] == 1'b1)) begin
              req_buffer = {ports{1'b0}};
           end else begin
              req_buffer = req_buffer;
           end
        end
     end

   always @(*) begin: req_mux
      if (|req_buffer == 1'b1) begin
         req_ports = req_buffer;
      end else begin
         req_ports = prio_req_masked;
      end
   end
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
   //                 ONE-HOT VECTOR TO UNSIGNED CONVERTION               //
   /////////////////////////////////////////////////////////////////////////
     always @(*) begin : convertonehot
        integer i;
        select_port_num = 0;
        for (i=0;i<ports;i=i+1) begin
           if (active_port[i] == 1'b1) begin
              select_port_num = i;
           end
        end
     end
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
     //                             OUTPUT LOGIC                            //
     /////////////////////////////////////////////////////////////////////////
     // Setup handshake signals
   always @(*) begin: output_logic1
      integer i;
      if (|active_port == 1'b1 && ready == 1'b1) begin
         for (i=0;i<ports;i=i+1) begin
            if (i == select_port_num) begin
               read_o[i] = 1'b1;
            end else begin
               read_o[i] = 1'b0;
            end
         end
         valid_o = 1'b1 && request[select_port_num];
      end else begin
         read_o  = {ports{1'b0}};
         valid_o = 1'b0;
      end
   end

   // output flit data
   assign flit_o = flit_array[select_port_num];
   /////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////
   //                    RESET & PORT TRANSITION LOGIC                    //
   /////////////////////////////////////////////////////////////////////////
   assign flit_type = flit_array[select_port_num][flit_width-1:flit_width-flit_type_width];

   always @(posedge clk) begin : clk_logic
      integer i;
      if (rst) begin
         old_port    <= {ports{1'b0}};
         mem_status  <= {ports{1'b0}};
         for (i=0;i<ports;i=i+1) begin
            flit_prio_field_mem[i] <= {ph_prio_width{1'b0}};
         end

      end else begin
         if ((flit_type == `FLIT_TYPE_SINGLE || flit_type == `FLIT_TYPE_LAST) && (ready == 1'b1) && (request[select_port_num] == 1'b1)) begin
            for (i=0;i<ports;i=i+1) begin
               flit_prio_field_mem[i] <= nxt_flit_prio_field_mem[i];
            end
            old_port   <= active_port;
            mem_status <= nxt_mem_status;
         end
      end
   end
   /////////////////////////////////////////////////////////////////////////
endmodule // lisnoc_router_arbiter_prio
`include "lisnoc_undef.vh"
