/* Copyright (c) 2012-2017 by the author(s)
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
 * The wishbone slave interface to access the simple message passing.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *
 */

module mpbuffer
  import optimsoc_config::*;
  #(parameter config_t CONFIG = 'x,
    parameter SIZE            = 16,
    parameter N               = 1
    )
   (
    input                                clk,
    input                                rst,

    output [N*CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit,
    output [N-1:0]                       noc_out_last,
    output [N-1:0]                       noc_out_valid,
    input [N-1:0]                        noc_out_ready,

    input [N*CONFIG.NOC_FLIT_WIDTH-1:0]  noc_in_flit,
    input [N-1:0]                        noc_in_last,
    input [N-1:0]                        noc_in_valid,
    output [N-1:0]                       noc_in_ready,

    // Bus side (generic)
    input [31:0]                         bus_addr,
    input                                bus_we,
    input                                bus_en,
    input [31:0]                         bus_data_in,
    output reg [31:0]                    bus_data_out,
    output                               bus_ack,
    output                               bus_err,

    output                               irq
    );

   wire [N:0]                            bus_sel_mod;
   wire [N:0]                            bus_err_mod;
   wire [N:0]                            bus_ack_mod;
   wire [N:0][31:0]                      bus_data_mod;
   wire [N-1:0]                          irq_mod;

   genvar                                n;
   generate
      for (n = 0; n <= N; n++) begin
         assign bus_sel_mod[n] = (bus_addr[19:13] == n);
      end
   endgenerate

   always @(*) begin
      bus_data_out = 32'hx;
      for (int i = 0; i <= N; i++) begin
         if (bus_sel_mod[i])
           bus_data_out = bus_data_mod[i];
      end
   end

   assign bus_ack = |bus_ack_mod;
   assign bus_err = |bus_err_mod;
   assign irq = |irq_mod;

   assign bus_ack_mod[0] = bus_en & bus_sel_mod[0] & !bus_we & (bus_addr[12:0] == 0);
   assign bus_err_mod[0] = bus_en & bus_sel_mod[0] & (bus_we | (bus_addr[12:0] != 0));
   assign bus_data_mod[0] = N;

   generate
      for (n = 0; n < N; n++) begin
         mpbuffer_endpoint
               #(.CONFIG(CONFIG), .SIZE(SIZE))
         u_endpoint
               (.*,
                .noc_out_flit  (noc_out_flit[n*CONFIG.NOC_FLIT_WIDTH +: CONFIG.NOC_FLIT_WIDTH]),
                .noc_out_last  (noc_out_last[n]),
                .noc_out_valid (noc_out_valid[n]),
                .noc_out_ready (noc_out_ready[n]),
                .noc_in_flit   (noc_in_flit[n*CONFIG.NOC_FLIT_WIDTH +: CONFIG.NOC_FLIT_WIDTH]),
                .noc_in_last   (noc_in_last[n]),
                .noc_in_valid  (noc_in_valid[n]),
                .noc_in_ready  (noc_in_ready[n]),

                .bus_en        (bus_en & bus_sel_mod[n+1]),
                .bus_data_out  (bus_data_mod[n+1]),
                .bus_ack       (bus_ack_mod[n+1]),
                .bus_err       (bus_err_mod[n+1]),

                .irq           (irq_mod[n])
                );
      end
   endgenerate

endmodule // mpbuffer
