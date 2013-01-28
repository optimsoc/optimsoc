/**
 * This file is part of OpTiMSoC.
 * 
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
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
 * This is the actual memory element implemented as Xilinx Virtex 5 block RAM
 * in the local tile memory.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module ct_ram_xilinxbram_virtex5(/*AUTOARG*/
   // Outputs
   dat_o,
   // Inputs
   clk_i, rst_i, sel_i, adr_i, we_i, dat_i
   );

   parameter dw = 32;

   parameter aw = 23;
   parameter mem_size  = 8388608;

   parameter memory_file = "sram.vmem";

   localparam blocknum = mem_size >> 12;

   input          clk_i;
   input      rst_i;
   input [3:0]      sel_i;
   input [aw-1:0] adr_i;
   input      we_i;
   input [31:0]   dat_i;
   output [31:0]  dat_o;

   wire [31:0]      do_array [0:blocknum-1];

   assign dat_o = do_array[adr_i[aw-1:12]];

   generate
      genvar i;
      for (i=0;i<blocknum;i=i+1) begin
         RAMB36
            #(.READ_WIDTH_A(36),
              .WRITE_WIDTH_A(36))
            mem(.DOA   (do_array[i]),
              .DIA   (dat_i),
              .ADDRA ({1'b0,adr_i[11:2],5'h0}),
              .WEA   ({4{we_i}}&sel_i),
              .ENA   ((adr_i[aw-1:12]==i)),
              .SSRA  (rst_i),
              .REGCEA(),
              .CLKA  (clk_i),

              .DOB   (),
              .DIB   (),
              .ADDRB (),
              .WEB   (),
              .ENB   (),
              .SSRB  (),
              .REGCEB(),
              .CLKB  (),

              .DIPA(),
              .DIPB(),
              .DOPA(),
                .DOPB(),
              .CASCADEOUTLATA (),
              .CASCADEOUTLATB (),
              .CASCADEOUTREGA (),
              .CASCADEOUTREGB (),
              .CASCADEINLATA (),
              .CASCADEINLATB (),
              .CASCADEINREGA (),
              .CASCADEINREGB ());
      end
   endgenerate
endmodule
