/* Copyright (c) 2014 by the author(s)
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
 * GLIP JTAG Looback example for the ZTEX 1.15d boards
 *
 * Author(s):
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */
module ztex_115_loopback(/*AUTOARG*/
   // Outputs
   tdo,
   // Inputs
   tck, tms, tdi, clk
   );

   input tck, tms, tdi;
   output tdo;

   input clk;

   parameter WORD_WIDTH = 16;

   wire [WORD_WIDTH-1:0] loop_data;
   wire                  loop_valid;
   wire                  loop_ready;

   glip_jtag_toplevel
      #(.WORD_WIDTH(WORD_WIDTH))
      u_adapter(.fifo_out_ready (loop_ready),
                .fifo_in_valid  (loop_valid),
                .fifo_in_data   (loop_data),
                .fifo_out_valid (loop_valid),
                .fifo_out_data  (loop_data),
                .fifo_in_ready  (loop_ready),

                .rst            (1'b0),
                .com_rst        (),
                .ctrl_logic_rst (),

                /*AUTOINST*/
                // Outputs
                .tdo                    (tdo),
                // Inputs
                .clk                    (clk),
                .tck                    (tck),
                .tms                    (tms),
                .tdi                    (tdi));
endmodule

// Local Variables:
// verilog-library-directories:("." "../../verilog/")
// verilog-auto-inst-param-value: t
// End:

