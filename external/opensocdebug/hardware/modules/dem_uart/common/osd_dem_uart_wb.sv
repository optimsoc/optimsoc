// Copyright 2017 by the authors
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

module osd_dem_uart_wb
   (
    input 	  clk, rst,

    input [9:0]   id,

    (* mark_debug = "yes" *) output 	  irq,

    (* mark_debug = "yes" *) input [3:0]   wb_adr_i,
    (* mark_debug = "yes" *) input 	  wb_cyc_i,
    (* mark_debug = "yes" *) input [31:0]  wb_dat_i,
    (* mark_debug = "yes" *) input [3:0]   wb_sel_i,
    (* mark_debug = "yes" *) input 	  wb_stb_i,
    (* mark_debug = "yes" *) input 	  wb_we_i,
    (* mark_debug = "yes" *) input [2:0]   wb_cti_i,
    (* mark_debug = "yes" *) input [1:0]   wb_bte_i,
    (* mark_debug = "yes" *) output 	  wb_ack_o,
    (* mark_debug = "yes" *) output 	  wb_rty_o,
    (* mark_debug = "yes" *) output 	  wb_err_o,
    (* mark_debug = "yes" *) output [31:0] wb_dat_o,


    (* mark_debug = "yes" *) input 	  dii_flit debug_in,
    (* mark_debug = "yes" *) output 	  debug_in_ready,
    (* mark_debug = "yes" *) output 	  dii_flit debug_out,
    (* mark_debug = "yes" *) input 	  debug_out_ready
    );

   (* mark_debug = "yes" *) logic 	 bus_req;
   (* mark_debug = "yes" *) logic [2:0] 	 bus_addr;
   (* mark_debug = "yes" *) logic 	 bus_write;
   (* mark_debug = "yes" *) logic [7:0] 	 bus_wdata;
   (* mark_debug = "yes" *) logic 	 bus_ack;
   (* mark_debug = "yes" *) logic [7:0] 	 bus_rdata;
   
   (* mark_debug = "yes" *) logic 	 drop;
   
   (* mark_debug = "yes" *) logic 	 out_valid;
   (* mark_debug = "yes" *) logic [7:0] 	 out_char;
   (* mark_debug = "yes" *) logic 	 out_ready;
   (* mark_debug = "yes" *) logic 	 in_valid;
   (* mark_debug = "yes" *) logic [7:0] 	 in_char;
   (* mark_debug = "yes" *) logic 	 in_ready;

   // .* doesn't exactly make understanding code easier ...
   osd_dem_uart_16550
     u_16550(.*);

   osd_dem_uart
     u_uart_emul(.*);

   //assign irq = in_valid;

   assign bus_req = wb_cyc_i & wb_stb_i;
   assign bus_addr = { wb_adr_i[2], (wb_sel_i[0] ? 2'b11 : (wb_sel_i[1] ? 2'b10 : (wb_sel_i[2] ? 2'b01 : 2'b00))) };
   assign bus_write = wb_we_i;
   assign bus_wdata = wb_dat_i[7:0];
   assign wb_ack_o = bus_ack;
   assign wb_err_o = 1'b0;
   assign wb_rty_o = 1'b0;
   assign wb_dat_o = {4{bus_rdata}};

endmodule // osd_dem_uart_wb
