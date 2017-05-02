
module noc_vchannel_mux
  #(parameter FLIT_WIDTH = 32,
    parameter CHANNELS = 2)
   (
    input 				 clk,
    input 				 rst,

    input [CHANNELS-1:0][FLIT_WIDTH-1:0] in_flit,
    input [CHANNELS-1:0] 		 in_last,
    input [CHANNELS-1:0] 		 in_valid,
    output [CHANNELS-1:0] 		 in_ready,
   
    output [FLIT_WIDTH-1:0] 		 out_flit,
    output 				 out_last,
    output [CHANNELS-1:0] 		 out_valid,
    input [CHANNELS-1:0] 		 out_ready
    );

   reg [CHANNELS-1:0] 			 select;
   logic [CHANNELS-1:0] 		 nxt_select;   

   assign out_valid = in_valid & select;
   assign in_ready  = out_ready & select;

   always @(*) begin
      out_flit = 'x;
      out_last = 'x;
      for (int c = 0; c < CHANNELS; c++) begin
	 if (select[c]) begin
	    out_flit = in_flit[c];
	    out_last = in_last[c];
	 end
      end
   end
   
   arb_rr
     #(.N (CHANNELS))
   u_arbiter
     (.req (in_valid & out_ready),
      .en  (1),
      .gnt (select),
      .nxt_gnt (nxt_select));

   always_ff @(posedge clk) begin
      if (rst) begin
	 select <= {{CHANNELS-1{1'b0}},1'b1};
      end else begin
	 select <= nxt_select;
      end
   end
   
endmodule // noc_vchannel_mux

