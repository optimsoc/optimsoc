/* Copyright (c) 2015-2017 by the author(s)
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
 * VCU108 loopback demo
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Max Koenen <max.koenen@tum.de>
 */

module vcu108_loopback
  (
   // 125 MHz clock & CPU Reset button
   input        CLK_125MHZ_P,
   input        CLK_125MHZ_N,
   input        CPU_RESET,

   // UART Interface
   // The signals here are named from the host point-of-view.
   // USB UART (onboard)
   input        USB_UART_TX,
   output       USB_UART_RX,
   input        USB_UART_RTS,
   output       USB_UART_CTS,

   // UART over PMOD (bottom row of J52)
   output       PMOD_UART_RX,
   input        PMOD_UART_TX,
   output       PMOD_UART_CTS, // active low (despite the name)
   input        PMOD_UART_RTS, // active low (despite the name)

   // GPIO DIP Switches SW0-SW3
   input [3:0]  GPIO_DIP_SW,

   // User Pushbuttons
   input   GPIO_SW_N,
   input   GPIO_SW_S,
   input   GPIO_SW_W,
   input   GPIO_SW_E,
   input   GPIO_SW_C,

   // GPIO LEDs
   output [7:0]   GPIO_LED
   );

   parameter WIDTH = 16;

   // source of the UART connection
   // onboard: Use the UART chip on the VCU108 board
   // pmod: Connect a pmodusbuart module to J52 (bottom row)
   parameter UART0_SOURCE = "pmod";

   // onboard: 921600, max. for CP2105
   // pmod: 3 MBaud, max. for FT232R
   parameter UART0_BAUD = (UART0_SOURCE == "pmod" ? 3000000 : 921600);

   localparam FREQ = 125000000; // frequency of clk [Hz]

   // Match pins for this demo:
   wire     rst;
   assign   rst = CPU_RESET;
   wire     clk;
   wire     clkp;
   wire     clkn;
   assign   clkp = CLK_125MHZ_P;
   assign   clkn = CLK_125MHZ_N;
   wire     uart_rx;
   wire     uart_tx;
   wire     uart_cts_n;
   wire     uart_rts_n;

   // Important note: We change to a different view of naming here.
   // The signals from/to the board are seen from a DTE (host PC) point of view,
   // all internally used signals are seen from a DCE (FPGA) point of view
   // (i.e. from our view).
   generate
      if (UART0_SOURCE == "onboard") begin
         assign uart_rx = USB_UART_TX;
         assign USB_UART_RX = uart_tx;
         assign USB_UART_CTS = uart_rts_n;
         assign uart_cts_n = USB_UART_RTS;
      end else if (UART0_SOURCE == "pmod") begin
         assign uart_rx = PMOD_UART_TX;
         assign PMOD_UART_RX = uart_tx;
         assign PMOD_UART_CTS = uart_rts_n;
         assign uart_cts_n = PMOD_UART_RTS;
      end
   endgenerate


   wire [WIDTH-1:0] in_data;
   wire             in_valid;
   wire             in_ready;
   wire [WIDTH-1:0] out_data;
   wire             out_valid;
   wire             out_ready;

   IBUFDS
     #(.DQS_BIAS("FALSE"))
   IBUFDS_inst (.O(clk),
                .I(clkp),
                .IB(clkn));


   // data loopback
   assign out_data = in_data;
   assign out_valid = in_valid;
   assign in_ready = out_ready;

   wire error;
   reg  error_reg;

   // Small LED and switch test demo
   assign GPIO_LED[0] = GPIO_SW_N ^ (GPIO_DIP_SW[2] & GPIO_DIP_SW[3]);
   assign GPIO_LED[1] = GPIO_SW_S ^ (GPIO_DIP_SW[2] & GPIO_DIP_SW[3]);
   assign GPIO_LED[2] = GPIO_SW_W ^ (GPIO_DIP_SW[2] & GPIO_DIP_SW[3]);
   assign GPIO_LED[3] = GPIO_SW_E ^ (GPIO_DIP_SW[2] & GPIO_DIP_SW[3]);
   assign GPIO_LED[4] = GPIO_SW_C ^ (GPIO_DIP_SW[2] & GPIO_DIP_SW[3]);
   assign GPIO_LED[5] = GPIO_DIP_SW[0] & GPIO_DIP_SW[1];
   assign GPIO_LED[6] = rst | ctrl_logic_rst;
   assign GPIO_LED[7] = error_reg;

   always @(posedge clk) begin
      if (rst) begin
         error_reg <= 0;
      end else begin
         error_reg <= error_reg | error;
      end
   end

   wire ctrl_logic_rst;

   glip_uart_toplevel
     #(.FREQ_CLK_IO(FREQ),
       .BAUD(UART0_BAUD),
       .WIDTH(WIDTH))
   u_uart(.clk_io         (clk),
          .clk            (clk),
          .rst            (rst),
          .uart_rx        (uart_rx),
          .uart_tx        (uart_tx),
          .uart_cts_n     (uart_cts_n),
          .uart_rts_n     (uart_rts_n),
          .error          (error),
          .ctrl_logic_rst (ctrl_logic_rst),
          .com_rst        (com_rst),
          .fifo_in_data   (in_data),
          .fifo_in_valid  (in_valid),
          .fifo_in_ready  (in_ready),
          .fifo_out_data  (out_data),
          .fifo_out_valid (out_valid),
          .fifo_out_ready (out_ready));

endmodule // vcu108
