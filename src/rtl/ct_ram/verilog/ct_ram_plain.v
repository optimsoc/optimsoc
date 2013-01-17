
module ct_ram_plain(/*AUTOARG*/
   // Outputs
   dat_o,
   // Inputs
   clk_i, sel_i, adr_i, we_i, dat_i
   );

   parameter dw = 32;

   parameter aw = 23;
   /* memory size in bytes */
   parameter mem_size  = 'hx;
   /* memory size in words */
   localparam mem_size_words = mem_size * 8 / dw;

   parameter memory_file = "sram.vmem";

   input clk_i;
   input [3:0] sel_i;
   input [aw-3:0] adr_i;
   input          we_i;
   input [31:0]   dat_i;
   output [31:0]  dat_o;

   reg [dw-1:0]         mem [0:mem_size_words-1] /*synthesis syn_ramstyle = "block_ram" */;

   assign dat_o = mem[adr_i];

   // Write logic
   always @ (posedge clk_i) begin
      if (we_i) begin
         mem[adr_i][31:24] <= sel_i[3] ? dat_i[31:24] : mem[adr_i][31:24];
         mem[adr_i][23:16] <= sel_i[2] ? dat_i[23:16] : mem[adr_i][23:16];
         mem[adr_i][15: 8] <= sel_i[1] ? dat_i[15: 8] : mem[adr_i][15: 8];
         mem[adr_i][ 7: 0] <= sel_i[0] ? dat_i[ 7: 0] : mem[adr_i][ 7: 0];
      end
   end


`ifdef verilator

   task do_readmemh;
      // verilator public
      $readmemh(memory_file, mem);
   endtask // do_readmemh

    // Function to access RAM (for use by Verilator).
   function [31:0] get_mem;
      // verilator public
      input [aw-1:0]            addr;
      get_mem = mem[addr];
   endfunction // get_mem

   // Function to write RAM (for use by Verilator).
   function set_mem;
      // verilator public
      input [aw-1:0]            addr;
      input [dw-1:0]            data;
      mem[addr] = data;
   endfunction // set_mem

`else

   initial
     begin
        $readmemh(memory_file, mem);
     end

`endif // !`ifdef verilator

endmodule