`ifndef PACKET_SV
`define PACKET_SV

/**
 * Singleton that generates unique packet ids
 */
class packetid;
   static packetid pid;
   integer id;
   integer maximum;

   local function new();
      id = 0;
      maximum = sysconfig::get().numPackets;
   endfunction // new

   static function integer get();
      if (pid==null) begin
         pid = new();
      end

      if (pid.id == pid.maximum)
        get = -1;
      else
        get = pid.id++;
   endfunction
endclass

/**
 * unicast vs. multicast
 * mc_supported: the network supports multicasts
 * is_mc: this is a multicast packet
 */

class packet #(int flit_data_width,flit_dest_width,mc_supported,mc_num_dest);

   localparam flit_dest_msb=31;
   localparam flit_dest_lsb=flit_dest_msb-flit_dest_width+1;
   localparam flit_id_msb=flit_data_width-1;
   localparam flit_id_lsb=0;

   localparam flit_dest_msb_mc = 31;
   localparam flit_dest_lsb_mc=flit_dest_msb_mc-mc_num_dest+1;
   localparam flit_id_msb_mc=flit_data_width-1;
   localparam flit_id_lsb_mc=0;


   bit is_mc = 0;
   integer valid_vchannels[];
   integer src;
   integer valid_dests[];
   bit [mc_num_dest-1:0] valid_dests_mc;
   bit [mc_num_dest-1:0] dest_set_tmp = {mc_num_dest{1'b0}};
   integer payload_size = 4;
   int dest_set_index = 0;
   int pack_copy      = 0;

   rand integer dest;
   rand bit [mc_num_dest-1:0] dest_mc;
   rand bit [flit_data_width-1:0] payload[];
   rand integer vc;
   integer id;

   constraint c_valid_dest    { dest inside {valid_dests}; }
   constraint len_lim { payload.size == 4; }
   constraint valid_vc { vc inside {valid_vchannels}; }
   constraint c_mc_valid_dest { (dest_mc & valid_dests_mc) != 0; }

   sysconfig conf;
   flit #(mc_supported) flits[$];
   time       gen;
   time       net;

   integer curgetflit = 0;
   integer curdestindex = 0;

   // Count destinations
   function integer numdests();
      int  i=0;
      for (int n=0;n<mc_num_dest;n++) begin
         i = i + dest_mc[n];
      end
      numdests = i;
   endfunction

   // Calculate hamiltonian position from cartesian coordinates
   function integer hamnum(int x,int y);
      if (y%2==0) begin
         return x+y*conf.xdim;
      end else begin
         return y*conf.xdim+(conf.xdim-x-1);
      end
   endfunction

   function integer curdestpacket(int curdestindex);
     int temp = 0;
     for(int index=0;index<mc_num_dest;index++) begin
       if (dest_mc[index]== 1'b1 && temp==curdestindex) begin
         return index;
       end else if (dest_mc[index]== 1'b1 && temp<curdestindex) begin
         temp++;
       end
     end
   endfunction

   function void splitdestset();
      /*
      * Split destination set into two parts
      * It depends on the current node position and the destination set
      * In addition it depends on the routing scheme(hp vs. rp multicast)
      */
      integer bound=0;
      integer ham_num=0;
      integer myHam=0;
      integer myX=0;
      integer myY=0;
      if (conf.select_router==2) begin
         // Hamiltonian routing
         myY = $floor(src/conf.xdim);
         myX = src - myY*conf.xdim;
         myHam = hamnum(myX,myY);
         for (int y=0;y<conf.ydim;y++) begin
            for (int x=0;x<conf.xdim;x++) begin
               ham_num = hamnum(x,y);
               if(ham_num<myHam && dest_mc[x+y*conf.xdim]==1)begin
                  dest_set_tmp[x+y*conf.xdim] = 0;
               end else if (ham_num>myHam && dest_mc[x+y*conf.xdim]==1) begin
                  dest_set_tmp[x+y*conf.xdim] = 1;
                  dest_mc[x+y*conf.xdim]=0;
               end
            end
         end
      end else begin
         // Recursive partitioning routing
         bound = $floor(src/conf.ydim)*conf.ydim;
         for (int index=bound;index<mc_num_dest;index++) begin
            if(dest_mc[index]==1)begin
               dest_set_tmp[index] = 1;
               dest_mc[index]      = 0;
            end
         end
      end
   endfunction


   function integer numtotalflits();
      if (!is_mc) begin
         return payload.size()+1; // Header + Payload
      end else begin
         if (mc_supported) begin
            return (payload.size()+2)*pack_copy; // Multicast Mask + Header + Payload
         end else begin
            return numdests()*(payload.size()+1);
         end
      end
   endfunction // numtotalflits


   function flit #(mc_supported) getflit();
      // Generate unicast packet
      if (!is_mc) begin
         flit #(mc_supported) f = new();

         if (curgetflit==0) begin
            if (payload.size>0) begin
               f.ftype[1:0]=2'b01;
            end else begin
               f.ftype=2'b11;
            end
            f.content[flit_dest_msb:flit_dest_lsb] = dest;

         end else if (curgetflit==1) begin
            // payload
            f.ftype=2'b00;
            f.content[flit_id_msb:flit_id_lsb] = id;

         end else if (curgetflit>1 && curgetflit<payload.size) begin
            // payload
            f.ftype=2'b00;
            f.content[flit_id_msb:flit_id_lsb] = payload[curgetflit-2];

         end else begin
            // last flit
            f.ftype=2'b10;
            f.content=payload[curgetflit-2];
         end
         curgetflit++;
         flits.push_back(f);

         getflit = f;
      end else begin // if (!is_mc)
         // Generate multicast packet
         if (mc_supported) begin
            // Generate real multicast packet
            flit #(mc_supported) f = new();

            if (curgetflit==0) begin
               if (payload.size>0) begin
                  f.ftype=3'b101;
               end else begin
                  f.ftype=3'b111;
               end
               if (dest_set_index>1) begin
                  splitdestset();
                  if (dest_set_index==3) begin
                     vc = valid_vchannels[1];
                     f.content[flit_dest_msb_mc:flit_dest_lsb_mc] = dest_set_tmp;
                  end else begin
                     if (curdestindex==0) begin
                        vc = valid_vchannels[0];
                        f.content[flit_dest_msb_mc:flit_dest_lsb_mc] = dest_mc;
                     end else begin
                        vc = valid_vchannels[1];
                        f.content[flit_dest_msb_mc:flit_dest_lsb_mc] = dest_set_tmp;
                     end
                  end
               end else begin
                  vc = valid_vchannels[0];
                  f.content[flit_dest_msb_mc:flit_dest_lsb_mc] = dest_mc;
               end

            end else if (curgetflit==1) begin
               // payload
               f.ftype=3'b100;
               f.content[flit_id_msb:flit_id_lsb] = id;

            end else if (curgetflit>1 && curgetflit<payload.size+1) begin
               // payload
               f.ftype=3'b100;
               f.content[flit_id_msb:flit_id_lsb] = payload[curgetflit-2];

            end else begin
               // last flit
               f.ftype=3'b110;
               f.content=payload[curgetflit-2];
            end

            if (curgetflit<payload.size+1) begin
               curgetflit++;
            end else begin
               curgetflit = 0;
               curdestindex++;
            end

            flits.push_back(f);
            getflit = f;
         end else begin // if (mc_supported)
            // Generate pseudo multicast
            flit #(mc_supported) f = new();

            if (curgetflit==0) begin
               if (payload.size>0) begin
                  f.ftype[1:0]=2'b01;
               end else begin
                  f.ftype=2'b11;
               end
               f.content[flit_dest_msb:flit_dest_lsb] = curdestpacket(curdestindex);

            end else if (curgetflit==1) begin
               // payload
               f.ftype=2'b00;
               f.content[flit_id_msb:flit_id_lsb] = id;

            end else if (curgetflit>1 && curgetflit<payload.size) begin
               // payload
               f.ftype=2'b00;
               f.content[flit_id_msb:flit_id_lsb] = payload[curgetflit-2];

            end else begin
               // last flit
               f.ftype=2'b10;
               f.content=payload[curgetflit-2];
            end

            if (curgetflit<payload.size) begin
               curgetflit++;
            end else begin
               curgetflit = 0;
               curdestindex++;
            end

            flits.push_back(f);
            getflit = f;
         end
      end

   endfunction // getflit

   function new();
      this.conf=sysconfig::get;
   endfunction

   function void post_randomize;
      bit low=0;
      bit high=0;
      integer bound=0;
      integer ham_num=0;
      integer myY=0;
      integer myX=0;
      integer myHam=0;

      curgetflit = 0;
      curdestindex = 0;
      id = packetid::get();
      dest_mc = dest_mc & valid_dests_mc;

      if (conf.select_router==2) begin
         // Hamiltonian routing
         myY = $floor(src/conf.xdim);
         myX = src - myY*conf.xdim;
         myHam = hamnum(myX,myY);
         for (int y=0;y<conf.ydim;y++) begin
            for (int x=0;x<conf.xdim;x++) begin
               ham_num = hamnum(x,y);
               if (ham_num<myHam) begin
                  low = low || dest_mc[x+y*conf.xdim];
               end else if (ham_num>myHam) begin
                  high = high || dest_mc[x+y*conf.xdim];
               end
            end
         end
      end else begin
         bound=$floor(src/conf.ydim)*conf.ydim;
         for (int index=0;index<mc_num_dest;index++) begin
            if (index<bound) begin
               low = low || dest_mc[index];
            end else if (index>src) begin
               high = high || dest_mc[index];
            end
         end
      end

      pack_copy = low + high;

      if (low==1 && high==0) begin
         dest_set_index = 1;
      end else if (low==1 && high==1) begin
         dest_set_index = 2;
      end else begin
         dest_set_index = 3;
      end
   endfunction
endclass // packet

class flit #(mc_supported);
   bit  [1+mc_supported:0] ftype;
   bit [31:0] content;

   function bit [33+mc_supported:0] get();
      get = {ftype,content};
   endfunction; // get
endclass // flit

`endif