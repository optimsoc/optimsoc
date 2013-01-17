

/*
 * BASE+
 * +-------------------------+
 * | 0x0 R: tile id          |
 * +-------------------------+
 * | 0x4 R: NoC x-dimension  |
 * +-------------------------+
 * | 0x8 R: NoC y-dimension  |
 * +-------------------------+
 * | 0xc R: configuration    |
 * |        bit 0: mp_simple |
 * |        bit 1: dma       |
 * +-------------------------+
 * 
 */

module networkadapter_conf(/*AUTOARG*/
   // Outputs
   data, ack, rty, err,
   // Inputs
   adr
   );

   parameter tileid = 0;
   parameter mp_simple_present = 0;
   parameter dma_present = 0;

   parameter noc_xdim = 4;
   parameter noc_ydim = 4;
   parameter num_dests = $clog2(noc_xdim*noc_ydim);

   parameter REG_TILEID = 0;
   parameter REG_XDIM   = 1;
   parameter REG_YDIM   = 2;
   parameter REG_CONF   = 3;
   parameter CONF_MPSIMPLE = 0;
   parameter CONF_DMA      = 1;
   
   input [15:0]      adr;
   output reg [31:0] data;
   output            ack;
   output            rty;
   output            err;

   assign ack = ~|adr[15:4] & ~|adr[1:0];
   assign err = ~ack;
   assign rty = 1'b0;
   
   always @(*) begin
      case (adr[3:2])
        REG_TILEID: begin
           data = tileid;
        end
        REG_XDIM: begin
           data = noc_xdim;
        end
        REG_YDIM: begin
           data = noc_ydim;
        end
        REG_CONF: begin
           data = 32'h0000_0000;
           data[CONF_MPSIMPLE] = mp_simple_present;
           data[CONF_DMA] = dma_present;
        end
      endcase
   end
   

endmodule // networkadapter_conf
