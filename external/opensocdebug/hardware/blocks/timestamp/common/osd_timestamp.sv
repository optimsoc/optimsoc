
module osd_timestamp
  #(parameter WIDTH = 16)
   (
    input                  clk,
    input                  enable,
    input                  rst,
    
    output reg [WIDTH-1:0] timestamp
    );

   always_ff @(posedge clk)
      if (rst)
        timestamp <= 0;
      else if (enable)
        timestamp <= timestamp + 1;

endmodule // osd_timestamp

