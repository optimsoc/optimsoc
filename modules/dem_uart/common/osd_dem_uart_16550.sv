
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

   output           out_valid,
   output [7:0]     out_char,
   input            out_ready,
   input            in_valid,
   input [7:0]      in_char,
   output           in_ready);

   localparam REG_TXRX = 0;
   localparam REG_LCR = 3;
   localparam REG_LSR = 5;
   
   reg          lcr_7;

   always @(posedge clk)
     if (rst)
       lcr_7 <= 0;
     else if (bus_req & bus_write & (bus_addr == REG_LCR))
       lcr_7 <= bus_wdata[7];

   assign out_valid = bus_req & bus_write & (bus_addr == REG_TXRX) & !lcr_7;
   assign out_char = bus_wdata;

   always @(*)
     case (bus_addr)
       REG_TXRX: bus_rdata = in_char;
       REG_LSR: bus_rdata = {7'h30, in_valid};
       default: bus_rdata = 8'h0;
     endcase
   
   assign bus_ack = lcr_7 | (bus_addr != REG_TXRX) | 
                    (bus_write ? (out_ready | drop) : 1'b1);

   assign in_ready = bus_req & !bus_write & (bus_addr == REG_TXRX);
   
endmodule // osd_dem_uart_16550
