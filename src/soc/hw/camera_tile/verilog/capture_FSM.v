`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 17.05.2016 15:16:50
// Design Name: 
// Module Name: capture_FSM_v2_verilog
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


module capture_FSM (
    // inputs
    clk, rst, start, empty, full, href, vsync, d, 
    // outputs
    wr_en_FIFO, rd_en_FIFO, irq_new_frame, pixel_buffer, word_valid
    );
    
    // WISHBONE bus width
    parameter wb_width = 32;
    // camera data bus width
    parameter d_width = 8;
        
    // inputs
    input clk, rst;
    input start, empty, full;
    input href, vsync;
    input[d_width-1:0] d;
    
    // outputs
    output reg wr_en_FIFO = 1;
    output reg rd_en_FIFO = 1;
    //output reg irq_new_frame = 0;
	output reg irq_new_frame;
    
    parameter RST = 2'b00, WAIT_FRAME_START = 2'b01, CAPTURE = 2'b10;
    reg[1:0] state = 0;   

	parameter IDLE = 2'b00, IRQ_ACTIVE = 2'b01, IRQ_RESET = 2'b10;
	reg [1:0] state_irq = 0;
	reg [1:0] next_state_irq;
   
    output reg[wb_width-1:0] pixel_buffer = 0;
    reg[2:0] byte_cntr = 0;
    
    reg[d_width-1:0] reg0, reg1, reg2, reg3;
    output reg word_valid = 0;
    //output reg [17:0] addr;
    

    //assign irq_new_frame = (vsync == 0 && empty == 0) ? 1 : 0;

	//always @ (posedge clk)
	//begin
	//	if (vsync == 0 && empty == 0) begin
	//		irq_new_frame <= 1;
	//	end else if (vsync == 1 && empty == 0) begin
	//		irq_new_frame <= 0;
	//	end
	//end


	// irq_new_frame FSM

	// next state logic
	always @ (*)
	begin
		case (state_irq)
			IDLE:
			begin
				next_state_irq = IDLE;
				if (vsync == 0 && empty == 0) begin
					next_state_irq = IRQ_ACTIVE;
				end
			end
			IRQ_ACTIVE:
			begin
				next_state_irq = IRQ_RESET;
				if (vsync == 1 && empty == 0) begin
					next_state_irq = IDLE;
				end
			end
			IRQ_RESET:
			begin
				next_state_irq = IRQ_RESET;
				if (vsync == 1 && empty == 0) begin
					next_state_irq = IDLE;
				end
			end
		endcase
	end

	// output logic
	always @ (*)
	begin
		case (state_irq)
			IDLE: irq_new_frame = 0;
			IRQ_ACTIVE: irq_new_frame = 1;
			IRQ_RESET: irq_new_frame = 0;
		endcase
	end

	// state transition
	always @ (posedge clk)
	begin
		if (rst == 1) begin
			state_irq <= IDLE;
		end else begin
			state_irq <= next_state_irq;
		end
	end
    
    always @ (posedge clk)
    begin
        case (state)
            RST:
            begin
                pixel_buffer <= 0;
                //addr <= 0;
                byte_cntr <= 0;
                if (start == 1) begin
                    state <= WAIT_FRAME_START;
                end
            end
            
            WAIT_FRAME_START :
            begin
                word_valid <= 0;
                pixel_buffer <= 0;
                //addr <= 0;
                if (vsync == 0 && empty == 0) begin
                    state <= CAPTURE;
                    //irq_new_frame <= 1;
                end 
            end
                       
            CAPTURE:
            begin
//                if (word_valid == 1) begin
//                    addr <= addr + 1;
//                end
                if (vsync == 1 && href == 1 && empty == 0) begin
                    byte_cntr <= byte_cntr + 1;
                    reg0 <= reg1;
                    reg1 <= reg2;
                    reg2 <= reg3;
                    reg3 <= d;
                    if (byte_cntr == 3'b011) begin
                        word_valid <= 1;
                        byte_cntr <= 0;
                        pixel_buffer[31:0] <= {reg1, reg2, reg3, d};  
                    end else begin 
                        word_valid <= 0;   
                    end
                    //irq_new_frame <= 0;
                //end else if (vsync == 0) begin
				end else if (vsync == 0 && empty == 0) begin
                    //addr <= 0;
					//if (empty == 0 && full == 0) begin
                    //	irq_new_frame <= 1;
					//end
                    byte_cntr <= 0;
                    word_valid <= 0;
                end else begin
                    //irq_new_frame <= 0;
                    word_valid <= 0;
                end
            end                        
        endcase
        if (rst == 1) begin
            state <= RST;
        end
    end
endmodule               
                

                           

