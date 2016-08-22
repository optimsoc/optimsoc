//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03.06.2016 20:25:58
// Design Name: 
// Module Name: function_set_format
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



function [15:0] set_format;
input [1:0] IMAGE_MODE;
input [7:0] cmd_ptr;
begin
    if (cmd_ptr == 8'hbf) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            if (IMAGE_MODE[1] == 1'b1) begin
                set_format = 16'hda10; // YUV + JPEG
            end else begin
                set_format = 16'hda00; // YUV 
            end
        end else begin
            if (IMAGE_MODE[1] == 1'b1) begin
                set_format = 16'hda18; // RGB + JPEG
            end else begin
                set_format = 16'hda08; // RGB
            end
        end
    end else if (cmd_ptr == 8'hc0) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'hd703; // YUV
        end else begin
            set_format = 16'hd703; // RGB
        end 
    end else if (cmd_ptr == 8'hc1) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'h33a0; // YUV
        end else begin
            set_format = 16'he000; // RGB
        end 
    end else if (cmd_ptr == 8'hc2) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'he51f; // YUV
        end else begin
            set_format = 16'h0500; // RGB
        end      
    end else if (cmd_ptr == 8'hc3) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'he167;
        end else begin
            set_format = 16'heeee; // dummy value
        end             
    end else if (cmd_ptr == 8'hc4) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'he000;
        end else begin
            set_format = 16'heeee; // dummy value
        end
    end else if (cmd_ptr == 8'hc5) begin
        if (IMAGE_MODE[0] == 1'b0) begin
            set_format = 16'h0500;
        end else begin
            set_format = 16'heeee; // dummy value
        end
    end           
end         
endfunction            
