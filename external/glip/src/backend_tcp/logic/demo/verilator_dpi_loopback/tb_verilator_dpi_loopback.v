

module tb_verilator_dpi_loopback
  (
   input clk,
   input rst
   );

   parameter WIDTH = 16;
   
   glip_channel #(.WIDTH(WIDTH)) u_channel(.clk(clk));
   
   wire             logic_rst;
   wire             com_rst;
   
   /* glip_tcp_toplevel AUTO_TEMPLATE(
    .fifo_out  (u_channel),
    .fifo_in   (u_channel),
    .clk_io    (clk),
    .clk_logic (clk),
    ); */
   glip_tcp_toplevel
     #(.WIDTH(WIDTH))
     u_glip(/*AUTOINST*/
            // Interfaces
            .fifo_out                   (u_channel),             // Templated
            .fifo_in                    (u_channel),             // Templated
            // Outputs
            .logic_rst                  (logic_rst),
            .com_rst                    (com_rst),
            // Inputs
            .clk_io                     (clk),                   // Templated
            .clk_logic                  (clk),                   // Templated
            .rst                        (rst));

endmodule

// Local Variables:
// verilog-library-directories:("." "../../dpi")
// End:
