//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03.06.2016 19:02:49
// Design Name: 
// Module Name: function_set_rez
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



function [15:0] set_rez;
input [2:0] RESOLUTION;
input [7:0] cmd_ptr;
begin
    if (cmd_ptr == 8'hb5) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'h509b; // QCIF
            3'b001 : set_rez = 16'h5092; // QVGA
            3'b010 : set_rez = 16'h5092; // CIF
            3'b011 : set_rez = 16'h5080; // VGA
            3'b100 : set_rez = 16'h5089; // SVGA
            3'b101 : set_rez = 16'h5080; // SXGA
            3'b110 : set_rez = 16'h5080; // UXGA
            3'b111 : set_rez = 16'h5080; // XGA
        endcase
    end else if (cmd_ptr == 8'hb6) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'h5a2c; // QCIF
            3'b001 : set_rez = 16'h5a50; // QVGA
            3'b010 : set_rez = 16'h5a58; // CIF
            3'b011 : set_rez = 16'h5aa0; // VGA
            3'b100 : set_rez = 16'h5ac8; // SVGA
            3'b101 : set_rez = 16'h5a40; // SXGA
            3'b110 : set_rez = 16'h5a90; // UXGA
            3'b111 : set_rez = 16'h5a00; // XGA                   
        endcase
    end else if (cmd_ptr == 8'hb7) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'h5b24; // QCIF
            3'b001 : set_rez = 16'h5b3c; // QVGA
            3'b010 : set_rez = 16'h5b48; // CIF
            3'b011 : set_rez = 16'h5b78; // VGA
            3'b100 : set_rez = 16'h5b96; // SVGA
            3'b101 : set_rez = 16'h5b00; // SXGA
            3'b110 : set_rez = 16'h5b2c; // UXGA 
            3'b111 : set_rez = 16'h5bc0; // XGA                 
        endcase
    end else if (cmd_ptr == 8'hb8) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'h5c00; // QCIF
            3'b001 : set_rez = 16'h5c00; // QVGA
            3'b010 : set_rez = 16'h5c00; // CIF
            3'b011 : set_rez = 16'h5c00; // VGA
            3'b100 : set_rez = 16'h5c00; // SVGA
            3'b101 : set_rez = 16'h5c05; // SXGA
            3'b110 : set_rez = 16'h5c05; // UXGA
            3'b111 : set_rez = 16'h5c01; // XGA
            
        endcase
    end else if (cmd_ptr == 8'hb9) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'hd304; // QCIF
            3'b001 : set_rez = 16'hd304; // QVGA
            3'b010 : set_rez = 16'hd308; // CIF
            3'b011 : set_rez = 16'hd302; // VGA
            3'b100 : set_rez = 16'hd302; // SVGA
            3'b101 : set_rez = 16'hd302; // SXGA
            3'b110 : set_rez = 16'hd300; // UXGA
            3'b111 : set_rez = 16'hd302; // XGA
        endcase
    end else if (cmd_ptr == 8'hba) begin
        case(RESOLUTION)
            3'b000 : set_rez = 16'he000; // QCIF
            3'b001 : set_rez = 16'he000; // QVGA
            3'b010 : set_rez = 16'he000; // CIF
            3'b011 : set_rez = 16'he000; // VGA
            3'b100 : set_rez = 16'he000; // SVGA
            3'b101 : set_rez = 16'he000; // SXGA
            3'b110 : set_rez = 16'he000; // UXGA
            3'b111 : set_rez = 16'he000; // XGA
        endcase
    end else if (cmd_ptr == 8'hbb) begin
        if (RESOLUTION == 3'b101 || RESOLUTION == 3'b110 || RESOLUTION == 3'b111) begin
            // RESOLUTION = SXGA or UXGA or XGA
            // CTRLI <= 0x00
            set_rez = 16'h5000;
        end else begin
            set_rez = 16'heeee; // Dummy-Wert der ignoriert wird
        end
    end else if (cmd_ptr == 8'hbc) begin
        if (RESOLUTION == 3'b101) begin
            // RESOLUTION = SXGA
            set_rez = 16'hd382; 
        end else if (RESOLUTION == 3'b110) begin
            // RESOLUTION = UXGA
            set_rez = 16'hd380;
        end else begin
            set_rez = 16'heeee; // Dummy-Wert der ignoriert wird
        end
    end 
end                     
endfunction
                
