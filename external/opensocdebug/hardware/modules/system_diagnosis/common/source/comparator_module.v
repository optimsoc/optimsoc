/* This module monitors signals of the cpu, 
 detects matches of preconfigured signal values and forwards a respective
 event signal along with the event id. 
 For further use, this module should be integrated into a wrapper for
 * program counter module
 * memory address module

 Author: Markus Goehrle, Markus.Goehrle@tum.de */


module comparator_module (/*AUTOARG*/
   // Outputs
   ev_valid, ev_id, ev_time,
   // Inputs
   clk, rst, diag_sys_enabled, conf_reg_flat_in, val, enable,
   time_global
   );

   /** Parameters **/
   // Number of 16 bit configuration registers
   parameter CONF_REG_SIZE = 3;
   // Bit width of event id signal
   parameter EVENT_ID_WIDTH = 5;
   // Bit width of timestamp signal
   parameter TIMESTAMP_WIDTH = 32;
   // Maximum number of allowed events that can be monitored at the same time
   parameter MAX_EVENT_COUNT = 8;
   // Index for multiplexer of lookuptable selection after comparator hit
   parameter INDEX_WIDTH = $clog2(MAX_EVENT_COUNT);

   /** Interfaces **/
   input clk, rst;
   /* configuration register interface */
   input diag_sys_enabled;
   input [16*CONF_REG_SIZE-1:0] conf_reg_flat_in;
   /* mor1kx interface */
   input [31:0] val;
   input        enable;
   /* event signal interface to LUT module */
   output reg   ev_valid;
   output reg [EVENT_ID_WIDTH-1:0] ev_id;
   output reg [TIMESTAMP_WIDTH-1:0] ev_time;
   /* interface to global timestamp provider module */
   input [TIMESTAMP_WIDTH-1:0]      time_global;
   


   
   //--------------
   /* output register wiring */
   wire                             nxt_ev_valid;
   wire [EVENT_ID_WIDTH-1:0]        nxt_ev_id;
   
   
   /* Configuration registers (wires) */
    /* 16bit config register format (Data = Program counter or memory address value):
    *
    *    47            32 31           16 15            0
    *    +---------------+---------------+--------------+
    *    |valid  xxx  ID |   Data MSB    |   Data LSB   |
    *    |      ...      |      ...      |     ...      |
    *    |      ...      |      ...      |     ...      |
    *    +---------------+---------------+--------------+
    * 
    */
   wire [31:0]                       pc_values [0:MAX_EVENT_COUNT-1];
   wire                              pc_values_valid [0:MAX_EVENT_COUNT-1];
   wire [EVENT_ID_WIDTH-1:0]         pc_ev_id [0:MAX_EVENT_COUNT-1];
   
   genvar                            j;
   generate
      for (j = 0; j < MAX_EVENT_COUNT; j = j + 1) begin: gen_conf_mem_mapping
         assign pc_values[j] = conf_reg_flat_in[j*48+31:j*48];
         assign pc_values_valid[j] = conf_reg_flat_in[j*48+47];
         assign pc_ev_id[j] = conf_reg_flat_in[j*48+32+EVENT_ID_WIDTH-1:j*48+32];
      end
   endgenerate
      

   /* Bitvector for comparator logic that indicates a match */
   wire [MAX_EVENT_COUNT-1:0]         comparator_match;

   /* LUT index wire */
   wire [INDEX_WIDTH-1:0]                      lut_index;

   /* log2 function for index evaluation */
   function [INDEX_WIDTH-1:0] log2;
      input [MAX_EVENT_COUNT-1:0] vector_tmp;
      begin
         vector_tmp = vector_tmp-1;
         for (log2=0; vector_tmp>0; log2=log2+1)
           vector_tmp = vector_tmp>>1;
      end
   endfunction
   //------------

   

   /* Comparator logic */
   genvar                                     i;
   for (i = 0; i < MAX_EVENT_COUNT; i=i+1) begin
      assign comparator_match[i] = (pc_values_valid[i] && enable && (val == pc_values[i])) ? 1'b1 : 1'b0; //ternary operator
   end

   /* Valid event detection logic */
   assign nxt_ev_valid = comparator_match ? 1'b1 : 1'b0;

   /* index generation logic */
   assign lut_index = log2(comparator_match);
   assign nxt_ev_id = pc_ev_id[lut_index];


   /* sequential */
   integer y;
   always @(posedge clk) begin
      if (rst || ~diag_sys_enabled) begin
         ev_valid <= 1'b0;
         ev_id <= 0;
         ev_time <= 0;
      end else begin
         ev_valid <= nxt_ev_valid;
         ev_id <= nxt_ev_id;
         ev_time <= time_global;
      end
   end // always @ (posedge clk)

  
endmodule // PC_monitor

      
   
         
   
   
