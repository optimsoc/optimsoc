`ifndef TRAFFICGEN_SV
`define TRAFFICGEN_SV

/**
 * This class generates traffic.
 */
class trafficgen #(int flit_data_width=32,flit_dest_width=5,vchannels=1,mc_supported,mc_num_dest);
   sysconfig conf;
   integer valid_dests[];
   bit [mc_num_dest-1:0] valid_dests_mc  = {mc_num_dest{1'b1}};
   integer valid_vchannels[];
   integer id;
   networkacc #(flit_data_width,flit_dest_width,vchannels,mc_supported,mc_num_dest) netacc;
   integer netid;
   real    genrate;
   poisson delay;

   string  traffic_type;

   function void init_uniform(string param[]);
      if (param.size()>=1) begin
         genrate = param[0].atoreal();
      end else begin
         genrate = 0.05;
      end

      if ((param.size()>=2)) begin
         // If parameter 2 is given..

         // Remove braces around definition
         string list = param[1].substr(1,param[1].len()-2);

         string valid_dests_s [];

         sysconfig::explode(list,"-",valid_dests_s);
         if (valid_dests_s.size()==1 && valid_dests_s[0] == "*") begin
            // All desinations
            for (int x=0;x<conf.xdim;x++) begin
               for (int y=0;y<conf.ydim;y++) begin
                  integer n = y*conf.xdim+x;
                  if (n!=id) begin
                     int idx = valid_dests.size();
                     valid_dests = new[(valid_dests.size()+1)](valid_dests);
                     valid_dests[idx] = n;
                  end
               end
            end
         end else begin
            for (int i=0;i<valid_dests_s.size();i++) begin
               valid_dests = new[(valid_dests.size()+1)](valid_dests);
               valid_dests[valid_dests.size()-1] = valid_dests_s[i].atoi();
            end
         end
      end else begin // if ((param.size()>=2))
         // If parameter 2 is not given, all nodes except me are
         // valid destinations
         for (int x=0;x<conf.xdim;x++) begin
            for (int y=0;y<conf.ydim;y++) begin
               integer n = y*conf.xdim+x;
               if (n!=id) begin
                  int idx = valid_dests.size();
                  valid_dests = new[(valid_dests.size()+1)](valid_dests);
                  valid_dests[idx] = n;
               end
            end
         end
      end // else: !if((param.size()>=2))

      if ((param.size()>=3)) begin
         // Remove braces around definition
         string list = param[2].substr(1,param[2].len()-2);

         string valid_vc_list [];

         sysconfig::explode(list,"-",valid_vc_list);
         if (valid_vc_list.size()==1 && valid_vc_list[0]=="*") begin
            // All virtaul channels
            valid_vchannels = new [sysconfig::get().vchannels];
            for (int i=0;i<sysconfig::get().vchannels;i++) begin
               valid_vchannels[i] = i;
            end
         end else begin
            for (int i=0;i<valid_vc_list.size();i++) begin
               valid_vchannels = new[(valid_vchannels.size()+1)](valid_vchannels);
               valid_vchannels[valid_vchannels.size()-1] = valid_vc_list[i].atoi();
            end
         end
      end else begin
         valid_vchannels = new [sysconfig::get().vchannels];
         for (int i=0;i<sysconfig::get().vchannels;i++) begin
            valid_vchannels[i] = i;
         end
      end
   endfunction // init_uniform


   function void init_multicast(string param[]);

      if (param.size()>=1) begin
         genrate = param[0].atoreal();
      end else begin
         genrate = 0.05;
      end

      if ((param.size()>=2)) begin
         // If parameter 2 is given..

         // Remove braces around definition
         string list = param[1].substr(1,param[1].len()-2);

         string valid_dests_s [];

         sysconfig::explode(list,"-",valid_dests_s);
         if (valid_dests_s.size()==1 && valid_dests_s[0]=="*") begin
            // All destinations
            valid_dests_mc = {mc_num_dest{1'b1}} & ~(1<<id);
         end else begin
            valid_dests_mc = {mc_num_dest{1'b0}};
            for (int i=0;i<valid_dests_s.size();i++) begin
               valid_dests_mc[valid_dests_s[i].atoi()] = 1'b1;
            end
         end

         valid_dests = new[(valid_dests.size()+1)](valid_dests);
         valid_dests[0] = 0;
      end else begin // if ((param.size()>=2))
         // If parameter 2 is not given, all nodes except me are
         // valid destinations
         valid_dests = new[(valid_dests.size()+1)](valid_dests);
         valid_dests[0] = 0;
         valid_dests_mc = {mc_num_dest{1'b1}} & ~(1<<id);
      end  // else: !if((param.size()>=2))


      if ((param.size()>=3)) begin
         // Remove braces around definition
         string list = param[2].substr(1,param[2].len()-2);

         string valid_vc_list [];

         sysconfig::explode(list,"-",valid_vc_list);
         if (valid_vc_list.size()==1 && valid_vc_list[0]=="*") begin
            // All virtual channels
            valid_vchannels = new [sysconfig::get().vchannels];
            for (int i=0;i<sysconfig::get().vchannels;i++) begin
               valid_vchannels[i] = i;
            end
         end else begin
            for (int i=0;i<valid_vc_list.size();i++) begin
               valid_vchannels = new[(valid_vchannels.size()+1)](valid_vchannels);
               valid_vchannels[valid_vchannels.size()-1] = valid_vc_list[i].atoi();
            end
         end
      end else begin
         valid_vchannels = new [sysconfig::get().vchannels];
         for (int i=0;i<sysconfig::get().vchannels;i++) begin
            valid_vchannels[i] = i;
         end
      end

   endfunction // init_multicast

   function new(int id,string traffic_type,networkacc #(flit_data_width,flit_dest_width,vchannels,mc_supported,mc_num_dest) net,string param[]);
      this.id = id;
      this.conf = sysconfig::get();
      this.netacc = net;

      if (traffic_type=="uniform") begin
         init_uniform(param);
      end else if (traffic_type=="multicast") begin
         init_multicast(param);
      end else begin
         $fatal("Unknown traffic type: %s",traffic_type);
      end

      this.traffic_type = traffic_type;
   endfunction // new

   task run_uniform();
      packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) p;
      flit #(mc_supported) f;
      integer d;
      integer N=0;
      integer D=0;
      delay = new(1.0/genrate);

      forever begin
         // Generate new packet
         d = delay.sample()+1;
         D = D + d;
         N++;

         repeat (d) #1ns;

         p                 = new();
         p.valid_dests     = valid_dests;
         p.valid_vchannels = valid_vchannels;
         p.valid_dests_mc  = valid_dests_mc;
         p.src             = id;
         p.is_mc           = 0;

         if(p.randomize()==0)
           $error("Randomization error");

         if (p.id==-1) begin
            netacc.measure.stop();
           break;
         end else
           netacc.send(p);
      end
   endtask // run_uniform

   task run_multicast();
      packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dest) p;
      flit #(mc_supported) f;
      integer d;
      integer N=0;
      integer D=0;
      delay = new(1.0/genrate);

      forever begin
         // Generate new packet
         d = delay.sample()+1;
         D = D + d;
         N++;

         repeat (d) #1ns;

         p                 = new();
         p.valid_dests     = valid_dests;
         p.valid_dests_mc  = valid_dests_mc;
         p.valid_vchannels = valid_vchannels;
         p.src             = id;
         p.is_mc           = 1;

         if(p.randomize()==0)
           $error("Randomization error");

         if (p.id==-1) begin
            netacc.measure.stop();
           break;
         end else
           netacc.send(p);
      end
   endtask // run_multicast

   task run();
      if (traffic_type=="uniform") begin
         run_uniform();
      end else if (traffic_type=="multicast") begin
         run_multicast();
      end
   endtask
endclass // trafficgen

`endif