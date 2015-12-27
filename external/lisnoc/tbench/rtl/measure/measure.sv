`timescale 1ns/1ps

`include "measure.svh"

/**
 * The measure module is the toplevel for the measurement. It
 * instantiates NoC itself and sets up the measurement environment.
 */
module measure();

   // NoC dimension parameters
   parameter xdim = 4;
   parameter ydim = 4;

   localparam nodes = xdim*ydim; // The number of nodes

   // The width of the links in the system
   parameter link_width = 32;

   // Number of virtual channels
   parameter vchannels = 1;

   // Select the router implementation:
   // 0: unicast, no priorization
   // 1: unicast, priorization
   // 2: multicast,
   // 3: multicast,
   parameter select_router   = 0;
   localparam payload_length = (select_router == 3) ? 5 : 0;
   localparam out_fifo_len   = (select_router == 3) ? 8 : 4;

   // Helper parameter that indicates whether hardware supports multicasts
   localparam mc_supported = (select_router < 2) ? 0 : 1;

   // Number of packets to simulate
   parameter numPackets = 10000;

   // Traffic generator configuration
   parameter traffic = "*:uniform(0.15);5:uniform(0.05)";
//   parameter traffic = "0:uniform(0.01,{1});1:uniform(0.1,{2});2:uniform(0.2,{15})";
//   parameter traffic = "0:uniform(0.01,{1-2})";
//   parameter traffic = "5:multicast(0.01,{1-2-3-4-6-7-8-10-11-12},{1-2})";
//   parameter traffic = "*:uniform(0.01,{*},{0-1});5:multicast(0.15,{1-2-3-4-6-7-8},{2-3})";
//   parameter traffic = "*:uniform(0.01,{*},{0-1});*:multicast(0.01,{*},{2-3})";


   // General flit format
   // +------+------------+
   // | type |    data    |
   // +------+------------+
   //    |      |-> link width
   //    |-> 2 bit + 1 bit when multicast

   parameter flit_data_width = link_width;
   parameter flit_type_width = 2+mc_supported;
   parameter flit_width = flit_data_width + flit_type_width;

   // Clock and reset
   reg clk;
   reg rst;

   // Interfaces that connect measurement with the mesh
   lisnoclink_if
     #(.vchannels(vchannels),
       .flit_width(flit_width))
   links_out[0:nodes-1] (.clk(clk));

   lisnoclink_if
     #(.vchannels(vchannels),
       .flit_width(flit_width))
   links_in[0:nodes-1] (.clk(clk));

   // The environment instantiates everything
   measurepkg::environment
     #(.flit_data_width(flit_data_width),
       .flit_dest_width($clog2(xdim*ydim)),
       .vchannels(vchannels),
       .mc_supported(mc_supported),
       .mc_num_dest(xdim*ydim)) env;

   // Initially set clock and do reset
   initial begin
      clk = 1;
      rst = 1;
      #5 rst = 0;
   end

   // Generate pseudo 1 GHz clock (time base 1ns)
   always clk = #500ps ~clk;

   initial begin
      // System configuration is a singleton that only exists once.
      static measurepkg::sysconfig conf = measurepkg::sysconfig::get();
      // Set the respective values
      conf.numPackets    = numPackets;
      conf.xdim          = xdim;
      conf.ydim          = ydim;
      conf.nodes         = xdim*ydim;
      conf.vchannels     = vchannels;
      conf.select_router = select_router;
      // Set the traffic generator configuration. It is parsed and
      // the environment creates the respective generators.
      conf.setTraffic(traffic);

      // Create the environment
      env = new(links_out,links_in);

      // Wait until end of reset of hardware and run environment
      @(negedge rst) env.run();
   end

   // Generate the mesh (with the selected router)
   mesh
     #(.xdim(xdim),.ydim(ydim),
       .flit_type_width(flit_type_width),
       .select_router(select_router),
       .vchannels(vchannels),
       .payload_length(payload_length),
       .out_fifo_length(out_fifo_len))
   u_mesh(.clk (clk),
          .rst (rst),
          .links_out (links_in),
          .links_in  (links_out));

endmodule // measure
