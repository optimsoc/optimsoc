/* Copyright (c) 2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * The testbench for a router.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

// TODO
//  * detect starvation of input!

`timescale 1 ns/1 ps
`include "lisnoc.svh"

`define DATA_WIDTH 32
`define TYPE_WIDTH 2

//**************************************************************
// Constraints to create random test patters
//**************************************************************
class packet;
   integer vchannels;
   bit [4:0] self;   // packet source

   rand bit [4:0] dest;
   rand bit [31:0] payload[];
   rand integer vc;
   rand bit [3:0] prio;

   rand integer unsigned burst_delay;

   constraint valid_dest { dest >= 0; dest < 5; dest != self; }
   constraint len_lim { payload.size > 0; payload.size <= 32; }
   constraint valid_vc { vc >= 0; vc < vchannels; }
   constraint dist_burst_delay { burst_delay dist { 0:= 80, 4:= 10, 10:= 10 }; }
   constraint dist_prio { prio dist { 4'b0000:=1, 4'b1000:= 1, 4'b1100:= 1, 4'b1110:=1, 4'b1111:= 1 }; }
endclass // packet

class randomdelay; // to delay receive acceptance
   rand integer unsigned delay;
   constraint dist_delay { delay dist { 0:= 80, 4:= 10, 10:= 10 }; }
endclass // randomdelay

//**************************************************************
// contraints declaration until here
//**************************************************************

typedef struct {
   flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit;
   time      timestamp;
} sent_flit;

class expect_queue #(int vchannels = 1);
   mailbox   #(sent_flit) queue[5][vchannels][5];

   semaphore sem_recv;
   integer   recv_success;
   real      avg_latency;

   integer   cur_src[5][vchannels];

   function new();
      for(int i=0;i<5;i++) begin
         for(int j=0;j<vchannels;j++) begin
            cur_src[i][j] = -1;
            for(int k=0;k<5;k++) begin
               queue[i][j][k] = new();
            end
         end
      end
      recv_success = 0;
      avg_latency = 0.0;
      sem_recv = new(1);
   endfunction // new

   // Its automatic, so we can re-enter the task while the previous task is still running!!!
   task automatic put(int source,int target,int vc,sent_flit flit);
      queue[target][vc][source].put(flit);
   endtask // put

   task automatic receive_stat(sent_flit f);
      integer c;
      integer hand1;

      sem_recv.get(1);
      avg_latency = (avg_latency * recv_success + real'($time - f.timestamp))/(recv_success+1);
      c = ++recv_success;
      sem_recv.put(1);

      if (c%10000==0) begin
        $display("%t flits: %0d %f",$time,c,avg_latency);
      end
   endtask // receive_stat

   // This task checks if the transmission was successfull or not
   task automatic check(int target,flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit,int vc,ref int source);
      sent_flit f;
      if (cur_src[target][vc] == -1 ) begin
         for (int s=0;s<5;s++) begin
            if(queue[target][vc][s].try_peek(f)) begin
               if (f.flit.content==flit.content) begin
                  queue[target][vc][s].get(f);
                  source = s;
                  cur_src[target][vc] = s;
                  receive_stat(f);
                  return;
               end
            end
         end // for (int s=0;s<5;s++)
            end else begin // if (cur_src == -1 )
         if(queue[target][vc][cur_src[target][vc]].try_peek(f)) begin
            if (f.flit.content==flit.content) begin
               queue[target][vc][cur_src[target][vc]].get(f);
               source = cur_src[target][vc];
               if (flit.ftype == LAST)
            cur_src[target][vc] = -1;
               receive_stat(f);
               return;
            end
         end
      end

      $display("%t Mismatch for %x on %0d[%1d]",$time,flit.content,target,vc);

      if (cur_src[target][vc]==-1)
         $display("%t not part of a packet",$time);
      else
         $display("%t expected on %0d",$time,cur_src[target][vc]);
      for (int s=0;s<5;s++) begin
         if(queue[target][vc][s].try_peek(f)) begin
            $display("%t   [%0d] (%x%x,%0t)",$time,s,f.flit.ftype,f.flit.content,f.timestamp);
         end
      end

      $fatal(1,"Mismatch");  // This stops the simulation immediately

   endtask // check
endclass // expect_queue

program tb_noc_router_test
  (
   input clk, rst,
   testbench_if tb_if
   );

   parameter vchannels = 1;

   expect_queue #(.vchannels(vchannels)) eq = new();

   initial begin
      eq = new();

      @(negedge rst);
      fork
         packet_generate(0);
         packet_generate(1);
         packet_generate(2);
         packet_generate(3);
         packet_generate(4);
         packet_monitor(0);
         packet_monitor(1);
         packet_monitor(2);
         packet_monitor(3);
         packet_monitor(4);
      join
   end

   task automatic packet_generate(int id);
      packet p;

      flit_header_t #(`DATA_WIDTH) header;
      flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit;
      sent_flit sflit;

      p = new();
      p.self = id;
      p.vchannels = vchannels;

      forever begin
         void'(p.randomize());
         header = new();
         flit = new();
         header.dest = p.dest;
         header.prio = p.prio;
         header.class_specific[3:0] = id;

         flit.ftype = HEADER;
         flit.content = {header.dest,header.prio,header.packet_class,header.class_specific};
         tb_if.send(id,p.dest,p.vc,flit,p.burst_delay);
         sflit.flit = flit;
         sflit.timestamp = $time;
         eq.put(id,p.dest,p.vc,sflit); //--> write into queue
         for (int i=0;i<$size(p.payload)-1;i=i+1) begin
            header = new();
            flit = new();
            flit.ftype = PAYLOAD;
            flit.content = p.payload[i];
            tb_if.send(id,p.dest,p.vc,flit,p.burst_delay);
            sflit.flit = flit;
            sflit.timestamp = $time;
            eq.put(id,p.dest,p.vc,sflit); //--> write into queue
         end
         header = new();
         flit = new();
         flit.ftype = LAST;
         flit.content = p.payload[$size(p.payload)-1];
         tb_if.send(id,p.dest,p.vc,flit,p.burst_delay);
         sflit.flit = flit;
         sflit.timestamp = $time;
         eq.put(id,p.dest,p.vc,sflit); //--> write into queue
      end
   endtask

   task automatic packet_monitor(int id);
      flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit = new();
      int vc;
      int source;
      randomdelay d = new();

      forever begin
         void'(d.randomize());
         tb_if.receive(id,d.delay,vc,flit);
         eq.check(id,flit,vc,source);
      end

   endtask // packet_monitor

endprogram // tb_noc_router_test

interface testbench_if
  (
   input clk,
   lisnoc_link_if north_in,
   lisnoc_link_if north_out,
   lisnoc_link_if east_in,
   lisnoc_link_if east_out,
   lisnoc_link_if south_in,
   lisnoc_link_if south_out,
   lisnoc_link_if west_in,
   lisnoc_link_if west_out,
   lisnoc_link_if local_in,
   lisnoc_link_if local_out
   );

   parameter vchannels = 1;

   reg  [vchannels-1:0] in_valid[5];
   reg  [vchannels-1:0] in_ready[5];
   reg  [`DATA_WIDTH-1:0] in_flit_content[5];
   reg  [`TYPE_WIDTH-1:0] in_flit_type[5];

   reg  [vchannels-1:0] out_valid[5];
   reg  [vchannels-1:0] out_ready[5];
   reg [`DATA_WIDTH-1:0] out_flit_content[5];
   reg [`DATA_WIDTH-1:0] out_flit_type[5];

   assign north_in.flit = {in_flit_type[0],in_flit_content[0]};
   assign east_in.flit  = {in_flit_type[1],in_flit_content[1]};
   assign south_in.flit = {in_flit_type[2],in_flit_content[2]};
   assign west_in.flit  = {in_flit_type[3],in_flit_content[3]};
   assign local_in.flit = {in_flit_type[4],in_flit_content[4]};

   assign north_in.valid = in_valid[0];
   assign east_in.valid = in_valid[1];
   assign south_in.valid = in_valid[2];
   assign west_in.valid = in_valid[3];
   assign local_in.valid = in_valid[4];

   assign north_out.ready = out_ready[0];
   assign east_out.ready = out_ready[1];
   assign south_out.ready = out_ready[2];
   assign west_out.ready = out_ready[3];
   assign local_out.ready = out_ready[4];

   always @(posedge clk) begin
      out_valid[0] <= north_out.valid;
      out_valid[1] <= east_out.valid;
      out_valid[2] <= south_out.valid;
      out_valid[3] <= west_out.valid;
      out_valid[4] <= local_out.valid;
      {out_flit_type[0],out_flit_content[0]} <= north_out.flit;
      {out_flit_type[1],out_flit_content[1]} <= east_out.flit;
      {out_flit_type[2],out_flit_content[2]} <= south_out.flit;
      {out_flit_type[3],out_flit_content[3]} <= west_out.flit;
      {out_flit_type[4],out_flit_content[4]} <= local_out.flit;
      in_ready[0] <= north_in.ready;
      in_ready[1] <= east_in.ready;
      in_ready[2] <= south_in.ready;
      in_ready[3] <= west_in.ready;
      in_ready[4] <= local_in.ready;
   end

   initial begin
      for(int i=0;i<5;i=i+1) begin
         for(int j=0;j<vchannels;j=j+1) begin
            out_ready[i][j] = 1'b0;
            in_valid[i][j] = 1'b0;
         end
      end
   end

   task automatic send(int source,int target,int vc,flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit,int delay);
      repeat(delay) @(posedge clk);
      in_valid[source][vc] = 1'b1;
      in_flit_content[source] = flit.content;
      in_flit_type[source] = flit.ftype;
      #1;
      in_valid[source][vc] = 1'b1;
      @(posedge clk iff in_ready[source][vc]);
      $display("%t sent from %3d to %3d on vc %1d: %x%x",$time,source,target,vc,flit.ftype,flit.content);
      in_valid[source][vc] = 1'b0;
   endtask // send

   task automatic receive(int target, int delay, ref int vc, ref flit_t #(`DATA_WIDTH,`TYPE_WIDTH) flit);
      repeat(delay) @(posedge clk);
      out_ready[target] = {vchannels{1'b1}};

      @(posedge clk iff |out_valid[target]);
      {flit.ftype,flit.content} = {out_flit_type[target],out_flit_content[target]};

      for (int i=0;i<vchannels;i=i+1) begin
         if (out_valid[target][i]) begin
            vc = i;
            $display("%t received for %3d on vc %1d: %x%x",$time,target,vc,flit.ftype,flit.content);
            break;
         end
      end
      out_ready[target] = {vchannels{1'b0}};
   endtask

endinterface // testbench_if

module tb_noc_router();

   reg clk;
   reg rst;

   parameter vchannels = 2;
   parameter use_prio  = 1;

   lisnoc_link_if #(.vchannels(vchannels)) north_in();
   lisnoc_link_if #(.vchannels(vchannels)) north_out();
   lisnoc_link_if #(.vchannels(vchannels)) east_in();
   lisnoc_link_if #(.vchannels(vchannels)) east_out();
   lisnoc_link_if #(.vchannels(vchannels)) south_in();
   lisnoc_link_if #(.vchannels(vchannels)) south_out();
   lisnoc_link_if #(.vchannels(vchannels)) west_in();
   lisnoc_link_if #(.vchannels(vchannels)) west_out();
   lisnoc_link_if #(.vchannels(vchannels)) local_in();
   lisnoc_link_if #(.vchannels(vchannels)) local_out();

   // Router interface
   lisnoc_router_2dgrid_sv #(.vchannels(vchannels),.use_prio(use_prio))
     uut(
      // Interfaces
      .north_out                (north_out),
      .north_in                 (north_in),
      .east_out                 (east_out),
      .east_in                  (east_in),
      .south_out                (south_out),
      .south_in                 (south_in),
      .west_out                 (west_out),
      .west_in                  (west_in),
      .local_out                (local_out),
      .local_in                 (local_in),
      // Inputs
      .clk                      (clk),
      .rst                      (rst));

   defparam uut.num_dests = 5;
   defparam uut.lookup = { SELECT_NORTH, SELECT_EAST, SELECT_SOUTH, SELECT_WEST, SELECT_LOCAL };


   initial begin
      clk = 0;
      rst = 1;
      #10 rst = 0;
   end

   always clk = #5 ~clk;

   testbench_if #(.vchannels(vchannels)) tb_if
     (// Interfaces
      .north_in                         (north_in),
      .north_out                        (north_out),
      .east_in                          (east_in),
      .east_out                         (east_out),
      .south_in                         (south_in),
      .south_out                        (south_out),
      .west_in                          (west_in),
      .west_out                         (west_out),
      .local_in                         (local_in),
      .local_out                        (local_out),
      // Inputs
      .clk                              (clk));

   tb_noc_router_test #(.vchannels(vchannels))
   test(// Interfaces
      .tb_if                            (tb_if),
      // Inputs
      .clk                              (clk),
      .rst                              (rst));

endmodule // tb_noc_router

// Local Variables:
// verilog-library-directories:("../rtl/")
// verilog-auto-inst-param-value: t
// End:
