

module tb_verilator_dpi_stress_test
(
   input clk,
   input rst
);

   parameter WIDTH = 16;
   
   glip_channel #(.WIDTH(WIDTH)) u_in_channel(.clk(clk));
   glip_channel #(.WIDTH(WIDTH)) u_out_channel(.clk(clk));
   
   wire             logic_rst;
   wire             com_rst;
   
   glip_tcp_toplevel
      #(.WIDTH(WIDTH))
   u_glip(/*AUTOINST*/
      // Interfaces
      .fifo_out                   (u_out_channel),
      .fifo_in                    (u_in_channel),
      // Outputs
      .logic_rst                  (logic_rst),
      .com_rst                    (com_rst),
      // Inputs
      .clk_io                     (clk),                   // Templated
      .clk_logic                  (clk),                   // Templated
      .rst                        (rst));
     
     
   io_stress_test
      #(.WIDTH(WIDTH))
   u_stress_test(
      .clk              (clk),
      .rst              (rst),
      .fifo_out_ready   (u_out_channel.master.ready),
      .fifo_out_valid   (u_out_channel.master.valid),
      .fifo_out_data    (u_out_channel.master.data),
      .fifo_in_valid    (u_in_channel.slave.valid),
      .fifo_in_data     (u_in_channel.slave.data),
      .fifo_in_ready    (u_in_channel.slave.ready),
      .stall_flag       (),
      .error            (),
      .idle             ());

endmodule

// Local Variables:
// verilog-library-directories:("." "../../dpi")
// End:
