/* Copyright (c) 2017 by the author(s)
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
 * GLIP I/O stress test module.
 * 
 * This module is used to stream random numbers from host to FPGA or vice versa
 * to test the resilience of the communication.
 * To simulate realistic traffic patterns random wait cycles can be included in
 * the test. It is also possible to manually stall the test by setting the
 * stall_flag high.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 */

module io_stress_test
#(
   parameter WIDTH = 16,
   parameter WORD_COUNT_LSHIFT = 20
)(
   // Clock/Reset.
   input    clk,
   input    rst,

   // GLIP FIFO Interface.
   output reg              fifo_out_valid,
   input                   fifo_out_ready,
   output reg [WIDTH-1:0]  fifo_out_data,
   input                   fifo_in_valid,
   output reg              fifo_in_ready,
   input [WIDTH-1:0]       fifo_in_data,

   // Flag to stall read/write to simulate back pressure.
   input                   stall_flag,

   // Flag to induce error. Simply stops the generation of new random numbers.
   input                   error_flag,

   // Error flag for read test.
   output reg              error,
   // Idle state flag.
   output reg              idle
);

   // Start word for read stress test.
   localparam INITIATE_WRITE_TEST = 16'h420A;
   localparam INITIATE_RND_WAIT_WRITE_TEST = 16'h421A;
   
   // Start word for write stress test.
   localparam INITIATE_READ_TEST = 16'h420B;
   localparam INITIATE_RND_WAIT_READ_TEST = 16'h421B;
   
   // Keyword to indicate an error.
   // Sent to host on failed write test and received by host on failed read
   // test.
   localparam ERROR = 16'hDEAD;
   
   // Keyword to indicate successful test.
   // Sent to host on successful write test and received by host on successful
   // read test.
   localparam SUCCESS = 16'hCAFE;
   
   // Test modes. Naming is from host's point of view.
   localparam WRITE_TEST = 1'b0;
   localparam READ_TEST = 1'b1;

   // Counter for the amount of Bytes to be read/written.
   // The counter is initialized by a 16-bit word that is shifted left for
   // WORD_COUNT_LSHIFT bits.
   // The resulting maximum amount of data is ~63GB.
   reg [35:0] byte_counter;
   reg [35:0] nxt_byte_counter;

   // Signal to set error.
   reg set_error;

   // Contains the generated random numbers.
   wire [WIDTH-1:0] rnd_data;
   
   // Set high when a new number shall be generated.
   reg gen_rnd;   
   
   // Set by FSM to initialize the lfsr with a seed value.
   reg rnd_init;   
   
   // Determines the current test mode.
   reg testmode;
   reg nxt_testmode;
   reg rnd_wait_mode;
   reg nxt_rnd_wait_mode;
   
   // Determines if random wait cycles are to be included in the test.
   reg rnd_wait;
   
   // 16-bit counter counts time between wait periods and the duration of the
   // wait periods.
   reg [15:0] delay_cnt;
   
   // Internal stall flag that is updated with each rising clock edge.
   reg int_stall_flag;  

   // The lfsr that generates the pseudo random numbers.
   // On initialization the first shift is performed so the first random number
   // following the seed is immediately available.
   stress_test_lfsr
      #(.WIDTH(WIDTH))
   u_rnd_generator(
      .clk           (clk),
      .rnd_seed      (fifo_in_data),
      .set_seed      (rnd_init),
      .generate_rnd  (gen_rnd & ~error_flag),
      .rnd_data      (rnd_data));

   // Naming from host's point of view.
   // (write test: stream data from host to device)
   localparam STATE_IDLE = 0;
   localparam STATE_SETUP_TEST_1 = 1;
   localparam STATE_SETUP_TEST_2 = 2;
   localparam STATE_WRITE_TEST = 3;
   localparam STATE_WRITE_TEST_WAIT = 4;
   localparam STATE_WRITE_TEST_SUCCESS = 5;
   localparam STATE_WRITE_ERROR_1 = 6;
   localparam STATE_WRITE_ERROR_2 = 7;
   localparam STATE_WRITE_ERROR_3 = 8;
   localparam STATE_READ_TEST = 9;
   localparam STATE_READ_TEST_WAIT = 10;
   localparam STATE_READ_TEST_SUCCESS = 11;

   reg [3:0] state;
   reg [3:0] nxt_state;
   
   // Control of the random wait mode.
   always @(posedge clk) begin
      int_stall_flag <= stall_flag | rnd_wait;
      if (rst) begin
         delay_cnt <= 'b0;
         rnd_wait <= 0;
         rnd_wait_mode <= 0;
      end else begin
         rnd_wait_mode <= nxt_rnd_wait_mode;
         if (rnd_wait_mode) begin
            if (delay_cnt > 0) begin
               delay_cnt <= delay_cnt - 1;
            end else begin
               rnd_wait <= ~rnd_wait;
               delay_cnt <= rnd_data[15:0] == 0 ? 16'h1000 : rnd_data[15:0];
            end
         end else begin
            delay_cnt <= 'b0;
         end
      end
   end
   
   // Control of the Byte counter.
   always @(posedge clk) begin
      if (rst) begin
         byte_counter <= 0;
      end else begin
         if (nxt_byte_counter > 0) begin
            byte_counter <= nxt_byte_counter;
         end else begin
            if (byte_counter > 0) begin
               if (gen_rnd) begin
                  byte_counter <= byte_counter - (WIDTH / 8);
               end else begin
                  byte_counter <= byte_counter;
               end
            end else begin
               byte_counter <= 0;
            end              
         end       
      end
   end

   // Control of the FSM.
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         testmode <= WRITE_TEST;
         error <= 0;
      end else begin
         state <= nxt_state;
         testmode <= nxt_testmode;
         error <= error | set_error;
      end
   end

   // Combinatoric part of the FSM.
   always @(*) begin
      nxt_state = state;
      nxt_testmode = testmode;
      nxt_rnd_wait_mode = rnd_wait_mode;
      nxt_byte_counter = 0;

      gen_rnd = 0;
      rnd_init = 0;
      set_error = 0;
      fifo_in_ready = 0;
      fifo_out_valid = 0;
      idle = 0;
      
      fifo_out_data = rnd_data;

      case (state)
         STATE_IDLE: begin
            idle = 1;
            nxt_rnd_wait_mode = 0;
            if (fifo_in_valid && ~error) begin
               fifo_in_ready = 1;
               if (fifo_in_data == INITIATE_WRITE_TEST
                     || fifo_in_data == INITIATE_RND_WAIT_WRITE_TEST) begin
                  nxt_state = STATE_SETUP_TEST_1;
                  nxt_testmode = WRITE_TEST;
                  nxt_rnd_wait_mode = fifo_in_data[4];
               end else if (fifo_in_data == INITIATE_READ_TEST
                     || fifo_in_data == INITIATE_RND_WAIT_READ_TEST) begin
                  nxt_state = STATE_SETUP_TEST_1;
                  nxt_testmode = READ_TEST;
                  nxt_rnd_wait_mode = fifo_in_data[4];
               end else begin
                  set_error = 1;
               end
            end
         end

         STATE_SETUP_TEST_1: begin
            if (fifo_in_valid) begin
               fifo_in_ready = 1;
               rnd_init = 1;
               nxt_state = STATE_SETUP_TEST_2;
            end
         end

         STATE_SETUP_TEST_2: begin
            if (fifo_in_valid) begin
               fifo_in_ready = 1;
               nxt_byte_counter = {{20-WORD_COUNT_LSHIFT{1'b0}},
                     fifo_in_data[15:0], {WORD_COUNT_LSHIFT{1'b0}}};
               if (testmode == WRITE_TEST) begin
                  nxt_state = STATE_WRITE_TEST;
               end else if (testmode == READ_TEST) begin
                  nxt_state = STATE_READ_TEST;
               end
            end
         end

         STATE_WRITE_TEST: begin
            if (byte_counter <= 0) begin
               fifo_out_data = SUCCESS;
               fifo_out_valid = 1;
               nxt_state = STATE_WRITE_TEST_SUCCESS;
            end else begin
               if (fifo_in_valid & ~int_stall_flag) begin
                  if (fifo_in_data == rnd_data) begin
                     fifo_in_ready = 1;
                     gen_rnd = 1;                     
                  end else begin
                     set_error = 1;
                     fifo_out_data = ERROR;
                     fifo_out_valid = 1;
                     nxt_state = STATE_WRITE_ERROR_1;
                  end
               end
            end
         end
         
         STATE_WRITE_TEST_SUCCESS: begin
            if (fifo_out_ready) begin
               nxt_state = STATE_IDLE;
            end else begin
               fifo_out_data = SUCCESS;
               fifo_out_valid = 1;
            end
         end

         STATE_WRITE_ERROR_1: begin
            fifo_out_valid = 1;
            if (fifo_out_ready) begin
               fifo_out_data = fifo_in_data;
               nxt_state = STATE_WRITE_ERROR_2;
            end else begin
               fifo_out_data = ERROR;
            end
         end

         STATE_WRITE_ERROR_2: begin
            fifo_out_valid = 1;
            if (fifo_out_ready) begin
               nxt_state = STATE_WRITE_ERROR_3;
            end else begin
               fifo_out_data = fifo_in_data;
            end
         end

         STATE_WRITE_ERROR_3: begin
            if (fifo_out_ready) begin
               nxt_state = STATE_IDLE;
            end else begin
               fifo_out_valid = 1;
            end
         end

         STATE_READ_TEST: begin
            if (byte_counter <= 0) begin
               nxt_state = STATE_READ_TEST_SUCCESS;
            end else begin
               if (fifo_in_valid) begin
                  fifo_in_ready = 1;
                  if (fifo_in_data == ERROR) begin
                     set_error = 1;
                     nxt_state = STATE_IDLE;
                  end
               end
               if (fifo_out_ready & ~int_stall_flag) begin
                  gen_rnd = 1;
                  fifo_out_valid = 1;
               end
            end
         end
         
         STATE_READ_TEST_SUCCESS: begin
            if (fifo_in_valid) begin
               fifo_in_ready = 1;
               if (fifo_in_data != SUCCESS) begin
                  set_error = 1;
               end
               nxt_state = STATE_IDLE;
            end
         end
      endcase

   end

endmodule
