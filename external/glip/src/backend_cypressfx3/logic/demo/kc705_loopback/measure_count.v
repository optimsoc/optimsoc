
module bcd_count_0
  #(
    parameter STEP = 2 // STEP can only be 2 or 4
    )
  (
   input clk, rst,
   input valid,
   output reg [3:0] count,
   output reg carry
   );

   always @(posedge clk) begin
      if (rst) begin
	 count <= valid ? STEP : 0;
	 carry <= 0;
      end else begin
	 if (valid) begin
	    if (((STEP == 2) && (count == 8)) ||
		((STEP == 4) && (count >= 6 ))) begin
	       carry <= 1;
	       if (STEP == 2) begin
		  count <= 0;
	       end else begin
		  count <= (count == 6) ? 0 : 2;
	       end
	    end else begin
	       count <= count + STEP;
	       carry <= 0;
	    end
	 end else begin // if (valid)
	    carry <= 0;
	 end
      end
   end
endmodule // bcd_count_0

module bcd_count
  (
   input clk, rst,
   input valid,
   output reg [3:0] count,
   output reg carry
   );

   always @(posedge clk) begin
      if (rst) begin
	 count <= 0;
      end else begin
	 if (valid) begin
	    if (count == 9) begin
	       carry <= 1;	       
	       count <= 0;
	    end else begin
	       carry <= 0;
	       count <= count + 1;
	    end
	 end else begin
	    carry <= 0;
	 end
      end
   end
endmodule

module measure_count
  #(
    parameter STEP = 2 // STEP can only be 2 or 4
    )
  (
   input 	clk, rst,
   input 	valid, 

   output reg [3:0] sample_3,
   output reg [3:0] sample_4,
   output reg [3:0] sample_5,
   output reg [3:0] sample_6,
   output reg [3:0] sample_7,
   output reg [3:0] sample_8
   );

   wire [3:0] 		bcd_count_0;
   wire [3:0]		bcd_count_1;
   wire [3:0]		bcd_count_2;
   wire [3:0]		bcd_count_3;
   wire [3:0]		bcd_count_4;
   wire [3:0]		bcd_count_5;
   wire [3:0]		bcd_count_6;
   wire [3:0]		bcd_count_7;
   wire [3:0]		bcd_count_8;

   wire [7:0] 		carry;

   always @(posedge clk) begin
      if (rst) begin
	 sample_3 <= bcd_count_3;
	 sample_4 <= bcd_count_4;
	 sample_5 <= bcd_count_5;
	 sample_6 <= bcd_count_6;
	 sample_7 <= bcd_count_7;
	 sample_8 <= bcd_count_8;	 
      end
   end
   
   bcd_count_0 #(.STEP(STEP)) u_count0
     (.clk(clk), .rst(rst), .valid(valid),
      .count(bcd_count_0), .carry(carry[0]));
   
   bcd_count bcd_count1
     (.clk(clk), .rst(rst), .valid(carry[0]),
      .count(bcd_count_1), .carry(carry[1]));
   
   bcd_count bcd_count2
     (.clk(clk), .rst(rst), .valid(carry[1]),
      .count(bcd_count_2), .carry(carry[2]));
   
   bcd_count bcd_count3
     (.clk(clk), .rst(rst), .valid(carry[2]),
      .count(bcd_count_3), .carry(carry[3]));
   
   bcd_count bcd_count4
     (.clk(clk), .rst(rst), .valid(carry[3]),
      .count(bcd_count_4), .carry(carry[4]));
   
   bcd_count bcd_count5
     (.clk(clk), .rst(rst), .valid(carry[4]),
      .count(bcd_count_5), .carry(carry[5]));
   
   bcd_count bcd_count6
     (.clk(clk), .rst(rst), .valid(carry[5]),
      .count(bcd_count_6), .carry(carry[6]));
   
   bcd_count bcd_count7
     (.clk(clk), .rst(rst), .valid(carry[6]),
      .count(bcd_count_7), .carry(carry[7]));
   
   bcd_count bcd_count8
     (.clk(clk), .rst(rst), .valid(carry[7]),
      .count(bcd_count_8), .carry());

endmodule // measure_count
