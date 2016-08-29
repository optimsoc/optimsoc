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

import dii_package::dii_flit;
module osd_mam_wb
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
      parameter BASE_ADDR7  = 'x
      )
   (
    input                   clk_i, rst_i,

    input                   dii_flit debug_in, output debug_in_ready,
    output                  dii_flit debug_out, input debug_out_ready,

    input [9:0]             id,

    output                  stb_o,
    output                  cyc_o,
    input                   ack_i,
    output                  we_o,
    output [ADDR_WIDTH-1:0] addr_o,
    output [DATA_WIDTH-1:0] dat_o,
    input [DATA_WIDTH-1:0]  dat_i,
    output [2:0]            cti_o,
    output [1:0]            bte_o,
    output [SW-1:0]         sel_o
    );

    //Byte select width
   localparam SW = (DATA_WIDTH == 32) ? 4 :
                   (DATA_WIDTH == 16) ? 2 :
                   (DATA_WIDTH ==  8) ? 1 : 'hx;

   logic                        req_valid;
   logic                        req_ready;
   logic                        req_rw;
   logic [ADDR_WIDTH-1:0]       req_addr;
   logic                        req_burst;
   logic [13:0]                 req_beats;

   logic                        write_valid;
   logic [DATA_WIDTH-1:0]       write_data;
   logic [DATA_WIDTH/8-1:0]     write_strb;
   logic                        write_ready;

   logic                        read_valid;
   logic [DATA_WIDTH-1:0]       read_data;
   logic                        read_ready;

   osd_mam
     #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH),
       .MAX_PKT_LEN(MAX_PKT_LEN), .REGIONS(REGIONS),
       .BASE_ADDR0(BASE_ADDR0), .MEM_SIZE0(MEM_SIZE0),
       .BASE_ADDR1(BASE_ADDR1), .MEM_SIZE1(MEM_SIZE1),
       .BASE_ADDR2(BASE_ADDR2), .MEM_SIZE2(MEM_SIZE2),
       .BASE_ADDR3(BASE_ADDR3), .MEM_SIZE3(MEM_SIZE3),
       .BASE_ADDR4(BASE_ADDR4), .MEM_SIZE4(MEM_SIZE4),
       .BASE_ADDR5(BASE_ADDR5), .MEM_SIZE5(MEM_SIZE5),
       .BASE_ADDR6(BASE_ADDR6), .MEM_SIZE6(MEM_SIZE6),
       .BASE_ADDR7(BASE_ADDR7), .MEM_SIZE7(MEM_SIZE7))
   u_mam(.*,
         .clk(clk_i),
         .rst(rst_i));

   osd_mam_wb_if
     #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH))
    u_mam_wb_if(.*);

endmodule
