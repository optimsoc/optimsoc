`ifndef SYSCONFIG_SV
`define SYSCONFIG_SV

/**
 * This class represents the configuration of a traffic.
 */
class trafficdesc;
   string  name;
   string  param [];
endclass // trafficdesc

/**
 * Singleton for configuration
 */
class sysconfig;
   integer xdim;
   integer ydim;
   integer nodes;
   integer vchannels;
   integer select_router;

   integer    numPackets;

   // index 0: node id
   // index 1: traffic generator id
   trafficdesc traffic [][];

   static sysconfig conf;

   local function new();

   endfunction // new

   static function sysconfig get();
      if (conf==null) begin
         conf = new();
      end

      get = conf;
   endfunction

   static function void explode(string s,string sep,ref string vec []);
      int offset = 0;
      int i;

      for (i=0;i<s.len();i++) begin
         if (s.getc(i)==sep.getc(0)) begin
            vec = new[vec.size()+1](vec);
            vec[vec.size()-1] = s.substr(offset,i-1);
            offset=i+1;
         end
      end

      if (offset<i) begin
         vec = new[vec.size()+1](vec);
         vec[vec.size()-1] = s.substr(offset,s.len()-1);
      end

   endfunction // explode

   static function bit explodefirst(string s,string sep,ref string first,ref string second);
      explodefirst = 0;

      for (int i=0;i<s.len();i++) begin
         if (s.getc(i)==sep.getc(0)) begin
            first = s.substr(0,i-1);
            second = s.substr(i+1,s.len()-1);
            explodefirst=1;
            break;
         end
      end
   endfunction // explodefirst

   static function trafficdesc trafficextract(string s);
      trafficdesc t = new ();
      int i = 0;
      int off;

      string p;

      for (i=0;i<s.len();i++) begin
         if (s.getc(i)=="(") begin
            t.name = s.substr(0,i-1);
            off = i+1;
            break;
         end
      end

      for (i=s.len()-1;i>=off;i--) begin
         if (s.getc(i)==")") begin
            p = s.substr(off,i-1);
            break;
         end
      end

      explode(p,",",t.param);

      trafficextract = t;
   endfunction // extract

   function void addTraffic(int id,trafficdesc t);
      traffic[id] = new [traffic[id].size()+1](traffic[id]);
      traffic[id][traffic[id].size()-1] = t;
   endfunction // addTraffic


   function void setTraffic(string conf);
      string t [];

      traffic = new [nodes];

      explode(conf,";",t);

      for (int i=0;i<t.size();i++) begin
         string id,desc;
         if (explodefirst(t[i],":",id,desc)) begin
            trafficdesc traf = trafficextract(desc);

            if (id=="*") begin
               for (int i=0;i<nodes;i++) begin
                  addTraffic(i,traf);
               end
            end else begin
               $display("%s\n",id);

               addTraffic(id.atoi(),traf);
            end
         end else begin
            $display("invalid: %s\n",t[i]);
         end
      end

   endfunction

   function int getTrafficNum(int node);
      getTrafficNum = traffic[node].size();
   endfunction // getTrafficNum

   function trafficdesc getTraffic(int node,int id);
      getTraffic = traffic[node][id];
   endfunction

endclass // sysconfig


`endif