`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 15.06.2016 15:59:29
// Design Name: 
// Module Name: mp_mmio_bridge
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module mp_to_wb_bridge (
    // Inputs
   clk, rst, noc_out_ready, noc_in_flit, noc_in_valid,
   wb_dat_i, wb_ack_i, wb_err_i, wb_rty_i,
   
   // Outputs
   wb_dat_o, wb_we_o, wb_stb_o, wb_adr_o, wb_cyc_o, wb_sel_o,
   noc_out_flit, noc_out_valid, noc_in_ready
    );
    
	parameter TILEID = 0;

    //parameter vchannels = 2;
    parameter flit_data_width = 32;
    parameter flit_type_width = 2;
    localparam flit_width = flit_data_width+flit_type_width;
    
    parameter N = 32;
    
    input clk, rst;
    input noc_out_ready; // only VC for Message Passing, DMA separate
    input [flit_width-1:0] noc_in_flit; 
    input noc_in_valid; // only VC for Message Passing, DMA separate
    
    input [N-1:0] wb_dat_i;
    input wb_ack_i;
    input wb_err_i;
    input wb_rty_i;
    
    output reg [N-1:0] wb_dat_o;
    output reg wb_we_o;
    output reg wb_stb_o;
    output reg [N-1:0] wb_adr_o;
    output reg wb_cyc_o;
    output reg [3:0] wb_sel_o = 4'hF;
    
    output [flit_width-1:0] noc_out_flit;
    output noc_out_valid; // only VC for Message Passing, DMA separate
    output noc_in_ready;  // only VC for Message Passing, DMA separate
    
    reg out_ready = 0;
    wire [flit_width-1:0] out_flit;
    wire out_valid;
    reg [N-1:0] reg_header = 0;
    
   
    lisnoc_packet_buffer inst_buffer_receive (
        .clk(clk),
        .rst(rst),
        .in_flit(noc_in_flit),
        .in_valid(noc_in_valid),
        .out_ready(out_ready),
        .in_ready(noc_in_ready),
        .out_flit(out_flit),
        .out_valid(out_valid),
        .out_size()
        );
    
    //---------------------------------------------------------------------------------
    // Receive FSM
    // reads incoming NoC packets from lisnoc_packet_buffer and stores header and read data in registers
    // format of incoming NoC packets: 
    //         bit #: 33 |32 |31  27|26   24|23   19|  18   |17     9|     8     |7         0|
    //  read request:  1 | 1 | DEST | CLASS |  SRC  | R = 0 | UNUSED | CLASS_NUM | R_ADDRESS |
    // write request:  0 | 1 | DEST | CLASS |  SRC  | W = 1 | UNUSED | CLASS_NUM | W_ADDRESS |
    //                 1 | 0 |                   32-bit WRITE DATA                           |
        
    reg [2:0] state_r = 0;
    reg [2:0] next_state_r;
    reg [N-1:0] reg_dat_wr = 0;
    reg [N-1:0] reg_dat_rd = 0;
    reg header_waiting_rst;
   
    parameter RESET_r = 3'b000, WRITE_r = 3'b001, READ_r = 3'b010, WAIT_ACK_WRITE_r = 3'b011, WAIT_READY_r = 3'b100;
    parameter R_W_bit = 18;
    
    parameter cam_module_base_adr = 32'hd0000000;
   
    reg header_ready = 0;
    reg header_waiting = 0;
    
    // output logic
    always @ (*)
    begin
        case (state_r)
        RESET_r:
        begin
            out_ready = 1;
            wb_dat_o = 0;
            wb_we_o = 0;
            wb_cyc_o = 0;
            wb_stb_o = 0;
            wb_adr_o = 0;
            header_waiting_rst = 0;                   
        end   
        READ_r:
        begin
            wb_we_o = 0;
            wb_adr_o = {cam_module_base_adr[31:8], reg_header[7:0]};
            wb_cyc_o = 1;
            wb_stb_o = 1;
            out_ready = 0;
            wb_dat_o = 0;
            header_waiting_rst = 1;
        end
        WRITE_r:
        begin
             wb_we_o = 1;
             wb_adr_o = {cam_module_base_adr[31:8], reg_header[7:0]};
             wb_cyc_o = 1;
             wb_stb_o = 1;
             wb_dat_o = out_flit[31:0];
             out_ready = 1;
             header_waiting_rst = 1;               
        end
        WAIT_ACK_WRITE_r:
        begin
            wb_we_o = 1;
            wb_adr_o = {cam_module_base_adr[31:8], reg_header[7:0]};
            wb_cyc_o = 1;
            wb_stb_o = 1;
            wb_dat_o = reg_dat_wr;
            out_ready = 0; 
            header_waiting_rst = 0;
        end
        WAIT_READY_r:
        begin
            out_ready = 0;
            wb_dat_o = 0;
            wb_we_o = 0;
            wb_cyc_o = 0;
            wb_stb_o = 0;
            wb_adr_o = 0;
            header_waiting_rst = 0;                  
        end
        default:
        begin
            out_ready = 1;
            wb_dat_o = 0;
            wb_we_o = 0;
            wb_cyc_o = 0;
            wb_stb_o = 0;
            wb_adr_o = 0;
            header_waiting_rst = 0;   
        end     
        endcase      
    end
               
    // next state logic
    always @ (*)
    begin
        case (state_r)
        RESET_r:
        begin
            next_state_r = RESET_r;
            if (wb_rty_i == 1) begin
                next_state_r = WAIT_READY_r;
            end else begin
                if (out_valid == 1) begin                
                    if (out_flit[R_W_bit] == 1) begin
                        next_state_r = WRITE_r; 
                    end else begin
                        next_state_r = READ_r;
                    end
                end
            end
        end
        READ_r:
        begin
            next_state_r = READ_r;
            if (wb_ack_i == 1) begin
                next_state_r = RESET_r;
            end
        end
        WRITE_r:
        begin
            next_state_r = WAIT_ACK_WRITE_r;
            if (wb_ack_i == 1) begin
                next_state_r = RESET_r;
            end else begin
                next_state_r = WAIT_ACK_WRITE_r;
            end
        end
        WAIT_ACK_WRITE_r:
        begin
            next_state_r = WAIT_ACK_WRITE_r;
            if (wb_ack_i == 1) begin
                next_state_r = RESET_r;
            end 
        end
        WAIT_READY_r:
        begin
            next_state_r = WAIT_READY_r;
            if (wb_rty_i == 0) begin
                if (header_waiting == 1) begin
                    if (reg_header[R_W_bit] == 1) begin
                        next_state_r = WRITE_r; 
                    end else begin
                        next_state_r = READ_r;
                    end
                 end else begin                    
                    next_state_r = RESET_r;
                 end
            end        
        end
        default:
        begin
            next_state_r = RESET_r;
        end
        endcase                        
    end
        
    // update register values
    always @ (posedge clk)
    begin
        if (rst == 1) begin
            state_r <= RESET_r;
            reg_header <= 0;
            reg_dat_rd <= 0;
            reg_dat_wr <= 0;
            header_waiting <= 0;
            header_ready = 0;
        end else begin
            if (header_waiting_rst == 1) begin
                header_waiting <= 0;
            end
            if ((state_r == RESET_r) && (next_state_r == WRITE_r || next_state_r == READ_r )) begin
                reg_header <= out_flit[31:0];
                header_ready = 1;
            end else if ((state_r == RESET_r) && (next_state_r == WAIT_READY_r && out_valid == 1)) begin
                reg_header <= out_flit[31:0];
                header_ready = 1;
                header_waiting <= 1;
            end else begin
                header_ready = 0;
            end
            if ((state_r == WRITE_r) && (next_state_r == WAIT_ACK_WRITE_r || next_state_r == RESET_r)) begin
                reg_dat_wr <= out_flit[31:0];
            end
            if ((state_r == READ_r) && (next_state_r == RESET_r)) begin
                reg_dat_rd <= wb_dat_i;
                //reg_dat_rd <= 32'hFEEFFEFF;
            end
            state_r <= next_state_r;
        end
    end
    //---------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------
    // Send FSM
    // writes response NoC packets into lisnoc_packet_buffer
    // write response: header
    // read response:  header, read data
    
    reg [flit_width-1:0] send_in_flit;
    reg send_in_flit_valid;
    wire w_in_ready;
    wire[3:0] w_out_size;
    
    lisnoc_packet_buffer inst_buffer_send (
            .clk(clk),
            .rst(rst),
            .in_flit(send_in_flit),
            .in_valid(send_in_flit_valid),
            .out_ready(noc_out_ready),
            .in_ready(w_in_ready),
            .out_flit(noc_out_flit),
            .out_valid(noc_out_valid),
            .out_size(w_out_size)
            );
    
    reg [2:0] state_s = 0;
    reg [2:0] next_state_s = 0;
    parameter RESET_s = 3'b000, HEADER_READ_s = 3'b001, HEADER_WRITE_s = 3'b010, READ_DATA_s = 3'b011, WAIT_ACK_s = 3'b100;

    
    always @ (*)
    begin
        case (state_s)
        RESET_s:
        begin
            send_in_flit = 0;
            send_in_flit_valid = 0;
        end
        HEADER_READ_s:
        begin
			send_in_flit[33:32] = 2'b01;
			send_in_flit[31:27] = reg_header[23:19];
			send_in_flit[26:24] = reg_header[26:24];
			send_in_flit[23:19] = TILEID;
			send_in_flit[18:0] = reg_header[18:0];
            //send_in_flit = {2'b01, reg_header[23:19], reg_header[26:24], reg_header[31:27], reg_header[18:0]};
            //send_in_flit = {2'b01, reg_header[31:0]};
            send_in_flit_valid = 1;
        end
        HEADER_WRITE_s:
        begin
			send_in_flit[33:32] = 2'b11;
			send_in_flit[31:27] = reg_header[23:19];
			send_in_flit[26:24] = reg_header[26:24];
			send_in_flit[23:19] = TILEID;
			send_in_flit[18:0] = reg_header[18:0];
            //send_in_flit = {2'b11, reg_header[23:19], reg_header[26:24], reg_header[31:27], reg_header[18:0]};
            //send_in_flit = {2'b11, reg_header[31:0]};
            send_in_flit_valid = 1;
        end
        READ_DATA_s:
        begin
            send_in_flit = {2'b10, reg_dat_rd};
            send_in_flit_valid = 1;
        end
        WAIT_ACK_s:
        begin
            send_in_flit = 0;
            send_in_flit_valid = 0;
        end
        default:
        begin
            send_in_flit = 0;
            send_in_flit_valid = 0;
        end
        endcase
    end
    
    always @ (*)
    begin
        case (state_s)
        RESET_s:
        begin
            next_state_s = RESET_s;
            if (header_ready == 1) begin
                if (reg_header[R_W_bit] == 0) begin
                    next_state_s = HEADER_READ_s;
                end else begin
                    if (wb_ack_i == 1) begin
                        next_state_s = HEADER_WRITE_s;
                    end else begin
                        next_state_s = WAIT_ACK_s;
                    end
                end
            end
        end
        HEADER_READ_s:
        begin
            next_state_s = WAIT_ACK_s;
            if (next_state_r == RESET_r || state_r == RESET_r) begin
                next_state_s = READ_DATA_s;
            end
        end 
        HEADER_WRITE_s:
        begin
            next_state_s = RESET_s;
            if (out_valid == 1) begin
            // if ACK there in same clock cycle
                if (out_flit[R_W_bit] == 0) begin
                    next_state_s = HEADER_READ_s;
                end else begin
                    if (wb_ack_i == 1) begin
                        next_state_s = HEADER_WRITE_s;
                    end else begin
                        next_state_s = WAIT_ACK_s;
                    end
                end
            end else if (state_r == WAIT_READY_r && header_waiting == 1) begin
                if (reg_header[R_W_bit] == 0) begin
                    next_state_s = HEADER_READ_s;
                end else begin
                    if (wb_ack_i == 1) begin
                        next_state_s = HEADER_WRITE_s;
                    end else begin
                        next_state_s = WAIT_ACK_s;
                    end
                end
            end else begin
                next_state_s = RESET_s;   
            end             
        end  
        READ_DATA_s:
        begin
            next_state_s = RESET_s;
            if (out_valid == 1) begin
                if (out_flit[R_W_bit] == 0) begin
                    next_state_s = HEADER_READ_s;
                end else begin
                    if (wb_ack_i == 1) begin
                        next_state_s = HEADER_WRITE_s;
                    end else begin
                        next_state_s = WAIT_ACK_s;
                    end
                end
            end
        end            
        WAIT_ACK_s:
        begin
            next_state_s = WAIT_ACK_s;
            if (wb_ack_i == 1) begin
                if (reg_header[R_W_bit] == 0) begin
                    next_state_s = READ_DATA_s;
                end else begin
                    next_state_s = HEADER_WRITE_s;
                end
            end
        end
        
        default:
            next_state_s = RESET_s;
        endcase            
    end
    
    always @ (posedge clk)
    begin
        if (rst == 1) begin
            state_s <= RESET_s;
        end else begin
            state_s <= next_state_s;
        end
    end
          
    
endmodule





