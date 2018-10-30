/* Copyright (c) 2013-2018 by the author(s)
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
 * ============================================================================
 *
 * Utility functions
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

package optimsoc_functions;
   /**
    * Math function: $clog2 as specified in Verilog-2005
    *
    * clog2 =          0        for value == 0
    *         ceil(log2(value)) for value >= 1
    *
    * This implementation is a synthesizable variant of the $clog2 function as
    * specified in the Verilog-2005 standard (IEEE 1364-2005).
    *
    * To quote the standard:
    *   The system function $clog2 shall return the ceiling of the log
    *   base 2 of the argument (the log rounded up to an integer
    *   value). The argument can be an integer or an arbitrary sized
    *   vector value. The argument shall be treated as an unsigned
    *   value, and an argument value of 0 shall produce a result of 0.
    */
   function automatic integer clog2;
      input integer value;
      begin
         value = value - 1;
         for (clog2 = 0; value > 0; clog2 = clog2 + 1) begin
            value = value >> 1;
         end
      end
   endfunction


   /**
    * Math function: enhanced clog2 function
    *
    *                        0        for value == 0
    * clog2_width =          1        for value == 1
    *               ceil(log2(value)) for value > 1
    *
    *
    * This function is a variant of the clog2() function, which returns 1 if the
    * input value is 1. In all other cases it behaves exactly like clog2().
    * This is useful to define registers which are wide enough to contain
    * "value" values.
    *
    * Example 1:
    *   parameter ITEMS = 1;
    *   localparam ITEMS_WIDTH = clog2_width(ITEMS); // 1
    *   reg [ITEMS_WIDTH-1:0] item_register; // items_register is now [0:0]
    *
    * Example 2:
    *   parameter ITEMS = 64;
    *   localparam ITEMS_WIDTH = clog2_width(ITEMS); // 6
    *   reg [ITEMS_WIDTH-1:0] item_register; // items_register is now [5:0]
    *
    * Note: I if you want to store the number "value" inside a
    * register, you need a register with size clog2(value + 1), since
    * you also need to store the number 0.
    *
    * Example 3:
    *   reg [clog2_width(64) - 1 : 0]     store_64_items;  // width is [5:0]
    *   reg [clog2_width(64 + 1) - 1 : 0] store_number_64; // width is [6:0]
    */
   function automatic integer clog2_width;
      input integer value;
      begin
         if (value == 1) begin
            clog2_width = 1;
         end else begin
            clog2_width = clog2(value);
         end
      end
   endfunction

   /**
    * Get a string representing an integer
    *
    * This function works only for up to three-digit numbers, e.g. 0 - 999.
    */
   function automatic [23:0] index2string;
      input integer index;
      integer       hundreds;
      integer       tens;
      integer       ones;
      begin
         hundreds = index / 100;
         tens = (index - (hundreds * 100)) / 10;
         ones = (index - (hundreds * 100) - (tens * 10));
         index2string[23:16] = 8'(hundreds) + 8'd48;
         index2string[15:8] = 8'(tens) + 8'd48;
         index2string[7:0] = 8'(ones) + 8'd48;
      end
   endfunction

endpackage // functions
