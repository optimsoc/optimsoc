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

package opensocdebug;

   typedef struct packed {
      logic [31:0] insn;
      logic [31:0] pc;
      logic        jb;
      logic        jal;
      logic        jr;
      logic [31:0] jbtarget;
      logic        valid;
      logic [31:0] wbdata;
      logic [4:0]  wbreg;
      logic        wben;
   } mor1kx_trace_exec;

endpackage // opensocdebug
