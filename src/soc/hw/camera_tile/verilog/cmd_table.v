`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03.06.2016 17:28:02
// Design Name: 
// Module Name: CMD_TABLE
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


module cmd_table(
    // Inputs
    cmd_ptr, IMAGE_MODE, TEST_PATTERN, RESOLUTION, CLKRC,
    // Outputs
    sreg
    );
    
    input [7:0] cmd_ptr;
    // control register values
    input TEST_PATTERN;
    input[1:0] IMAGE_MODE;
    input[2:0] RESOLUTION;
    input[6:0] CLKRC;
    
    output reg[15:0] sreg;
    
    //parameter YUV = 1'b0, JPEG = 1'b1;
    //parameter XGA = 3'b111, SXGA = 3'b101, UXGA = 3'b110;
    `include "function_set_rez.v"
    `include "function_set_format.v"
    

    
    always @ (*)
    begin
        case(cmd_ptr)
            8'h00 : sreg = 16'hff00;
            8'h01 : sreg = 16'h2cff;
            8'h02 : sreg = 16'h2edf;
            8'h03 : sreg = 16'hff01;
            8'h04 : sreg = 16'h3c32;
            8'h05 :
            begin 
                sreg = {8'h11, CLKRC[6], 1'b0, CLKRC[5:0]};
            end
            8'h06 : sreg = 16'h0902;
            8'h07 : sreg = 16'h0428;
            8'h08 : sreg = 16'h13e5;
            8'h09 : sreg = 16'h1448;
            8'h0a : sreg = 16'h2c0c;
            8'h0b : sreg = 16'h3378;
            8'h0c : sreg = 16'h3a33;
            8'h0d : sreg = 16'h3bfb;
            8'h0e : sreg = 16'h3e00;
            8'h0f : sreg = 16'h4311;
            8'h10 : sreg = 16'h1610;
            8'h11 : sreg = 16'h3902;
            8'h12 : sreg = 16'h3588;
            8'h13 : sreg = 16'h220a;
            8'h14 : sreg = 16'h3740;
            8'h15 : sreg = 16'h2300;
            8'h16 : sreg = 16'h34a0;
            8'h17 : sreg = 16'h0602;
            8'h18 : sreg = 16'h0688;
            8'h19 : sreg = 16'h07c0;
            8'h1a : sreg = 16'h0db7;
            8'h1b : sreg = 16'h0e01;
            8'h1c : sreg = 16'h4c00;
            8'h1d : sreg = 16'h4a81;
            8'h1e : sreg = 16'h2199;
            8'h1f : sreg = 16'h2440;
            8'h20 : sreg = 16'h2538;
            8'h21 : sreg = 16'h2682;
            8'h22 : sreg = 16'h5c00;
            8'h23 : sreg = 16'h6300;
            8'h24 : sreg = 16'h4622;
            8'h25 : sreg = 16'h0c3a;
            8'h26 : sreg = 16'h5d55;
            8'h27 : sreg = 16'h5e7d;
            8'h28 : sreg = 16'h5f7d;
            8'h29 : sreg = 16'h6055;
            8'h2a : sreg = 16'h6170;
            8'h2b : sreg = 16'h6280;
            8'h2c : sreg = 16'h7c05;
            8'h2d : sreg = 16'h2080;
            8'h2e : sreg = 16'h2830;
            8'h2f : sreg = 16'h6c00;
            8'h30 : sreg = 16'h6d80;
            8'h31 : sreg = 16'h6e00;
            8'h32 : sreg = 16'h7002;
            8'h33 : sreg = 16'h7194;
            8'h34 : sreg = 16'h73c1;
            8'h35 : sreg = 16'h3d34;
            8'h36 : 
            begin
                if (TEST_PATTERN == 1) begin
                    sreg = 16'h1206;
                end else begin
                    sreg = 16'h1204;
                end
            end
            8'h37 : sreg = 16'h5a57;
            8'h38 : sreg = 16'h4fbb;
            8'h39 : sreg = 16'h509c;
            8'h3a : sreg = 16'hff00;
            8'h3b : sreg = 16'he57f;
            8'h3c : sreg = 16'hf9c0;
            8'h3d : sreg = 16'h4124;
            8'h3e : sreg = 16'he014;
            8'h3f : sreg = 16'h76ff;
            8'h40 : sreg = 16'h33a0;
            8'h41 : sreg = 16'h4220;
            8'h42 : sreg = 16'h4318;
            8'h43 : sreg = 16'h4c00;
            8'h44 : sreg = 16'h87d0;
            8'h45 : sreg = 16'h883f;
            8'h46 : sreg = 16'hd703;
            8'h47 : sreg = 16'hd910;
            8'h48 : sreg = 16'hd382;
            8'h49 : sreg = 16'hc808;
            8'h4a : sreg = 16'hc980;
            8'h4b : sreg = 16'h7c00;
            8'h4c : sreg = 16'h7d00;
            8'h4d : sreg = 16'h7c03;
            8'h4e : sreg = 16'h7d48;
            8'h4f : sreg = 16'h7d48;
            8'h50 : sreg = 16'h7c08;
            8'h51 : sreg = 16'h7d20;
            8'h52 : sreg = 16'h7d10;
            8'h53 : sreg = 16'h7d0e;
            8'h54 : sreg = 16'h9000;
            8'h55 : sreg = 16'h910e;
            8'h56 : sreg = 16'h911a;
            8'h57 : sreg = 16'h9131;
            8'h58 : sreg = 16'h915a;
            8'h59 : sreg = 16'h9169;
            8'h5a : sreg = 16'h9175;
            8'h5b : sreg = 16'h917e;
            8'h5c : sreg = 16'h9188;
            8'h5d : sreg = 16'h918f;
            8'h5e : sreg = 16'h9196;
            8'h5f : sreg = 16'h91a3;
            8'h60 : sreg = 16'h91af;
            8'h61 : sreg = 16'h91c4;
            8'h62 : sreg = 16'h91d7; 
            8'h63 : sreg = 16'h91e8;
            8'h64 : sreg = 16'h9120;
            8'h65 : sreg = 16'h9200;
            8'h66 : sreg = 16'h9306;
            8'h67 : sreg = 16'h93e3;
            8'h68 : sreg = 16'h9303;
            8'h69 : sreg = 16'h9303;
            8'h6a : sreg = 16'h9300;
            8'h6b : sreg = 16'h9302;
            8'h6c : sreg = 16'h9300;
            8'h6d : sreg = 16'h9300;
            8'h6e : sreg = 16'h9300;
            8'h6f : sreg = 16'h9300;
            8'h70 : sreg = 16'h9300;
            8'h71 : sreg = 16'h9300;
            8'h72 : sreg = 16'h9300;
            8'h73 : sreg = 16'h9600;
            8'h74 : sreg = 16'h9708;
            8'h75 : sreg = 16'h9719;
            8'h76 : sreg = 16'h9702;
            8'h77 : sreg = 16'h970c;
            8'h78 : sreg = 16'h9724;
            8'h79 : sreg = 16'h9730;
            8'h7a : sreg = 16'h9728;
            8'h7b : sreg = 16'h9726;
            8'h7c : sreg = 16'h9702;
            8'h7d : sreg = 16'h9798;
            8'h7e : sreg = 16'h9780;
            8'h7f : sreg = 16'h9700;
            8'h80 : sreg = 16'h9700;
            8'h81 : sreg = 16'ha400;
            8'h82 : sreg = 16'ha800;
            8'h83 : sreg = 16'hc511;
            8'h84 : sreg = 16'hc651;
            8'h85 : sreg = 16'hbf80;
            8'h86 : sreg = 16'hc710;
            8'h87 : sreg = 16'hb666;
            8'h88 : sreg = 16'hb8a5;
            8'h89 : sreg = 16'hb764;
            8'h8a : sreg = 16'hb97c;
            8'h8b : sreg = 16'hb3af;
            8'h8c : sreg = 16'hb497;
            8'h8d : sreg = 16'hb5ff;
            8'h8e : sreg = 16'hb0c5;
            8'h8f : sreg = 16'hb194;
            8'h90 : sreg = 16'hb20f;
            8'h91 : sreg = 16'hc45c;
            8'h92 : sreg = 16'ha600;
            8'h93 : sreg = 16'ha720;
            8'h94 : sreg = 16'ha7d8;
            8'h95 : sreg = 16'ha71b;
            8'h96 : sreg = 16'ha731;
            8'h97 : sreg = 16'ha700;
            8'h98 : sreg = 16'ha718;
            8'h99 : sreg = 16'ha720;
            8'h9a : sreg = 16'ha7d8;
            8'h9b : sreg = 16'ha719;
            8'h9c : sreg = 16'ha731;
            8'h9d : sreg = 16'ha700;
            8'h9e : sreg = 16'ha718;
            8'h9f : sreg = 16'ha720;
            8'ha0 : sreg = 16'ha7d8;
            8'ha1 : sreg = 16'ha719;
            8'ha2 : sreg = 16'ha731;
            8'ha3 : sreg = 16'ha700;
            8'ha4 : sreg = 16'ha718;
            8'ha5 : sreg = 16'h7f00;
            8'ha6 : sreg = 16'he51f;
            8'ha7 : sreg = 16'he177;
            8'ha8 : sreg = 16'hdd7f;
            8'ha9 : sreg = 16'hc20e;
            
            // size change preamble
            8'haa : sreg = 16'hff00;
            8'hab : sreg = 16'he004;
            8'hac : sreg = 16'hc0c8;
            8'had : sreg = 16'hc196;
            8'hae : sreg = 16'h863d;
            8'haf : sreg = 16'h5190;
            8'hb0 : sreg = 16'h522c;
            8'hb1 : sreg = 16'h5300;
            8'hb2 : sreg = 16'h5400;
            8'hb3 : sreg = 16'h5588;
            8'hb4 : sreg = 16'h5700;
            
            // resolution registers
            8'hb5 : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hb6 : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hb7 : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hb8 : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hb9 : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hba : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hbb : sreg = set_rez(RESOLUTION, cmd_ptr);
            8'hbc : sreg = set_rez(RESOLUTION, cmd_ptr);
            
            // format change preamble
            8'hbd : sreg = 16'hff00;
            8'hbe : sreg = 16'h0500;
            
            // output format registers
            8'hbf : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc0 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc1 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc2 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc3 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc4 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            8'hc5 : sreg = set_format(IMAGE_MODE, cmd_ptr);
            default : sreg = 16'hffff;
                        
        endcase
    end 
        
    
endmodule
