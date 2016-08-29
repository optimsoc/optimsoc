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

module osd_timestamp
  #(parameter WIDTH = 16)
   (
    input                  clk,
    input                  enable,
    input                  rst,

    output reg [WIDTH-1:0] timestamp
    );

   always_ff @(posedge clk)
      if (rst)
        timestamp <= 0;
      else if (enable)
        timestamp <= timestamp + 1;

endmodule // osd_timestamp
