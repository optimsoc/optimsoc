/* Copyright (c) 2016 by the author(s)
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
 * Control layer between interface and the FIFOs that handles control messages
 * like credits etc.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_uart_control
  #(parameter FIFO_CREDIT_WIDTH = 1'bx,
    parameter INPUT_FIFO_CREDIT = 1'bx)
   (
    input        clk,
    input        rst,

    input [7:0]  ingress_in_data,
    input        ingress_in_valid,
    output       ingress_in_ready,

    output [7:0] ingress_out_data,
    output       ingress_out_valid,
    input        ingress_out_ready,

    input [7:0]  egress_in_data,
    input        egress_in_valid,
    output       egress_in_ready,

    output [7:0] egress_out_data,
    output       egress_out_enable,
    input        egress_out_done,

    input        transfer_in,

    output reg   ctrl_logic_rst,
    output       com_rst,
    output       error
    );

   // Submodule errors
   wire [3:0]    mod_error;

   // Transfer on egress path
   wire          transfer_egress;

   // Sufficient credit
   wire          can_send;

   // Debt we get from the host
   wire [13:0]   debt;
   wire          debt_en;

   // Credit we send to the host
   wire [FIFO_CREDIT_WIDTH-1:0] credit;
   reg                          credit_en;
   wire                         credit_ack;
   reg                          get_credit;
   wire                         get_credit_ack;   

   // Reset registers
   wire                         logic_rst_en;
   wire                         logic_rst_val;
   wire                         com_rst_en;
   wire                         com_rst_val;

   // Collect submodule errors as control error
   assign error = |mod_error;

   // Count the ingress transfers to know when we can give new credit
   reg [FIFO_CREDIT_WIDTH-1:0]  transfer_counter;
   reg [FIFO_CREDIT_WIDTH-1:0]  nxt_transfer_counter;

   // We want to send new credit
   reg                          send_credit_pnd;
   reg                          nxt_send_credit_pnd;
   // Start a request to the creditor module. It can be one cycle
   // delayed, hence make it a register
   reg                          nxt_get_credit;

   // Sequential part of credit generation logic
   always @(posedge clk) begin
      if (rst | com_rst) begin
         transfer_counter <= 0;
         send_credit_pnd <= 0;
         get_credit <= 0;
      end else begin
         transfer_counter <= nxt_transfer_counter;
         send_credit_pnd <= nxt_send_credit_pnd;
         get_credit <= nxt_get_credit;
      end
   end

   // Combinational part of credit generation logic
   always @(*) begin
      nxt_send_credit_pnd = send_credit_pnd;
      nxt_get_credit = get_credit;
      nxt_transfer_counter = transfer_counter;

      credit_en = 0;
      nxt_get_credit = get_credit;
      
      if (send_credit_pnd) begin
         // We are in the process of sending new credit
         if (get_credit) begin
            // .. and still wait for the creditor module to update
            if (get_credit_ack) begin
               // done
               nxt_get_credit = 0;
               credit_en = 1;
            end
         end else begin
            // We have a new credit, send it
            credit_en = 1;
            if (credit_ack) begin
               // completed
               nxt_send_credit_pnd = 0;
            end
         end // else: !if(get_credit)

         // If a transfer happens in between capture it. There should
         // never occur more than one transfer actually
         if (transfer_counter != 0) begin
            if (transfer_in) begin
               nxt_transfer_counter = transfer_counter - 1;
            end
         end
      end else begin // if (send_credit_pnd)
         // We are not in the process of sending a credit
         if (transfer_counter == 0) begin
            // But if we have counted down, we reset the counter to
            // the 1/2 threshold and go into sending the credit
            nxt_transfer_counter = INPUT_FIFO_CREDIT >> 1;
            nxt_send_credit_pnd = 1;
            nxt_get_credit = 1;
         end else if (transfer_in) begin
            // Count transfer in this cycle. We cannot miss a transfer
            // from above as there are always many cycles (at least
            // 10) between transfers
            nxt_transfer_counter = nxt_transfer_counter - 1;
         end
      end
   end

   // Control the reset registers
   reg com_rst_host;
   always @(posedge clk) begin
      if (rst) begin
         com_rst_host <= 0;
         ctrl_logic_rst <= 0;
      end else begin
         if (logic_rst_en) begin
            ctrl_logic_rst <= logic_rst_val;
         end
         if (com_rst_en) begin
            com_rst_host <= com_rst_val;
         end
      end
   end
   assign com_rst = com_rst_host | rst;
   
   /* debtor AUTO_TEMPLATE(
    .rst     (com_rst),
    .owing   (can_send),
    .error   (mod_error[3]),
    .payback (transfer_egress),
    .tranche (debt),
    .lend    (debt_en),
    ); */
   debtor
     #(.WIDTH(15), .TRANCHE_WIDTH(14))
   u_debtor(/*AUTOINST*/
            // Outputs
            .owing                      (can_send),              // Templated
            .error                      (mod_error[3]),          // Templated
            // Inputs
            .clk                        (clk),
            .rst                        (com_rst),               // Templated
            .payback                    (transfer_egress),       // Templated
            .tranche                    (debt),                  // Templated
            .lend                       (debt_en));              // Templated

   /* creditor AUTO_TEMPLATE(
    .rst     (com_rst),
    .payback (transfer_in),
    .borrow  (get_credit),
    .grant   (get_credit_ack),
    .error   (mod_error[2]),
    .credit  (credit[FIFO_CREDIT_WIDTH-1:0]),
    ); */
   creditor
     #(.WIDTH(15), .CREDIT_WIDTH(FIFO_CREDIT_WIDTH),
       .INITIAL_VALUE(INPUT_FIFO_CREDIT))
   u_creditor(/*AUTOINST*/
              // Outputs
              .credit                   (credit[FIFO_CREDIT_WIDTH-1:0]), // Templated
              .grant                    (get_credit_ack),        // Templated
              .error                    (mod_error[2]),          // Templated
              // Inputs
              .clk                      (clk),
              .rst                      (com_rst),               // Templated
              .payback                  (transfer_in),           // Templated
              .borrow                   (get_credit));           // Templated
   
   /* glip_uart_control_egress AUTO_TEMPLATE(
    .rst        (com_rst),
    .in_\(.*\)  (egress_in_\1),
    .out_\(.*\) (egress_out_\1),
    .transfer   (transfer_egress),
    .error      (mod_error[1]),
    .credit     ({{15-FIFO_CREDIT_WIDTH{1'b0}},credit}),
    ); */
   glip_uart_control_egress
     u_egress(/*AUTOINST*/
              // Outputs
              .in_ready                 (egress_in_ready),       // Templated
              .out_data                 (egress_out_data),       // Templated
              .out_enable               (egress_out_enable),     // Templated
              .transfer                 (transfer_egress),       // Templated
              .credit_ack               (credit_ack),
              .error                    (mod_error[1]),          // Templated
              // Inputs
              .clk                      (clk),
              .rst                      (com_rst),               // Templated
              .in_data                  (egress_in_data),        // Templated
              .in_valid                 (egress_in_valid),       // Templated
              .out_done                 (egress_out_done),       // Templated
              .can_send                 (can_send),
              .credit                   ({{15-FIFO_CREDIT_WIDTH{1'b0}},credit}), // Templated
              .credit_en                (credit_en));

   /* glip_uart_control_ingress AUTO_TEMPLATE(
    .in_\(.*\)  (ingress_in_\1),
    .out_\(.*\) (ingress_out_\1),
    .credit_val (debt),
    .credit_en  (debt_en),
    .transfer   (),
    .error      (mod_error[0]),
    .rst_en     (ctrl_rst_en),
    .rst_val    (ctrl_rst_val),
    ); */
   glip_uart_control_ingress
     u_ingress(/*AUTOINST*/
               // Outputs
               .in_ready                (ingress_in_ready),      // Templated
               .out_data                (ingress_out_data),      // Templated
               .out_valid               (ingress_out_valid),     // Templated
               .transfer                (),                      // Templated
               .credit_en               (debt_en),               // Templated
               .credit_val              (debt),                  // Templated
               .logic_rst_en            (logic_rst_en),
               .logic_rst_val           (logic_rst_val),
               .com_rst_en              (com_rst_en),
               .com_rst_val             (com_rst_val),
               .error                   (mod_error[0]),          // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .in_data                 (ingress_in_data),       // Templated
               .in_valid                (ingress_in_valid),      // Templated
               .out_ready               (ingress_out_ready));    // Templated

endmodule

// Local Variables:
// verilog-library-directories:("." "../../../common/logic/credit/verilog")
// End:
