`define NORTH 5'b00001
`define EAST  5'b00010
`define SOUTH 5'b00100
`define WEST  5'b01000
`define LOCAL 5'b10000

module mesh(links_in, links_out,
            /*AUTOARG*/
            // Inputs
            clk, rst
            );
   parameter select_router   = 0;   //0 = unicast; 1 = prio unicast; 2 = path-based multicast; 3 = tree-based multicast
   parameter flit_type_width = 3;   // Must be 2 for (0) & (1) or 3 for (2) & (3)
   parameter flit_data_width = 32;
   parameter vchannels = 1;

   parameter xdim = 2;
   parameter ydim = 2;

   parameter ph_prio_width = 4;    //Defines the lenght of the priority field in the header flit if priority unicast routing is selected
   parameter payload_length = 6;    //Maximum packet lenght if recursive partitioning is selected
   parameter out_fifo_length = 8;  //Output FIFO depth if recursive partitioning is selected

   localparam nodes = xdim*ydim;
   parameter dest_width = $clog2(nodes);


   localparam flit_width = flit_data_width + flit_type_width;

   input clk;
   input rst;

   lisnoclink_if links_in [nodes];
   lisnoclink_if links_out [nodes];

   wire [flit_width-1:0] local_in_flit[0:nodes-1];
   wire [vchannels-1:0]  local_in_valid[0:nodes-1];
   wire [vchannels-1:0]  local_in_ready[0:nodes-1];
   wire [flit_width-1:0] local_out_flit[0:nodes-1];
   wire [vchannels-1:0]  local_out_valid[0:nodes-1];
   wire [vchannels-1:0]  local_out_ready[0:nodes-1];
   wire [flit_width-1:0] north_in_flit[0:nodes-1];
   wire [vchannels-1:0]  north_in_valid[0:nodes-1];
   wire [vchannels-1:0]  north_in_ready[0:nodes-1];
   wire [flit_width-1:0] north_out_flit[0:nodes-1];
   wire [vchannels-1:0]  north_out_valid[0:nodes-1];
   wire [vchannels-1:0]  north_out_ready[0:nodes-1];
   wire [flit_width-1:0] east_in_flit[0:nodes-1];
   wire [vchannels-1:0]  east_in_valid[0:nodes-1];
   wire [vchannels-1:0]  east_in_ready[0:nodes-1];
   wire [flit_width-1:0] east_out_flit[0:nodes-1];
   wire [vchannels-1:0]  east_out_valid[0:nodes-1];
   wire [vchannels-1:0]  east_out_ready[0:nodes-1];
   wire [flit_width-1:0] south_in_flit[0:nodes-1];
   wire [vchannels-1:0]  south_in_valid[0:nodes-1];
   wire [vchannels-1:0]  south_in_ready[0:nodes-1];
   wire [flit_width-1:0] south_out_flit[0:nodes-1];
   wire [vchannels-1:0]  south_out_valid[0:nodes-1];
   wire [vchannels-1:0]  south_out_ready[0:nodes-1];
   wire [flit_width-1:0] west_in_flit[0:nodes-1];
   wire [vchannels-1:0]  west_in_valid[0:nodes-1];
   wire [vchannels-1:0]  west_in_ready[0:nodes-1];
   wire [flit_width-1:0] west_out_flit[0:nodes-1];
   wire [vchannels-1:0]  west_out_valid[0:nodes-1];
   wire [vchannels-1:0]  west_out_ready[0:nodes-1];

   genvar x;
   genvar y;

   generate
      for (y=0;y<ydim;y++) begin
         for (x=0;x<xdim;x++) begin
            if (select_router == 0 || select_router == 1) begin

              // Select basic unicast(0) or priority-based(1) unicast router
              lisnoc_router_2dgrid #(.vchannels(vchannels),.num_dests(nodes),.lookup(genlookup(x,y)),
                 .flit_data_width(flit_data_width),.flit_type_width(flit_type_width),.ph_dest_width(dest_width),
                 .use_prio(select_router),.ph_prio_width(ph_prio_width))
                  u_router(
                       .clk (clk),
                       .rst (rst),
                       .local_in_valid_i  (local_in_valid[nodenum(x,y)]),
                       .local_in_flit_i   (local_in_flit[nodenum(x,y)]),
                       .local_in_ready_o  (local_in_ready[nodenum(x,y)]),
                       .local_out_valid_o (local_out_valid[nodenum(x,y)]),
                       .local_out_flit_o  (local_out_flit[nodenum(x,y)]),
                       .local_out_ready_i (local_out_ready[nodenum(x,y)]),
                       .north_in_valid_i  (north_in_valid[nodenum(x,y)]),
                       .north_in_flit_i   (north_in_flit[nodenum(x,y)]),
                       .north_in_ready_o  (north_in_ready[nodenum(x,y)]),
                       .north_out_valid_o (north_out_valid[nodenum(x,y)]),
                       .north_out_flit_o  (north_out_flit[nodenum(x,y)]),
                       .north_out_ready_i (north_out_ready[nodenum(x,y)]),
                       .east_in_valid_i   (east_in_valid[nodenum(x,y)]),
                       .east_in_flit_i    (east_in_flit[nodenum(x,y)]),
                       .east_in_ready_o   (east_in_ready[nodenum(x,y)]),
                       .east_out_valid_o  (east_out_valid[nodenum(x,y)]),
                       .east_out_flit_o   (east_out_flit[nodenum(x,y)]),
                       .east_out_ready_i  (east_out_ready[nodenum(x,y)]),
                       .south_in_valid_i  (south_in_valid[nodenum(x,y)]),
                       .south_in_flit_i   (south_in_flit[nodenum(x,y)]),
                       .south_in_ready_o  (south_in_ready[nodenum(x,y)]),
                       .south_out_valid_o (south_out_valid[nodenum(x,y)]),
                       .south_out_flit_o  (south_out_flit[nodenum(x,y)]),
                       .south_out_ready_i (south_out_ready[nodenum(x,y)]),
                       .west_in_valid_i   (west_in_valid[nodenum(x,y)]),
                       .west_in_flit_i    (west_in_flit[nodenum(x,y)]),
                       .west_in_ready_o   (west_in_ready[nodenum(x,y)]),
                       .west_out_valid_o  (west_out_valid[nodenum(x,y)]),
                       .west_out_flit_o   (west_out_flit[nodenum(x,y)]),
                       .west_out_ready_i  (west_out_ready[nodenum(x,y)])
                       );
            end else if (select_router == 2 || select_router == 3) begin

              // Select hamiltonian path-based(2) or recursive partitioning tree-based(3) multicast router
              lisnoc_router_2dgrid_mc
                  #(.vchannels(vchannels),.destwidth(dest_width),.num_dest(nodes),.lookup(genlookup(x,y)),.flit_data_width(flit_data_width),
                  .flit_type_width(flit_type_width),.myX(x),.myY(y),.rc_size(xdim),.hpmc_rpmc(select_router-2),.lookup_hp(genlookup_hp(x,y)),
                  .myH_num(hamnum(x,y)),.packet_length(payload_length),.out_fifo_length(out_fifo_length))
                  router_v(/*AUTOINST*/
                          .clk (clk),
                          .rst (rst),
                          .local_in_valid_i  (local_in_valid[nodenum(x,y)]),
                          .local_in_flit_i   (local_in_flit[nodenum(x,y)]),
                          .local_in_ready_o  (local_in_ready[nodenum(x,y)]),
                          .local_out_valid_o (local_out_valid[nodenum(x,y)]),
                          .local_out_flit_o  (local_out_flit[nodenum(x,y)]),
                          .local_out_ready_i (local_out_ready[nodenum(x,y)]),
                          .north_in_valid_i  (north_in_valid[nodenum(x,y)]),
                          .north_in_flit_i   (north_in_flit[nodenum(x,y)]),
                          .north_in_ready_o  (north_in_ready[nodenum(x,y)]),
                          .north_out_valid_o (north_out_valid[nodenum(x,y)]),
                          .north_out_flit_o  (north_out_flit[nodenum(x,y)]),
                          .north_out_ready_i (north_out_ready[nodenum(x,y)]),
                          .east_in_valid_i   (east_in_valid[nodenum(x,y)]),
                          .east_in_flit_i    (east_in_flit[nodenum(x,y)]),
                          .east_in_ready_o   (east_in_ready[nodenum(x,y)]),
                          .east_out_valid_o  (east_out_valid[nodenum(x,y)]),
                          .east_out_flit_o   (east_out_flit[nodenum(x,y)]),
                          .east_out_ready_i  (east_out_ready[nodenum(x,y)]),
                          .south_in_valid_i  (south_in_valid[nodenum(x,y)]),
                          .south_in_flit_i   (south_in_flit[nodenum(x,y)]),
                          .south_in_ready_o  (south_in_ready[nodenum(x,y)]),
                          .south_out_valid_o (south_out_valid[nodenum(x,y)]),
                          .south_out_flit_o  (south_out_flit[nodenum(x,y)]),
                          .south_out_ready_i (south_out_ready[nodenum(x,y)]),
                          .west_in_valid_i   (west_in_valid[nodenum(x,y)]),
                          .west_in_flit_i    (west_in_flit[nodenum(x,y)]),
                          .west_in_ready_o   (west_in_ready[nodenum(x,y)]),
                          .west_out_valid_o  (west_out_valid[nodenum(x,y)]),
                          .west_out_flit_o   (west_out_flit[nodenum(x,y)]),
                          .west_out_ready_i  (west_out_ready[nodenum(x,y)])
                         );
            end

            assign local_in_valid[nodenum(x,y)]  = links_in[nodenum(x,y)].valid;
            assign local_in_flit[nodenum(x,y)]   = links_in[nodenum(x,y)].flit;
            assign local_out_ready[nodenum(x,y)] = links_out[nodenum(x,y)].ready;
            always @(*) begin
               links_in[nodenum(x,y)].ready  = local_in_ready[nodenum(x,y)];
               links_out[nodenum(x,y)].valid = local_out_valid[nodenum(x,y)];
               links_out[nodenum(x,y)].flit  = local_out_flit[nodenum(x,y)];
            end

            // Generate connection in north-south direction
            if (y>0 && y<ydim-1) begin
               // North port connection
               assign north_in_valid[nodenum(x,y)]  = south_out_valid[northof(x,y)];
               assign north_in_flit[nodenum(x,y)]   = south_out_flit[northof(x,y)];
               assign north_out_ready[nodenum(x,y)] = south_in_ready[northof(x,y)];
               assign south_in_valid[northof(x,y)]  = north_out_valid[nodenum(x,y)];
               assign south_in_flit[northof(x,y)]   = north_out_flit[nodenum(x,y)];
               assign south_out_ready[northof(x,y)] = north_in_ready[nodenum(x,y)];

               // South port connection
               assign south_in_valid[nodenum(x,y)]  = north_out_valid[southof(x,y)];
               assign south_in_flit[nodenum(x,y)]   = north_out_flit[southof(x,y)];
               assign south_out_ready[nodenum(x,y)] = north_in_ready[southof(x,y)];
               assign north_out_ready[southof(x,y)] = south_in_ready[nodenum(x,y)];
               assign north_in_valid[southof(x,y)]  = south_out_valid[nodenum(x,y)];
               assign north_in_flit[southof(x,y)]   = south_out_flit[nodenum(x,y)];
            end

            if (y==0) begin
               // North port connection
               assign north_in_valid[nodenum(x,y)]  = south_out_valid[northof(x,y)];
               assign north_in_flit[nodenum(x,y)]   = south_out_flit[northof(x,y)];
               assign north_out_ready[nodenum(x,y)] = south_in_ready[northof(x,y)];
               assign south_in_valid[northof(x,y)]  = north_out_valid[nodenum(x,y)];
               assign south_in_flit[northof(x,y)]   = north_out_flit[nodenum(x,y)];
               assign south_out_ready[northof(x,y)] = north_in_ready[nodenum(x,y)];

               // South border of the mesh
               assign south_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
               assign south_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            if (y==ydim-1) begin
               // South port connection
               assign south_in_valid[nodenum(x,y)]  = north_out_valid[southof(x,y)];
               assign south_in_flit[nodenum(x,y)]   = north_out_flit[southof(x,y)];
               assign south_out_ready[nodenum(x,y)] = north_in_ready[southof(x,y)];
               assign north_out_ready[southof(x,y)] = south_in_ready[nodenum(x,y)];
               assign north_in_valid[southof(x,y)]  = south_out_valid[nodenum(x,y)];
               assign north_in_flit[southof(x,y)]   = south_out_flit[nodenum(x,y)];

               // North border of the mesh
               assign north_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
               assign north_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            // Generate connection in east-west direction
            if (x>0 && x<xdim-1) begin
               // West port connection
               assign west_in_valid[nodenum(x,y)]  = east_out_valid[westof(x,y)];
               assign west_in_flit[nodenum(x,y)]   = east_out_flit[westof(x,y)];
               assign west_out_ready[nodenum(x,y)] = east_in_ready[westof(x,y)];
               assign east_out_ready[westof(x,y)]  = west_in_ready[nodenum(x,y)];
               assign east_in_valid[westof(x,y)]   = west_out_valid[nodenum(x,y)];
               assign east_in_flit[westof(x,y)]    = west_out_flit[nodenum(x,y)];

               // East port connection
               assign east_in_valid[nodenum(x,y)]  = west_out_valid[eastof(x,y)];
               assign east_in_flit[nodenum(x,y)]   = west_out_flit[eastof(x,y)];
               assign west_out_ready[eastof(x,y)]  = east_in_ready[nodenum(x,y)];
               assign west_in_valid[eastof(x,y)]   = east_out_valid[nodenum(x,y)];
               assign west_in_flit[eastof(x,y)]    = east_out_flit[nodenum(x,y)];
               assign east_out_ready[nodenum(x,y)] = west_in_ready[eastof(x,y)];
            end

            if (x==0) begin
              // East port connection
              assign east_in_valid[nodenum(x,y)]  = west_out_valid[eastof(x,y)];
              assign east_in_flit[nodenum(x,y)]   = west_out_flit[eastof(x,y)];
              assign west_out_ready[eastof(x,y)]  = east_in_ready[nodenum(x,y)];
              assign west_in_valid[eastof(x,y)]   = east_out_valid[nodenum(x,y)];
              assign west_in_flit[eastof(x,y)]    = east_out_flit[nodenum(x,y)];
              assign east_out_ready[nodenum(x,y)] = west_in_ready[eastof(x,y)];

              // West border of the mesh
              assign west_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
              assign west_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end

            if (x==xdim-1) begin
              // West port connection
              assign west_in_valid[nodenum(x,y)]  = east_out_valid[westof(x,y)];
              assign west_in_flit[nodenum(x,y)]   = east_out_flit[westof(x,y)];
              assign west_out_ready[nodenum(x,y)] = east_in_ready[westof(x,y)];
              assign east_out_ready[westof(x,y)]  = west_in_ready[nodenum(x,y)];
              assign east_in_valid[westof(x,y)]   = west_out_valid[nodenum(x,y)];
              assign east_in_flit[westof(x,y)]    = west_out_flit[nodenum(x,y)];

              // East border of the mesh
              assign east_in_valid[nodenum(x,y)]  = {vchannels{1'b0}};
              assign east_out_ready[nodenum(x,y)] = {vchannels{1'b0}};
            end
         end
      end

   endgenerate

   function integer nodenum(int x,int y);
      nodenum = x+y*xdim;
   endfunction // nodenum

   function integer hamnum(int x,int y);
      if (y%2==0) begin
         hamnum = x+y*xdim;
      end else begin
         hamnum = y*xdim+(xdim-x-1);
      end
   endfunction // nodenum

   function integer northof(int x,int y);
      northof = x+(y+1)*xdim;
   endfunction // northof
   function integer eastof(int x,int y);
      eastof  = (x+1)+y*xdim;
   endfunction // eastof
   function integer southof(int x,int y);
      southof = x+(y-1)*xdim;
   endfunction // southof
   function integer westof(int x,int y);
      westof = (x-1)+y*xdim;
   endfunction // westof

   // This generates the lookup table for each single node
   function [nodes*5-1:0] genlookup(int x,int y);
      genlookup = {nodes{5'b00000}};

      for (int yd=0;yd<xdim;yd++) begin            // x-destination
         for (int xd=0;xd<xdim;xd++) begin         // y-destination

            automatic int nd = nodenum(xd,yd);
            automatic bit [4:0] d = 5'b00000;
            if ((xd==x) && (yd==y)) begin
               d = `LOCAL;
            end else if (xd==x) begin
               if (yd<y) begin
                  d = `SOUTH;
               end else begin
                  d = `NORTH;
               end
            end else begin
               if (xd<x) begin
                  d = `WEST;
               end else begin
                  d = `EAST;
               end
            end // else: !if(xd==x)
            genlookup |= (d<<((nodes-nd-1)*5));
         end
      end
   endfunction

   // This generates the lookup table for the hamiltonian multicast
   function [nodes*5-1:0] genlookup_hp(int x,int y);
       automatic int hp;
       genlookup_hp     = {nodes{5'b00000}};
       hp = hamnum(x,y);

       for (int xd=0;xd<xdim;xd++) begin
          for (int yd=0;yd<xdim;yd++) begin

             automatic int hd = hamnum(xd,yd);
             automatic int nd = nodenum(xd,yd);
             automatic bit [4:0] d = 5'b00000;

             if (hd==hp) begin
               d = `LOCAL;
             end else if (hd>hp) begin
               if (y%2!=0) begin //ungerade
                 if (x==0) begin
                   d = `NORTH;
                 end else begin
                   d = `WEST;
                 end
               end else begin    //gerade
                 if (x==xdim-1) begin
                    d = `NORTH;
                 end else begin
                    d = `EAST;
                 end
               end
             end else begin // hd<hp
               if (y%2!=0) begin   //ungerade
                 if (x==xdim-1) begin
                   d = `SOUTH;
                 end else begin
                   d = `EAST;
                 end
               end else begin   //gerade
                 if (x==0) begin
                    d = `SOUTH;
                 end else begin
                    d = `WEST;
                 end
               end
             end
             genlookup_hp |= (d<<((nodes-nd-1)*5));
          end
       end
    endfunction



endmodule // mesh
