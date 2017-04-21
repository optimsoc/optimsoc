module noc_tracer
  #(
    parameter LINKS = 1,
    parameter CHANNELS = 1
    )
   (
    input                             clk,
    input                             rst,

    input [LINKS-1:0][1:0][CHANNELS-1:0][FLIT_WIDTH-1:0] flit,
    input [LINKS-1:0][1:0][CHANNELS-1:0]                 last,
    input [LINKS-1:0][1:0][CHANNELS-1:0]                 valid,
    input [LINKS-1:0][1:0][CHANNELS-1:0]                 ready
    );

   localparam FLIT_WIDTH = 32;
     
   import "DPI-C" function
     void noc_tracer_init(input int links);

   import "DPI-C" function
     void noc_tracer_trace(input int              link,
                           input int unsigned 	  flit,
                           input bit 		  last,
                           input longint unsigned timestamp);
   
   initial begin
      noc_tracer_init(LINKS*CHANNELS*2);
   end

   always @(posedge clk) begin
      for (int l = 0; l < LINKS; l++) begin
	 for (int d = 0; d < 2; d++) begin
	    for (int c = 0; c < CHANNELS; c++) begin
               if (valid[l][d][c] & ready[l][d][c]) begin
		 noc_tracer_trace((l*CHANNELS+c)*2+d, flit[l][d][c], last[l][d][c], $time);
	       end
	    end
	 end
      end
   end
endmodule // noc_tracer

