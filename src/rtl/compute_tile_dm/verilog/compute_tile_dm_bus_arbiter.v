// Based on:
// WISHBONE Connection Bus Top Level                             
//  Author: Johny Chi, chisuhua@yahoo.com.cn
//
// Modified for the Wishbone Bus Generator
//  Author: Stefan Wallentowitz, stefan.wallentowitz@tum.de
//
// Copyright (C) 2000, 2011 Authors and OPENCORES.ORG
//
// This source file may be used and distributed without         
// restriction provided that this copyright statement is not    
// removed from the file and that any derivative work contains  
// the original copyright notice and the associated disclaimer. 
//                                                              
// This source file is free software; you can redistribute it   
// and/or modify it under the terms of the GNU Lesser General   
// Public License as published by the Free Software Foundation; 
// either version 2.1 of the License, or (at your option) any   
// later version.                                               
//                                                              
// This source is distributed in the hope that it will be       
// useful, but WITHOUT ANY WARRANTY; without even the implied   
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
// PURPOSE.  See the GNU Lesser General Public License for more 
// details.                                                     
//                                                              
// You should have received a copy of the GNU Lesser General    
// Public License along with this source; if not, download it   
// from http://www.opencores.org/lgpl.shtml

`include "timescale.v"


// Automatically generated on: 10-01-2012 16:47:02

module compute_tile_dm_bus_arbiter(clk, rst, req, gnt);

   input                clk;
   input                rst;
   input [2:0]          req;            // Req input
   output [1:0]         gnt;            // Grant output

   ///////////////////////////////////////////////////////////////////////
   //
   // Parameters
   //


   parameter    [1:0] grant0 = 2'd0, grant1 = 2'd1, grant2 = 2'd2;

   ///////////////////////////////////////////////////////////////////////
   //
   // Local Registers and Wires
   //

   reg [1:0]            state, next_state;

   ///////////////////////////////////////////////////////////////////////
   //
   //  Misc Logic 
   //

   assign       gnt = state;

   always@(posedge clk or posedge rst)
     if(rst)            state <= #1 grant0;
     else               state <= #1 next_state;

   ///////////////////////////////////////////////////////////////////////
   //
   // Next State Logic
   //   - implements round robin arbitration algorithm
   //   - switches grant if current req is dropped or next is asserted
   //   - parks at last grant
   //

   always@(state or req )
     begin
        next_state = state;     // Default Keep State
        case(state)             // synopsys parallel_case full_case

          grant0:
        if(!req[0]) begin
        if(req[1]) next_state = grant1;
        else
        if(req[2]) next_state = grant2;
             
            end

          grant1:
        if(!req[1]) begin
        if(req[2]) next_state = grant2;
        else
        if(req[0]) next_state = grant0;
             
            end

          grant2:
        if(!req[2]) begin
        if(req[0]) next_state = grant0;
        else
        if(req[1]) next_state = grant1;
             
            end

        endcase
   end

endmodule