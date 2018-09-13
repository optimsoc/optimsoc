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
//    Wei Song <ws327@cam.ac.uk>
//    Stefan Wallentowitz <stefan@wallentowitz.de>

package dii_package;

   typedef struct packed unsigned {
      logic       valid;
      logic       last;
      logic [15:0] data;
   } dii_flit;

   function automatic dii_flit
     dii_flit_assemble(
                       input logic        m_valid,
                       input logic        m_last,
                       input logic [15:0] m_data
                       );
      return dii_flit'{m_valid, m_last, m_data};
   endfunction

endpackage // dii_package
