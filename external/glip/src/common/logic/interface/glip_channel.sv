/* Copyright (c) 2016 by the author(s)
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
 * GLIP communication channel interface
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */
interface glip_channel
  #(parameter WIDTH=16)
   (input clk);

   logic [WIDTH-1:0] data;
   logic             valid;
   logic             ready;

   modport master(output data, output valid, input ready);

   modport slave(input data, input valid, output ready);

   // a helper function to ease the assembly of interface signals
   function logic assemble ( input logic [WIDTH-1:0] m_data,
                             input logic m_valid
                             );
      data = m_data;
      valid = m_valid;
      return ready;
   endfunction // assemble

endinterface // glip_channel
