`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 21.07.2016 18:03:02
// Design Name: 
// Module Name: cam_emulator
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


module cam_emulator(
    // Inputs
    XVCLK, rst, control_reg,
    // Outputs
    PCLK, HREF, VSYNC, D
    );
    
    input XVCLK, rst;
    input [31:0] control_reg;
    
    output PCLK, HREF, VSYNC;
    output reg [7:0] D;
    
    parameter WIDTH = 3;
	// IMPORTANT: N must be an ODD NUMBER!
    parameter N = 3; // PCLK clock period = (N * XVCLK clock period)
    reg [WIDTH-1:0] pos_count = 0;
    reg [WIDTH-1:0] neg_count = 0;
    
    reg [10:0] col_counter = 0;
    reg [10:0] row_counter = 0;
    reg [10:0] vsync_counter = 0;
    reg [10:0] href_counter = 0;
    reg frame_blank = 0;
    reg row_blank = 0;
    
    parameter hRes = 8; // # of pixels in each row
    parameter vRes = 6; // # number of rows in each frame
    parameter vsync_delay = 2; // # of clock cycles between VSYNC pulses
    parameter href_delay = 1;  // # of clock cycles between HREF pulses
    
   
    
    assign VSYNC = (frame_blank == 1) ? 0 : 1;
    assign HREF = (row_blank == 1 || frame_blank == 1) ? 0 : 1;
    
    
    always @(posedge PCLK or posedge rst)
    begin
        if (rst == 1) begin
            D <= 0;
            col_counter <= 0;
            row_counter <= 0;
			vsync_counter <= 0;
	        href_counter <= 0;
        end else begin
            //D <= D + 1;
            if (col_counter >= (hRes-1)) begin
                if (row_counter >= (vRes-1)) begin                    
                    if (vsync_counter < vsync_delay) begin
                        vsync_counter <= vsync_counter + 1;
                        frame_blank <= 1;
						D <= 0;
                    end else begin
                        frame_blank <= 0;
                        vsync_counter <= 0;
                        col_counter <= 0;
                        row_counter <= 0;
						D <= D + 1;
                    end
                end else if (href_counter < href_delay) begin
                    href_counter <= href_counter + 1;
					row_blank <= 1;
				end else begin
					href_counter <= 0;
					row_counter <= row_counter + 1;
                    col_counter <= 0;
                    row_blank <= 0;
					D <= D + 1;                    
                end
            end else begin
                col_counter <= col_counter + 1;
                row_blank <= 0;
                frame_blank <= 0;
                D <= D + 1;
            end
                    
            
        end
    end
    
    assign PCLK = ((pos_count > (N>>1)) | (neg_count > (N>>1)));
    
    always @(posedge XVCLK)
    begin
        if (rst) begin
            pos_count <= 0;
        end else if (pos_count == (N-1)) begin
            pos_count <= 0;
        end else begin
            pos_count<= pos_count + 1;
        end
    end 
     
    always @(negedge XVCLK)
    begin
        if (rst) begin
            neg_count <= 0;
        end else if (neg_count == (N-1)) begin
            neg_count <= 0;
        end else begin 
            neg_count<= neg_count + 1;
        end
    end
    
    
    
endmodule
