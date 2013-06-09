

module ct_select(
   input [31:0]  m_dat_i,
   input [31:0]  m_adr_i,
   input [3:0]   m_sel_i,
   input         m_we_i,
   input         m_cyc_i,
   input         m_stb_i,
   input [2:0]   m_cti_i,
   input [1:0]   m_bte_i,
   output        m_ack_o,
   output        m_err_o,
   output        m_rty_o,
   output [31:0] m_dat_o

   ,output [31:0] s_0_dat_o
   ,output [31:0] s_0_adr_o
   ,output [3:0]  s_0_sel_o
   ,output        s_0_we_o
   ,output        s_0_cyc_o
   ,output        s_0_stb_o
   ,output [2:0]  s_0_cti_o
   ,output [1:0]  s_0_bte_o
   ,input         s_0_ack_i
   ,input         s_0_err_i
   ,input         s_0_rty_i
   ,input [31:0]  s_0_dat_i

   ,output [31:0] s_1_dat_o
   ,output [31:0] s_1_adr_o
   ,output [3:0]  s_1_sel_o
   ,output        s_1_we_o
   ,output        s_1_cyc_o
   ,output        s_1_stb_o
   ,output [2:0]  s_1_cti_o
   ,output [1:0]  s_1_bte_o
   ,input         s_1_ack_i
   ,input         s_1_err_i
   ,input         s_1_rty_i
   ,input [31:0]  s_1_dat_i

);

  parameter s0_addr_w = 1;
  parameter s0_addr   = 1'd0;
  parameter s1_addr_w = 4;
  parameter s1_addr   = 4'd14;
 
  parameter sselectw = 2;

  wire [sselectw-1:0] sselect;

  assign sselect[0] = (m_adr_i[31:32-s0_addr_w] == s0_addr);
  assign sselect[1] = (m_adr_i[31:32-s1_addr_w] == s1_addr);


  assign s_0_stb_o = m_stb_i & sselect[0];
  assign s_0_dat_o = m_dat_i;
  assign s_0_adr_o = m_adr_i;
  assign s_0_sel_o = m_sel_i;
  assign s_0_we_o = m_we_i;
  assign s_0_cyc_o = m_cyc_i;
  assign s_0_cti_o = m_cti_i;
  assign s_0_bte_o = m_bte_i;

  assign s_1_stb_o = m_stb_i & sselect[1];
  assign s_1_dat_o = m_dat_i;
  assign s_1_adr_o = m_adr_i;
  assign s_1_sel_o = m_sel_i;
  assign s_1_we_o = m_we_i;
  assign s_1_cyc_o = m_cyc_i;
  assign s_1_cti_o = m_cti_i;
  assign s_1_bte_o = m_bte_i;


   reg [34:0]         sbus;
   always @(*) begin
      case (sselect)
        2'b01:
          sbus = {s_0_dat_i,s_0_ack_i,s_0_err_i,s_0_rty_i};
        2'b10:
          sbus = {s_1_dat_i,s_1_ack_i,s_1_err_i,s_1_rty_i};
        default:
          sbus = {32'h0,1'b0,m_cyc_i&m_stb_i,1'b0};
      endcase // case (sselect)
   end

  assign {m_dat_o,m_ack_o,m_err_o,m_rty_o} = sbus;


endmodule