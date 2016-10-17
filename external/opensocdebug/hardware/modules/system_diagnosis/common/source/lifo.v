// -*- mode: Verilog; verilog-auto-lineup-declaration: nil; -*-
//-----------------------------------------------------------------------------
// Title         : LIFO stack
// Project       : Common
//-----------------------------------------------------------------------------
// File          : lifo.v
// Date          : $Date: 2009/10/24 $
//-----------------------------------------------------------------------------
// Copyright 2009 Beyond Circuits. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//   1. Redistributions of source code must retain the above copyright notice, 
//      this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright notice, 
//      this list of conditions and the following disclaimer in the documentation 
//      and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE BEYOND CIRCUITS ``AS IS'' AND ANY EXPRESS OR 
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL BEYOND CIRCUITS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------
// Description :
// 
// Synchronous LIFO stack using BRAM.
//
// This is trickier than you might think. The problem is that if you
// do a read and a write to a RAM, you need to use the same address
// otherwise you incurr another port. The idea is to keep a
// top-of-stack register to hold the top of the stack. If you push but
// don't pop, then you write the top-of-stack and accept the new value
// onto the top-of-stack. If you pop then you read from the BRAM into
// the top-of-stack.  If you push and pop then you just swap with the
// top-of-stack and don't do any BRAM operation.
//
// A further wrinkle is that you can't have a read enable on the read
// from the BRAM. So you need to always read from the BRAM but not
// always use the result.  I use a read MUX to select which to
// actually use for the top-of-stack - the top-of-stack register or
// the memory read.
//-----------------------------------------------------------------------------

//`timescale 1ns/1ns
module lifo
  #(
    parameter depth = 32,
    parameter width = 32,
    parameter log2_depth = log2(depth),
    parameter log2_depthp1 = log2(depth+1)
    )
  (
   input clk,
   input reset,
   output reg empty,
   output reg full,
   output reg [log2_depthp1-1:0] count,
   input push,
   input [width-1:0] push_data,
   input pop,
   output [width-1:0] tos
   );

   function integer log2;
      input [31:0] value;
      begin
         value = value-1;
         for (log2=0; value>0; log2=log2+1)
           value = value>>1;
      end
   endfunction

  // We are writing if we are asked to push and are not full or we are also popping
  wire writing = push && (count < depth || pop);
  // We are reading if we are asked to pop and there is data in the stack
  wire reading = pop && count > 0;

  // A count of the number of items in the LIFO is useful and we can use this later
  // as both our read and write pointer.
  reg [log2_depthp1-1:0] next_count;
  always @(*)
    if (reset)
      next_count = 0;
    else if (writing && !reading)
      next_count = count+1;
    else if (reading && !writing)
      next_count = count-1;
    else
      next_count = count;

  always @(posedge clk)
    count <= next_count;

  // Full is easy when you have the count
  always @(posedge clk)
    full <= next_count == depth;

  // As is empty
  always @(posedge clk) 
    empty <= next_count == 0;

  // This is the read/write pointer. If we are writing we use the count value.
  // Otherwise we read from one location before.
  wire [log2_depth-1:0] ptr = writing ? count [log2_depth-1:0] : (count [log2_depth-1:0])-1;
  
  // Here's the RAM that holds the data
  reg [width-1:0] mem [depth-1:0];

  // Writing is so easy
  always @(posedge clk)
    if (writing && !reading)
      mem[ptr] <= tos;

  // Reading is different...
  // What we really want to do is this
  //
  // always @(posedge clk)
  //   if (reading && !writing)
  //     tos <= mem[ptr];
  //   else if (writing)
  //     tos <= push_data;
  //
  // but we can't because XST won't map that into a block RAM. So
  // instead, we coax it a little.
  
  // First, a memory output register that it can fold into the block RAM
  reg [width-1:0] mem_rd;
  always @(posedge clk)
    if (reading)
      mem_rd <= mem[ptr];

  // Next, we need to keep a shadow register that will stand in for
  // the top of stack if we don't need the RAM output
  reg [width-1:0] tos_shadow;
  always @(posedge clk)
    if (writing)
      tos_shadow <= push_data;

  // Now, we need a flag to track whether or not we need to use the
  // RAM output register or the shadow register
  reg use_mem_rd;
  always @(posedge clk)
    if (reset)
      use_mem_rd <= 0;
    else if (writing)
      use_mem_rd <= 0;
    else if (reading)
      use_mem_rd <= 1;

  // And finally a mux to select the correct output
  assign tos = use_mem_rd ? mem_rd : tos_shadow;

endmodule

