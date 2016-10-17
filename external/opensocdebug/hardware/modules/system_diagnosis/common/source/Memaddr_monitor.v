/* This module monitors memory writes of the cpu, 
 detects matches of preconfigured memory address signal values 
 and forwards a respective event signal along with the event id. 
 Author: Markus Goehrle, Markus.Goehrle@tum.de */

`include "diagnosis_config.vh"

module Memaddr_monitor (/*AUTOARG*/
   // Outputs
   memaddr_ev_valid, memaddr_ev_id, memaddr_ev_time,
   // Inputs
   clk, rst, diag_sys_enabled, conf_memaddr_flat_in, memaddr_val,
   sram_ce, sram_we, time_global
   );

   
   /** Parameters **/
   // Number of 16 bit configuration registers
   parameter CONF_MEMADDR_SIZE = 3;
   // Bit width of event id signal
   parameter EVENT_ID_WIDTH = `DIAGNOSIS_EV_ID_WIDTH;
   // Bit width of timestamp signal
   parameter TIMESTAMP_WIDTH = `DIAGNOSIS_TIMESTAMP_WIDTH;
   // Maximum number of allowed events that can be monitored at the same time
   parameter MAX_EVENT_COUNT = `DIAGNOSIS_MEMADDR_EVENTS_MAX;
   // Index for multiplexer of lookuptable selection after comparator hit
   parameter INDEX_WIDTH = $clog2(MAX_EVENT_COUNT);
   
   /** Interfaces **/
   input clk, rst;
   /* configuration register interface */
   input diag_sys_enabled;
   input [16*CONF_MEMADDR_SIZE-1:0] conf_memaddr_flat_in;
   /* Memory address interface */
   input [31:0] memaddr_val;
   // chip enable
   input        sram_ce;
   // write enable
   input        sram_we;
   /* event signal interface to LUT module */
   output       memaddr_ev_valid;
   output [`DIAGNOSIS_EV_ID_WIDTH-1:0] memaddr_ev_id;
   output [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] memaddr_ev_time;
   /* interface to global timestamp provider module */
   input [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0]  time_global;


   /* enable signal: only track writes (with enabled chip) */
   wire                                    enable;
   assign enable = sram_ce && sram_we;
   
   /* comparator_module AUTO_TEMPLATE(
    .val (memaddr_val),
    .enable (enable),
    .ev_\(.*\) (memaddr_ev_\1[]),
    .conf_reg_flat_in (conf_memaddr_flat_in),
    );*/

   comparator_module
     #(.EVENT_ID_WIDTH(EVENT_ID_WIDTH),
       .TIMESTAMP_WIDTH(TIMESTAMP_WIDTH),
       .MAX_EVENT_COUNT(MAX_EVENT_COUNT),
       .INDEX_WIDTH(INDEX_WIDTH),
       .CONF_REG_SIZE(CONF_MEMADDR_SIZE))
   comparator_module (/*AUTOINST*/
                      // Outputs
                      .ev_valid         (memaddr_ev_valid),      // Templated
                      .ev_id            (memaddr_ev_id[EVENT_ID_WIDTH-1:0]), // Templated
                      .ev_time          (memaddr_ev_time[TIMESTAMP_WIDTH-1:0]), // Templated
                      // Inputs
                      .clk              (clk),
                      .rst              (rst),
                      .diag_sys_enabled (diag_sys_enabled),
                      .conf_reg_flat_in (conf_memaddr_flat_in),  // Templated
                      .val              (memaddr_val),           // Templated
                      .enable           (enable),                // Templated
                      .time_global      (time_global[TIMESTAMP_WIDTH-1:0]));

endmodule // PC_monitor

