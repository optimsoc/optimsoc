/* This module generates the snapshot packets for each event, which are then forwarded to the Co-CPU (e.g. via debug NoC).
 Therefore it detects the occurrence of a new event and collects all necessary data:
 * Event ID
 * Event Time

 This is the LITE version of the packetizer, which does not support of packetizer modules and is thus smaller.
 This data is collected, (in the current implementation wrapped into a LISNoC Packet), and then forwarded via NoC interface.
 */

`include "diagnosis_config.vh"
`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module Packetizer_lite (/*AUTOARG*/
   // Outputs
   conf_mem_flat_out, in_GPR_rdy, in_stack_rdy, dbgnoc_in_ready,
   dbgnoc_out_flit, dbgnoc_out_valid,
   // Inputs
   clk, rst, event_valid_global, ev_id, ev_time, in_GPR_data,
   in_GPR_type, in_GPR_valid, in_stack_data, in_stack_type,
   in_stack_valid, dbgnoc_in_flit, dbgnoc_in_valid, dbgnoc_out_ready
   );

   /*******************************************************************/
   /*** Parameters ***/
   /** Core ID **/
   parameter CORE_ID = 16'hx;
   
   /** Fifo lengths **/
   parameter gpr_fifo_length = `DIAGNOSIS_GPR_FIFO_LENGTH;
   parameter stack_fifo_length = `DIAGNOSIS_STACK_FIFO_LENGTH;
   parameter event_fifo_length = `DIAGNOSIS_EVENT_FIFO_LENGTH;

   /** Configuration memory: 16 bit flits **/
   /** We have currently 3 x 16 bit flits for one config entry (including valid flag). **/
   parameter CONF_MEM_SIZE = 2;
   
   
   /** NoC Parameters **/
   // parameters for the Debug NoC interface
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;

   /** Header Format: **/
   /*       +------------------------------------------------------------------------------+
    *       | Type  |  Dest  |  Src  |  Class  |  --  |  #Stack  |  #GPR  |  #Time  |  ID  |  
    *       +------------------------------------------------------------------------------+
    */ 
   //localparam flit_width = flit_data_width+flit_type_width;

   /*** Interfaces ***/
   input clk, rst;
   /*** Configuration registers ***/
   output [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;
   /* LUT module interface */
   input event_valid_global;
   input [`DIAGNOSIS_EV_ID_WIDTH-1:0] ev_id;
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] ev_time;
   /* GPR module interface */
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] in_GPR_data;
   input [2:0]                        in_GPR_type;
   input                              in_GPR_valid;
   output                             in_GPR_rdy;
   /* Stack module interface */
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] in_stack_data;
   input [2:0]                        in_stack_type;
   input                              in_stack_valid;
   output                             in_stack_rdy;
   /* Debug NoC interface */
   input [DBG_NOC_FLIT_WIDTH-1:0]     dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0]      dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;
  
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0]  dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0]       dbgnoc_out_ready;
   

   
   
   /*** Wires and Regs ***/
   
   
   // Event fifo output wiring
   wire                                ev_fifo_valid;
   wire [`DIAGNOSIS_EV_ID_WIDTH-1:0]   ev_fifo_id;
   wire [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] ev_fifo_time;
   reg                                   ev_fifo_ready;
   wire                                  ev_fifo_in_ready;

 

   /** dbgnoc_if  **/
   reg [33:0]                          out_data34; // Datastream of Packetizer FSM
   reg                                 out_data34_valid;
   wire                                out_data34_ready;

   packetizer_dbgnoc_if
     #(.CONF_MEM_SIZE(CONF_MEM_SIZE),
       .DBG_NOC_VCHANNELS(DBG_NOC_VCHANNELS),
       .CORE_ID(CORE_ID))
     u_packetizer_dbgnoc_if(
                            // Outputs
                            .in_data34_ready    (out_data34_ready),
                            .dbgnoc_out_flit    (dbgnoc_out_flit[DBG_NOC_FLIT_WIDTH-1:0]),
                            .dbgnoc_out_valid   (dbgnoc_out_valid[DBG_NOC_VCHANNELS-1:0]),
                            .dbgnoc_in_ready    (dbgnoc_in_ready[DBG_NOC_VCHANNELS-1:0]),
                            .sys_clk_disable    (),
                            .conf_mem_flat_out  (conf_mem_flat_out),
                            // Inputs
                            .clk                (clk),
                            .rst                (rst),
                            .in_data34          (out_data34[33:0]),
                            .in_data34_valid    (out_data34_valid),
                            .dbgnoc_out_ready   (dbgnoc_out_ready[DBG_NOC_VCHANNELS-1:0]),
                            .dbgnoc_in_flit     (dbgnoc_in_flit[DBG_NOC_FLIT_WIDTH-1:0]),
                            .dbgnoc_in_valid    (dbgnoc_in_valid[DBG_NOC_VCHANNELS-1:0]),
                            .full_packetizer_fifo(1'b0));

   
   /*** Input Fifos for Snapshot Modules and Event Fingerprints ***/
 


   lisnoc_fifo
     #(.LENGTH(event_fifo_length),
       .flit_data_width(`DIAGNOSIS_EV_ID_WIDTH + `DIAGNOSIS_TIMESTAMP_WIDTH),
       .flit_type_width(0))
     u_event_fifo(
                  // Outputs
                  .in_ready             (ev_fifo_in_ready),
                  .out_flit             ({ev_fifo_time,ev_fifo_id}),
                  .out_valid            (ev_fifo_valid),
                  // Inputs
                  .clk                  (clk),
                  .rst                  (rst),
                  .in_flit              ({ev_time, ev_id}),
                  .in_valid             (event_valid_global),
                  .out_ready            (ev_fifo_ready));


   /*** Finite State Machine ***/
   localparam STATE_WIDTH = 3;
   localparam STATE_IDLE = 3'd0;
   localparam STATE_Time_out = 3'd1;
   localparam STATE_Time_out2 = 3'd4;   
   localparam STATE_GPR_out = 3'd2;
   localparam STATE_Stack_out = 3'd3;
   
   reg [STATE_WIDTH-1:0]                                        state, nxt_state;

   /* FSM next state logic*/
   always @(*) begin
      case(state)
        STATE_IDLE: begin
           out_data34_valid = 1'b0;
           ev_fifo_ready = 1'b0;
           out_data34 = {2'b01,{(32-`DIAGNOSIS_EV_ID_WIDTH){1'hx}},ev_fifo_id}; // only forward ID at the moment
           // if a new event is in input buffer and output channel is ready, forward flit
           if (ev_fifo_valid) begin
              out_data34_valid = 1'b1;
              if (out_data34_ready) begin
                 nxt_state = STATE_Time_out;
              end else begin
                 nxt_state = state;
              end
           end else begin // if (ev_fifo_valid)
              nxt_state = state;
              out_data34_valid = 1'b0;
           end
              
        end // case: `STATE_IDLE_Headerout

        STATE_Time_out: begin
               
           ev_fifo_ready = 1'b0;
       
              out_data34_valid = 1'b1;
              // the time flit is the only payload and therefore a 'last' flit
             
                 out_data34 = {2'b10,{(32-`DIAGNOSIS_TIMESTAMP_WIDTH){1'hx}},ev_fifo_time}; //last flit
                 if (out_data34_ready) begin
                    nxt_state = STATE_IDLE;
                    ev_fifo_ready = 1'b1;// pop event from event fifo
                 end else begin
                    nxt_state = state;
                    ev_fifo_ready = 1'b0;
                 end
    
        end // case: `STATE_Time_out
  
        default: begin
           out_data34_valid = 1'b0;
           ev_fifo_ready = 1'b0;
           out_data34 = 34'd0;
           nxt_state = STATE_IDLE;
        end
        
      endcase // case (state)
   end
   
   /* sequential */
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
      end else begin
         state <= nxt_state;
      end
   end //always
   
endmodule // packetizer
