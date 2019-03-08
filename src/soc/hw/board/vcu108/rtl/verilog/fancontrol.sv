/* Copyright (c) 2018 by the author(s)
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
 * Fancontrol, simple version, just overheating protection.
 *
 * Author(s):
 *   Dominik Langen <d.langen@denso-auto.de>
 *   Max Koenen <max.koenen@tum.de>
 */

module fancontrol
(
   input       clk,
   input       reset,
   input       user_temp_alarm,
   input       ot,
   input[9:0]  temp,
   input       sm_fan_tach,
   output reg  sm_fan_pwm
   );

   always_comb begin
      if (user_temp_alarm == 1 || ot == 1) begin
         sm_fan_pwm <= 1'b1;
      end else begin
         sm_fan_pwm <= 1'b0;
      end
   end

endmodule
