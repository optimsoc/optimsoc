/* Copyright (c) 2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * As SystemVerilog allows for better definition of datatypes than
 * the macros in lisnoc_def.vh this is provided for convenience.
 * You always must assure that the settings correspond to the ones
 * in Verilog.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Andreas Lankes <andreas.lankes@tum.de>
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */

typedef enum bit[4:0] {SELECT_NONE=0,SELECT_NORTH=1,SELECT_EAST=2,SELECT_SOUTH=4,SELECT_WEST=8,SELECT_LOCAL=16} dir_select_t;

`define NORTH 0
`define EAST 1
`define SOUTH 2
`define WEST 3
`define LOCAL 4

typedef enum bit [1:0] { HEADER=2'b01, SINGLE=2'b11, PAYLOAD=2'b00, LAST=2'b10 } flit_type_t;

class flit_t #(int data_width=32,int type_width=2);
        bit [type_width-1:0] ftype;
        bit [data_width-1:0] content;
endclass

class flit_header_t #(int data_width=32);
        bit [4:0] dest;
        bit [3:0] prio;
        bit [2:0] packet_class;
        bit [data_width-13:0] class_specific;
endclass
