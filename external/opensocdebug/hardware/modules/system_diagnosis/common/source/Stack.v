/* This modules enables access to function arguments that are stored on the stack. 
 Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module Stack (/*AUTOARG*/
   // Outputs
   gpr_addr_o, out_stack_data, out_stack_type, out_stack_valid,
   // Inputs
   clk, rst, trace_insn, trace_enable, args_in, args_valid,
   gpr_data_i, out_stack_rdy
   );

   input clk, rst;
   /* mor1kx instruction trace interface */
   input [31:0] trace_insn;
   input        trace_enable;
   /* LUT demanded data interface */
   input [5:0]  args_in;
   input        args_valid;
   /* GPR Module interface for register access */
   output [`DIAGNOSIS_WB_REG_WIDTH-1:0] gpr_addr_o;
   input [`DIAGNOSIS_WB_DATA_WIDTH-1:0] gpr_data_i;
   /* Packetizer interface */
   output [`DIAGNOSIS_WB_DATA_WIDTH:0] out_stack_data;
   output reg [2:0] out_stack_type;
   output reg    out_stack_valid;
   input         out_stack_rdy;


   /**** Parameters ****/
   parameter stack_sets = `DIAGNOSIS_parallel_stackshots;
   localparam event_fifo_length = `DIAGNOSIS_EVENT_FIFO_LENGTH;
   
   /** Buffer for storage of temporary stack arguments */
   reg [`DIAGNOSIS_WB_DATA_WIDTH-1:0] stackargs[0:`DIAGNOSIS_STACKARGS_MAX - 1];
   reg [`DIAGNOSIS_WB_DATA_WIDTH-1:0] stackargs_shot[0:`DIAGNOSIS_STACKARGS_MAX - 1][0:stack_sets-1];
   reg [5:0]                          args_rdptr, nxt_args_rdptr;
   /** Buffer for storage of number of desired function arguments **/
   reg [5:0]                          argsbuf, nxt_argsbuf;

   /* Pointers for selection of read and write to the set of Stack snapshots */
   reg [2:0]                          rd_ptr, nxt_rd_ptr;
   reg [2:0]                          wr_ptr, nxt_wr_ptr;
   reg [1:0]                          swapflags, nxt_swapflags; // flag that indicates if stackarg "buffer" is full or empty: [rd,wr]
   wire                               full;
   /* we need this signal to avoid overwriting sets that still need to be processed */
   assign full = (rd_ptr == wr_ptr) && (swapflags == 2'b01 || swapflags == 2'b10);
   
   /* Multiplexer for argument output to packetizer module */
   /* rd_ptr selects the set, args_rdptr selects the line, i.e. the argument in the set */
   assign out_stack_data = stackargs_shot[args_rdptr][rd_ptr];

   /* copy Stack snapshot logic */
   /* if all sets are full, do not write and also not increment wr_ptr */
   /* if no args wanted, do it neither */
   integer i;
   always @(posedge clk) begin
      if (args_valid && args_in != 0 && !full) begin
         for (i = 0; i < `DIAGNOSIS_STACKARGS_MAX; i=i+1) begin
            stackargs_shot[i][wr_ptr] <= stackargs[i];
         end
      end
   end

   /* write pointer for stackarg snapshot logic */
   /* if all sets are full, do not write and also not increment wr_ptr */
   /* if no args wanted, do it neither */
   always @(*) begin
      if (args_valid && args_in != 0 && !full) begin
         if (wr_ptr == stack_sets - 1) begin
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
   
   /*** Input fifo for gpr bitvectors, for pipelining of events ***/
   wire [31:0]                         args_fifo;
   wire                                args_valid_fifo;
   reg                                 args_ready_fifo;
   //indicates, whether the arguments snapshot could be saved or not (full arguments set)
   wire                                full_flag;
   wire [32:0]                         fifo_output;
   wire [32:0]                         fifo_input;
   assign args_fifo = fifo_output[31:0];
   assign full_flag = fifo_output[32];
   assign fifo_input[31:0] = args_in;
   assign fifo_input[32] = full;

   lisnoc_fifo
     #(.LENGTH(event_fifo_length),
       .flit_data_width(33),
       .flit_type_width(0))
   u_bv_fifo(
             // Outputs
             .in_ready                  (),
             .out_flit                  (fifo_output),
             .out_valid                 (args_valid_fifo),
             // Inputs
             .clk                       (clk),
             .rst                       (rst),
             .in_flit                   (fifo_input),
             .in_valid                  (args_valid),
             .out_ready                 (args_ready_fifo));
   
    
   
   /* delay of instruction signals with shift registers */
   wire [32:0]   trace_in;
   wire [32:0]   trace_out_delayed;
   wire [31:0]   trace_insn_delayed;
   wire          trace_enable_delayed;
   assign trace_in = {trace_enable, trace_insn};
   assign trace_insn_delayed = trace_out_delayed[31:0];
   assign trace_enable_delayed = trace_out_delayed[32];
   
   stm_data_sr
     #(.DELAY_CYCLES(`DIAGNOSIS_SNAPSHOT_DELAY),
       .DATA_WIDTH(33))
     shiftreg(
              // Outputs
              .dout                     (trace_out_delayed),
              // Inputs
              .clk                      (clk),
              .rst                      (rst),
              .din                      (trace_in));


   /**** Instruction decode, GPR fetch and stackargument storage ****/
   wire [4:0]    rB;
   assign rB = trace_insn_delayed[15:11];
   wire [10:0]   offset;
   wire [10:0]   offset_index;
   assign offset = trace_insn_delayed[10:0];
   assign offset_index = offset >> 2;
   assign gpr_addr_o = rB;
    /* Instruction format for 'l.sw I(rA), rB' on openrisc:
    *
    *            +---------------------------------------------------------+
    *            |31     26|25      21|20      16|15      11|10           0| 
    *            |  Opcode |   sign   |    rA    |    rB    |       I      |
    *            |  110101 |   xxxx   |   00001  |   store  | offset_index |
    *            +---------------------------------------------------------+
    */
   
   always @(posedge clk) begin
      // if ( instr valid && instr == l.sw && rA == 1 && offset >= 0 && offset_index < STACKARGS_MAX)
      if ( trace_enable_delayed && (trace_insn_delayed[31:26] ==  6'b110101) && 
           (trace_insn_delayed[20:16] == 5'd1) && (offset[10] == 1'b0) && 
           (offset_index < `DIAGNOSIS_STACKARGS_MAX)) begin

         // store stack argument on respective index in local buffer
         stackargs[offset_index] = gpr_data_i;
      end     
   end

    /*** FSM  ***/
   localparam STATE_WIDTH = 1;
   localparam STATE_IDLE = 1'b0;
   localparam STATE_forwardargs = 1'b1;
   reg [STATE_WIDTH-1:0]      state, nxt_state;
   
 /* FSM next state logic */
   always @(*) begin
      case(state)
        STATE_IDLE: begin
           // check if new event occurred and save desired number of fcn arguments
           out_stack_valid = 1'b0;
           out_stack_type = `SNAPSHOT_FLIT_TYPE_NONE;
           nxt_args_rdptr = 5'd0;
           nxt_rd_ptr = rd_ptr;
           nxt_swapflags[1] = swapflags[1];
           
           if (args_valid_fifo) begin
              args_ready_fifo = 1'b1;
              // check if the args snapshot could be generated
              // this is not the case, if the args set was full during the event
              if (!full_flag) begin
                 nxt_argsbuf = args_fifo;
              end else begin
                 nxt_argsbuf = 0;
              end
              nxt_state = STATE_forwardargs;
           end else begin // if (args_valid_fifo)
              args_ready_fifo = 1'b0;
              nxt_state = state;
              nxt_argsbuf = argsbuf;
           end
        end // case: `STATE_IDLE

        STATE_forwardargs: begin
           args_ready_fifo = 1'b0;
           // forward desired amount of function arguments
           out_stack_valid = 1'b1;
           nxt_argsbuf = argsbuf;
           // if amount of desired function args > 0, forward them
           if (out_stack_rdy) begin
             
              if (argsbuf) begin
                 nxt_args_rdptr = args_rdptr + 1;
                 // if rdptr is about to get out of range, it is last or single flit
                 if (nxt_args_rdptr == argsbuf) begin
                    nxt_state = STATE_IDLE;
                    if (rd_ptr == stack_sets - 1) begin
                       nxt_rd_ptr = 0;
                       nxt_swapflags[1] = ~swapflags[1];
                    end else begin
                       nxt_rd_ptr = rd_ptr + 1;
                       nxt_swapflags[1] = swapflags[1];
                    end
                    if (argsbuf == 6'd1) begin
                       out_stack_type = `SNAPSHOT_FLIT_TYPE_SINGLE;
                    end else begin
                       out_stack_type = `SNAPSHOT_FLIT_TYPE_LAST;
                    end
                    // otherwise: if rdptr is still in range, it is first or middle
                 end else begin
                    nxt_state = state;
                    nxt_rd_ptr = rd_ptr;
                    nxt_swapflags[1] = swapflags[1];
                    if (args_rdptr) begin
                       out_stack_type = `SNAPSHOT_FLIT_TYPE_MIDDLE;
                    end else begin
                       out_stack_type = `SNAPSHOT_FLIT_TYPE_FIRST;
                    end
                 end
              end else begin 
                 // if (!argsbuf): desired args is zero, send 'NONE' type
                 // and also dont touch rd_ptr, as we do not have a set
                 out_stack_type = `SNAPSHOT_FLIT_TYPE_NONE;
                 nxt_args_rdptr = args_rdptr;
                 nxt_state = STATE_IDLE;
                 nxt_rd_ptr = rd_ptr;
                 nxt_swapflags[1] = swapflags[1];
              end // else: !if(argsbuf)
           end else begin // if (out_stack_rdy)
              nxt_state = state;
              nxt_rd_ptr = rd_ptr;
              nxt_swapflags[1] = swapflags[1];
              nxt_args_rdptr = args_rdptr;
              out_stack_type = `SNAPSHOT_FLIT_TYPE_NONE;
           end
           
        end // case: `STATE_forwardargs

        default: begin
           out_stack_valid = 1'b0;
           out_stack_type = `SNAPSHOT_FLIT_TYPE_NONE;
           nxt_args_rdptr = 5'd0;
           nxt_rd_ptr = 0;
           nxt_swapflags[1] = swapflags[1];
           nxt_state = STATE_IDLE;
           nxt_argsbuf = 6'd0;
           args_ready_fifo = 1'b0;
        end
      endcase // case (state)
   end // always @ (*)
   
      
  
  
    /* sequential */
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         argsbuf <= 5'd0;
         args_rdptr <= 5'd0;
         rd_ptr <= 0;
         wr_ptr <= 0;
         swapflags <= 2'b00;
      end else begin
         state <= nxt_state;
         argsbuf <= nxt_argsbuf;
         args_rdptr <= nxt_args_rdptr;
         rd_ptr <= nxt_rd_ptr;
         wr_ptr <= nxt_wr_ptr;
         swapflags <= nxt_swapflags;
      end
   end //always
   
   
endmodule
