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
 * The Global Timestamp Provider (GTP) Module
 *
 * This module provides all other debug units with a global timestamp with
 * single-clock-cycle resolution to be able to exactly reconstruct the original
 * order of the collected information.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

module global_timestamp_provider(/*AUTOARG*/
   // Outputs
   timestamp,
   // Inputs
   clk, rst
   );

   parameter TIMESTAMP_WIDTH = `DBG_TIMESTAMP_WIDTH;
   parameter RESET_VALUE = 0;

   input clk;
   input rst;

   output reg[TIMESTAMP_WIDTH-1:0] timestamp;

   always @(posedge clk) begin
      if (rst) begin
         timestamp <= RESET_VALUE;
      end else begin
         timestamp <= timestamp + 1;
      end
   end

endmodule
