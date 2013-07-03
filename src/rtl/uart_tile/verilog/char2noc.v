
`include "uart_defines.v"

`define IDLE            2'b00
`define READY           2'b10
 
module char2noc(/*AUTOARG*/
   // Inputs
   noc_ready, char_valid, char_data,
   // Outputs
   clk, rst, noc_flit, noc_valid, char_ready
   );
   parameter ph_dest_width = 5;
   parameter ph_cls_width = 3;
   parameter ph_src_width = 5;
   
   parameter destination = 0;
   parameter pkt_class = 0;
   parameter id=0;  
     
   input clk;
   input rst;
   
   output [33:0] noc_flit;
   output reg    noc_valid;
   input         noc_ready;
   
   output reg   char_ready;
   input        char_valid;
   input [7:0]  char_data;
   
   reg [1:0]    state;
   reg [1:0]    nxt_state;  
   reg [33:0]   flit;
   reg [33:0]   nxt_flit;
   
   assign noc_flit = flit;
   
   always @(posedge clk) begin
      if(rst) begin
         flit <= 'bx;  
         state <= `IDLE;
      end else begin
         flit <= nxt_flit;
         state <= nxt_state;
      end
   end
   
   always @(*) begin        
      nxt_state= state;     
      char_ready = 1'b0;
      noc_valid  = 1'b0;
      nxt_flit = flit;
      
      case(state)
        `IDLE: begin 
           char_ready = 1'b1;
           if (char_valid) begin
                 nxt_state = `READY;
                 nxt_flit[33:32] = 2'b11;
                 nxt_flit[31:(32-ph_dest_width)] = destination;
                 nxt_flit[(32-ph_dest_width-1):(32-ph_dest_width-ph_cls_width)] = pkt_class;
                 nxt_flit[23:19] = id;
                 nxt_flit[18:8] = 'x;
                 nxt_flit[7:0] = char_data;
//                 nxt_flit = {2'b11,5'b0,19'b0,char_data};
           end else begin
              nxt_state = `IDLE;                        
           end
        end
        
        `READY :  begin         
           noc_valid = 1'b1;
           if (noc_ready) begin
              nxt_state = `IDLE;                               
           end else begin
              nxt_state = `READY;  
           end
        end     
      endcase
   end
 endmodule