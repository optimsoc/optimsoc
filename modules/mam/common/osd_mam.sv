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

module osd_mam
  #(parameter DATA_WIDTH  = 16, // in bits, must be multiple of 16
    parameter ADDR_WIDTH  = 32,
    parameter MAX_PKT_LEN = 'x,
    parameter REGIONS     = 1,
    parameter MEM_SIZE0   = 'x,
    parameter BASE_ADDR0  = 'x,
    parameter MEM_SIZE1   = 'x,
    parameter BASE_ADDR1  = 'x,
    parameter MEM_SIZE2   = 'x,
    parameter BASE_ADDR2  = 'x,
    parameter MEM_SIZE3   = 'x,
    parameter BASE_ADDR3  = 'x,
    parameter MEM_SIZE4   = 'x,
    parameter BASE_ADDR4  = 'x,
    parameter MEM_SIZE5   = 'x,
    parameter BASE_ADDR5  = 'x,
    parameter MEM_SIZE6   = 'x,
    parameter BASE_ADDR6  = 'x,
    parameter MEM_SIZE7   = 'x,
    parameter BASE_ADDR7  = 'x,
    parameter ENDIAN      = 1   // 0 little-endian, 1 big-endian
    )
   (
    input                         clk, rst,

    input                         dii_flit debug_in, output debug_in_ready,
    output                        dii_flit debug_out, input debug_out_ready,

    input [9:0]                   id,

    output reg                    req_valid, // Start a new memory access request
    input                         req_ready, // Acknowledge the new memory access request
    output reg                    req_rw, // 0: Read, 1: Write
    output reg [ADDR_WIDTH-1:0]   req_addr, // Request base address
    output reg                    req_burst, // 0 for single beat access, 1 for incremental burst
    output reg [13:0]             req_beats, // Burst length in number of words

    output reg                    write_valid, // Next write data is valid
    output reg [DATA_WIDTH-1:0]   write_data, // Write data
    output reg [DATA_WIDTH/8-1:0] write_strb, // Byte strobe if req_burst==0
    input                         write_ready, // Acknowledge this data item

    input                         read_valid, // Next read data is valid
    input [DATA_WIDTH-1:0]        read_data, // Read data
    output reg                    read_ready // Acknowledge this data item
   );

   initial begin
      assert(DATA_WIDTH[2:0] == 0)
        else $fatal(1, "datawidth of MAM read/write port must be times of bytes!");
   end

   function logic [DATA_WIDTH-1:0] endian_conv(input logic [DATA_WIDTH-1:0] din);
      int i;
      int total = DATA_WIDTH/8;
      for(i=0; i<total; i++)
        endian_conv[i*8 +: 8] = din[(total-i-1)*8 +: 8];
   endfunction // endian_conv

   logic [DATA_WIDTH-1:0] read_data_m, write_data_m;
   assign read_data_m = ENDIAN ? read_data    : endian_conv(read_data);
   assign write_data  = ENDIAN ? write_data_m : endian_conv(write_data_m);

   logic        reg_request;
   logic        reg_write;
   logic [15:0] reg_addr;
   logic [1:0]  reg_size;
   logic [15:0] reg_wdata;
   logic        reg_ack;
   logic        reg_err;
   logic [15:0] reg_rdata;

   logic        stall;

   dii_flit dp_out, dp_in;
   logic        dp_out_ready, dp_in_ready;

   osd_regaccess_layer
     #(.MODID(16'h3), .MODVERSION(16'h0),
       .MAX_REG_SIZE(16), .CAN_STALL(0))
   u_regaccess(.*,
               .module_in (dp_out),
               .module_in_ready (dp_out_ready),
               .module_out (dp_in),
               .module_out_ready (dp_in_ready));

   assign reg_ack = 1;

   logic [63:0] base_addr [8];
   assign base_addr[0] = 64'(BASE_ADDR0);
   assign base_addr[1] = 64'(BASE_ADDR1);
   assign base_addr[2] = 64'(BASE_ADDR2);
   assign base_addr[3] = 64'(BASE_ADDR3);
   assign base_addr[4] = 64'(BASE_ADDR4);
   assign base_addr[5] = 64'(BASE_ADDR5);
   assign base_addr[6] = 64'(BASE_ADDR6);
   assign base_addr[7] = 64'(BASE_ADDR7);
   logic [63:0] mem_size [8];
   assign mem_size[0] = 64'(MEM_SIZE0);
   assign mem_size[1] = 64'(MEM_SIZE1);
   assign mem_size[2] = 64'(MEM_SIZE2);
   assign mem_size[3] = 64'(MEM_SIZE3);
   assign mem_size[4] = 64'(MEM_SIZE4);
   assign mem_size[5] = 64'(MEM_SIZE5);
   assign mem_size[6] = 64'(MEM_SIZE6);
   assign mem_size[7] = 64'(MEM_SIZE7);

   always_comb begin
      reg_err = 0;
      reg_rdata = 16'hx;

      if (reg_addr[15:7] == 9'h4) // 0x200
        case (reg_addr)
          16'h200: reg_rdata = 16'(DATA_WIDTH);
          16'h201: reg_rdata = 16'(ADDR_WIDTH);
          16'h202: reg_rdata = 16'(REGIONS);
          default: reg_err = 1;
        endcase
      else if (reg_addr[15:7] == 9'h5) // 0x280-0x300
        if (reg_addr[3])
          reg_err = 1;
        else if (reg_addr[6:4] > REGIONS)
          reg_err = 1;
        else if (reg_addr[2] == 0) // addr
          case (reg_addr[1:0])
            0: reg_rdata = base_addr[reg_addr[6:4]][15:0];
            1: reg_rdata = base_addr[reg_addr[6:4]][31:16];
            2: reg_rdata = base_addr[reg_addr[6:4]][47:32];
            3: reg_rdata = base_addr[reg_addr[6:4]][63:48];
          endcase // case (reg_addr[1:0])
        else
          case (reg_addr[1:0])
            0: reg_rdata = mem_size[reg_addr[6:4]][15:0];
            1: reg_rdata = mem_size[reg_addr[6:4]][31:16];
            2: reg_rdata = mem_size[reg_addr[6:4]][47:32];
            3: reg_rdata = mem_size[reg_addr[6:4]][63:48];
          endcase // case (reg_addr[1:0])
   end

   enum {
         STATE_INACTIVE, STATE_CMD_SKIP, STATE_CMD, STATE_ADDR,
         STATE_REQUEST, STATE_WRITE_PACKET, STATE_WRITE, STATE_WRITE_WAIT,
         STATE_READ_PACKET, STATE_READ, STATE_READ_WAIT, STATE_WRITE_SINGLE,
         STATE_WRITE_SINGLE_WAIT
         } state, nxt_state;

   // The counter is used to count flits
   reg [$clog2(MAX_PKT_LEN)-1:0] counter;
   logic [$clog2(MAX_PKT_LEN)-1:0] nxt_counter;

   // This counter is used to count words (that can span packets)
   localparam WCOUNTER_WIDTH = (DATA_WIDTH == 16) ? 1 : $clog2(DATA_WIDTH);
   reg [WCOUNTER_WIDTH-1:0] wcounter;
   logic [WCOUNTER_WIDTH-1:0] nxt_wcounter;

   // Stores whether we are inside a packet
   reg                               in_packet;
   logic                             nxt_in_packet;

   // Stores whether the last address flit is the last flit in a packet
   // Decides whether to go to STATE_WRITE or STATE_WRITE_PACKET
   reg                               is_last_flit;
   logic                             nxt_is_last_flit;

   // Combinational part of interface
   logic [13:0]                      nxt_req_beats;
   logic                             nxt_req_rw;
   logic                             nxt_req_burst;
   logic [ADDR_WIDTH-1:0]            nxt_req_addr;
   logic [DATA_WIDTH/8-1:0]          nxt_write_strb;

   reg   [DATA_WIDTH-1:0]            write_data_reg;
   logic [DATA_WIDTH-1:0]            nxt_write_data_reg;

   // This is the number of (16 bit) words needed to form an address
   localparam ADDR_WORDS = ADDR_WIDTH >> 4;

   always_ff @(posedge clk) begin
      if (rst) begin
         state <= STATE_INACTIVE;
      end else begin
         state <= nxt_state;
      end

      req_beats <= nxt_req_beats;
      req_rw <= nxt_req_rw;
      req_burst <= nxt_req_burst;
      req_addr <= nxt_req_addr;
      counter <= nxt_counter;
      write_data_reg <= nxt_write_data_reg;
      wcounter <= nxt_wcounter;
      in_packet <= nxt_in_packet;
      is_last_flit <= nxt_is_last_flit;
      write_strb <= nxt_write_strb;
   end

   integer i;
   always_comb begin
      nxt_state = state;
      nxt_counter = counter;
      nxt_req_beats = req_beats;
      nxt_write_data_reg = write_data_reg;
      nxt_wcounter = wcounter;
      nxt_in_packet = in_packet;
      nxt_is_last_flit = is_last_flit;
      nxt_write_strb = write_strb;
      nxt_req_rw = req_rw;
      nxt_req_burst = req_burst;

      nxt_req_addr = req_addr;

      dp_in_ready = 0;
      dp_out.valid = 0;
      dp_out.data = 16'hx;
      dp_out.last = 0;
      req_valid = 0;
      write_valid = 0;
      write_data_m = write_data_reg;
      read_ready = 0;

      case (state)
         STATE_INACTIVE: begin
            dp_in_ready = 1;
            if (dp_in.valid) begin
               nxt_state = STATE_CMD_SKIP;
            end
         end
        STATE_CMD_SKIP: begin
           dp_in_ready = 1;
           if (dp_in.valid) begin
              nxt_state = STATE_CMD;
           end
        end
        STATE_CMD: begin
           dp_in_ready = 1;
           nxt_write_strb = dp_in.data[DATA_WIDTH/8-1:0];
           nxt_req_rw = dp_in.data[15];
           nxt_req_burst = dp_in.data[14];

           if (nxt_req_burst)
             nxt_req_beats = dp_in.data[13:0];
           else
             nxt_req_beats = 1;

           if (dp_in.valid) begin
              nxt_state = STATE_ADDR;
              nxt_counter = 0;
           end
        end
        STATE_ADDR: begin
           dp_in_ready = 1;
           nxt_req_addr[(counter+1)*16-1 -: 16] = dp_in.data;
           if (dp_in.valid) begin
              nxt_counter = counter + 1;
              if (counter == ADDR_WORDS - 1) begin
                 nxt_is_last_flit = dp_in.last;
                 nxt_state = STATE_REQUEST;
              end
           end
        end
        STATE_REQUEST: begin
           req_valid = 1;
           if (req_ready) begin
              nxt_is_last_flit = 0;
              if (req_rw) begin
                 if (req_burst) begin
                    if (is_last_flit) begin
                       nxt_state = STATE_WRITE_PACKET;
                    end else begin
                       nxt_state = STATE_WRITE;
                    end
                 end else begin
                    nxt_state = STATE_WRITE_SINGLE;
                 end
              end else begin
                 nxt_state = STATE_READ_PACKET;
              end
              nxt_wcounter = 0;
              nxt_counter = 0;
              nxt_in_packet = 0;
           end
        end
        STATE_WRITE_PACKET: begin
           dp_in_ready = 1;
           if (dp_in.valid) begin
              nxt_counter = counter + 1;
              if (counter == 1) begin
                 nxt_state = STATE_WRITE;
              end
           end
        end
        STATE_WRITE: begin
           nxt_write_data_reg[(DATA_WIDTH/16-wcounter)*16-1 -: 16] = dp_in.data;
           write_data_m[(DATA_WIDTH/16-wcounter)*16-1 -: 16] = dp_in.data;
           dp_in_ready = 1;
           if (dp_in.valid) begin
              nxt_wcounter = wcounter + 1;
              if (wcounter == DATA_WIDTH/16 - 1) begin
                 write_valid = 1;
                 if (!write_ready) begin
                    nxt_state = STATE_WRITE_WAIT;
                    nxt_in_packet = !dp_in.last;
                 end else begin
                    nxt_req_beats = req_beats - 1;
                    if (req_beats == 1) begin
                       nxt_state = STATE_INACTIVE;
                    end else if (dp_in.last) begin
                       nxt_counter = 0;
                       nxt_state = STATE_WRITE_PACKET;
                    end
                 end
                 nxt_wcounter = 0;
              end else begin
                 if (dp_in.last) begin
                    nxt_counter = 0;
                    nxt_state = STATE_WRITE_PACKET;
                 end
              end
           end
        end // case: STATE_WRITE
        STATE_WRITE_WAIT: begin
           write_valid = 1;
           if (write_ready) begin
              nxt_req_beats = req_beats - 1;
              if (req_beats == 1) begin
                 nxt_state = STATE_INACTIVE;
              end else begin
                 if (in_packet) begin
                    nxt_state = STATE_WRITE;
                 end else begin
                    nxt_counter = 0;
                    nxt_state = STATE_WRITE_PACKET;
                 end
              end
           end
        end // case: STATE_WRITE_WAIT
        STATE_WRITE_SINGLE: begin
           nxt_write_data_reg[(DATA_WIDTH/16-wcounter)*16-1 -: 16] = dp_in.data;
           write_data_m[(DATA_WIDTH/16-wcounter)*16-1 -: 16] = dp_in.data;
           dp_in_ready = 1;
           if (dp_in.valid) begin
              nxt_wcounter = wcounter + 1;
              if (wcounter == DATA_WIDTH/16 - 1) begin
                 write_valid = 1;
                 if (!write_ready) begin
                    nxt_state = STATE_WRITE_SINGLE_WAIT;
                 end else begin
                    nxt_state = STATE_INACTIVE;
                 end
              end
           end
        end
        STATE_WRITE_SINGLE_WAIT: begin
           write_valid = 1;
           if (write_ready) begin
              nxt_state = STATE_INACTIVE;
           end
        end
        STATE_READ_PACKET: begin
           dp_out.valid = 1;
           if (counter == 0) begin
              dp_out.data = 16'h0;
           end else begin
              dp_out.data = { 2'b01, 4'b1111, id };
           end
           if (dp_out_ready) begin
              nxt_counter = counter + 1;
              if (counter == 1) begin
                 nxt_state = STATE_READ;
              end
           end
        end // case: STATE_READ_PACKET
        STATE_READ: begin
           if (read_valid) begin
              dp_out.valid = 1;
              dp_out.last = (counter == MAX_PKT_LEN-1) ||
                            ((wcounter == DATA_WIDTH/16 - 1) && (req_beats == 1));
              dp_out.data = read_data_m[(DATA_WIDTH/16-wcounter)*16-1 -: 16];
              if (dp_out_ready) begin
                 nxt_wcounter = wcounter + 1;
                 if (wcounter == DATA_WIDTH/16-1) begin
                    nxt_req_beats = req_beats - 1;
                    nxt_wcounter = 0;
                    read_ready = 1;

                    if (req_beats == 1) begin
                       nxt_state = STATE_INACTIVE;
                    end else begin
                       if (counter == MAX_PKT_LEN - 1) begin
                          nxt_state = STATE_READ_PACKET;
                          nxt_counter = 0;
                       end else begin
                          nxt_counter = counter + 1;
                       end
                    end
                 end else begin
                    if (counter == MAX_PKT_LEN - 1) begin
                       nxt_state = STATE_READ_PACKET;
                       nxt_counter = 0;
                    end else begin
                       nxt_counter = counter + 1;
                    end
                 end
              end
           end
        end
      endcase
   end


endmodule // osd_dem_uart
