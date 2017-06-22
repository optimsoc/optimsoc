
package constants;
   // Maximum packet length
   localparam NOC_MAX_LEN = 32;

   // NoC packet header
   // Mandatory fields
   localparam NOC_DEST_MSB = 31;
   localparam NOC_DEST_LSB = 27;
   localparam NOC_CLASS_MSB = 26;
   localparam NOC_CLASS_LSB = 24;
   localparam NOC_SRC_MSB = 23;
   localparam NOC_SRC_LSB = 19;

   // Classes
   localparam NOC_CLASS_LSU = 3'h2;

   // NoC LSU
   localparam NOC_LSU_MSGTYPE_MSB = 18;
   localparam NOC_LSU_MSGTYPE_LSB = 16;
   localparam NOC_LSU_MSGTYPE_READREQ = 3'h0;
   localparam NOC_LSU_SIZE_IDX = 15;
   localparam NOC_LSU_SIZE_SINGLE = 0;
   localparam NOC_LSU_SIZE_BURST = 1;
   
endpackage // constants


