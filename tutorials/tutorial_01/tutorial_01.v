/* Copyright (c) 2015 by the author(s)
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
 * Tutorial 1: Flow control
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

// The source sends two flits and pauses between them
module source(/*AUTOARG*/
   // Outputs
   flit, valid,
   // Inputs
   clk, rst, ready
   );

   parameter FLIT_DATA_WIDTH = 32;
   localparam FLIT_TYPE_WIDTH = 2;
   localparam FLIT_WIDTH = FLIT_DATA_WIDTH + FLIT_TYPE_WIDTH;

   input clk;
   input rst;

   output reg [FLIT_WIDTH-1:0] flit;
   output reg                  valid;
   input                       ready;

   // For waiting phases
   int                         clkcount;
   // State variable
   int                         state;

   // The state machine is triggered on the positive edge
   always @(posedge clk) begin
      // Increment clock counter for wait phases
      clkcount <= clkcount + 1;

      // During reset: Set initial state and counter
      if (rst) begin
         state <= 0;
         clkcount <= 0;
      end else begin
         // The state machine
         case(state)
           0: begin
              // Wait for five clock cycles
              if (clkcount == 5) begin
                 state <= 1;
              end
           end
           1: begin
              // The combinational part (below) asserts flit, switch
              // state when sink is also ready at positive edge
              if (ready) begin
                 state <= 2;
                 clkcount <= 0;
              end
           end
           2: begin
              // Wait for two clock cycles
              if (clkcount == 2) begin
                 state <= 3;
              end
           end
           3: begin
              // Send a second flit like above (state 1)
              if (ready) begin
                 state <= 4;
              end
           end
           4: begin
              // do nothing
           end
         endcase // case (state)
      end
   end

   // This is the combinational part
   always @(negedge clk) begin
      // Default values
      valid <= 0;
      flit <= 'x;

      case (state)
        1: begin
           // Assert first flit
           valid <= 1;
           flit <= {2'b01, 32'h0123_4567};
        end
        3: begin
           // Assert second flit
           valid <= 1;
           flit <= {2'b10, 32'hdead_beef};
        end
      endcase
   end
endmodule // source

// The sink accepts two flits after a startup delay
module sink(/*AUTOARG*/
   // Outputs
   ready,
   // Inputs
   clk, rst, flit, valid
   );

   parameter FLIT_DATA_WIDTH = 32;
   localparam FLIT_TYPE_WIDTH = 2;
   localparam FLIT_WIDTH = FLIT_DATA_WIDTH + FLIT_TYPE_WIDTH;

   input clk;
   input rst;

   input [FLIT_WIDTH-1:0] flit;
   input                  valid;
   output reg             ready;

   // Clock counting variable
   int                         clkcount;
   // The state
   int                         state;

   // The state machine is triggered on the positive edge
   always @(posedge clk) begin
      // Increment clock counter
      clkcount <= clkcount + 1;

      // Set initials during reset
      if (rst) begin
         state <= 0;
         clkcount <= 0;
      end else begin
         case(state)
           0: begin
              // Wait six cycles
              if (clkcount == 6) begin
                 state <= 1;
              end
           end
           1: begin
              // Wait for flit
              if (valid) begin
                 $display("Received %x", flit);
                 state <= 2;
              end
           end
           2: begin
              // Wait for second flit
              if (valid) begin
                 $display("Received %x", flit);
                 state <= 3;
              end
           end
           3: begin

           end
         endcase // case (state)
      end
   end

   // This is the combinational part
   always @(negedge clk) begin
      // Default
      ready <= 0;

      case (state)
        1: begin
           // Set ready
           ready <= 1;
        end
        2: begin
           // Set ready
           ready <= 1;
        end
      endcase
   end
endmodule // sink

// The tutorial tstbench that connects both modules
module tutorial_01(input clk, input rst);

   localparam FLIT_WIDTH = 34;

   wire [FLIT_WIDTH:0] flit;
   wire                valid;
   wire                ready;

   source
     u_source(/*AUTOINST*/
              // Outputs
              .flit                     (flit[FLIT_WIDTH-1:0]),
              .valid                    (valid),
              // Inputs
              .clk                      (clk),
              .rst                      (rst),
              .ready                    (ready));

   sink
     u_sink(/*AUTOINST*/
            // Outputs
            .ready                      (ready),
            // Inputs
            .clk                        (clk),
            .rst                        (rst),
            .flit                       (flit[FLIT_WIDTH-1:0]),
            .valid                      (valid));

endmodule // tutorial_01
