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

interface wb_channel
  #(
    ADDR_WIDTH = 32,
    DATA_WIDTH = 32
    );

   logic                    cyc_o;
   logic                    stb_o;
   logic                    we_o;
   logic [ADDR_WIDTH-1:0]   adr_o;
   logic [DATA_WIDTH-1:0]   dat_o;
   logic [DATA_WIDTH/8-1:0] sel_o;
   logic [2:0]              cti_o;
   logic [1:0]              bte_o;
   logic                    ack_i;
   logic                    err_i;
   logic                    rty_i;
   logic [DATA_WIDTH-1:0]   dat_i;
   
endinterface // wb_channel




