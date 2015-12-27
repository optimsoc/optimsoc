`ifndef NETWORKACC_SV
`define NETWORKACC_SV

class networkacc #(integer flit_data_width,flit_dest_width,vchannels,mc_supported,mc_num_dest);

   virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) link;
   measurement #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) measure;

   mailbox queue;

   function new(virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) link,measurement #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) m);
      this.link = link;
      queue = new();
      measure = m;
   endfunction // new

   task send(packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) p);
      p.gen = $time+1ns;
      measure.add(p);
      queue.put(p);
   endtask // send

   task run();
      packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) p;
      flit #(mc_supported) f;

      forever begin
         queue.get(p);
         f = p.getflit();

         link.send(f.get(),p.vc);
         p.net = $time;

         for (int i=0;i<p.numtotalflits()-1;i++) begin
            link.send(p.getflit().get(),p.vc);
         end
      end
   endtask // run

endclass // networkacc
`endif