module mor1kx_simple_dpram_sclk
#(
   parameter ADDR_WIDTH = 32,
   parameter DATA_WIDTH = 32,
   parameter CLEAR_ON_INIT = 0,
   parameter ENABLE_BYPASS = 1
)
(
   input 		    clk,
   input [ADDR_WIDTH-1:0]  raddr,
   input 		    re,
   input [ADDR_WIDTH-1:0]  waddr,
   input 		    we,
   input [DATA_WIDTH-1:0]  din,
   output [DATA_WIDTH-1:0] dout
 );

   // byte select width
   localparam SW = DATA_WIDTH / 8;

   // Is the data width byte-sized?
   localparam DATA_WIDTH_IS_BYTE = (DATA_WIDTH % 8 == 0);

   // word address width
   localparam WORD_AW = ADDR_WIDTH - (SW >> 1);


   generate
      if (DATA_WIDTH_IS_BYTE) begin
         logic [WORD_AW-1:0] waddr_word;
         logic [WORD_AW-1:0] raddr_word;
         assign waddr_word = waddr[ADDR_WIDTH-1:(ADDR_WIDTH-WORD_AW)];
         assign raddr_word = raddr[ADDR_WIDTH-1:(ADDR_WIDTH-WORD_AW)];

         sram_sdp_sclk
            #(.AW(ADDR_WIDTH), .DW(DATA_WIDTH))
            u_impl(
               .clk(clk),
               .rst(1'b0),
               .ce_a(1'b1),
               .we_a(we),
               .waddr_a(waddr_word),
               .din_a(din),
               .sel_a({SW{1'b0}}),
               .ce_b(1'b1),
               .oe_b(re),
               .waddr_b(raddr_word),
               .dout_b(dout));
      end else begin
         // not supported right now; should forward to original implementation
         // possibly (or one without byte select?)
         initial begin
            $display("Not supported right now.");
            $fatal;
         end
      end
   endgenerate

endmodule
