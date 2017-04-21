module noc_tracer
  #(
    parameter LINKS = 1
    )
   (
    input 			      clk,
    input 			      rst,

    input [LINKS-1:0][FLIT_WIDTH-1:0] flit,
    input [LINKS-1:0] 		      last,
    input [LINKS-1:0] 		      valid,
    input [LINKS-1:0] 		      ready
    );

   localparam FLIT_WIDTH = 32;
     
   import "DPI-C" function
     void noc_tracer_init(input int links);

   import "DPI-C" function
     void noc_tracer_trace(input int link,
			   input int unsigned flit,
			   input bit 	      last,
			   input longint unsigned timestamp);
   
   initial begin
      noc_tracer_init(LINKS);
   end

   always @(posedge clk) begin
      for (int l = 0; l < LINKS; l++)
	if (valid[l] & ready[l])
	  noc_tracer_trace(l, flit[l], last[l], $time);
   end
endmodule // noc_tracer

