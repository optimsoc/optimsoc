/* This module is the interface between the event monitor modules on the
 one hand and the snapshot collector modules and the packetizer on the other.
 Every time an event signal occurs, it triggers and configures the snapshot modules to 
 collect the desired data. Furthermore it configures the packetizer to generate
 a snapshot packet.
 Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module LUT (/*AUTOARG*/
   // Outputs
   event_valid_global, bv_GPR, stackargs, ev_id, ev_time,
   // Inputs
   clk, rst, conf_lut_flat_in, pc_ev_valid, pc_ev_id, pc_ev_time,
   fcnret_ev_valid, fcnret_ev_id, fcnret_ev_time, memaddr_ev_valid,
   memaddr_ev_id, memaddr_ev_time
   );
   
   /*******************************************************************/
   /** Parameters **/
   // Number of 16 bit configuration registers (three registers per event slot in the LUT)
   parameter CONF_LUT_SIZE = 3;
   // Maximum total number of simultaneously tracked events
   // Given as a sum of the individual maximum number of events of each monitor
   parameter MAX_TOTAL_EVENT_COUNT = `DIAGNOSIS_TOTAL_EVENTS_MAX;
   // Index for multiplexer of lookuptable selection after comparator hit
   parameter INDEX_WIDTH = $clog2(MAX_TOTAL_EVENT_COUNT);
   
   /*******************************************************************/
   /** Interfaces **/
   input clk, rst;
   /* configuration register interface */
   input [16*CONF_LUT_SIZE-1:0] conf_lut_flat_in;
   /* PC Monitor Interface */
   input pc_ev_valid;
   input [`DIAGNOSIS_EV_ID_WIDTH-1:0] pc_ev_id;
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] pc_ev_time;
   /* Fcn Return Monitor Interface */
   input                                  fcnret_ev_valid;
   input [`DIAGNOSIS_EV_ID_WIDTH-1:0]     fcnret_ev_id;
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] fcnret_ev_time;
   /* Memory Address Monitor Interface */
   input                                  memaddr_ev_valid;
   input [`DIAGNOSIS_EV_ID_WIDTH-1:0]     memaddr_ev_id;
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] memaddr_ev_time;
   /* Global Event Signal */
   output reg                             event_valid_global;
   /* GPR Copy Collector Interface */
   output reg [31:0]                      bv_GPR;
   /* Stack Arguments Collector Interface */
   output reg [5:0]                       stackargs;
   /* Packetizer Interface */
   output reg [`DIAGNOSIS_EV_ID_WIDTH-1:0] ev_id;
   output reg [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] ev_time;

   /*******************************************************************/
   /*** Wires ***/
   /** Output buffer wiring **/
   wire                                        nxt_event_valid_global;
   reg [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0]        nxt_ev_time;
   reg [`DIAGNOSIS_EV_ID_WIDTH-1:0]            nxt_ev_id;
   wire [31:0]                                 nxt_bv_GPR;
   wire [5:0]                                  nxt_stackargs;

   /* Bitvector for comparator logic that indicates a match */
   wire [MAX_TOTAL_EVENT_COUNT-1:0]            comparator_match;

   /* LUT index wire */
   wire [INDEX_WIDTH-1:0]                      lut_index;

   /*******************************************************************/
   /** Lookuptable **/
   /* (Logical) Format:
    * GPR Bitvector: 32bit vector with desired General Purpose Registers as high bits
    * Stackarguments Nr: 6bit unsigned integer that holds number of desired stackargument words
    *
    *            +--------------------------------------------------------------+
    *            |  Event ID  |  GPR Bitvector  |  Stackarguments Nr  |  valid  | 
    *            |     ...    |       ...       |         ...         |   ...   | 
    *            |     ...    |       ...       |         ...         |   ...   |
    *            +--------------------------------------------------------------+
    */
   /* Actual 16bit config register format:
    *
    *    47                      32 31              16 15                 0
    *    +-------------------------+------------------+-------------------+
    *    |valid  xxx Stackargs  ID |    GPR_bv MSB    |     GPR_bv LSB    |
    *    |           ...           |        ...       |        ...        |
    *    |           ...           |        ...       |        ...        |
    *    +-------------------------+------------------+-------------------+
    * 
    */
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0]            lut_ev_id[0:MAX_TOTAL_EVENT_COUNT-1];
   wire [31:0]                                  lut_GPRbv[0:MAX_TOTAL_EVENT_COUNT-1];
   wire [5:0]                                   lut_stackargs[0:MAX_TOTAL_EVENT_COUNT-1];
   wire                                         lut_validcolumn[0:MAX_TOTAL_EVENT_COUNT-1];

   genvar                            j;
   generate
      for (j = 0; j < MAX_TOTAL_EVENT_COUNT; j = j + 1) begin: gen_conf_mem_mapping
         assign lut_validcolumn[j] = conf_lut_flat_in[j*48+47];
         assign lut_stackargs[j] = conf_lut_flat_in[j*48+32+`DIAGNOSIS_EV_ID_WIDTH+5:j*48+32+`DIAGNOSIS_EV_ID_WIDTH];
         assign lut_ev_id[j] = conf_lut_flat_in[j*48+32+`DIAGNOSIS_EV_ID_WIDTH-1:j*48+32];
         assign lut_GPRbv[j] = conf_lut_flat_in[j*48+31:j*48];
      end
   endgenerate
   
   /*******************************************************************/
   /* log2 function for index evaluation */
   function [INDEX_WIDTH-1:0] log2;
      input [MAX_TOTAL_EVENT_COUNT-1:0] vector_tmp;
      begin
         vector_tmp = vector_tmp-1;
         for (log2=0; vector_tmp>0; log2=log2+1)
           vector_tmp = vector_tmp>>1;
      end
   endfunction 
   
   /*******************************************************************/
   /** Logic **/
   /* Global event signal */
   assign nxt_event_valid_global = pc_ev_valid || fcnret_ev_valid || memaddr_ev_valid;

   /* Event ID and Time Signal Multiplexer */
   always @(*) begin
      if (pc_ev_valid) begin
         nxt_ev_id = pc_ev_id;
         nxt_ev_time = pc_ev_time;
      end else if (fcnret_ev_valid) begin
         nxt_ev_id = fcnret_ev_id;
         nxt_ev_time = fcnret_ev_time;
      end else if (memaddr_ev_valid) begin
         nxt_ev_id = memaddr_ev_id;
         nxt_ev_time = memaddr_ev_time;
      end else begin
         nxt_ev_id = 0;
         nxt_ev_time = 0;
      end
   end // always @ (*)

   /* Comparator logic to find proper LUT entry (index)*/
   genvar                                     i;
   for (i = 0; i < MAX_TOTAL_EVENT_COUNT; i=i+1) begin
      assign comparator_match[i] = (lut_validcolumn[i] && lut_ev_id[i] == nxt_ev_id) ? 1'b1 : 1'b0; 
   end
   
   /* LUT index generation logic */
   assign lut_index = log2(comparator_match);

   /* Snapshot Modules Output Multiplexer */
   assign nxt_bv_GPR = lut_GPRbv[lut_index];
   assign nxt_stackargs = lut_stackargs[lut_index];
   
   /*******************************************************************/
   /* sequential */
   always @(posedge clk) begin
      if (rst) begin
         event_valid_global <= 1'b0;
         ev_time <= 0;
         ev_id <= 0;
         bv_GPR <= 32'd0;
         stackargs <= 6'd0;
         //TODO: reset lut_validcolumn to zero
      end else begin
         event_valid_global <= nxt_event_valid_global;
         ev_time <= nxt_ev_time;
         ev_id <= nxt_ev_id;
         bv_GPR <= nxt_bv_GPR;
         stackargs <= nxt_stackargs;
      end
   end //always

  
endmodule // LUT_module
