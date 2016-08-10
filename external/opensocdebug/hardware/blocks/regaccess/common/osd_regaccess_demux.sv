import dii_package::dii_flit;

module osd_regaccess_demux
  (input clk, input rst,

   input  dii_flit in, output logic in_ready,

   output dii_flit out_reg, input out_reg_ready,
   output dii_flit out_bypass, input out_bypass_ready);
   
   reg [16:0] buf_reg;

   logic      buf_last;
   assign buf_last = buf_reg[16];

   assign out_reg.data = buf_reg[15:0];
   assign out_reg.last = buf_last;
   assign out_bypass.data = buf_reg[15:0];
   assign out_bypass.last = buf_last;

   logic      in_is_regaccess;
   assign in_is_regaccess = ~|in.data[15:14];
   
   logic      in_transfer, out_transfer;
   assign in_transfer = in.valid & in_ready;
   assign out_transfer = (out_reg.valid & out_reg_ready) |
                         (out_bypass.valid & out_bypass_ready);
   
   // The flit in the register is part of a worm
   reg        worm;
   logic      nxt_worm;
   // The flit in the register is a first
   reg        first;
   logic      nxt_first;
   // The current worm is a register access
   reg        regaccess;
   logic      nxt_regaccess;
   
   always_ff @(posedge clk) begin
      if (rst) begin
         worm <= 0;
         first <= 0;
      end else begin
         worm <= nxt_worm;
         first <= nxt_first;
      end

      regaccess <= nxt_regaccess;

      if (in_transfer)
        buf_reg <= { in.last, in.data };
   end

   logic active;
   assign active = first | worm;

   logic nxt_worm_first, nxt_worm_keep;
   assign nxt_worm = nxt_worm_first | nxt_worm_keep;
   assign nxt_worm_first = first & in_ready & in.valid;
   assign nxt_worm_keep = worm & !(buf_last & out_transfer);

   logic nxt_first_inactive, nxt_first_keep, nxt_first_worm;
   assign nxt_first = nxt_first_inactive | nxt_first_keep | nxt_first_worm;
   assign nxt_first_inactive = !active & in.valid;
   assign nxt_first_keep = first & !in_transfer;
   assign nxt_first_worm = worm & in_transfer & buf_last;

   assign nxt_regaccess = first ? in_is_regaccess : regaccess;

   assign out_reg.valid = (first & in_is_regaccess & in.valid) |
                          (worm & regaccess & in.valid) |
                          (worm & regaccess & buf_last & !in.valid);
   assign out_bypass.valid = (first & !in_is_regaccess & in.valid) |
                             (worm & !regaccess & in.valid) |
                             (worm & !regaccess & buf_last & !in.valid);

   logic out_ready, out_ready_first, out_ready_worm;
   assign out_ready = (first & out_ready_first) |
                      (worm & out_ready_worm);
   assign out_ready_first = in_is_regaccess ? out_reg_ready : out_bypass_ready;
   assign out_ready_worm = regaccess ? out_reg_ready : out_bypass_ready;   

   assign in_ready = !active | out_ready;
   
endmodule // osd_regaccess_demux

