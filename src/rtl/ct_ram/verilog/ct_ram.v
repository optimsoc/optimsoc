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
 * This is a RAM module used for local tile memory. It is mostly taken
 * from the ORPSoCv2 project (module wb_ram_b3) of opencores.org.
 * 
 * (c) by the author(s)
 * 
 * Author(s):
 *    Opencores.org authors of wb_ram_b3
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */


module ct_ram(/*AUTOARG*/
   // Outputs
   wb_ack_o, wb_err_o, wb_rty_o, wb_dat_o,
   // Inputs
   wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i, wb_clk_i, wb_rst_i
   );

   // Memory parameters
   /* data width (word size) */
   parameter dw = 32;
   /* address width */
   parameter aw = 23;
   /* memory size in Bytes */
   parameter mem_size  = 'hx;


   parameter memory_file = "sram.vmem";

   input [aw-1:0]       wb_adr_i;
   input [1:0]          wb_bte_i;
   input [2:0]          wb_cti_i;
   input                wb_cyc_i;
   input [dw-1:0]       wb_dat_i;
   input [3:0]          wb_sel_i;
   input                wb_stb_i;
   input                wb_we_i;

   output               wb_ack_o;
   output               wb_err_o;
   output               wb_rty_o;
   output [dw-1:0]      wb_dat_o;

   input                wb_clk_i;
   input                wb_rst_i;

   //reg [aw-1:2] wb_adr_i_r;
   reg [aw-1:0] adr;

   // Register to indicate if the cycle is a Wishbone B3-registered feedback
   // type access
   reg                             wb_b3_trans;
   wire                            wb_b3_trans_start, wb_b3_trans_stop;

   // Register to use for counting the addresses when doing burst accesses
   reg [aw-1:0]  burst_adr_counter;
   reg [2:0]                       wb_cti_i_r;
   reg [1:0]                       wb_bte_i_r;
   wire                            using_burst_adr;
   wire                            burst_access_wrong_wb_adr;

   reg                             random_ack_negate;

`ifdef OPTIMSOC_CTRAM_PLAIN
   /* ct_ram_plain AUTO_TEMPLATE(
    .clk_i (wb_clk_i),
    .dat_o (wb_dat_o[]),
    .sel_i (wb_sel_i[]),
    .adr_i (adr[aw-3:0]),
    .we_i  (wb_we_i & wb_ack_o),
    .dat_i (wb_dat_i[]),
    ); */
   ct_ram_plain
     #(.dw(dw),.aw(aw),.mem_size(mem_size),.memory_file(memory_file))
   memory(/*AUTOINST*/
          // Outputs
          .dat_o                        (wb_dat_o[31:0]),        // Templated
          // Inputs
          .clk_i                        (wb_clk_i),              // Templated
          .sel_i                        (wb_sel_i[3:0]),         // Templated
          .adr_i                        (adr[aw-3:0]),           // Templated
          .we_i                         (wb_we_i & wb_ack_o),    // Templated
          .dat_i                        (wb_dat_i[31:0]));        // Templated
`elsif OPTIMSOC_CTRAM_PLAINBETTER
   bram_infer
     #(.dw(dw),.aw(aw),.mem_size(mem_size))
   memory(/*AUTOINST*/
          // Outputs
          .dat_o                        (wb_dat_o[31:0]),        // Templated
          // Inputs
          .clk_i                        (wb_clk_i),              // Templated
          .sel_i                        (wb_sel_i[3:0]),         // Templated
          .adr_i                        (adr[aw-3:0]),           // Templated
          .we_i                         (wb_we_i & wb_ack_o),    // Templated
          .dat_i                        (wb_dat_i[31:0]));        // Templated
`elsif OPTIMSOC_CTRAM_XILINXBRAM
   /* ct_ram_xilinxbram_virtex5 AUTO_TEMPLATE(
    .clk_i (wb_clk_i),
    .rst_i (wb_rst_i),
    .dat_o (wb_dat_o[]),
    .sel_i (wb_sel_i[]),
    .adr_i ({adr[aw-1:2],2'b00}),
    .we_i  (wb_we_i & wb_ack_o),
    .dat_i (wb_dat_i[]),
    ); */
  ct_ram_xilinxbram_virtex5
    #(.dw(dw),.aw(aw),.mem_size(mem_size))
   memory(/*AUTOINST*/
          // Outputs
          .dat_o                        (wb_dat_o[31:0]),        // Templated
          // Inputs
          .clk_i                        (wb_clk_i),              // Templated
          .rst_i                        (wb_rst_i),              // Templated
          .sel_i                        (wb_sel_i[3:0]),         // Templated
          .adr_i                        ({adr[aw-1:2],2'b00}),   // Templated
          .we_i                         (wb_we_i & wb_ack_o),    // Templated
          .dat_i                        (wb_dat_i[31:0]));       // Templated
`elsif OPTIMSOC_CTRAM_WIRES
   // wiring will be done per hyperconnect in top level
`else // !`ifdef OPTIMSOC_CTRAM_PLAIN
   // synthesis translate_off
   $display("Set an OpTiMSoC compute tile RAM implementation.");
   $finish();
   // synthesis translate_on
`endif


   // Logic to detect if there's a burst access going on
   assign wb_b3_trans_start = ((wb_cti_i == 3'b001)|(wb_cti_i == 3'b010)) &
                              wb_stb_i & !wb_b3_trans;

   assign  wb_b3_trans_stop = (wb_cti_i == 3'b111) &
                              wb_stb_i & wb_b3_trans & wb_ack_o;

   always @(posedge wb_clk_i)
     if (wb_rst_i)
       wb_b3_trans <= 0;
     else if (wb_b3_trans_start)
       wb_b3_trans <= 1;
     else if (wb_b3_trans_stop)
       wb_b3_trans <= 0;

   // Burst address generation logic
   always @*
     if (wb_rst_i) begin
       burst_adr_counter = 0;
     end else begin
       burst_adr_counter = adr;
        if (wb_b3_trans_start)
          burst_adr_counter = {2'b00,wb_adr_i[aw-1:2]};
        else if ((wb_cti_i_r == 3'b010) & wb_ack_o & wb_b3_trans)
          // Incrementing burst
          begin
             if (wb_bte_i_r == 2'b00) // Linear burst
               burst_adr_counter = adr + 1;
             if (wb_bte_i_r == 2'b01) // 4-beat wrap burst
               burst_adr_counter[1:0] = adr[1:0] + 1;
             if (wb_bte_i_r == 2'b10) // 8-beat wrap burst
               burst_adr_counter[2:0] = adr[2:0] + 1;
             if (wb_bte_i_r == 2'b11) // 16-beat wrap burst
               burst_adr_counter[3:0] = adr[3:0] + 1;
          end // if ((wb_cti_i_r == 3'b010) & wb_ack_o_r)
        else if (!wb_ack_o & wb_b3_trans)
          burst_adr_counter = adr;
     end


   always @(posedge wb_clk_i)
     wb_bte_i_r <= wb_bte_i;

   // Register it locally
   always @(posedge wb_clk_i)
     wb_cti_i_r <= wb_cti_i;

   assign using_burst_adr = wb_b3_trans;

   assign burst_access_wrong_wb_adr = (using_burst_adr & (adr != {2'b00,wb_adr_i[aw-1:2]}));

   // Address registering logic
   always@(posedge wb_clk_i)
     if(wb_rst_i)
       adr <= 0;
     else if (using_burst_adr)
       adr <= burst_adr_counter;
     else if (wb_cyc_i & wb_stb_i)
       adr <= {2'b00,wb_adr_i[aw-1:2]};

   assign wb_rty_o = 0;

   wire ram_we;
   assign ram_we = wb_we_i & wb_ack_o;

   // Ack Logic
   reg wb_ack_o_r;

   assign wb_ack_o = wb_ack_o_r & wb_stb_i;

   always @(posedge wb_clk_i)
     if (wb_rst_i)
       begin
          wb_ack_o_r <= 1'b0;
       end
     else if (wb_cyc_i) // We have bus
       begin
          if (wb_cti_i == 3'b111)
            begin
               // End of burst
               if (wb_ack_o_r)
                 // ALWAYS de-assert ack after burst end
                 wb_ack_o_r <= 0;
               else if (wb_stb_i & !random_ack_negate)
                 wb_ack_o_r <= 1;
               else
                 wb_ack_o_r <= 0;
            end
          else if (wb_cti_i == 3'b000)
            begin
               // Classic cycle acks
               if (wb_stb_i & !random_ack_negate)
                 begin
                    if (!wb_ack_o_r)
                      wb_ack_o_r <= 1;
                    else
                      wb_ack_o_r <= 0;
                 end
               else
                 wb_ack_o_r <= 0;
            end // if (wb_cti_i == 3'b000)
          else if ((wb_cti_i == 3'b001) | (wb_cti_i == 3'b010))
            begin
               // Increment/constant address bursts
               if (wb_stb_i & !random_ack_negate)
                 wb_ack_o_r <= 1;
               else
                 wb_ack_o_r <= 0;
            end
          else if (wb_cti_i == 3'b111)
            begin
               // End of cycle
               if (wb_stb_i & !random_ack_negate)
                 wb_ack_o_r <= 1;
               else
                 wb_ack_o_r <= 0;
            end
       end // if (wb_cyc_i)
     else
       wb_ack_o_r <= 0;

   assign wb_err_o = 1'b0;// wb_ack_o & (burst_access_wrong_wb_adr); // OR in other errors here


   // Random ACK negation logic
`ifdef RANDOM_ACK_NEGATION
   reg [31:0] lfsr;
   always @(posedge wb_clk_i)
     if (wb_rst_i)
       lfsr <= 32'h273e2d4a;
     else lfsr <= {lfsr[30:0], ~(lfsr[30]^lfsr[6]^lfsr[4]^lfsr[1]^lfsr[0])};

   always @(posedge wb_clk_i)
     random_ack_negate <= lfsr[26];

`else
   always @(wb_rst_i)
     random_ack_negate = 0;
`endif



endmodule