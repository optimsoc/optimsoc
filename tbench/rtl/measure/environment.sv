`ifndef ENVIRONMENT_SV
`define ENVIRONMENT_SV

class environment #(flit_data_width=32,flit_dest_width=4,vchannels=1,mc_supported,mc_num_dest=16);
   trafficgen
     #(.flit_data_width(flit_data_width),
       .flit_dest_width(flit_dest_width),
       .vchannels(vchannels),
       .mc_supported(mc_supported),
       .mc_num_dest(mc_num_dest))
   traffic[][]; // index 0: node, index 1: traffic gen at node

   sink #(flit_data_width,flit_dest_width,vchannels,mc_supported,mc_num_dest) sinks[];

   networkacc
     #(.flit_data_width(flit_data_width),
       .flit_dest_width(flit_dest_width),
       .vchannels(vchannels),
       .mc_supported(mc_supported),
       .mc_num_dest(mc_num_dest)) netacc[];

   measurement #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) measure;
   sysconfig conf;

   function new(input virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) links_out[],
                input virtual lisnoclink_if #(.vchannels(vchannels),.flit_width(flit_data_width+2+mc_supported)) links_in[]);

      if (mc_supported == 1 && vchannels < 2) begin
         $fatal("Parameter mismatch! ==> mc_supported vs. vchannels");
      end

      conf = sysconfig::get();

      traffic = new[links_out.size()];
      sinks = new[links_in.size()];
      netacc = new[links_out.size()];
      measure = new();

      foreach (links_out[i]) begin
         trafficdesc td;

         netacc[i]  = new(links_out[i],measure);
         $display("%d %d",i,conf.getTrafficNum(i));

         traffic[i] = new[conf.getTrafficNum(i)];
         for (int t=0;t<conf.getTrafficNum(i);t++) begin
            td = conf.getTraffic(i,t);
            traffic[i][t] = new(i,td.name,netacc[i],td.param);
         end
      end
      foreach (links_in[i]) begin
         sinks[i] = new(i,links_in[i],measure);
      end

   endfunction // new

   task run();
      for (integer i=0;i<traffic.size();i++) begin
         automatic int id = i;
         fork
            for (integer t=0;t<this.traffic[id].size();t++) begin
               automatic int tid = t;
               fork
                  this.traffic[id][tid].run();
               join_none
            end
            this.netacc[id].run();
            this.sinks[id].run();
         join_none
      end

      wait fork;

      endtask

endclass // environment

`endif