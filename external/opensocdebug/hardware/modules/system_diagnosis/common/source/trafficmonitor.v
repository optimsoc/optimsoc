/* Dieses Modul dient nur zum Messen der Traffic
 im Diagnosissystem sowie zum Zaehlen, wie oft events vorkommen*/

module trafficmonitor (/*AUTOARG*/
   // Outputs
   ev_counter,
   // Inputs
   clk, rst, event_global, flit_valid
   );

   input clk, rst;

   input event_global;
   input flit_valid;

   //f(event)
   reg [31:0] event_cycle_counter, nxt_event_cycle_counter; //zaehlt hoch, bis neues event kommt
   reg [31:0] ev_cycle_counter_stack[0:1000];
   reg [16:0] ev_index, nxt_ev_index;
   
   // noc auslastung
   reg [31:0] cycle_counter, nxt_cycle_counter;
   reg [31:0] flit_valid_counter, nxt_flit_valid_counter;
   reg [31:0] flit_valid_counter_reg[0:1000];
   reg [31:0] fl_index, nxt_fl_index;

   output [31:0]    ev_counter;
   assign ev_counter = event_cycle_counter;
 
    always@(*) begin
        if (event_global) begin
            ev_cycle_counter_stack[ev_index] = event_cycle_counter;
            nxt_event_cycle_counter = 32'd1;
            nxt_ev_index = ev_index + 1;
         end else begin
            nxt_event_cycle_counter = event_cycle_counter + 1;
            nxt_ev_index = ev_index;
         end
    end
    
    always@(*) begin
        if (cycle_counter == 32'd50) begin
      
            flit_valid_counter_reg[fl_index] = flit_valid_counter;
            if (fl_index == 32'd999) begin
            nxt_fl_index = 'd0;
            end else begin
            nxt_fl_index = fl_index + 1;
            end
            nxt_flit_valid_counter = 32'd0;
            nxt_cycle_counter = 'd1;
         end else begin
            nxt_fl_index = fl_index;
            nxt_cycle_counter = cycle_counter + 1;
            if (flit_valid) begin
               nxt_flit_valid_counter = flit_valid_counter + 1;
            end else begin
               nxt_flit_valid_counter = flit_valid_counter;
            end  
         end
    end
 
   always@(posedge clk) begin
      if (rst) begin
         event_cycle_counter <= 32'd1;
         flit_valid_counter <= 32'd0;
         cycle_counter <= 32'd1;
         ev_index <= 17'd0;
         fl_index <= 'd0;
      end else begin
         ev_index <= nxt_ev_index;
         event_cycle_counter <= nxt_event_cycle_counter;
         //
        fl_index <= nxt_fl_index;
        
        
         cycle_counter <= nxt_cycle_counter;
         
         flit_valid_counter <= nxt_flit_valid_counter;
         
         //
         
      end // else: !if(rst)
   end // always@ (posedge clk)


endmodule // trafficmonitor
