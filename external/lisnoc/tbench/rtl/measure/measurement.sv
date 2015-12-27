`ifndef MEASUREMENT_SV
`define MEASUREMENT_SV

/**
 * This class performs the actual measurement
 */
class measurement #(int flit_data_width,flit_dest_width,mc_supported,mc_num_dests);
   packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dests) packets[integer];
   integer n = 0;
   integer acc_lat=0;
   integer net_lat=0;
   integer N;
   integer nflits = 0;
   bit     stopnow = 0;

   function void stop();
      stopnow = 1;
   endfunction // stop


   function new();
      N = sysconfig::get().numPackets;
   endfunction // new


   function void add(packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dests) p);
//      $display("add %0d",p.id);

      packets[p.id] = p;
   endfunction // add

   function void received(integer id);
      packet #(flit_data_width,flit_dest_width,mc_supported,mc_num_dests) p;

      p = packets[id];

      acc_lat = acc_lat + (p.net-p.gen);
      net_lat = net_lat + ($time-p.net);

      n++;

      nflits = nflits + p.flits.size();

      if (stopnow || n==N) begin
         $display("RESULT packets: %0d, flits: %0d, throughput: %f, acc: %f, net: %f",n,nflits,nflits*1.0/($time*sysconfig::get().xdim*sysconfig::get().ydim),acc_lat*1.0/n,net_lat*1.0/n);
         $finish();
      end
   endfunction
endclass // measurement

`endif