interface lisnoclink_if(input clk);
   parameter flit_width = 34;
   parameter vchannels = 1;

   logic [flit_width-1:0] flit;
   logic [vchannels-1:0]  ready;
   logic [vchannels-1:0]  valid;

   initial begin
      ready = 0;
      valid = 0;
   end

   task send(input logic [flit_width-1:0] sflit,
             input integer vc);
      flit = sflit;
      valid = {vchannels{1'b0}};
      valid[vc] = 1'b1;
      @(posedge clk iff ready[vc]);
      valid = {vchannels{1'b0}};
   endtask

   task receive(output logic [flit_width-1:0] rflit,
                input logic [vchannels-1:0] vcready={vchannels{1'b1}});
      ready = vcready;
      @(negedge clk iff |valid);
      rflit = flit;
      @(posedge clk);

      ready = {vchannels{1'b0}};
   endtask // receive

   modport tx(input flit,input valid,output ready);
   modport rx(output flit,output valid,input ready);

endinterface // noclink_if
