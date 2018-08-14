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

/**
 * Module: osd_dem_uart_16550
 *
 * This module behaves like a 16550 UART towards the bus, but translates
 * all incoming signals into a simple ready-valid format that is
 * then used by osd_dem_uart
 */
module osd_dem_uart_16550
  (input clk, rst,

   input            bus_req,
   input [2:0]      bus_addr,
   input            bus_write,
   input [7:0]      bus_wdata,
   output reg       bus_ack,
   output reg [7:0] bus_rdata,

   input            drop,

   output reg       out_valid,
   output reg [7:0] out_char,
   input            out_ready,

   input            in_valid,
   input [7:0]      in_char,
   output reg       in_ready,

   output           irq
   );

   localparam REG_RBR_THR = 0;
   localparam REG_IER     = 1;
   localparam REG_IIR_FCR = 2;
   localparam REG_LCR     = 3;
   localparam REG_LSR     = 5;
   localparam REG_SCR     = 7;

   // Interrupt identification codes as per the UART 16550 specification
   localparam INTR_NONE = 4'b0001;
   localparam INTR_LS   = 4'b0110;
   localparam INTR_RBF  = 4'b0100;
   localparam INTR_TBE  = 4'b0010;

   // DLAB (LCR7):
   // 0: RBR, THR and IER accessible (during communication)
   // 1: DLL and DLM accessible (during initialization to set baud rate)
   reg [7:0]    lcr;
   logic [7:0]  nxt_lcr;

   reg [15:0]   divisor_latch;

   // Interrupt enable registers
   reg          erbfi, etbei, elsi;
   logic        nxt_erbfi, nxt_etbei, nxt_elsi;

   // Interrupts
   logic        irq_rbf, irq_tbe, irq_ls;

   assign irq_rbf = erbfi & in_valid;            // Receive data available
   assign irq_tbe = etbei & (out_ready | drop);  // (THRE) Transmitter holding register empty
   assign irq_ls  = elsi & 1'b0;                 // Receiver line status

   assign irq = irq_rbf | irq_tbe | irq_ls;

   // FIFO
   reg          fifo_enable, fifo_rx_clear, fifo_tx_clear, dma_mode;
   logic        nxt_fifo_enable, nxt_fifo_rx_clear, nxt_fifo_tx_clear, nxt_dma_mode;

   reg          req;
   reg [2:0]    addr;
   reg          write;

   always_ff @(posedge clk) begin
      if (rst) begin
         req <= 1'b0;
         addr <= 3'h0;
         write <= 1'b0;

         erbfi <= 1'b0;
         etbei <= 1'b0;
         elsi <= 1'b0;
         fifo_enable <= 1'b0;
         fifo_rx_clear <= 1'b0;
         fifo_tx_clear <= 1'b0;
         dma_mode <= 1'b0;
         lcr <= 8'h0;
      end else begin
         req <= bus_req;
         addr <= bus_addr;
         write <= bus_write;

         erbfi <= nxt_erbfi;
         etbei <= nxt_etbei;
         elsi <= nxt_elsi;
         fifo_enable <= nxt_fifo_enable;
         fifo_rx_clear <= nxt_fifo_rx_clear;
         fifo_tx_clear <= nxt_fifo_tx_clear;
         dma_mode <= nxt_dma_mode;
         lcr <= nxt_lcr;
      end
   end

   always_comb begin
      nxt_erbfi = erbfi;
      nxt_etbei = etbei;
      nxt_elsi = elsi;
      nxt_fifo_enable = fifo_enable;
      nxt_fifo_rx_clear = fifo_rx_clear;
      nxt_fifo_tx_clear = fifo_tx_clear;
      nxt_dma_mode = dma_mode;
      nxt_lcr = lcr;
      
      out_char = 8'h0;
      out_valid = 1'b0;
      in_ready = 1'b0;
      bus_rdata = 8'h0;
      bus_ack = 1'b0;

      if (req) begin
         case (addr)
            REG_RBR_THR: begin
               if (lcr[7]) begin
                  if (write) begin
                     divisor_latch[7:0] = bus_wdata;
                  end else begin
                     bus_rdata = divisor_latch[7:0];
                  end
                  bus_ack = bus_req;
               end else begin
                  if (write) begin
                     out_char = bus_wdata;
                     out_valid = 1;
                     bus_ack = out_ready;
                  end else begin
                     bus_rdata = in_char;
                     in_ready = 1;
                     bus_ack = in_valid;
                  end
               end
            end
            REG_IER: begin
               if (lcr[7]) begin
                  if (write) begin
                     divisor_latch[15:8] = bus_wdata;
                  end else begin
                     bus_rdata = divisor_latch[15:8];
                  end
                  bus_ack = bus_req;
               end else begin
                  if (write) begin
                     {nxt_elsi, nxt_etbei, nxt_erbfi} = bus_wdata[2:0];
                  end else begin
                     bus_rdata = {5'h0, nxt_elsi, nxt_etbei, nxt_erbfi};
                  end
                  bus_ack = bus_req;
               end
            end
            REG_IIR_FCR: begin
               if (write) begin
                  {nxt_dma_mode, nxt_fifo_tx_clear, nxt_fifo_rx_clear, nxt_fifo_enable} = bus_wdata[3:0];
               end else begin
                  bus_rdata[7:6] = {fifo_enable, fifo_enable};
                  bus_rdata[5:4] = 2'h0;

                  if (irq_ls) begin
                     bus_rdata[3:0] = INTR_LS;
                  end else if (irq_rbf) begin
                     bus_rdata[3:0] = INTR_RBF;
                  end else if (irq_tbe) begin
                     bus_rdata[3:0] = INTR_TBE;
                  end else begin
                     bus_rdata[3:0] = INTR_NONE;
                  end
               end
               bus_ack = bus_req;
            end
            REG_LCR: begin
               if (write) begin
                  nxt_lcr = bus_wdata;
               end else begin
                  bus_rdata = lcr;
               end
               bus_ack = bus_req;
            end
            REG_LSR: begin
               // TEMT=1, THRE=1, DR (data ready) when input
               bus_rdata = {7'b0110000, in_valid};
               bus_ack = bus_req;
            end
         endcase
      end // if (req)

      if (fifo_enable & fifo_rx_clear) begin
         // TODO Clear all data in RXFifo
         nxt_fifo_rx_clear = 0;
      end

      if (fifo_enable & fifo_tx_clear) begin
         // TODO Clear all data in TXFifo
         nxt_fifo_tx_clear = 0;
      end
   end

endmodule // osd_dem_uart_16550
