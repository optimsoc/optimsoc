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
 * This interface is used to integrate the System Verilog router
 * wrapper in your designs.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc.svh"

/* The transmission on a link is controlled by the sender.
 * The receiver signals on which virtual channels it is ready
 * to receive a flit. Based on this information and the arbitration
 * policy the sender sets the flit and signals with the selected
 * vchannel's bit in the valid signal that there is valid data
 * for this vchannel on the flit wires.
 */
interface lisnoc_link_if #(data_width=32,type_width=2,vchannels=1) ();
   wire [data_width+type_width-1:0] flit;
   wire [vchannels-1:0] valid;
   wire [vchannels-1:0] ready;

   modport out(output flit, valid, input ready);
   modport in(input flit, valid, output ready);

endinterface
