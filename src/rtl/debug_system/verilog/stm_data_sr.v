/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Submodule of the Software Trace Module (STM): the data shift register
 *
 * All incoming data is delayed by DELAY_CYCLES before passing it to the output.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

module stm_data_sr(/*AUTOARG*/
   // Outputs
   dout,
   // Inputs
   clk, rst, din
   );

   parameter DELAY_CYCLES = `DBG_TRIGGER_DELAY;
   parameter DATA_WIDTH = `DBG_TIMESTAMP_WIDTH+32+16;

   input clk;
   input rst;

   input [DATA_WIDTH-1:0] din;
   output [DATA_WIDTH-1:0] dout;

   reg [(DATA_WIDTH*(DELAY_CYCLES+1))-1:0] sr;

   assign dout = sr[(DATA_WIDTH*(DELAY_CYCLES+1))-1:(DATA_WIDTH*DELAY_CYCLES)];

   always @ (posedge clk) begin
      if (rst) begin
         sr <= 0;
      end else begin
         sr <= sr << DATA_WIDTH;
         sr[DATA_WIDTH-1:0] <= din;
      end
   end
endmodule
