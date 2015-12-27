`ifndef SINK_SV
`define SINK_SV

class sink #(int flit_data_width=32,flit_dest_width=5,vchannels=1,mc_supported,mc_num_dests);

   virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) link;
   measurement #(flit_data_width,flit_dest_width,mc_supported,mc_num_dests) measure;
   integer id;

   function new(integer id,virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) link,measurement #(flit_data_width,flit_dest_width,mc_supported,mc_num_dests) measure);
      this.id = id;
      this.link = link;
      this.measure = measure;
   endfunction // new

   task run();
      logic [33:0] flit;
      integer      packetid=0;
      integer      h_detect=0;
      bit [vchannels-1:0] vc={vchannels{1'b0}};

      forever begin
         link.receive(flit);
         if (flit[32]) begin  // If header flit ==> store packet ID
           h_detect = 1;
           vc = link.valid;
         end

         if ((flit[33:32] == 2'b00) && h_detect==1 && vc==link.valid) begin
           packetid = flit[31:0];
           h_detect = 0;
         end

         if (flit[33]) begin  // If tail flit ==> send packet ID to class measure
            measure.received(packetid);
            h_detect = 0;
         end
      end
   endtask
endclass // sink

`endif