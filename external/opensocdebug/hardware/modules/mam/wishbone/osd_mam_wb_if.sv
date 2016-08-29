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
//    Nico Gutmann <nicolai.gutmann@gmail.com>

module osd_mam_wb_if
  #(parameter DATA_WIDTH  = 16, // in bits, must be multiple of 16
    parameter ADDR_WIDTH  = 32)
   (
    input                       clk_i, rst_i,

    input                       req_valid, // Start a new memory access request
    output reg                  req_ready, // Acknowledge the new memory access request
    input                       req_rw, // 0: Read, 1: Write
    input [ADDR_WIDTH-1:0]      req_addr, // Request base address
    input                       req_burst, // 0 for single beat access, 1 for incremental burst
    input [13:0]                req_beats, // Burst length in number of words

    input                       write_valid, // Next write data is valid
    input [DATA_WIDTH-1:0]      write_data, // Write data
    input [DATA_WIDTH/8-1:0]    write_strb, // Byte strobe if req_burst==0
    output reg                  write_ready, // Acknowledge this data item

    output reg                  read_valid, // Next read data is valid
    output reg [DATA_WIDTH-1:0] read_data, // Read data
    input                       read_ready, // Acknowledge this data item

    output reg                  stb_o,
    output                      cyc_o,
    input                       ack_i,
    output reg                  we_o,
    output reg [ADDR_WIDTH-1:0] addr_o,
    output reg [DATA_WIDTH-1:0] dat_o,
    input [DATA_WIDTH-1:0]      dat_i,
    output reg [2:0]            cti_o,
    output reg [1:0]            bte_o,
    output reg [SW-1:0]         sel_o
    );

    //Byte select width
   localparam SW = (DATA_WIDTH == 32) ? 4 :
                   (DATA_WIDTH == 16) ? 2 :
                   (DATA_WIDTH ==  8) ? 1 : 'hx;

   enum { STATE_IDLE, STATE_WRITE_LAST, STATE_WRITE_LAST_WAIT,
          STATE_WRITE, STATE_WRITE_WAIT, STATE_READ,
          STATE_READ_WAIT } state, nxt_state;

   logic nxt_we_o;
   logic [2:0] nxt_cti_o;
   logic [1:0] nxt_bte_o;

   reg [DATA_WIDTH-1:0] read_data_reg;
   logic [DATA_WIDTH-1:0] nxt_read_data_reg;

   reg [DATA_WIDTH-1:0]   dat_o_reg;
   logic [DATA_WIDTH-1:0] nxt_dat_o_reg;

   logic [ADDR_WIDTH-1:0] nxt_addr_o;

   reg [13:0]             beats;
   logic [13:0]           nxt_beats;

   //registers
   always_ff @(posedge clk_i) begin
      if (rst_i) begin
         state <= STATE_IDLE;
      end else begin
         state <= nxt_state;
      end

      we_o <= nxt_we_o;
      cti_o <= nxt_cti_o;
      bte_o <= nxt_bte_o;
      read_data_reg <= nxt_read_data_reg;
      dat_o_reg <= nxt_dat_o_reg;
      addr_o <= nxt_addr_o;
      beats <= nxt_beats;
   end

   assign cyc_o = stb_o;

   //state & output logic
   always_comb begin
      nxt_state = state;
      nxt_we_o = we_o;
      nxt_cti_o = cti_o;
      nxt_bte_o = 2'b0;
      nxt_read_data_reg = read_data_reg;
      nxt_dat_o_reg = dat_o_reg;
      nxt_addr_o = addr_o;
      nxt_beats = beats;
      sel_o = '{default:'1};

      stb_o = 0;
      req_ready = 0;
      write_ready = 0;
      read_valid = 0;

      dat_o = dat_o_reg;
      read_data = read_data_reg;

      case (state)
        STATE_IDLE: begin
           req_ready = 1;
           nxt_beats = req_beats;
           nxt_addr_o = req_addr;
           if (req_valid) begin
              if (req_rw) begin
                 nxt_we_o = 1;
                 if (req_burst) begin
                    if (nxt_beats == 1) begin
                       nxt_cti_o = 3'b111;
                       if (write_valid) begin
                          nxt_state = STATE_WRITE_LAST;
                          nxt_dat_o_reg = write_data;
                       end else begin
                          nxt_state = STATE_WRITE_LAST_WAIT;
                       end
                    end else begin
                       nxt_cti_o = 3'b010;
                       nxt_bte_o = 2'b00;
                       if (write_valid) begin
                          nxt_state = STATE_WRITE;
                          nxt_dat_o_reg = write_data;
                       end else begin
                          nxt_state = STATE_WRITE_WAIT;
                       end
                    end
                 end else begin // !req_burst
                    nxt_cti_o = 3'b111;
                    if (write_valid) begin
                       nxt_state = STATE_WRITE_LAST;
                       nxt_dat_o_reg = write_data;
                    end else begin
                       nxt_state = STATE_WRITE_LAST_WAIT;
                    end
                 end // if (req_burst)
              end else begin // req_rw == 0
                 nxt_we_o = 0;
                 nxt_state = STATE_READ;
                 if (req_burst) begin
                    if (nxt_beats == 1) begin
                       nxt_cti_o = 3'b111;
                    end else begin
                       nxt_cti_o = 3'b010;
                    end
                 end else begin // !req_burst
                    nxt_cti_o = 3'b111;
                 end // if (req_burst)
              end // if (req_rw)
           end // if (req_valid)
        end //STATE_IDLE
        STATE_WRITE_LAST_WAIT: begin
           write_ready = 1;
           if (write_valid) begin
              nxt_state = STATE_WRITE_LAST;
              nxt_dat_o_reg = write_data;
           end
        end //STATE_WRITE_LAST_WAIT
        STATE_WRITE_LAST: begin
           stb_o = 1;
           if (ack_i) begin
              nxt_state = STATE_IDLE;
              nxt_cti_o = 3'b000;
           end
        end //STATE_WRITE_LAST
        STATE_WRITE_WAIT: begin
           write_ready = 1;
           if (write_valid) begin
              nxt_state = STATE_WRITE;
              nxt_dat_o_reg = write_data;
              nxt_beats = beats - 1;
           end
        end //STATE_WRITE_WAIT
        STATE_WRITE: begin
           stb_o = 1;
           if (ack_i) begin
              write_ready = 1;
              nxt_addr_o = addr_o + DATA_WIDTH/8;
                    if (beats == 1) begin
                       nxt_cti_o=3'b111;
                       if (write_valid) begin
                          nxt_state = STATE_WRITE_LAST;
                          nxt_dat_o_reg = write_data;
                       end else begin
                          nxt_state = STATE_WRITE_LAST_WAIT;
                       end
                    end else begin // beats != 1
                       if (write_valid) begin
                          nxt_state = STATE_WRITE;
                          nxt_dat_o_reg = write_data;
                          nxt_beats = beats - 1;
                       end else begin
                          nxt_state = STATE_WRITE_WAIT;
                       end
                    end // if (beats == 1)
           end // if (ack_i)
        end // STATE_WRITE
        STATE_READ: begin
           stb_o = 1;
           if (ack_i) begin
              nxt_read_data_reg = dat_i;
              nxt_beats = beats - 1;
              nxt_addr_o = addr_o + DATA_WIDTH/8;
              nxt_state = STATE_READ_WAIT;
           end
        end
        STATE_READ_WAIT: begin
           read_valid = 1;
           if (read_ready) begin
              if (beats == 1) begin
                 nxt_cti_o = 3'b111;
              end

              if (beats == 0) begin
                 nxt_state = STATE_IDLE;
              end else begin
                 nxt_state = STATE_READ;
              end
           end
        end //STATE_READ_WAIT
        endcase// Case (state)
   end // always_comb

endmodule
