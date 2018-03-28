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
 * Simple (software-controlled) message passing module.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

/*
 *
 *                   +-> Input path <- packet buffer <-- Ingress
 *                   |    * raise interrupt (!empty)
 * Bus interface --> +    * read size flits from packet buffer
 *                   |
 *                   +-> Output path -> packet buffer --> Egress
 *                        * set size
 *                        * write flits to packet buffer
 *
 * Ingress <---+----- NoC
 *             |
 *       Handle control message
 *             |
 *  Egress ----+----> NoC
 */

module mpbuffer_endpoint
  import optimsoc_config::*;
   #(
    parameter config_t CONFIG = 'x,
    parameter SIZE            = 16
    )
   (
    input                                  clk,
    input                                  rst,

    output reg [CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit,
    output reg                             noc_out_last,
    output reg                             noc_out_valid,
    input                                  noc_out_ready,

    input [CONFIG.NOC_FLIT_WIDTH-1:0]      noc_in_flit,
    input                                  noc_in_last,
    input                                  noc_in_valid,
    output reg                             noc_in_ready,

    // Bus side (generic)
    input [31:0]                           bus_addr,
    input                                  bus_we,
    input                                  bus_en,
    input [31:0]                           bus_data_in,
    output reg [31:0]                      bus_data_out,
    output reg                             bus_ack,
    output reg                             bus_err,

    output                                 irq
    );

   import optimsoc_functions::*;

   localparam SIZE_WIDTH = clog2_width(SIZE);

   // Connect from the outgoing state machine to the packet buffer
   wire [CONFIG.NOC_FLIT_WIDTH-1:0] out_flit;
   reg                              out_last;
   reg                              out_valid;
   wire                             out_ready;

   wire [CONFIG.NOC_FLIT_WIDTH-1:0] in_flit;
   wire                             in_last;
   wire                             in_valid;
   reg                              in_ready;

   reg                              enabled;
   reg                              nxt_enabled;

   assign irq = in_valid;

   assign out_flit = bus_data_in;

   reg        if_fifo_in_en;
   reg        if_fifo_in_ack;
   reg [31:0] if_fifo_in_data;
   reg        if_fifo_out_en;
   reg        if_fifo_out_ack;

   /*
    * +------+---+------------------------+
    * | 0x0  | R | Read from Ingress FIFO |
    * +------+---+------------------------+
    * |      | W | Write to Egress FIFO   |
    * +------+---+------------------------+
    * | 0x4  | W | Enable interface       |
    * +------+---+------------------------+
    * |      | R | Status                 |
    * +------+---+------------------------+
    *
    */
   always @(*) begin
      bus_ack = 0;
      bus_err = 0;
      bus_data_out = 32'hx;
      nxt_enabled = enabled;

      if_fifo_in_en = 1'b0;
      if_fifo_out_en = 1'b0;

      if (bus_en) begin
         if (bus_addr[5:2] == 4'h0) begin
            if (!bus_we) begin
               if_fifo_in_en = 1'b1;
               bus_ack = if_fifo_in_ack;
               bus_data_out = if_fifo_in_data;
            end else begin
               if_fifo_out_en = 1'b1;
               bus_ack = if_fifo_out_ack;
            end
         end else if (bus_addr[5:2] == 4'h1) begin
            bus_ack = 1'b1;
            if (bus_we) begin
               nxt_enabled = 1'b1;
            end else begin
               bus_data_out = {30'h0, noc_out_valid, in_valid};
            end
         end else begin
            bus_err = 1'b1;
         end
      end // if (bus_en)
   end // always @ begin

   always @(posedge clk) begin
      if (rst) begin
         enabled <= 1'b0;
      end else begin
         enabled <= nxt_enabled;
      end
   end

   /**
    * Simple writes to 0x0
    *  * Start transfer and set size S
    *  * For S flits: Write flit
    */

   // State register
   reg [1:0]                  state_out;
   reg [1:0]                  nxt_state_out;

   reg                        state_in;
   reg                        nxt_state_in;

   // Size register that is also used to count down the remaining
   // flits to be send out
   reg [SIZE_WIDTH-1:0]       size_out;
   reg [SIZE_WIDTH-1:0]       nxt_size_out;

   wire [SIZE_WIDTH:0]      size_in;

   // States of output state machine
   localparam OUT_IDLE    = 0;
   localparam OUT_FIRST   = 1;
   localparam OUT_PAYLOAD = 2;

   // States of input state machine
   localparam IN_IDLE = 0;
   localparam IN_FLIT = 1;

   // Combinational part of input state machine
   always @(*) begin
      in_ready = 1'b0;
      if_fifo_in_ack = 1'b0;
      if_fifo_in_data = 32'hx;
      nxt_state_in = state_in;

      case(state_in)
        IN_IDLE: begin
           if (if_fifo_in_en) begin
              if (in_valid) begin
                 if_fifo_in_data = {{31-SIZE_WIDTH{1'b0}},size_in};
                 if_fifo_in_ack = 1'b1;
                 if (size_in!=0) begin
                    nxt_state_in = IN_FLIT;
                 end
              end else begin
                 if_fifo_in_data = 0;
                 if_fifo_in_ack = 1'b1;
                 nxt_state_in = IN_IDLE;
              end
           end else begin
              nxt_state_in = IN_IDLE;
           end
        end
        IN_FLIT: begin
           if (if_fifo_in_en) begin
              if_fifo_in_data = in_flit[31:0];
              in_ready = 1'b1;
              if_fifo_in_ack = 1'b1;
              if (size_in==1) begin
                 nxt_state_in = IN_IDLE;
              end else begin
                 nxt_state_in = IN_FLIT;
              end
           end else begin
              nxt_state_in = IN_FLIT;
           end
        end // case: IN_FLIT
        default: begin
           nxt_state_in = IN_IDLE;
        end
      endcase
   end

   // Combinational part of output state machine
   always @(*) begin
      // default values
      out_valid = 1'b0; // no flit
      nxt_size_out = size_out;  // keep size
      if_fifo_out_ack = 1'b0;   // don't acknowledge
      out_last = 1'bx; // Default is undefined

      case(state_out)
        OUT_IDLE: begin
           // Transition from IDLE to FIRST
           // when write on bus, which is the size
           if (if_fifo_out_en) begin
              // Store the written value as size
              nxt_size_out = bus_data_in[SIZE_WIDTH-1:0];
              // Acknowledge to the bus
              if_fifo_out_ack = 1'b1;
              nxt_state_out = OUT_FIRST;
           end else begin
              nxt_state_out = OUT_IDLE;
           end
        end
        OUT_FIRST: begin
           // The first flit is written from the bus now.
           // This can be either the only flit (size==1)
           // or a further flits will follow.
           // Forward the flits to the packet buffer.
           if (if_fifo_out_en) begin
              // When the bus writes, the data is statically assigned
              // to out_flit. Set out_valid to signal the flit should
              // be output
              out_valid = 1'b1;

              if (size_out==1) begin
                 out_last = 1'b1;
              end

              if (out_ready) begin
                 // When the output packet buffer is ready this cycle
                 // the flit has been stored in the packet buffer

                 // Decrement size
                 nxt_size_out = size_out-1;

                 // Acknowledge to the bus
                 if_fifo_out_ack = 1'b1;

                 if (size_out==1) begin
                    // When this was the only flit, go to IDLE again
                    nxt_state_out = OUT_IDLE;
                 end else begin
                    // Otherwise accept further flis as payload
                    nxt_state_out = OUT_PAYLOAD;
                 end
              end else begin // if (out_ready)
                 // If the packet buffer is not ready, we simply hold
                 // the data and valid and wait another cycle for the
                 // packet buffer to become ready
                 nxt_state_out = OUT_FIRST;
              end
           end else begin // if (bus_we && bus_en)
              // Wait for the bus
              nxt_state_out = OUT_FIRST;
           end
        end
        OUT_PAYLOAD: begin
           // After the first flit (HEADER) further flits are
           // forwarded in this state. The essential difference to the
           // FIRST state is in the output type which can here be
           // PAYLOAD or LAST
           if (bus_we && bus_en) begin
              // When the bus writes, the data is statically assigned
              // to out_flit. Set out_valid to signal the flit should
              // be output
              out_valid = 1'b1;

              if (size_out==1) begin
                 out_last = 1'b1;
              end

              if (out_ready) begin
                 // When the output packet buffer is ready this cycle
                 // the flit has been stored in the packet buffer

                 // Decrement size
                 nxt_size_out = size_out-1;

                 // Acknowledge to the bus
                 if_fifo_out_ack = 1'b1;

                 if (size_out==1) begin
                    // When this was the last flit, go to IDLE again
                    nxt_state_out = OUT_IDLE;
                 end else begin
                    // Otherwise accept further flis as payload
                    nxt_state_out = OUT_PAYLOAD;
                 end
              end else begin // if (out_ready)
                 // If the packet buffer is not ready, we simply hold
                 // the data and valid and wait another cycle for the
                 // packet buffer to become ready
                 nxt_state_out = OUT_PAYLOAD;
              end
           end else begin // if (bus_we && bus_en)
              // Wait for the bus
              nxt_state_out = OUT_PAYLOAD;
           end
        end
        default: begin
           // Defaulting to go to idle
           nxt_state_out = OUT_IDLE;
        end
      endcase
   end

   // Sequential part of both state machines
   always @(posedge clk) begin
      if (rst) begin
         state_out <= OUT_IDLE; // Start in idle state
         // size does not require a reset value (not used before set)
         state_in <= IN_IDLE;
      end else begin
         // Register combinational values
         state_out <= nxt_state_out;
         size_out <= nxt_size_out;
         state_in <= nxt_state_in;
      end
   end

   reg [CONFIG.NOC_FLIT_WIDTH-1:0] ingress_flit;
   reg                             ingress_last;
   reg                             ingress_valid;
   wire                            ingress_ready;

   wire [CONFIG.NOC_FLIT_WIDTH-1:0] egress_flit;
   wire                             egress_last;
   wire                             egress_valid;
   reg                              egress_ready;

   reg [CONFIG.NOC_FLIT_WIDTH-1:0]  control_flit;
   reg [CONFIG.NOC_FLIT_WIDTH-1:0]  nxt_control_flit;
   reg                              control_pending;
   reg                              nxt_control_pending;

   always @(*) begin
      noc_in_ready = !control_pending & ingress_ready;
      ingress_flit = noc_in_flit;
      nxt_control_pending = control_pending;
      nxt_control_flit = control_flit;

      // Ingress part
      ingress_valid = noc_in_valid;
      ingress_last = noc_in_last;
      if ((noc_in_valid & !control_pending) &&
          (noc_in_flit[26:24] == 3'b111) &&
          !noc_in_flit[0]) begin
         nxt_control_pending = 1'b1;
         nxt_control_flit[31:27] = noc_in_flit[23:19];
         nxt_control_flit[26:24] = 3'b111;
         nxt_control_flit[23:19] = noc_in_flit[31:27];
         nxt_control_flit[18:2] = noc_in_flit[18:2];
         nxt_control_flit[1] = enabled;
         nxt_control_flit[0] = 1'b1;
         ingress_valid = 1'b0;
         ingress_last = 1'b1;
      end

      // Egress part
      if (egress_valid & !egress_last) begin
         egress_ready = noc_out_ready;
         noc_out_valid = egress_valid;
         noc_out_flit = egress_flit;
         noc_out_last = egress_last;
      end else if (control_pending) begin
         egress_ready = 1'b0;
         noc_out_valid = 1'b1;
         noc_out_flit = control_flit;
         noc_out_last = 1'b1;
         if (noc_out_ready) begin
            nxt_control_pending = 1'b0;
         end
      end else begin
         egress_ready = noc_out_ready;
         noc_out_valid = egress_valid;
         noc_out_last = egress_last;
         noc_out_flit = egress_flit;
      end
   end // always @ begin

   always @(posedge clk) begin
      if (rst) begin
         control_pending <= 1'b0;
         control_flit <= {CONFIG.NOC_FLIT_WIDTH{1'hx}};
      end else begin
         control_pending <= nxt_control_pending;
         control_flit <= nxt_control_flit;
      end
   end

   // The output packet buffer
   noc_buffer
     #(.DEPTH(SIZE), .FLIT_WIDTH(CONFIG.NOC_FLIT_WIDTH), .FULLPACKET(1))
   u_packetbuffer_out(.*,
                      .in_ready         (out_ready),
                      .in_flit          (out_flit),
                      .in_last          (out_last),
                      .in_valid         (out_valid),
                      .packet_size      (),
                      .out_flit         (egress_flit),
                      .out_last         (egress_last),
                      .out_valid        (egress_valid),
                      .out_ready        (egress_ready));

   // The input packet buffer
   noc_buffer
     #(.DEPTH(SIZE), .FLIT_WIDTH(CONFIG.NOC_FLIT_WIDTH), .FULLPACKET(1))
   u_packetbuffer_in(.*,
                     .in_ready          (ingress_ready),
                     .in_flit           (ingress_flit),
                     .in_last           (ingress_last),
                     .in_valid          (ingress_valid),
                     .packet_size       (size_in),
                     .out_flit          (in_flit),
                     .out_last          (in_last),
                     .out_valid         (in_valid),
                     .out_ready         (in_ready));

endmodule // mpbuffer_endpoint
