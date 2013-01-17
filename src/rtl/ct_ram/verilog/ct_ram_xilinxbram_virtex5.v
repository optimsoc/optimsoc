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
