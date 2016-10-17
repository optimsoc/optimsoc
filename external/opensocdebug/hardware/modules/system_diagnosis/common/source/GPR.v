/* This module enables access to arbitrary General Purpose Registers 
 Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module GPR (/*AUTOARG*/
   // Outputs
   out_GPR_data, out_GPR_type, out_GPR_valid, stackarg_data_o,
   // Inputs
   clk, rst, wb_enable, wb_reg, wb_data, bv_GPR, bv_valid,
   out_GPR_rdy, stackarg_addr_i
   );

   input clk, rst;
   /* writeback register interface */
   input        wb_enable;
   input [`DIAGNOSIS_WB_REG_WIDTH-1:0]  wb_reg;
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] wb_data;
   /* LUT demanded data interface */
   input [31:0] bv_GPR;
   input        bv_valid;
   /* Packetizer interface */
   output [`DIAGNOSIS_WB_DATA_WIDTH:0] out_GPR_data;
   output reg [2:0] out_GPR_type;
   output reg    out_GPR_valid;
   input         out_GPR_rdy;
   /* Stack module interface */
   input [`DIAGNOSIS_WB_REG_WIDTH-1:0]   stackarg_addr_i;
   output [`DIAGNOSIS_WB_DATA_WIDTH-1:0] stackarg_data_o;
   

   /*** Parameters ***/
   // depth of input buffer for incoming event triggers
   localparam event_fifo_length = `DIAGNOSIS_EVENT_FIFO_LENGTH;
   // amount of GPR shadow sets (more than one enables bursts of events while processing)
   localparam parallel_gpr_sets = `DIAGNOSIS_parallel_gprshots;

   /* select signal for data multiplexer */
   reg [4:0]    mux_select, mux_select_tmp;
   reg [4:0]    old_mux_sel, nxt_old_mux_sel;
  

   /* Registers for local GPR copy */
   reg [`DIAGNOSIS_WB_DATA_WIDTH-1:0]    GPR[0:31];
   reg [`DIAGNOSIS_WB_DATA_WIDTH-1:0]    GPR_shot[0:31][0:parallel_gpr_sets-1];
   // bitvector that indicates which GPRs are wanted
   reg [31:0]    GPR_bv, nxt_GPR_bv;

   /* Pointers for selection of read and write to the set of GPR snapshots */
   reg [2:0]     rd_ptr, nxt_rd_ptr;
   reg [2:0]     wr_ptr, nxt_wr_ptr;
   reg [1:0]      swapflags, nxt_swapflags; // flag that indicates if GPR_shot "buffer" is full or empty: [rd,wr]
   wire           full;
   assign full = (rd_ptr == wr_ptr) && (swapflags == 2'b01 || swapflags == 2'b10);
   
   /* log2 function for bitvector decryption */
   function [4:0] log2;
      input [31:0] bv_tmp;
      begin
         bv_tmp = bv_tmp-1;
         for (log2=0; bv_tmp>0; log2=log2+1)
           bv_tmp = bv_tmp>>1;
      end
   endfunction

   /* Multiplexer for Stack module read interface */
   assign stackarg_data_o = GPR[stackarg_addr_i];
   
   /* Shift register for delay of write back signal */
   wire wb_enable_delayed;
   wire [`DIAGNOSIS_WB_REG_WIDTH-1:0] wb_reg_delayed;
   wire [`DIAGNOSIS_WB_DATA_WIDTH-1:0] wb_data_delayed;
   
   wb_sr
     wb_sr(
           // Outputs
           .wb_enable_o                 (wb_enable_delayed),
           .wb_reg_o                    (wb_reg_delayed),
           .wb_data_o                   (wb_data_delayed),
           // Inputs
           .clk                         (clk),
           .rst                         (rst),
           .wb_enable_i                 (wb_enable),
           .wb_reg_i                    (wb_reg),
           .wb_data_i                   (wb_data));

   /*** Input fifo for gpr bitvectors, for pipelining of events ***/
   wire [31:0]                         bv_GPR_fifo;
   wire                                bv_valid_fifo;
   reg                                 bv_ready_fifo;
   //indicates, whether the GPR snapshot could be saved or not (full GPR set)
   wire                                full_flag;
   wire [32:0]                         fifo_output;
   wire [32:0]                         fifo_input;
   assign bv_GPR_fifo = fifo_output[31:0];
   assign full_flag = fifo_output[32];
   assign fifo_input[31:0] = bv_GPR;
   assign fifo_input[32] = full;

   lisnoc_fifo
     #(.LENGTH(event_fifo_length),
       .flit_data_width(33),
       .flit_type_width(0))
   u_bv_fifo(
             // Outputs
             .in_ready                  (),
             .out_flit                  (fifo_output),
             .out_valid                 (bv_valid_fifo),
             // Inputs
             .clk                       (clk),
             .rst                       (rst),
             .in_flit                   (fifo_input),
             .in_valid                  (bv_valid),
             .out_ready                 (bv_ready_fifo));  
 

   /* writeback read logic */
   always @(posedge clk) begin
      if (wb_enable_delayed) begin
         GPR[wb_reg_delayed] <= wb_data_delayed;
      end
   end 

   /* copy GPR snapshot logic */
   /* only make a set copy, if sets are not full and at least one GPR wanted */
   integer i;
   always @(posedge clk) begin
      if (bv_valid && bv_GPR != 0 && !full) begin
         for (i = 0; i < 32; i=i+1) begin
            GPR_shot[i][wr_ptr] <= GPR[i];
         end
      end
   end

   /* write pointer for GPR snapshot logic */
   always @(*) begin
      if (bv_valid && bv_GPR != 0 && !full) begin
         if (wr_ptr == parallel_gpr_sets - 1) begin
            nxt_wr_ptr = 0;
            nxt_swapflags[0] = ~swapflags[0];
         end else begin
            nxt_wr_ptr = wr_ptr + 1;
            nxt_swapflags[0] = swapflags[0];
         end
      end else begin
         nxt_wr_ptr = wr_ptr;
         nxt_swapflags[0] = swapflags[0];
      end
   end

   /* Multiplexer GPR->output */
   assign out_GPR_data = GPR_shot[mux_select][rd_ptr];
   
 /*** FSM ***/
   
   localparam STATE_WIDTH = 2;
   localparam STATE_IDLE = 2'b00;
   localparam STATE_noregswanted = 2'b01;
   localparam STATE_decodebv = 2'b10;
   
   reg [STATE_WIDTH-1:0]              state, nxt_state;
   
   /* FSM next state logic*/
   always @(*) begin
      case(state)
        /* This state checks if there is a new event in the event input buffer.
           The buffer contains a tuple (full_flag, bv_GPR_fifo), i.e. 
           an indicator if a GPR set could be saved for the event, and which registers
           are wanted. If a set could be stored and registers are wanted, it reads
           them out to the packetizer module */
        STATE_IDLE: begin
           out_GPR_valid = 1'b0;
           out_GPR_type = `SNAPSHOT_FLIT_TYPE_NONE;
           mux_select = 5'd0;
           nxt_old_mux_sel = 5'd0;
           nxt_rd_ptr = rd_ptr;
           nxt_swapflags[1] = swapflags[1];
           if (bv_valid_fifo) begin
              bv_ready_fifo = 1'b1;
              // check if the GPR snapshot could be generated
              // this is not the case, if the GPR set was full during the event
              if (!full_flag) begin
                 // check if bv contains flag bits
                 if (bv_GPR_fifo) begin
                    nxt_state = STATE_decodebv;
                    nxt_GPR_bv = bv_GPR_fifo;
                 end else begin
                    nxt_state = STATE_noregswanted;
                    nxt_GPR_bv = GPR_bv;
                 end   
              end else begin
                 nxt_state = STATE_noregswanted;
                 nxt_GPR_bv = GPR_bv;
              end
           end else begin // if (bv_valid_fifo)
              bv_ready_fifo = 1'b0;
              nxt_state = state;
              nxt_GPR_bv = GPR_bv;
           end
        end

        /* This state is needed if either no GPR registers are wanted for
           an event, or if the GPR shot could not be stored as all sets were full */
        STATE_noregswanted: begin
           bv_ready_fifo = 1'b0;
           out_GPR_valid = 1'b1;
           out_GPR_type = `SNAPSHOT_FLIT_TYPE_NONE;
           mux_select = 5'd0;
           nxt_old_mux_sel = 5'd0;
           nxt_GPR_bv = GPR_bv;
           nxt_rd_ptr = rd_ptr; // as we do not have a valid set, we keep rd_ptr
           nxt_swapflags[1] = swapflags[1];
           if (out_GPR_rdy) begin
              nxt_state = STATE_IDLE;
           end else begin
              nxt_state = state;
           end
        end

        /* This state sequentially forwards the requested GPR registers to the packetizer */
        STATE_decodebv: begin
           out_GPR_valid = 1'b1;
           bv_ready_fifo = 1'b0;
           /* Index of next '1' in bitvector bv is
            index = log2(bv AND (twocomplement(bv))). 
            Add value of old select, as bv is shifted afterwards */
           mux_select_tmp = log2(GPR_bv & (~GPR_bv + 'b1));
           mux_select = mux_select_tmp + old_mux_sel;
           if (out_GPR_rdy) begin
              nxt_old_mux_sel = mux_select + 1;
              nxt_GPR_bv = GPR_bv >> (mux_select_tmp + 1);
           end else begin
              nxt_old_mux_sel = old_mux_sel;
              nxt_GPR_bv = GPR_bv;
           end

           if (out_GPR_rdy) begin
              if (nxt_GPR_bv == 32'd0) begin
                 nxt_state = STATE_IDLE;
                 if (rd_ptr == parallel_gpr_sets - 1) begin
                    nxt_rd_ptr = 0;
                    nxt_swapflags[1] = ~swapflags[1];
                 end else begin
                    nxt_rd_ptr = rd_ptr + 1;
                    nxt_swapflags[1] = swapflags[1];
                 end
                 if (old_mux_sel == 5'd0) begin
                    out_GPR_type = `SNAPSHOT_FLIT_TYPE_SINGLE;
                 end else begin 
                    out_GPR_type = `SNAPSHOT_FLIT_TYPE_LAST;
                 end
              end else begin // if (nxt_GPR_bv == 32'd0)
                 nxt_rd_ptr = rd_ptr;
                 nxt_state = state;
                 nxt_swapflags[1] = swapflags[1];
                 if (old_mux_sel == 5'd0) begin
                    out_GPR_type = `SNAPSHOT_FLIT_TYPE_FIRST; 
                 end else begin
                    out_GPR_type = `SNAPSHOT_FLIT_TYPE_MIDDLE; 
                 end
              end // else: !if(nxt_GPR_bv == 32'd0)
           end else begin // if (out_GPR_rdy)
              nxt_rd_ptr = rd_ptr;
              nxt_state = state;
              nxt_swapflags[1] = swapflags[1];
              out_GPR_type = `SNAPSHOT_FLIT_TYPE_NONE;
           end
        end

        default: begin
           mux_select = 5'd0;
           out_GPR_valid = 1'b0;
           bv_ready_fifo = 1'b0;
           nxt_GPR_bv = GPR_bv;
           nxt_rd_ptr = 0;
           nxt_swapflags[1] = swapflags[1];
           out_GPR_type = `SNAPSHOT_FLIT_TYPE_NONE;
           nxt_state = STATE_IDLE;
           nxt_old_mux_sel = old_mux_sel;
        end
      endcase // case (state)
      
      
   end // always @ (*)
   

      
   /* sequential */
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         GPR_bv <= 32'd0;
         old_mux_sel <= 5'd0;
         rd_ptr <= 0;
         wr_ptr <= 0;
         swapflags <= 2'b00;
      end else begin
         state <= nxt_state;
         GPR_bv <= nxt_GPR_bv;
         old_mux_sel <= nxt_old_mux_sel;
         rd_ptr <= nxt_rd_ptr;
         wr_ptr <= nxt_wr_ptr;
         swapflags <= nxt_swapflags;
      end
   end //always

endmodule // GPR
