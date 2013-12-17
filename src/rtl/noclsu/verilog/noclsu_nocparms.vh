/* Copyright (c) 2013 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

parameter ph_dest_width = 5;
parameter ph_class_width = 2;
parameter ph_lsu_class = 2'b00;

localparam flit_data_width = 32;
localparam ph_dest_msb = flit_data_width - 1;
localparam ph_dest_lsb = ph_dest_msb - ph_dest_width + 1;

localparam ph_src_msb = ph_dest_lsb - 1;
localparam ph_src_lsb = ph_src_msb - ph_dest_width + 1;

localparam ph_class_msb = ph_src_lsb - 1;
localparam ph_class_lsb = ph_class_msb - ph_class_width + 1;

localparam ph_msgtype_width = 2;
localparam ph_msgtype_msb = ph_class_lsb - 1;
localparam ph_msgtype_lsb = ph_msgtype_msb - ph_msgtype_width + 1;

localparam msgtype_readreq   = 2'd0;
localparam msgtype_readresp  = 2'd1;
localparam msgtype_writereq  = 2'd2;
localparam msgtype_writeresp = 2'd3;

localparam ph_type_width = 1;
localparam ph_type_msb = ph_msgtype_lsb - 1;
localparam ph_type_lsb = ph_type_msb - ph_type_width + 1;

localparam type_i = 1'b0;
localparam type_d = 1'b1;

localparam ph_size_width = 1;
localparam ph_size_msb = ph_type_lsb - 1;
localparam ph_size_lsb = ph_size_msb - ph_size_width + 1;

localparam size_single = 1'b0;
localparam size_burst = 1'b1;

localparam ph_bsel_width = 4;
localparam ph_bsel_msb = ph_size_lsb - 1;
localparam ph_bsel_lsb = ph_bsel_msb - ph_bsel_width + 1;
