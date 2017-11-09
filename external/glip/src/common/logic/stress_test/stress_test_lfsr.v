/* Copyright (c) 2017 by the author(s)
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
 * Pseudo random number generator for 16 or 32-bit numbers with a maximum length
 * period. The first shift operation is performed on initialization so the first
 * random number following the seed is immediately available.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 */

module stress_test_lfsr
#(
   parameter WIDTH = 16
)(
   // Clock
   input                clk,

   input [WIDTH-1:0]    rnd_seed,
   input                set_seed,
   input                generate_rnd,
   output [WIDTH-1:0]   rnd_data
);

   // Contains the generated random numbers.
   reg [WIDTH-1:0] rnd_gen;
   assign rnd_data = rnd_gen;

   // Shift register generating pseudo random numbers.
   generate
      if (WIDTH == 32) begin
         always @(posedge clk) begin
            if (set_seed) begin
               // Make sure the lfsr is initialized with a valid value.
               if (rnd_seed == 0) begin
                  rnd_gen <= 32'h00000001;
               end else begin
                  // Immediately perform first shift operation when initializing
                  rnd_gen[0] <= rnd_seed[31] ^ rnd_seed[21] ^ rnd_seed[1]
                     ^ rnd_seed[0];
                  rnd_gen[31:1] <= rnd_seed[30:0];
               end
            end else if (generate_rnd) begin
               rnd_gen[0] <= rnd_gen[31] ^ rnd_gen[21] ^ rnd_gen[1]
                  ^ rnd_gen[0];
               rnd_gen[31:1] <= rnd_gen[30:0];
            end else begin
               rnd_gen <= rnd_gen;
            end
         end
      end
      else if (WIDTH == 16) begin
         always @(posedge clk) begin
            if (set_seed) begin
               // Make sure the lfsr is initialized with a valid value.
               if (rnd_seed == 0) begin
                  rnd_gen <= 16'h0001;
               end else begin               
                  // Immediately perform first shift operation when initializing
                  rnd_gen[0] <= rnd_seed[15] ^ rnd_seed[14] ^ rnd_seed[12]
                     ^ rnd_seed[3];
                  rnd_gen[15:1] <= rnd_seed[14:0];
               end
            end else if (generate_rnd) begin
               rnd_gen[0] <= rnd_gen[15] ^ rnd_gen[14] ^ rnd_gen[12]
                  ^ rnd_gen[3];
               rnd_gen[15:1] <= rnd_gen[14:0];
            end else begin
               rnd_gen <= rnd_gen;
            end
         end
      end
   endgenerate

endmodule
