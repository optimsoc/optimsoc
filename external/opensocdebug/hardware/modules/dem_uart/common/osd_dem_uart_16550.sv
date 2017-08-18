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

module osd_dem_uart_16550
  (input clk, rst,

   (* mark_debug = "yes" *) input            bus_req,
   (* mark_debug = "yes" *) input [2:0]      bus_addr,
   (* mark_debug = "yes" *) input            bus_write,
   (* mark_debug = "yes" *) input [7:0]      bus_wdata,
   (* mark_debug = "yes" *) output           bus_ack,
   (* mark_debug = "yes" *) output reg [7:0] bus_rdata,

   (* mark_debug = "yes" *) input            drop,

   (* mark_debug = "yes" *) output           out_valid,
   (* mark_debug = "yes" *) output [7:0]     out_char,
   (* mark_debug = "yes" *) input            out_ready,
   (* mark_debug = "yes" *) input            in_valid,
   (* mark_debug = "yes" *) input [7:0]      in_char,
   (* mark_debug = "yes" *) output           in_ready,
   
   (* mark_debug = "yes" *) output           irq);

   localparam REG_TXRX = 0;
   localparam REG_IER = 1;
   localparam REG_IIR = 2;
   localparam REG_LCR = 3;
   localparam REG_LSR = 5;

   // DLAB: 
   // 0: RBR, THR and IER accessible (during communicatino)
   // 1: DLL and DLM accessible (during initialization to set baud rate)
   (* mark_debug = "yes" *) reg          lcr_7; 

   // trigger an interrupt if we are ready for TX data from the software (THE)
   (* mark_debug = "yes" *) reg enable_tx_empty_interrupt;
   
   // we're always ready to get data whenever the CPU is ready ...
   // XXX: make this depending on the NoC status
   assign irq = enable_tx_empty_interrupt;
   
   always @(posedge clk)
     if (rst) begin
       lcr_7 <= 0;
       enable_tx_empty_interrupt <= 0;
     end else if (bus_req & bus_write & (bus_addr == REG_LCR))
       lcr_7 <= bus_wdata[7];
     else if (bus_req & bus_write & (bus_addr == REG_IER) & lcr_7 == 0)
        enable_tx_empty_interrupt <= bus_wdata[1];

   assign out_valid = bus_req & bus_write & (bus_addr == REG_TXRX) & !lcr_7;
   assign out_char = bus_wdata;

   always @(*)
     if (!bus_write) begin
         case (bus_addr)
           REG_TXRX: bus_rdata = in_char;
           REG_LSR: bus_rdata = {7'h30, in_valid}; // 0x30 == THR is empty AND THR is empty, and line is idle 
           REG_IIR: begin
              if (lcr_7 /* DLAB */ == 0) begin
                 // we trigger only one interrupt, hardcode the interrupt reason to "transmit empty"
                 bus_rdata = 8'h0;
                 bus_rdata[3:1] = 3'b001; // Transmit Holding Register Interrupt (THRE) 
              end
           end
           REG_IER: begin
              if (lcr_7 /* DLAB */ == 0) begin
                 bus_rdata = 8'h0;
                 bus_rdata[1] = enable_tx_empty_interrupt; // enable readback, it's required by the Linux kernel
              end
           end
           default: bus_rdata = 8'h0;
         endcase
     end

   assign bus_ack = lcr_7 | (bus_addr != REG_TXRX) |
                    (bus_write ? (out_ready | drop) : 1'b1);

   assign in_ready = bus_req & !bus_write & (bus_addr == REG_TXRX);

endmodule // osd_dem_uart_16550
