
`include "uart_defines.v"

`define IDLE            2'b00
`define READY           2'b10
 
module noc2char(/*AUTOARG*/
   // Outputs
   noc_ready, char_valid, char_data,
   // Inputs
   clk, rst, noc_flit, noc_valid, char_ready
   );
  
   input clk;
   input rst;
   
   input [33:0] noc_flit;
   input        noc_valid;
   output reg   noc_ready;
   
   input        char_ready;
   output reg   char_valid;
   output [7:0] char_data;
   
   reg [1:0]    state;
   reg [1:0]    nxt_state;  
   reg [7:0]    character;
   reg [7:0]   nxt_character;
   
   assign char_data = character;
   
   always @(posedge clk) begin
      if(rst) begin
         character <= 'bx;  
         state <= `IDLE;
      end else begin
         character <= nxt_character;
         state <= nxt_state;
      end
   end
   
   always @(*) begin        
      nxt_state= state;     
      noc_ready = 1'b0;
      char_valid = 1'b0;
      nxt_character = character;
      
      case(state)
        `IDLE: begin 
           noc_ready = 1'b1;
           if (noc_valid) begin
              if(noc_flit[33:32]==2'b11) begin
                 nxt_state = `READY;
                 nxt_character = noc_flit[7:0];
              end else begin  
                 nxt_state = `IDLE;                          
              end
           end else begin
              nxt_state = `IDLE;                        
           end
        end
        
        `READY :  begin         
           char_valid = 1'b1;
           if (char_ready) begin
              nxt_state = `IDLE;                               
           end else begin
              nxt_state = `READY;  
           end
        end     
      endcase
   end
 endmodule