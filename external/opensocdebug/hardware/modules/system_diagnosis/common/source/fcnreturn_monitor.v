/* The function return monitor detects arbitrary function
 returns of a given function, independent of the function's
 return point.
 It bases on the fact, that even though a function may have
 several different return points, they however all have the
 same return address.
 A function is identified over the program counter value
 of its first point in the programme flow. This value has
 to be declared in the corresponding configuration registers.
 
 Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module fcnreturn_monitor (/*AUTOARG*/
   // Outputs
   fcnret_ev_valid, fcnret_ev_id, fcnret_ev_time,
   // Inputs
   clk, rst, diag_sys_enabled, conf_fcnret_flat_in, pc_val, pc_enable,
   wb_enable, wb_reg, wb_data, time_global
   );

   
   /** Parameters **/
   // Number of 16 bit configuration registers (3 per event declaration, for structure see below)
   parameter CONF_FCNRET_SIZE = 3;
   // Maximum number of allowed events that can be monitored at the same time
   parameter MAX_EVENT_COUNT = `DIAGNOSIS_FCNRET_EVENTS_MAX;
   // Index for multiplexer of lookuptable selection after comparator hit
   parameter INDEX_WIDTH = $clog2(MAX_EVENT_COUNT);
   // 
   // Maximum number of supported return addresses, before the first one returns 
   // This is needed, as e.g. the first function call and consecutive resursive calls
   // have different return addresses, and would overwrite each other otherwise
   parameter MAX_RETURN_ADDRESSES = `DIAGNOSIS_R9_ADDRESSES;
   
   /** Interface **/
   input clk, rst;
   /* configuration register interface */
   input diag_sys_enabled;
   input [16*CONF_FCNRET_SIZE-1:0] conf_fcnret_flat_in;
   /* mor1kx program counter interface */
   input [31:0] pc_val;
   input        pc_enable;
   /* writeback register interface */
   input        wb_enable;
   input [`DIAGNOSIS_WB_REG_WIDTH-1:0] wb_reg;
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] wb_data;
   /* event signal interface to LUT module */
   output reg                              fcnret_ev_valid;
   output reg [`DIAGNOSIS_EV_ID_WIDTH-1:0]  fcnret_ev_id;
   output reg [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] fcnret_ev_time;
   /* interface to global timestamp provider module */
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0]  time_global;
   


   /*********************************************************************************/
   /*** Registers and wiring ***/

   /* Output registers wiring */
   wire                                    nxt_fcnret_ev_valid;
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0]       nxt_fcnret_ev_id;
   
   /** Lookuptable for function entry and respective event id **/
   /* (Logical) Format:
    *
    *            +----------------------------------------------+
    *            | Event ID | Entry-PC-value |  valid entry PC  | 
    *            |   ...    |       ...      |        ...       |      
    *            |   ...    |       ...      |        ...       |  
    *            +----------------------------------------------+
    */
   /* Equivalent 16bit config register format (Data = Entry-PC value):
    *
    *    47            32 31           16 15            0
    *    +---------------+---------------+--------------+
    *    |valid  xxx  ID |   Data MSB    |   Data LSB   |
    *    |      ...      |      ...      |     ...      |
    *    |      ...      |      ...      |     ...      |
    *    +---------------+---------------+--------------+
    * 
    */
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0]        lut_ev_id[0:MAX_EVENT_COUNT-1];
   wire [31:0]                              lut_entry_pc[0:MAX_EVENT_COUNT-1];
   wire                                     lut_valid_entrypc[0:MAX_EVENT_COUNT-1];

   genvar                            j;
   generate
      for (j = 0; j < MAX_EVENT_COUNT; j = j + 1) begin: gen_conf_mem_mapping
         assign lut_ev_id[j] = conf_fcnret_flat_in[j*48+32+`DIAGNOSIS_EV_ID_WIDTH-1:j*48+32];
         assign lut_entry_pc[j] = conf_fcnret_flat_in[j*48+31:j*48];
         assign lut_valid_entrypc[j] = conf_fcnret_flat_in[j*48+47];    
      end
   endgenerate

   /* Copy of General Purpose Register R9 */
   reg [`DIAGNOSIS_WB_DATA_WIDTH-1:0]                             R9_copy;
   
   /* Bitvector for comparator logic that indicates a match */
   wire [MAX_EVENT_COUNT-1:0]         comparator_entry_match;
   //wire [MAX_EVENT_COUNT-1:0]         comparator_R9_match;

   /* LUT index wires */
   wire [INDEX_WIDTH-1:0]                      lut_entry_index;

   /* log2 function for index evaluation */
   function [INDEX_WIDTH-1:0] log2;
      input [MAX_EVENT_COUNT-1:0] vector_tmp;
      begin
         vector_tmp = vector_tmp-1;
         for (log2=0; vector_tmp>0; log2=log2+1)
           vector_tmp = vector_tmp>>1;
      end
   endfunction // for

   /*****************************************************************************/
   /*** R9 LIFO that stacks the next expected return values. Tuples: (ID, R9) ***/
   localparam lifo_width = `DIAGNOSIS_EV_ID_WIDTH + 32;
   wire push;
   wire pop;
   wire lifo_full;
   wire lifo_empty;
   wire [31:0] lifo_R9_in;
   wire [31:0] lifo_R9_out;
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0] lifo_id_in;
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0] lifo_id_out;
   wire [lifo_width-1:0]             lifo_in;
   wire [lifo_width-1:0]             lifo_out;
   assign lifo_in[31:0] = lifo_R9_in;
   assign lifo_in[lifo_width-1:32] = lifo_id_in;
   assign lifo_R9_out = lifo_out[31:0];
   assign lifo_id_out = lifo_out[lifo_width-1:32];

       
   lifo
     #(.depth(MAX_RETURN_ADDRESSES),
       .width(lifo_width))
   u_lifo(
          // Outputs
          .empty                        (lifo_empty),
          .full                         (lifo_full),
          .count                        (),
          .tos                          (lifo_out),
          // Inputs
          .clk                          (clk),
          .reset                        (rst),
          .push                         (push),
          .push_data                    (lifo_in),
          .pop                          (pop));

   
   
   /******************************************************************************/
   /*** Logic ***/
   
   /* writeback read logic */
   always @(posedge clk) begin
      if (wb_enable && wb_reg == 9) begin
         R9_copy <= wb_data;
      end
   end

   /* PC evaluation logic 
    * If function entry is detected, it pushes the current R9 value in LIFO.
    */
   genvar c;
   generate
      for (c = 0; c < MAX_EVENT_COUNT; c = c + 1) begin
         assign comparator_entry_match[c] = (pc_enable && lut_valid_entrypc[c] && pc_val == lut_entry_pc[c]) ? 1'b1 : 1'b0;   
      end
   endgenerate

   /* Input multiplexer and logic for (ID,R9) LIFO */
   assign lifo_id_in = lut_ev_id[lut_entry_index]; // push id in LIFO according to index of the signaling comparator
   assign lifo_R9_in = R9_copy;

   /** Push and pop logic **/
   /* if a program counter value matches one of the values in the lookuptable, push R9 in LIFO */
   assign push = (comparator_entry_match) ? 1'b1 : 1'b0; // if one comparator finds a match, push
   /* if we have a valid event, we pop R9 out of LIFO to discard it */
   assign pop = nxt_fcnret_ev_valid;

   /** Valid event detection logic: if we have a valid R9 in LIFO and it matches the current PC **/
   assign nxt_fcnret_ev_valid = (!lifo_empty && pc_enable && pc_val ==  lifo_R9_out) ? 1'b1 : 1'b0;
   
   /* index generation logic: index for LUT is index of high comparator bit */
   assign lut_entry_index = log2(comparator_entry_match);
   /* forward the event id in LIFO to output (which is valid, when the R9 in LIFO matches */
   assign nxt_fcnret_ev_id = lifo_id_out;
  
   /* sequential */
   always @(posedge clk) begin
      if (rst || ~diag_sys_enabled) begin
         fcnret_ev_valid <= 1'b0;
         fcnret_ev_id <= 0;
         fcnret_ev_time <= 0;
      end else begin
         fcnret_ev_valid <= nxt_fcnret_ev_valid;
         fcnret_ev_id <= nxt_fcnret_ev_id;
         fcnret_ev_time <= time_global;
      end
   end // always @ (posedge clk)

      
endmodule // fcnreturn_monitor
