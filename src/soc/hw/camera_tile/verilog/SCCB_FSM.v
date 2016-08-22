`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Wagner David
// 
// Create Date: 07.05.2016 12:01:59
// Design Name: 
// Module Name: SCCB_FSM
// Project Name: 
// Target Devices: XC7A100T-CSG324 (Nexys 4 DDR board)
// Tool Versions: 
// Description: 
// Emulates Serial Camera Control Bus (SCCB) protocol to send configuration 
// register values to the camera
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Resources: 
// http://lauri.xn--vsandi-pxa.com/hdl/zynq/zybo-ov7670-to-vga.html
// Application Note: OmniVision Serial Camera Control Bus (SCCB) Functional Specification
//////////////////////////////////////////////////////////////////////////////////


module SCCB_FSM(
    // Inputs
    clk, rst, UPDATE, IMAGE_MODE, TEST_PATTERN, RESOLUTION, CLKRC,
    // Outputs
    sioc, siod, s_mux_rst_update, config_finished//, wb_rty_o
    );
    
    input clk;
    input rst;
    // control register values
    input UPDATE, TEST_PATTERN;
    input[1:0] IMAGE_MODE;
    input[2:0] RESOLUTION;
    input[6:0] CLKRC;
    
    output reg sioc;
    output reg s_mux_rst_update;
    //output reg wb_rty_o;
    output reg config_finished;
    
    inout siod;
    
    reg[2:0] state = 0;
    parameter RESET = 3'b000, SEND = 3'b001, SEND_DONE = 3'b010, RST_UPDATE_BIT = 3'b011;
    
    // camera write address, see OV2640 datasheet p. 18
    parameter camera_write_addr = 8'h60;
    
    // wait 254 cycles (for camera to be ready) before first configuration data is sent
    reg[7:0] divider = 8'b00000001;
    // 32-bit shift register that indicates current send status
    reg[31:0] busy_sr = 0;
    // 32-bit shift register containing data to be sent
    reg[31:0] data_sr = 0;
    // pointer to current command in CMD_TABLE that is sent out
    reg[7:0] cmd_ptr = 0;
    
    wire[15:0] w_sreg;
    
    reg cfg_fin = 0;
    
    
    // set inout port siod to high impedance when Don't-Care-Bit is reached
    // because camera could then drive this port, see SCCB AppNote p. 13
    assign siod = (busy_sr[11:10] == 2'b10 || busy_sr[20:19] == 2'b10 || busy_sr[29:28] == 2'b10) ? 'bz : data_sr[31]; 
    
    cmd_table inst_cmd_table (
        .cmd_ptr(cmd_ptr),
        .IMAGE_MODE(IMAGE_MODE),
        .TEST_PATTERN(TEST_PATTERN),
        .RESOLUTION(RESOLUTION),
        .CLKRC(CLKRC),
        .sreg(w_sreg)
        );
    
    always @ (posedge clk)
    begin
        case(state)
            RESET:
            begin
                sioc <= 1;
                data_sr <= 0;
                busy_sr <= 0;
                divider <= 8'b00000001;
                //cmd_ptr <= 0;
                cmd_ptr <= 8'hbf;  
                s_mux_rst_update <= 0; 
                cfg_fin <= 1;                            
                if (UPDATE == 1) begin
                    state <= SEND;                   
                end
            end
            
            SEND:
            begin
                cfg_fin <= 0; 
                if (busy_sr[31] == 0) begin
                    sioc <= 1;
                    if (divider == 0) begin
                        if (w_sreg == 16'hffff) begin
                            state <= SEND_DONE;
                        end else begin 
                            // data_sr <= {Start_Sequence, Camera_ID, DC-Bit, Register_Address, DC-Bit, Register_Value, DC-Bit, End_Sequence}
                            data_sr <= {3'b100, camera_write_addr, 1'b0, w_sreg[15:8], 1'b0, w_sreg[7:0], 1'b0, 2'b01};
                            busy_sr <= {3'b111, 9'b111111111, 9'b111111111, 9'b111111111, 2'b11};
                            // signal that data has been "taken" and will be sent to the camera now
                            // when taken = 1, address is increased in OV2640_init_register.v to access next register
                            //taken <= 1;
                            cmd_ptr <= cmd_ptr + 1;
                        end
                    end else begin                
                        // is only excuted on power-up
                        divider <= divider + 1; 
                    end 
                end else begin
                    // generate clock pulses for sioc    
                    case ({busy_sr[32-1:32-3], busy_sr[2:0]})
                        {3'b111,3'b111} :
                        // start sequence #1
                        case (divider[7:6])
                            2'b00 : sioc <= 1;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 1;                                                
                        endcase
                        {3'b111,3'b110} :
                        // start sequence #2
                        case (divider[7:6])
                            2'b00 : sioc <= 1;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 1;                                                
                        endcase
                        {3'b111,3'b100} :
                        // start sequence #3
                        case (divider[7:6])
                            2'b00 : sioc <= 0;
                            2'b01 : sioc <= 0;
                            2'b10 : sioc <= 0;
                            default : sioc <= 0;                                                
                        endcase                    
                        {3'b110,3'b000} :
                        // end sequence #1
                        case (divider[7:6])
                            2'b00 : sioc <= 0;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 1;                                                
                        endcase                 
                        {3'b100,3'b000} :
                        // end sequence #2
                        case (divider[7:6])
                            2'b00 : sioc <= 1;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 1;                                                
                        endcase   
                        {3'b000,3'b000} :
                        // Idle state  
                       
                        case (divider[7:6])
                            2'b00 : sioc <= 1;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 1;                                                                         
                        endcase
                        
                        default :
                        case (divider[7:6])
                            2'b00 : sioc <= 0;
                            2'b01 : sioc <= 1;
                            2'b10 : sioc <= 1;
                            default : sioc <= 0;                                                
                        endcase         
                    endcase
                    
                    if (divider == 8'b11111111) begin
                        // shift out sent bit and update busy shift register
                        busy_sr <= {busy_sr[32-2:0], 1'b0};
                        data_sr <= {data_sr[32-2:0], 1'b1};
                        divider <= 0;                       
                    end else begin 
                        divider <= divider + 1;
                    end
                end            
            end  // end state SEND
            
            SEND_DONE:
            begin
                cfg_fin <= 0; 
                cmd_ptr <= 0;
                //wb_rty_o <= 1;
                // set data and enable muxes so that UPDATE bit of control register is cleared again
                s_mux_rst_update <= 1;
                state <= RST_UPDATE_BIT;
            end
            
            RST_UPDATE_BIT:
            begin
                s_mux_rst_update <= 0;
                state <= RESET;
                //wb_rty_o <= 1;  
                cfg_fin <= 0;              
            end

			default:
			begin
				state <= RESET;
			end
                                  
        endcase
    
    
        if (rst == 1) begin
            state <= RESET;
            sioc <= 1;
            data_sr <= 0;
            busy_sr <= 0;
            //taken <= 0;
            divider <= 8'b00000001;
            //cmd_ptr <= 0;
            cmd_ptr <= 8'hbf;
            cfg_fin <= 0;
        end    
    end
    
    // addition cycle delay needed
    always @ (posedge clk)
    begin
        config_finished <= cfg_fin & !UPDATE;
    end
       
endmodule
