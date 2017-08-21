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

   input            bus_req,
   input [2:0]      bus_addr,
   input            bus_write,
   input [7:0]      bus_wdata,
   output           bus_ack,
   output reg [7:0] bus_rdata,

   input            drop,

   output reg       out_valid,
   output reg [7:0] out_char,
   input            out_ready,
   input            in_valid,
   input [7:0]      in_char,
   output reg       in_ready,

   output           irq);

   localparam REG_TXRX = 0;
   localparam REG_IER = 1;
   localparam REG_IIR_FCR = 2;
   localparam REG_LCR = 3;
   localparam REG_LSR = 5;

   reg bus_resp;

   assign bus_ack = bus_resp;

   // DLAB:
   // 0: RBR, THR and IER accessible (during communicatino)
   // 1: DLL and DLM accessible (during initialization to set baud rate)
   reg          dlab;

   // Interrupt enable registers
   reg          erbfi; // Receive Data Available Interrupt
   reg          etbei; // Transmitter Holding Register Empty Interrupt
   reg          elsi;  // Receiver Line Status Interrupt

   // Interrupts
   logic        irq_ls, irq_rbf, irq_tbe;

   assign irq = irq_ls | irq_rbf | irq_tbe;

   assign irq_tbe = etbei & out_ready;
   assign irq_rbf = erbfi & in_valid;
   assign irq_ls = 0;

   // FIFO
   reg          fifo_enable, fifo_rcvr_enable, fifo_xmit_enable, dma_mode;

   always @(posedge clk) begin
      if (rst) begin
         dlab <= 0;
         bus_resp <= 0;
         out_valid <= 0;
         in_ready <= 0;
         {erbfi, etbei, elsi} <= 0;
         {fifo_enable, fifo_rcvr_enable, fifo_xmit_enable, dma_mode} <= 0;
      end else begin
         out_valid <= 0;
         in_ready <= 0;

         if (bus_resp) begin
            bus_resp <= 0;
         end else if (bus_req) begin
            bus_resp <= 1;

            if (dlab) begin
               case (bus_addr)
                 REG_LCR: begin
                    if (bus_write) begin
                       dlab <= bus_wdata[7];
                    end
                 end
               endcase // case (bus_addr)
            end else begin
               case (bus_addr)
                 REG_TXRX: begin
                    bus_rdata <= in_char;

                    if (bus_write) begin
                       out_valid <= 1;
                       out_char <= bus_wdata;
                    end else begin
                       in_ready <= 1;
                    end
                 end
                 REG_LCR: begin
                    if (bus_write) begin
                       dlab <= bus_wdata[7];
                    end
                 end
                 REG_IER: begin
                    bus_rdata <= {5'h0, elsi, etbei, erbfi};
                    if (bus_write) begin
                       {elsi, etbei, erbfi} <= bus_wdata[2:0];
                    end
                 end
                 REG_IIR_FCR: begin
                    bus_rdata[7:3] <= 5'h0;
                    if (irq_ls)
                      bus_rdata[2:0] <= 3'b110;
                    else if (irq_rbf)
                      bus_rdata[2:0] <= 3'b100;
                    else if (irq_tbe)
                      bus_rdata[2:0] <= 3'b010;
                    else
                      bus_rdata[2:0] <= 3'b001;

                    if (bus_write) begin
                       // FCR
                       {fifo_enable, fifo_rcvr_enable, fifo_xmit_enable, dma_mode} <= bus_wdata[3:0];
                    end
                 end // case: REG_IIR_FCR
               REG_LSR: begin
                   // TMTE=1, THRE=1, DR (data ready) when input
                  bus_rdata <= {7'h30, in_valid};
               end
               endcase // case (bus_addr)
            end
         end // if (bus_req)
      end
   end

endmodule // osd_dem_uart_16550
