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
 * This module monitors program counter values of the cpu,
 * detects matches of preconfigured program counter signal values
 * and forwards a respective event signal along with the event id.
 *
 * Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module PC_monitor (/*AUTOARG*/
   // Outputs
   pc_ev_valid, pc_ev_id, pc_ev_time,
   // Inputs
   clk, rst, diag_sys_enabled, conf_pc_flat_in, pc_val, pc_enable,
   time_global
   );


   /** Parameters **/
   // Number of 16 bit configuration registers
   parameter CONF_PC_SIZE = 3;
   // Bit width of event id signal
   parameter EVENT_ID_WIDTH = `DIAGNOSIS_EV_ID_WIDTH;
   // Bit width of timestamp signal
   parameter TIMESTAMP_WIDTH = `DIAGNOSIS_TIMESTAMP_WIDTH;
   // Maximum number of allowed events that can be monitored at the same time
   parameter MAX_EVENT_COUNT = `DIAGNOSIS_PC_EVENTS_MAX;
   // Index for multiplexer of lookuptable selection after comparator hit
   parameter INDEX_WIDTH = $clog2(MAX_EVENT_COUNT);

   /** Interfaces **/
   input clk, rst;
   /* configuration register interface */
   input diag_sys_enabled;
   input [16*CONF_PC_SIZE-1:0] conf_pc_flat_in;
   /* mor1kx program counter interface */
   input [31:0] pc_val;
   input        pc_enable;
   /* event signal interface to LUT module */
   output       pc_ev_valid;
   output [`DIAGNOSIS_EV_ID_WIDTH-1:0] pc_ev_id;
   output [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] pc_ev_time;
   /* interface to global timestamp provider module */
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0]  time_global;


   /* comparator_module AUTO_TEMPLATE(
    .val (pc_val),
    .enable (pc_enable),
    .ev_\(.*\) (pc_ev_\1[]),
    .conf_reg_flat_in (conf_pc_flat_in),
    );*/

   comparator_module
     #(.EVENT_ID_WIDTH(EVENT_ID_WIDTH),
       .TIMESTAMP_WIDTH(TIMESTAMP_WIDTH),
       .MAX_EVENT_COUNT(MAX_EVENT_COUNT),
       .INDEX_WIDTH(INDEX_WIDTH),
       .CONF_REG_SIZE(CONF_PC_SIZE))
   comparator_module (/*AUTOINST*/
                      // Outputs
                      .ev_valid         (pc_ev_valid),           // Templated
                      .ev_id            (pc_ev_id[EVENT_ID_WIDTH-1:0]), // Templated
                      .ev_time          (pc_ev_time[TIMESTAMP_WIDTH-1:0]), // Templated
                      // Inputs
                      .clk              (clk),
                      .rst              (rst),
                      .diag_sys_enabled (diag_sys_enabled),
                      .conf_reg_flat_in (conf_pc_flat_in),       // Templated
                      .val              (pc_val),                // Templated
                      .enable           (pc_enable),             // Templated
                      .time_global      (time_global[TIMESTAMP_WIDTH-1:0]));

endmodule // PC_monitor

