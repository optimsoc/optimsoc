`include "lisnoc_def.vh"

module lisnoc_mesh8x8_vc2
  (

    link0_in_flit_i, link0_in_valid_i, link0_in_ready_o,
    link0_out_flit_o, link0_out_valid_o, link0_out_ready_i,

    link1_in_flit_i, link1_in_valid_i, link1_in_ready_o,
    link1_out_flit_o, link1_out_valid_o, link1_out_ready_i,

    link2_in_flit_i, link2_in_valid_i, link2_in_ready_o,
    link2_out_flit_o, link2_out_valid_o, link2_out_ready_i,

    link3_in_flit_i, link3_in_valid_i, link3_in_ready_o,
    link3_out_flit_o, link3_out_valid_o, link3_out_ready_i,

    link4_in_flit_i, link4_in_valid_i, link4_in_ready_o,
    link4_out_flit_o, link4_out_valid_o, link4_out_ready_i,

    link5_in_flit_i, link5_in_valid_i, link5_in_ready_o,
    link5_out_flit_o, link5_out_valid_o, link5_out_ready_i,

    link6_in_flit_i, link6_in_valid_i, link6_in_ready_o,
    link6_out_flit_o, link6_out_valid_o, link6_out_ready_i,

    link7_in_flit_i, link7_in_valid_i, link7_in_ready_o,
    link7_out_flit_o, link7_out_valid_o, link7_out_ready_i,

    link8_in_flit_i, link8_in_valid_i, link8_in_ready_o,
    link8_out_flit_o, link8_out_valid_o, link8_out_ready_i,

    link9_in_flit_i, link9_in_valid_i, link9_in_ready_o,
    link9_out_flit_o, link9_out_valid_o, link9_out_ready_i,

    link10_in_flit_i, link10_in_valid_i, link10_in_ready_o,
    link10_out_flit_o, link10_out_valid_o, link10_out_ready_i,

    link11_in_flit_i, link11_in_valid_i, link11_in_ready_o,
    link11_out_flit_o, link11_out_valid_o, link11_out_ready_i,

    link12_in_flit_i, link12_in_valid_i, link12_in_ready_o,
    link12_out_flit_o, link12_out_valid_o, link12_out_ready_i,

    link13_in_flit_i, link13_in_valid_i, link13_in_ready_o,
    link13_out_flit_o, link13_out_valid_o, link13_out_ready_i,

    link14_in_flit_i, link14_in_valid_i, link14_in_ready_o,
    link14_out_flit_o, link14_out_valid_o, link14_out_ready_i,

    link15_in_flit_i, link15_in_valid_i, link15_in_ready_o,
    link15_out_flit_o, link15_out_valid_o, link15_out_ready_i,

    link16_in_flit_i, link16_in_valid_i, link16_in_ready_o,
    link16_out_flit_o, link16_out_valid_o, link16_out_ready_i,

    link17_in_flit_i, link17_in_valid_i, link17_in_ready_o,
    link17_out_flit_o, link17_out_valid_o, link17_out_ready_i,

    link18_in_flit_i, link18_in_valid_i, link18_in_ready_o,
    link18_out_flit_o, link18_out_valid_o, link18_out_ready_i,

    link19_in_flit_i, link19_in_valid_i, link19_in_ready_o,
    link19_out_flit_o, link19_out_valid_o, link19_out_ready_i,

    link20_in_flit_i, link20_in_valid_i, link20_in_ready_o,
    link20_out_flit_o, link20_out_valid_o, link20_out_ready_i,

    link21_in_flit_i, link21_in_valid_i, link21_in_ready_o,
    link21_out_flit_o, link21_out_valid_o, link21_out_ready_i,

    link22_in_flit_i, link22_in_valid_i, link22_in_ready_o,
    link22_out_flit_o, link22_out_valid_o, link22_out_ready_i,

    link23_in_flit_i, link23_in_valid_i, link23_in_ready_o,
    link23_out_flit_o, link23_out_valid_o, link23_out_ready_i,

    link24_in_flit_i, link24_in_valid_i, link24_in_ready_o,
    link24_out_flit_o, link24_out_valid_o, link24_out_ready_i,

    link25_in_flit_i, link25_in_valid_i, link25_in_ready_o,
    link25_out_flit_o, link25_out_valid_o, link25_out_ready_i,

    link26_in_flit_i, link26_in_valid_i, link26_in_ready_o,
    link26_out_flit_o, link26_out_valid_o, link26_out_ready_i,

    link27_in_flit_i, link27_in_valid_i, link27_in_ready_o,
    link27_out_flit_o, link27_out_valid_o, link27_out_ready_i,

    link28_in_flit_i, link28_in_valid_i, link28_in_ready_o,
    link28_out_flit_o, link28_out_valid_o, link28_out_ready_i,

    link29_in_flit_i, link29_in_valid_i, link29_in_ready_o,
    link29_out_flit_o, link29_out_valid_o, link29_out_ready_i,

    link30_in_flit_i, link30_in_valid_i, link30_in_ready_o,
    link30_out_flit_o, link30_out_valid_o, link30_out_ready_i,

    link31_in_flit_i, link31_in_valid_i, link31_in_ready_o,
    link31_out_flit_o, link31_out_valid_o, link31_out_ready_i,

    link32_in_flit_i, link32_in_valid_i, link32_in_ready_o,
    link32_out_flit_o, link32_out_valid_o, link32_out_ready_i,

    link33_in_flit_i, link33_in_valid_i, link33_in_ready_o,
    link33_out_flit_o, link33_out_valid_o, link33_out_ready_i,

    link34_in_flit_i, link34_in_valid_i, link34_in_ready_o,
    link34_out_flit_o, link34_out_valid_o, link34_out_ready_i,

    link35_in_flit_i, link35_in_valid_i, link35_in_ready_o,
    link35_out_flit_o, link35_out_valid_o, link35_out_ready_i,

    link36_in_flit_i, link36_in_valid_i, link36_in_ready_o,
    link36_out_flit_o, link36_out_valid_o, link36_out_ready_i,

    link37_in_flit_i, link37_in_valid_i, link37_in_ready_o,
    link37_out_flit_o, link37_out_valid_o, link37_out_ready_i,

    link38_in_flit_i, link38_in_valid_i, link38_in_ready_o,
    link38_out_flit_o, link38_out_valid_o, link38_out_ready_i,

    link39_in_flit_i, link39_in_valid_i, link39_in_ready_o,
    link39_out_flit_o, link39_out_valid_o, link39_out_ready_i,

    link40_in_flit_i, link40_in_valid_i, link40_in_ready_o,
    link40_out_flit_o, link40_out_valid_o, link40_out_ready_i,

    link41_in_flit_i, link41_in_valid_i, link41_in_ready_o,
    link41_out_flit_o, link41_out_valid_o, link41_out_ready_i,

    link42_in_flit_i, link42_in_valid_i, link42_in_ready_o,
    link42_out_flit_o, link42_out_valid_o, link42_out_ready_i,

    link43_in_flit_i, link43_in_valid_i, link43_in_ready_o,
    link43_out_flit_o, link43_out_valid_o, link43_out_ready_i,

    link44_in_flit_i, link44_in_valid_i, link44_in_ready_o,
    link44_out_flit_o, link44_out_valid_o, link44_out_ready_i,

    link45_in_flit_i, link45_in_valid_i, link45_in_ready_o,
    link45_out_flit_o, link45_out_valid_o, link45_out_ready_i,

    link46_in_flit_i, link46_in_valid_i, link46_in_ready_o,
    link46_out_flit_o, link46_out_valid_o, link46_out_ready_i,

    link47_in_flit_i, link47_in_valid_i, link47_in_ready_o,
    link47_out_flit_o, link47_out_valid_o, link47_out_ready_i,

    link48_in_flit_i, link48_in_valid_i, link48_in_ready_o,
    link48_out_flit_o, link48_out_valid_o, link48_out_ready_i,

    link49_in_flit_i, link49_in_valid_i, link49_in_ready_o,
    link49_out_flit_o, link49_out_valid_o, link49_out_ready_i,

    link50_in_flit_i, link50_in_valid_i, link50_in_ready_o,
    link50_out_flit_o, link50_out_valid_o, link50_out_ready_i,

    link51_in_flit_i, link51_in_valid_i, link51_in_ready_o,
    link51_out_flit_o, link51_out_valid_o, link51_out_ready_i,

    link52_in_flit_i, link52_in_valid_i, link52_in_ready_o,
    link52_out_flit_o, link52_out_valid_o, link52_out_ready_i,

    link53_in_flit_i, link53_in_valid_i, link53_in_ready_o,
    link53_out_flit_o, link53_out_valid_o, link53_out_ready_i,

    link54_in_flit_i, link54_in_valid_i, link54_in_ready_o,
    link54_out_flit_o, link54_out_valid_o, link54_out_ready_i,

    link55_in_flit_i, link55_in_valid_i, link55_in_ready_o,
    link55_out_flit_o, link55_out_valid_o, link55_out_ready_i,

    link56_in_flit_i, link56_in_valid_i, link56_in_ready_o,
    link56_out_flit_o, link56_out_valid_o, link56_out_ready_i,

    link57_in_flit_i, link57_in_valid_i, link57_in_ready_o,
    link57_out_flit_o, link57_out_valid_o, link57_out_ready_i,

    link58_in_flit_i, link58_in_valid_i, link58_in_ready_o,
    link58_out_flit_o, link58_out_valid_o, link58_out_ready_i,

    link59_in_flit_i, link59_in_valid_i, link59_in_ready_o,
    link59_out_flit_o, link59_out_valid_o, link59_out_ready_i,

    link60_in_flit_i, link60_in_valid_i, link60_in_ready_o,
    link60_out_flit_o, link60_out_valid_o, link60_out_ready_i,

    link61_in_flit_i, link61_in_valid_i, link61_in_ready_o,
    link61_out_flit_o, link61_out_valid_o, link61_out_ready_i,

    link62_in_flit_i, link62_in_valid_i, link62_in_ready_o,
    link62_out_flit_o, link62_out_valid_o, link62_out_ready_i,

    link63_in_flit_i, link63_in_valid_i, link63_in_ready_o,
    link63_out_flit_o, link63_out_valid_o, link63_out_ready_i,

    clk, rst
    );

   parameter vchannels = 2;
   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   parameter flit_width = flit_data_width + flit_type_width;

   input clk;
   input rst;

   input [flit_width-1:0] link0_in_flit_i;
   input [vchannels-1:0] link0_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link0_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link0_out_flit_o;
   output [vchannels-1:0] link0_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link0_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link1_in_flit_i;
   input [vchannels-1:0] link1_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link1_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link1_out_flit_o;
   output [vchannels-1:0] link1_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link1_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link2_in_flit_i;
   input [vchannels-1:0] link2_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link2_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link2_out_flit_o;
   output [vchannels-1:0] link2_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link2_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link3_in_flit_i;
   input [vchannels-1:0] link3_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link3_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link3_out_flit_o;
   output [vchannels-1:0] link3_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link3_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link4_in_flit_i;
   input [vchannels-1:0] link4_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link4_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link4_out_flit_o;
   output [vchannels-1:0] link4_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link4_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link5_in_flit_i;
   input [vchannels-1:0] link5_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link5_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link5_out_flit_o;
   output [vchannels-1:0] link5_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link5_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link6_in_flit_i;
   input [vchannels-1:0] link6_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link6_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link6_out_flit_o;
   output [vchannels-1:0] link6_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link6_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link7_in_flit_i;
   input [vchannels-1:0] link7_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link7_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link7_out_flit_o;
   output [vchannels-1:0] link7_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link7_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link8_in_flit_i;
   input [vchannels-1:0] link8_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link8_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link8_out_flit_o;
   output [vchannels-1:0] link8_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link8_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link9_in_flit_i;
   input [vchannels-1:0] link9_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link9_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link9_out_flit_o;
   output [vchannels-1:0] link9_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link9_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link10_in_flit_i;
   input [vchannels-1:0] link10_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link10_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link10_out_flit_o;
   output [vchannels-1:0] link10_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link10_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link11_in_flit_i;
   input [vchannels-1:0] link11_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link11_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link11_out_flit_o;
   output [vchannels-1:0] link11_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link11_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link12_in_flit_i;
   input [vchannels-1:0] link12_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link12_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link12_out_flit_o;
   output [vchannels-1:0] link12_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link12_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link13_in_flit_i;
   input [vchannels-1:0] link13_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link13_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link13_out_flit_o;
   output [vchannels-1:0] link13_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link13_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link14_in_flit_i;
   input [vchannels-1:0] link14_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link14_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link14_out_flit_o;
   output [vchannels-1:0] link14_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link14_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link15_in_flit_i;
   input [vchannels-1:0] link15_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link15_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link15_out_flit_o;
   output [vchannels-1:0] link15_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link15_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link16_in_flit_i;
   input [vchannels-1:0] link16_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link16_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link16_out_flit_o;
   output [vchannels-1:0] link16_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link16_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link17_in_flit_i;
   input [vchannels-1:0] link17_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link17_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link17_out_flit_o;
   output [vchannels-1:0] link17_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link17_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link18_in_flit_i;
   input [vchannels-1:0] link18_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link18_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link18_out_flit_o;
   output [vchannels-1:0] link18_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link18_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link19_in_flit_i;
   input [vchannels-1:0] link19_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link19_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link19_out_flit_o;
   output [vchannels-1:0] link19_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link19_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link20_in_flit_i;
   input [vchannels-1:0] link20_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link20_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link20_out_flit_o;
   output [vchannels-1:0] link20_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link20_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link21_in_flit_i;
   input [vchannels-1:0] link21_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link21_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link21_out_flit_o;
   output [vchannels-1:0] link21_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link21_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link22_in_flit_i;
   input [vchannels-1:0] link22_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link22_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link22_out_flit_o;
   output [vchannels-1:0] link22_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link22_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link23_in_flit_i;
   input [vchannels-1:0] link23_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link23_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link23_out_flit_o;
   output [vchannels-1:0] link23_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link23_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link24_in_flit_i;
   input [vchannels-1:0] link24_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link24_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link24_out_flit_o;
   output [vchannels-1:0] link24_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link24_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link25_in_flit_i;
   input [vchannels-1:0] link25_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link25_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link25_out_flit_o;
   output [vchannels-1:0] link25_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link25_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link26_in_flit_i;
   input [vchannels-1:0] link26_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link26_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link26_out_flit_o;
   output [vchannels-1:0] link26_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link26_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link27_in_flit_i;
   input [vchannels-1:0] link27_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link27_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link27_out_flit_o;
   output [vchannels-1:0] link27_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link27_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link28_in_flit_i;
   input [vchannels-1:0] link28_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link28_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link28_out_flit_o;
   output [vchannels-1:0] link28_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link28_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link29_in_flit_i;
   input [vchannels-1:0] link29_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link29_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link29_out_flit_o;
   output [vchannels-1:0] link29_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link29_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link30_in_flit_i;
   input [vchannels-1:0] link30_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link30_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link30_out_flit_o;
   output [vchannels-1:0] link30_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link30_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link31_in_flit_i;
   input [vchannels-1:0] link31_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link31_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link31_out_flit_o;
   output [vchannels-1:0] link31_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link31_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link32_in_flit_i;
   input [vchannels-1:0] link32_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link32_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link32_out_flit_o;
   output [vchannels-1:0] link32_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link32_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link33_in_flit_i;
   input [vchannels-1:0] link33_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link33_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link33_out_flit_o;
   output [vchannels-1:0] link33_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link33_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link34_in_flit_i;
   input [vchannels-1:0] link34_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link34_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link34_out_flit_o;
   output [vchannels-1:0] link34_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link34_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link35_in_flit_i;
   input [vchannels-1:0] link35_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link35_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link35_out_flit_o;
   output [vchannels-1:0] link35_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link35_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link36_in_flit_i;
   input [vchannels-1:0] link36_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link36_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link36_out_flit_o;
   output [vchannels-1:0] link36_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link36_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link37_in_flit_i;
   input [vchannels-1:0] link37_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link37_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link37_out_flit_o;
   output [vchannels-1:0] link37_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link37_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link38_in_flit_i;
   input [vchannels-1:0] link38_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link38_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link38_out_flit_o;
   output [vchannels-1:0] link38_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link38_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link39_in_flit_i;
   input [vchannels-1:0] link39_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link39_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link39_out_flit_o;
   output [vchannels-1:0] link39_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link39_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link40_in_flit_i;
   input [vchannels-1:0] link40_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link40_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link40_out_flit_o;
   output [vchannels-1:0] link40_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link40_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link41_in_flit_i;
   input [vchannels-1:0] link41_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link41_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link41_out_flit_o;
   output [vchannels-1:0] link41_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link41_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link42_in_flit_i;
   input [vchannels-1:0] link42_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link42_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link42_out_flit_o;
   output [vchannels-1:0] link42_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link42_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link43_in_flit_i;
   input [vchannels-1:0] link43_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link43_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link43_out_flit_o;
   output [vchannels-1:0] link43_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link43_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link44_in_flit_i;
   input [vchannels-1:0] link44_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link44_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link44_out_flit_o;
   output [vchannels-1:0] link44_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link44_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link45_in_flit_i;
   input [vchannels-1:0] link45_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link45_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link45_out_flit_o;
   output [vchannels-1:0] link45_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link45_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link46_in_flit_i;
   input [vchannels-1:0] link46_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link46_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link46_out_flit_o;
   output [vchannels-1:0] link46_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link46_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link47_in_flit_i;
   input [vchannels-1:0] link47_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link47_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link47_out_flit_o;
   output [vchannels-1:0] link47_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link47_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link48_in_flit_i;
   input [vchannels-1:0] link48_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link48_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link48_out_flit_o;
   output [vchannels-1:0] link48_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link48_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link49_in_flit_i;
   input [vchannels-1:0] link49_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link49_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link49_out_flit_o;
   output [vchannels-1:0] link49_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link49_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link50_in_flit_i;
   input [vchannels-1:0] link50_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link50_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link50_out_flit_o;
   output [vchannels-1:0] link50_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link50_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link51_in_flit_i;
   input [vchannels-1:0] link51_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link51_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link51_out_flit_o;
   output [vchannels-1:0] link51_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link51_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link52_in_flit_i;
   input [vchannels-1:0] link52_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link52_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link52_out_flit_o;
   output [vchannels-1:0] link52_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link52_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link53_in_flit_i;
   input [vchannels-1:0] link53_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link53_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link53_out_flit_o;
   output [vchannels-1:0] link53_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link53_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link54_in_flit_i;
   input [vchannels-1:0] link54_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link54_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link54_out_flit_o;
   output [vchannels-1:0] link54_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link54_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link55_in_flit_i;
   input [vchannels-1:0] link55_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link55_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link55_out_flit_o;
   output [vchannels-1:0] link55_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link55_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link56_in_flit_i;
   input [vchannels-1:0] link56_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link56_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link56_out_flit_o;
   output [vchannels-1:0] link56_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link56_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link57_in_flit_i;
   input [vchannels-1:0] link57_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link57_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link57_out_flit_o;
   output [vchannels-1:0] link57_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link57_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link58_in_flit_i;
   input [vchannels-1:0] link58_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link58_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link58_out_flit_o;
   output [vchannels-1:0] link58_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link58_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link59_in_flit_i;
   input [vchannels-1:0] link59_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link59_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link59_out_flit_o;
   output [vchannels-1:0] link59_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link59_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link60_in_flit_i;
   input [vchannels-1:0] link60_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link60_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link60_out_flit_o;
   output [vchannels-1:0] link60_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link60_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link61_in_flit_i;
   input [vchannels-1:0] link61_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link61_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link61_out_flit_o;
   output [vchannels-1:0] link61_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link61_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link62_in_flit_i;
   input [vchannels-1:0] link62_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link62_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link62_out_flit_o;
   output [vchannels-1:0] link62_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link62_out_ready_i /*verilator sc_bv*/;

   input [flit_width-1:0] link63_in_flit_i;
   input [vchannels-1:0] link63_in_valid_i /*verilator sc_bv*/;
   output [vchannels-1:0] link63_in_ready_o /*verilator sc_bv*/;
   output [flit_width-1:0] link63_out_flit_o;
   output [vchannels-1:0] link63_out_valid_o /*verilator sc_bv*/;
   input [vchannels-1:0] link63_out_ready_i /*verilator sc_bv*/;


   wire [flit_width-1:0] north_in_flit[0:7][0:7];
   wire [vchannels-1:0] north_in_valid[0:7][0:7];
   wire [vchannels-1:0] north_in_ready[0:7][0:7];
   wire [flit_width-1:0] north_out_flit[0:7][0:7];
   wire [vchannels-1:0] north_out_valid[0:7][0:7];
   wire [vchannels-1:0] north_out_ready[0:7][0:7];
   wire [flit_width-1:0] east_in_flit[0:7][0:7];
   wire [vchannels-1:0] east_in_valid[0:7][0:7];
   wire [vchannels-1:0] east_in_ready[0:7][0:7];
   wire [flit_width-1:0] east_out_flit[0:7][0:7];
   wire [vchannels-1:0] east_out_valid[0:7][0:7];
   wire [vchannels-1:0] east_out_ready[0:7][0:7];
   wire [flit_width-1:0] south_in_flit[0:7][0:7];
   wire [vchannels-1:0] south_in_valid[0:7][0:7];
   wire [vchannels-1:0] south_in_ready[0:7][0:7];
   wire [flit_width-1:0] south_out_flit[0:7][0:7];
   wire [vchannels-1:0] south_out_valid[0:7][0:7];
   wire [vchannels-1:0] south_out_ready[0:7][0:7];
   wire [flit_width-1:0] west_in_flit[0:7][0:7];
   wire [vchannels-1:0] west_in_valid[0:7][0:7];
   wire [vchannels-1:0] west_in_ready[0:7][0:7];
   wire [flit_width-1:0] west_out_flit[0:7][0:7];
   wire [vchannels-1:0] west_out_valid[0:7][0:7];
   wire [vchannels-1:0] west_out_ready[0:7][0:7];



   /* Connecting (0,0) */
   assign north_in_flit[0][0]   = {flit_width{1'b0}};
   assign north_in_valid[0][0]  = {vchannels{1'b0}};
   assign north_out_ready[0][0] = {vchannels{1'b0}};


   assign west_in_flit[0][0]   = {flit_width{1'b0}};
   assign west_in_valid[0][0]  = {vchannels{1'b0}};
   assign west_out_ready[0][0] = {vchannels{1'b0}};



   /* Connecting (0,1) */
   assign north_in_flit[0][1]   = {flit_width{1'b0}};
   assign north_in_valid[0][1]  = {vchannels{1'b0}};
   assign north_out_ready[0][1] = {vchannels{1'b0}};


   assign west_in_flit[0][1]     = east_out_flit[0][0];
   assign west_in_valid[0][1]    = east_out_valid[0][0];
   assign east_out_ready[0][0] = west_in_ready[0][1];
   assign east_in_flit[0][0]   = west_out_flit[0][1];
   assign east_in_valid[0][0]  = west_out_valid[0][1];
   assign west_out_ready[0][1]   = east_in_ready[0][0];



   /* Connecting (0,2) */
   assign north_in_flit[0][2]   = {flit_width{1'b0}};
   assign north_in_valid[0][2]  = {vchannels{1'b0}};
   assign north_out_ready[0][2] = {vchannels{1'b0}};


   assign west_in_flit[0][2]     = east_out_flit[0][1];
   assign west_in_valid[0][2]    = east_out_valid[0][1];
   assign east_out_ready[0][1] = west_in_ready[0][2];
   assign east_in_flit[0][1]   = west_out_flit[0][2];
   assign east_in_valid[0][1]  = west_out_valid[0][2];
   assign west_out_ready[0][2]   = east_in_ready[0][1];



   /* Connecting (0,3) */
   assign north_in_flit[0][3]   = {flit_width{1'b0}};
   assign north_in_valid[0][3]  = {vchannels{1'b0}};
   assign north_out_ready[0][3] = {vchannels{1'b0}};


   assign west_in_flit[0][3]     = east_out_flit[0][2];
   assign west_in_valid[0][3]    = east_out_valid[0][2];
   assign east_out_ready[0][2] = west_in_ready[0][3];
   assign east_in_flit[0][2]   = west_out_flit[0][3];
   assign east_in_valid[0][2]  = west_out_valid[0][3];
   assign west_out_ready[0][3]   = east_in_ready[0][2];



   /* Connecting (0,4) */
   assign north_in_flit[0][4]   = {flit_width{1'b0}};
   assign north_in_valid[0][4]  = {vchannels{1'b0}};
   assign north_out_ready[0][4] = {vchannels{1'b0}};


   assign west_in_flit[0][4]     = east_out_flit[0][3];
   assign west_in_valid[0][4]    = east_out_valid[0][3];
   assign east_out_ready[0][3] = west_in_ready[0][4];
   assign east_in_flit[0][3]   = west_out_flit[0][4];
   assign east_in_valid[0][3]  = west_out_valid[0][4];
   assign west_out_ready[0][4]   = east_in_ready[0][3];



   /* Connecting (0,5) */
   assign north_in_flit[0][5]   = {flit_width{1'b0}};
   assign north_in_valid[0][5]  = {vchannels{1'b0}};
   assign north_out_ready[0][5] = {vchannels{1'b0}};


   assign west_in_flit[0][5]     = east_out_flit[0][4];
   assign west_in_valid[0][5]    = east_out_valid[0][4];
   assign east_out_ready[0][4] = west_in_ready[0][5];
   assign east_in_flit[0][4]   = west_out_flit[0][5];
   assign east_in_valid[0][4]  = west_out_valid[0][5];
   assign west_out_ready[0][5]   = east_in_ready[0][4];



   /* Connecting (0,6) */
   assign north_in_flit[0][6]   = {flit_width{1'b0}};
   assign north_in_valid[0][6]  = {vchannels{1'b0}};
   assign north_out_ready[0][6] = {vchannels{1'b0}};


   assign west_in_flit[0][6]     = east_out_flit[0][5];
   assign west_in_valid[0][6]    = east_out_valid[0][5];
   assign east_out_ready[0][5] = west_in_ready[0][6];
   assign east_in_flit[0][5]   = west_out_flit[0][6];
   assign east_in_valid[0][5]  = west_out_valid[0][6];
   assign west_out_ready[0][6]   = east_in_ready[0][5];



   /* Connecting (0,7) */
   assign north_in_flit[0][7]   = {flit_width{1'b0}};
   assign north_in_valid[0][7]  = {vchannels{1'b0}};
   assign north_out_ready[0][7] = {vchannels{1'b0}};


   assign west_in_flit[0][7]     = east_out_flit[0][6];
   assign west_in_valid[0][7]    = east_out_valid[0][6];
   assign east_out_ready[0][6] = west_in_ready[0][7];
   assign east_in_flit[0][6]   = west_out_flit[0][7];
   assign east_in_valid[0][6]  = west_out_valid[0][7];
   assign west_out_ready[0][7]   = east_in_ready[0][6];

   assign east_in_flit[0][7]   = {flit_width{1'b0}};
   assign east_in_valid[0][7]  = {vchannels{1'b0}};
   assign east_out_ready[0][7] = {vchannels{1'b0}};




   /* Connecting (1,0) */
   assign north_in_flit[1][0]     = south_out_flit[0][0];
   assign north_in_valid[1][0]    = south_out_valid[0][0];
   assign south_out_ready[0][0] = north_in_ready[1][0];
   assign south_in_flit[0][0]   = north_out_flit[1][0];
   assign south_in_valid[0][0]  = north_out_valid[1][0];
   assign north_out_ready[1][0]   = south_in_ready[0][0];


   assign west_in_flit[1][0]   = {flit_width{1'b0}};
   assign west_in_valid[1][0]  = {vchannels{1'b0}};
   assign west_out_ready[1][0] = {vchannels{1'b0}};



   /* Connecting (1,1) */
   assign north_in_flit[1][1]     = south_out_flit[0][1];
   assign north_in_valid[1][1]    = south_out_valid[0][1];
   assign south_out_ready[0][1] = north_in_ready[1][1];
   assign south_in_flit[0][1]   = north_out_flit[1][1];
   assign south_in_valid[0][1]  = north_out_valid[1][1];
   assign north_out_ready[1][1]   = south_in_ready[0][1];


   assign west_in_flit[1][1]     = east_out_flit[1][0];
   assign west_in_valid[1][1]    = east_out_valid[1][0];
   assign east_out_ready[1][0] = west_in_ready[1][1];
   assign east_in_flit[1][0]   = west_out_flit[1][1];
   assign east_in_valid[1][0]  = west_out_valid[1][1];
   assign west_out_ready[1][1]   = east_in_ready[1][0];



   /* Connecting (1,2) */
   assign north_in_flit[1][2]     = south_out_flit[0][2];
   assign north_in_valid[1][2]    = south_out_valid[0][2];
   assign south_out_ready[0][2] = north_in_ready[1][2];
   assign south_in_flit[0][2]   = north_out_flit[1][2];
   assign south_in_valid[0][2]  = north_out_valid[1][2];
   assign north_out_ready[1][2]   = south_in_ready[0][2];


   assign west_in_flit[1][2]     = east_out_flit[1][1];
   assign west_in_valid[1][2]    = east_out_valid[1][1];
   assign east_out_ready[1][1] = west_in_ready[1][2];
   assign east_in_flit[1][1]   = west_out_flit[1][2];
   assign east_in_valid[1][1]  = west_out_valid[1][2];
   assign west_out_ready[1][2]   = east_in_ready[1][1];



   /* Connecting (1,3) */
   assign north_in_flit[1][3]     = south_out_flit[0][3];
   assign north_in_valid[1][3]    = south_out_valid[0][3];
   assign south_out_ready[0][3] = north_in_ready[1][3];
   assign south_in_flit[0][3]   = north_out_flit[1][3];
   assign south_in_valid[0][3]  = north_out_valid[1][3];
   assign north_out_ready[1][3]   = south_in_ready[0][3];


   assign west_in_flit[1][3]     = east_out_flit[1][2];
   assign west_in_valid[1][3]    = east_out_valid[1][2];
   assign east_out_ready[1][2] = west_in_ready[1][3];
   assign east_in_flit[1][2]   = west_out_flit[1][3];
   assign east_in_valid[1][2]  = west_out_valid[1][3];
   assign west_out_ready[1][3]   = east_in_ready[1][2];



   /* Connecting (1,4) */
   assign north_in_flit[1][4]     = south_out_flit[0][4];
   assign north_in_valid[1][4]    = south_out_valid[0][4];
   assign south_out_ready[0][4] = north_in_ready[1][4];
   assign south_in_flit[0][4]   = north_out_flit[1][4];
   assign south_in_valid[0][4]  = north_out_valid[1][4];
   assign north_out_ready[1][4]   = south_in_ready[0][4];


   assign west_in_flit[1][4]     = east_out_flit[1][3];
   assign west_in_valid[1][4]    = east_out_valid[1][3];
   assign east_out_ready[1][3] = west_in_ready[1][4];
   assign east_in_flit[1][3]   = west_out_flit[1][4];
   assign east_in_valid[1][3]  = west_out_valid[1][4];
   assign west_out_ready[1][4]   = east_in_ready[1][3];



   /* Connecting (1,5) */
   assign north_in_flit[1][5]     = south_out_flit[0][5];
   assign north_in_valid[1][5]    = south_out_valid[0][5];
   assign south_out_ready[0][5] = north_in_ready[1][5];
   assign south_in_flit[0][5]   = north_out_flit[1][5];
   assign south_in_valid[0][5]  = north_out_valid[1][5];
   assign north_out_ready[1][5]   = south_in_ready[0][5];


   assign west_in_flit[1][5]     = east_out_flit[1][4];
   assign west_in_valid[1][5]    = east_out_valid[1][4];
   assign east_out_ready[1][4] = west_in_ready[1][5];
   assign east_in_flit[1][4]   = west_out_flit[1][5];
   assign east_in_valid[1][4]  = west_out_valid[1][5];
   assign west_out_ready[1][5]   = east_in_ready[1][4];



   /* Connecting (1,6) */
   assign north_in_flit[1][6]     = south_out_flit[0][6];
   assign north_in_valid[1][6]    = south_out_valid[0][6];
   assign south_out_ready[0][6] = north_in_ready[1][6];
   assign south_in_flit[0][6]   = north_out_flit[1][6];
   assign south_in_valid[0][6]  = north_out_valid[1][6];
   assign north_out_ready[1][6]   = south_in_ready[0][6];


   assign west_in_flit[1][6]     = east_out_flit[1][5];
   assign west_in_valid[1][6]    = east_out_valid[1][5];
   assign east_out_ready[1][5] = west_in_ready[1][6];
   assign east_in_flit[1][5]   = west_out_flit[1][6];
   assign east_in_valid[1][5]  = west_out_valid[1][6];
   assign west_out_ready[1][6]   = east_in_ready[1][5];



   /* Connecting (1,7) */
   assign north_in_flit[1][7]     = south_out_flit[0][7];
   assign north_in_valid[1][7]    = south_out_valid[0][7];
   assign south_out_ready[0][7] = north_in_ready[1][7];
   assign south_in_flit[0][7]   = north_out_flit[1][7];
   assign south_in_valid[0][7]  = north_out_valid[1][7];
   assign north_out_ready[1][7]   = south_in_ready[0][7];


   assign west_in_flit[1][7]     = east_out_flit[1][6];
   assign west_in_valid[1][7]    = east_out_valid[1][6];
   assign east_out_ready[1][6] = west_in_ready[1][7];
   assign east_in_flit[1][6]   = west_out_flit[1][7];
   assign east_in_valid[1][6]  = west_out_valid[1][7];
   assign west_out_ready[1][7]   = east_in_ready[1][6];

   assign east_in_flit[1][7]   = {flit_width{1'b0}};
   assign east_in_valid[1][7]  = {vchannels{1'b0}};
   assign east_out_ready[1][7] = {vchannels{1'b0}};




   /* Connecting (2,0) */
   assign north_in_flit[2][0]     = south_out_flit[1][0];
   assign north_in_valid[2][0]    = south_out_valid[1][0];
   assign south_out_ready[1][0] = north_in_ready[2][0];
   assign south_in_flit[1][0]   = north_out_flit[2][0];
   assign south_in_valid[1][0]  = north_out_valid[2][0];
   assign north_out_ready[2][0]   = south_in_ready[1][0];


   assign west_in_flit[2][0]   = {flit_width{1'b0}};
   assign west_in_valid[2][0]  = {vchannels{1'b0}};
   assign west_out_ready[2][0] = {vchannels{1'b0}};



   /* Connecting (2,1) */
   assign north_in_flit[2][1]     = south_out_flit[1][1];
   assign north_in_valid[2][1]    = south_out_valid[1][1];
   assign south_out_ready[1][1] = north_in_ready[2][1];
   assign south_in_flit[1][1]   = north_out_flit[2][1];
   assign south_in_valid[1][1]  = north_out_valid[2][1];
   assign north_out_ready[2][1]   = south_in_ready[1][1];


   assign west_in_flit[2][1]     = east_out_flit[2][0];
   assign west_in_valid[2][1]    = east_out_valid[2][0];
   assign east_out_ready[2][0] = west_in_ready[2][1];
   assign east_in_flit[2][0]   = west_out_flit[2][1];
   assign east_in_valid[2][0]  = west_out_valid[2][1];
   assign west_out_ready[2][1]   = east_in_ready[2][0];



   /* Connecting (2,2) */
   assign north_in_flit[2][2]     = south_out_flit[1][2];
   assign north_in_valid[2][2]    = south_out_valid[1][2];
   assign south_out_ready[1][2] = north_in_ready[2][2];
   assign south_in_flit[1][2]   = north_out_flit[2][2];
   assign south_in_valid[1][2]  = north_out_valid[2][2];
   assign north_out_ready[2][2]   = south_in_ready[1][2];


   assign west_in_flit[2][2]     = east_out_flit[2][1];
   assign west_in_valid[2][2]    = east_out_valid[2][1];
   assign east_out_ready[2][1] = west_in_ready[2][2];
   assign east_in_flit[2][1]   = west_out_flit[2][2];
   assign east_in_valid[2][1]  = west_out_valid[2][2];
   assign west_out_ready[2][2]   = east_in_ready[2][1];



   /* Connecting (2,3) */
   assign north_in_flit[2][3]     = south_out_flit[1][3];
   assign north_in_valid[2][3]    = south_out_valid[1][3];
   assign south_out_ready[1][3] = north_in_ready[2][3];
   assign south_in_flit[1][3]   = north_out_flit[2][3];
   assign south_in_valid[1][3]  = north_out_valid[2][3];
   assign north_out_ready[2][3]   = south_in_ready[1][3];


   assign west_in_flit[2][3]     = east_out_flit[2][2];
   assign west_in_valid[2][3]    = east_out_valid[2][2];
   assign east_out_ready[2][2] = west_in_ready[2][3];
   assign east_in_flit[2][2]   = west_out_flit[2][3];
   assign east_in_valid[2][2]  = west_out_valid[2][3];
   assign west_out_ready[2][3]   = east_in_ready[2][2];



   /* Connecting (2,4) */
   assign north_in_flit[2][4]     = south_out_flit[1][4];
   assign north_in_valid[2][4]    = south_out_valid[1][4];
   assign south_out_ready[1][4] = north_in_ready[2][4];
   assign south_in_flit[1][4]   = north_out_flit[2][4];
   assign south_in_valid[1][4]  = north_out_valid[2][4];
   assign north_out_ready[2][4]   = south_in_ready[1][4];


   assign west_in_flit[2][4]     = east_out_flit[2][3];
   assign west_in_valid[2][4]    = east_out_valid[2][3];
   assign east_out_ready[2][3] = west_in_ready[2][4];
   assign east_in_flit[2][3]   = west_out_flit[2][4];
   assign east_in_valid[2][3]  = west_out_valid[2][4];
   assign west_out_ready[2][4]   = east_in_ready[2][3];



   /* Connecting (2,5) */
   assign north_in_flit[2][5]     = south_out_flit[1][5];
   assign north_in_valid[2][5]    = south_out_valid[1][5];
   assign south_out_ready[1][5] = north_in_ready[2][5];
   assign south_in_flit[1][5]   = north_out_flit[2][5];
   assign south_in_valid[1][5]  = north_out_valid[2][5];
   assign north_out_ready[2][5]   = south_in_ready[1][5];


   assign west_in_flit[2][5]     = east_out_flit[2][4];
   assign west_in_valid[2][5]    = east_out_valid[2][4];
   assign east_out_ready[2][4] = west_in_ready[2][5];
   assign east_in_flit[2][4]   = west_out_flit[2][5];
   assign east_in_valid[2][4]  = west_out_valid[2][5];
   assign west_out_ready[2][5]   = east_in_ready[2][4];



   /* Connecting (2,6) */
   assign north_in_flit[2][6]     = south_out_flit[1][6];
   assign north_in_valid[2][6]    = south_out_valid[1][6];
   assign south_out_ready[1][6] = north_in_ready[2][6];
   assign south_in_flit[1][6]   = north_out_flit[2][6];
   assign south_in_valid[1][6]  = north_out_valid[2][6];
   assign north_out_ready[2][6]   = south_in_ready[1][6];


   assign west_in_flit[2][6]     = east_out_flit[2][5];
   assign west_in_valid[2][6]    = east_out_valid[2][5];
   assign east_out_ready[2][5] = west_in_ready[2][6];
   assign east_in_flit[2][5]   = west_out_flit[2][6];
   assign east_in_valid[2][5]  = west_out_valid[2][6];
   assign west_out_ready[2][6]   = east_in_ready[2][5];



   /* Connecting (2,7) */
   assign north_in_flit[2][7]     = south_out_flit[1][7];
   assign north_in_valid[2][7]    = south_out_valid[1][7];
   assign south_out_ready[1][7] = north_in_ready[2][7];
   assign south_in_flit[1][7]   = north_out_flit[2][7];
   assign south_in_valid[1][7]  = north_out_valid[2][7];
   assign north_out_ready[2][7]   = south_in_ready[1][7];


   assign west_in_flit[2][7]     = east_out_flit[2][6];
   assign west_in_valid[2][7]    = east_out_valid[2][6];
   assign east_out_ready[2][6] = west_in_ready[2][7];
   assign east_in_flit[2][6]   = west_out_flit[2][7];
   assign east_in_valid[2][6]  = west_out_valid[2][7];
   assign west_out_ready[2][7]   = east_in_ready[2][6];

   assign east_in_flit[2][7]   = {flit_width{1'b0}};
   assign east_in_valid[2][7]  = {vchannels{1'b0}};
   assign east_out_ready[2][7] = {vchannels{1'b0}};




   /* Connecting (3,0) */
   assign north_in_flit[3][0]     = south_out_flit[2][0];
   assign north_in_valid[3][0]    = south_out_valid[2][0];
   assign south_out_ready[2][0] = north_in_ready[3][0];
   assign south_in_flit[2][0]   = north_out_flit[3][0];
   assign south_in_valid[2][0]  = north_out_valid[3][0];
   assign north_out_ready[3][0]   = south_in_ready[2][0];


   assign west_in_flit[3][0]   = {flit_width{1'b0}};
   assign west_in_valid[3][0]  = {vchannels{1'b0}};
   assign west_out_ready[3][0] = {vchannels{1'b0}};



   /* Connecting (3,1) */
   assign north_in_flit[3][1]     = south_out_flit[2][1];
   assign north_in_valid[3][1]    = south_out_valid[2][1];
   assign south_out_ready[2][1] = north_in_ready[3][1];
   assign south_in_flit[2][1]   = north_out_flit[3][1];
   assign south_in_valid[2][1]  = north_out_valid[3][1];
   assign north_out_ready[3][1]   = south_in_ready[2][1];


   assign west_in_flit[3][1]     = east_out_flit[3][0];
   assign west_in_valid[3][1]    = east_out_valid[3][0];
   assign east_out_ready[3][0] = west_in_ready[3][1];
   assign east_in_flit[3][0]   = west_out_flit[3][1];
   assign east_in_valid[3][0]  = west_out_valid[3][1];
   assign west_out_ready[3][1]   = east_in_ready[3][0];



   /* Connecting (3,2) */
   assign north_in_flit[3][2]     = south_out_flit[2][2];
   assign north_in_valid[3][2]    = south_out_valid[2][2];
   assign south_out_ready[2][2] = north_in_ready[3][2];
   assign south_in_flit[2][2]   = north_out_flit[3][2];
   assign south_in_valid[2][2]  = north_out_valid[3][2];
   assign north_out_ready[3][2]   = south_in_ready[2][2];


   assign west_in_flit[3][2]     = east_out_flit[3][1];
   assign west_in_valid[3][2]    = east_out_valid[3][1];
   assign east_out_ready[3][1] = west_in_ready[3][2];
   assign east_in_flit[3][1]   = west_out_flit[3][2];
   assign east_in_valid[3][1]  = west_out_valid[3][2];
   assign west_out_ready[3][2]   = east_in_ready[3][1];



   /* Connecting (3,3) */
   assign north_in_flit[3][3]     = south_out_flit[2][3];
   assign north_in_valid[3][3]    = south_out_valid[2][3];
   assign south_out_ready[2][3] = north_in_ready[3][3];
   assign south_in_flit[2][3]   = north_out_flit[3][3];
   assign south_in_valid[2][3]  = north_out_valid[3][3];
   assign north_out_ready[3][3]   = south_in_ready[2][3];


   assign west_in_flit[3][3]     = east_out_flit[3][2];
   assign west_in_valid[3][3]    = east_out_valid[3][2];
   assign east_out_ready[3][2] = west_in_ready[3][3];
   assign east_in_flit[3][2]   = west_out_flit[3][3];
   assign east_in_valid[3][2]  = west_out_valid[3][3];
   assign west_out_ready[3][3]   = east_in_ready[3][2];



   /* Connecting (3,4) */
   assign north_in_flit[3][4]     = south_out_flit[2][4];
   assign north_in_valid[3][4]    = south_out_valid[2][4];
   assign south_out_ready[2][4] = north_in_ready[3][4];
   assign south_in_flit[2][4]   = north_out_flit[3][4];
   assign south_in_valid[2][4]  = north_out_valid[3][4];
   assign north_out_ready[3][4]   = south_in_ready[2][4];


   assign west_in_flit[3][4]     = east_out_flit[3][3];
   assign west_in_valid[3][4]    = east_out_valid[3][3];
   assign east_out_ready[3][3] = west_in_ready[3][4];
   assign east_in_flit[3][3]   = west_out_flit[3][4];
   assign east_in_valid[3][3]  = west_out_valid[3][4];
   assign west_out_ready[3][4]   = east_in_ready[3][3];



   /* Connecting (3,5) */
   assign north_in_flit[3][5]     = south_out_flit[2][5];
   assign north_in_valid[3][5]    = south_out_valid[2][5];
   assign south_out_ready[2][5] = north_in_ready[3][5];
   assign south_in_flit[2][5]   = north_out_flit[3][5];
   assign south_in_valid[2][5]  = north_out_valid[3][5];
   assign north_out_ready[3][5]   = south_in_ready[2][5];


   assign west_in_flit[3][5]     = east_out_flit[3][4];
   assign west_in_valid[3][5]    = east_out_valid[3][4];
   assign east_out_ready[3][4] = west_in_ready[3][5];
   assign east_in_flit[3][4]   = west_out_flit[3][5];
   assign east_in_valid[3][4]  = west_out_valid[3][5];
   assign west_out_ready[3][5]   = east_in_ready[3][4];



   /* Connecting (3,6) */
   assign north_in_flit[3][6]     = south_out_flit[2][6];
   assign north_in_valid[3][6]    = south_out_valid[2][6];
   assign south_out_ready[2][6] = north_in_ready[3][6];
   assign south_in_flit[2][6]   = north_out_flit[3][6];
   assign south_in_valid[2][6]  = north_out_valid[3][6];
   assign north_out_ready[3][6]   = south_in_ready[2][6];


   assign west_in_flit[3][6]     = east_out_flit[3][5];
   assign west_in_valid[3][6]    = east_out_valid[3][5];
   assign east_out_ready[3][5] = west_in_ready[3][6];
   assign east_in_flit[3][5]   = west_out_flit[3][6];
   assign east_in_valid[3][5]  = west_out_valid[3][6];
   assign west_out_ready[3][6]   = east_in_ready[3][5];



   /* Connecting (3,7) */
   assign north_in_flit[3][7]     = south_out_flit[2][7];
   assign north_in_valid[3][7]    = south_out_valid[2][7];
   assign south_out_ready[2][7] = north_in_ready[3][7];
   assign south_in_flit[2][7]   = north_out_flit[3][7];
   assign south_in_valid[2][7]  = north_out_valid[3][7];
   assign north_out_ready[3][7]   = south_in_ready[2][7];


   assign west_in_flit[3][7]     = east_out_flit[3][6];
   assign west_in_valid[3][7]    = east_out_valid[3][6];
   assign east_out_ready[3][6] = west_in_ready[3][7];
   assign east_in_flit[3][6]   = west_out_flit[3][7];
   assign east_in_valid[3][6]  = west_out_valid[3][7];
   assign west_out_ready[3][7]   = east_in_ready[3][6];

   assign east_in_flit[3][7]   = {flit_width{1'b0}};
   assign east_in_valid[3][7]  = {vchannels{1'b0}};
   assign east_out_ready[3][7] = {vchannels{1'b0}};




   /* Connecting (4,0) */
   assign north_in_flit[4][0]     = south_out_flit[3][0];
   assign north_in_valid[4][0]    = south_out_valid[3][0];
   assign south_out_ready[3][0] = north_in_ready[4][0];
   assign south_in_flit[3][0]   = north_out_flit[4][0];
   assign south_in_valid[3][0]  = north_out_valid[4][0];
   assign north_out_ready[4][0]   = south_in_ready[3][0];


   assign west_in_flit[4][0]   = {flit_width{1'b0}};
   assign west_in_valid[4][0]  = {vchannels{1'b0}};
   assign west_out_ready[4][0] = {vchannels{1'b0}};



   /* Connecting (4,1) */
   assign north_in_flit[4][1]     = south_out_flit[3][1];
   assign north_in_valid[4][1]    = south_out_valid[3][1];
   assign south_out_ready[3][1] = north_in_ready[4][1];
   assign south_in_flit[3][1]   = north_out_flit[4][1];
   assign south_in_valid[3][1]  = north_out_valid[4][1];
   assign north_out_ready[4][1]   = south_in_ready[3][1];


   assign west_in_flit[4][1]     = east_out_flit[4][0];
   assign west_in_valid[4][1]    = east_out_valid[4][0];
   assign east_out_ready[4][0] = west_in_ready[4][1];
   assign east_in_flit[4][0]   = west_out_flit[4][1];
   assign east_in_valid[4][0]  = west_out_valid[4][1];
   assign west_out_ready[4][1]   = east_in_ready[4][0];



   /* Connecting (4,2) */
   assign north_in_flit[4][2]     = south_out_flit[3][2];
   assign north_in_valid[4][2]    = south_out_valid[3][2];
   assign south_out_ready[3][2] = north_in_ready[4][2];
   assign south_in_flit[3][2]   = north_out_flit[4][2];
   assign south_in_valid[3][2]  = north_out_valid[4][2];
   assign north_out_ready[4][2]   = south_in_ready[3][2];


   assign west_in_flit[4][2]     = east_out_flit[4][1];
   assign west_in_valid[4][2]    = east_out_valid[4][1];
   assign east_out_ready[4][1] = west_in_ready[4][2];
   assign east_in_flit[4][1]   = west_out_flit[4][2];
   assign east_in_valid[4][1]  = west_out_valid[4][2];
   assign west_out_ready[4][2]   = east_in_ready[4][1];



   /* Connecting (4,3) */
   assign north_in_flit[4][3]     = south_out_flit[3][3];
   assign north_in_valid[4][3]    = south_out_valid[3][3];
   assign south_out_ready[3][3] = north_in_ready[4][3];
   assign south_in_flit[3][3]   = north_out_flit[4][3];
   assign south_in_valid[3][3]  = north_out_valid[4][3];
   assign north_out_ready[4][3]   = south_in_ready[3][3];


   assign west_in_flit[4][3]     = east_out_flit[4][2];
   assign west_in_valid[4][3]    = east_out_valid[4][2];
   assign east_out_ready[4][2] = west_in_ready[4][3];
   assign east_in_flit[4][2]   = west_out_flit[4][3];
   assign east_in_valid[4][2]  = west_out_valid[4][3];
   assign west_out_ready[4][3]   = east_in_ready[4][2];



   /* Connecting (4,4) */
   assign north_in_flit[4][4]     = south_out_flit[3][4];
   assign north_in_valid[4][4]    = south_out_valid[3][4];
   assign south_out_ready[3][4] = north_in_ready[4][4];
   assign south_in_flit[3][4]   = north_out_flit[4][4];
   assign south_in_valid[3][4]  = north_out_valid[4][4];
   assign north_out_ready[4][4]   = south_in_ready[3][4];


   assign west_in_flit[4][4]     = east_out_flit[4][3];
   assign west_in_valid[4][4]    = east_out_valid[4][3];
   assign east_out_ready[4][3] = west_in_ready[4][4];
   assign east_in_flit[4][3]   = west_out_flit[4][4];
   assign east_in_valid[4][3]  = west_out_valid[4][4];
   assign west_out_ready[4][4]   = east_in_ready[4][3];



   /* Connecting (4,5) */
   assign north_in_flit[4][5]     = south_out_flit[3][5];
   assign north_in_valid[4][5]    = south_out_valid[3][5];
   assign south_out_ready[3][5] = north_in_ready[4][5];
   assign south_in_flit[3][5]   = north_out_flit[4][5];
   assign south_in_valid[3][5]  = north_out_valid[4][5];
   assign north_out_ready[4][5]   = south_in_ready[3][5];


   assign west_in_flit[4][5]     = east_out_flit[4][4];
   assign west_in_valid[4][5]    = east_out_valid[4][4];
   assign east_out_ready[4][4] = west_in_ready[4][5];
   assign east_in_flit[4][4]   = west_out_flit[4][5];
   assign east_in_valid[4][4]  = west_out_valid[4][5];
   assign west_out_ready[4][5]   = east_in_ready[4][4];



   /* Connecting (4,6) */
   assign north_in_flit[4][6]     = south_out_flit[3][6];
   assign north_in_valid[4][6]    = south_out_valid[3][6];
   assign south_out_ready[3][6] = north_in_ready[4][6];
   assign south_in_flit[3][6]   = north_out_flit[4][6];
   assign south_in_valid[3][6]  = north_out_valid[4][6];
   assign north_out_ready[4][6]   = south_in_ready[3][6];


   assign west_in_flit[4][6]     = east_out_flit[4][5];
   assign west_in_valid[4][6]    = east_out_valid[4][5];
   assign east_out_ready[4][5] = west_in_ready[4][6];
   assign east_in_flit[4][5]   = west_out_flit[4][6];
   assign east_in_valid[4][5]  = west_out_valid[4][6];
   assign west_out_ready[4][6]   = east_in_ready[4][5];



   /* Connecting (4,7) */
   assign north_in_flit[4][7]     = south_out_flit[3][7];
   assign north_in_valid[4][7]    = south_out_valid[3][7];
   assign south_out_ready[3][7] = north_in_ready[4][7];
   assign south_in_flit[3][7]   = north_out_flit[4][7];
   assign south_in_valid[3][7]  = north_out_valid[4][7];
   assign north_out_ready[4][7]   = south_in_ready[3][7];


   assign west_in_flit[4][7]     = east_out_flit[4][6];
   assign west_in_valid[4][7]    = east_out_valid[4][6];
   assign east_out_ready[4][6] = west_in_ready[4][7];
   assign east_in_flit[4][6]   = west_out_flit[4][7];
   assign east_in_valid[4][6]  = west_out_valid[4][7];
   assign west_out_ready[4][7]   = east_in_ready[4][6];

   assign east_in_flit[4][7]   = {flit_width{1'b0}};
   assign east_in_valid[4][7]  = {vchannels{1'b0}};
   assign east_out_ready[4][7] = {vchannels{1'b0}};




   /* Connecting (5,0) */
   assign north_in_flit[5][0]     = south_out_flit[4][0];
   assign north_in_valid[5][0]    = south_out_valid[4][0];
   assign south_out_ready[4][0] = north_in_ready[5][0];
   assign south_in_flit[4][0]   = north_out_flit[5][0];
   assign south_in_valid[4][0]  = north_out_valid[5][0];
   assign north_out_ready[5][0]   = south_in_ready[4][0];


   assign west_in_flit[5][0]   = {flit_width{1'b0}};
   assign west_in_valid[5][0]  = {vchannels{1'b0}};
   assign west_out_ready[5][0] = {vchannels{1'b0}};



   /* Connecting (5,1) */
   assign north_in_flit[5][1]     = south_out_flit[4][1];
   assign north_in_valid[5][1]    = south_out_valid[4][1];
   assign south_out_ready[4][1] = north_in_ready[5][1];
   assign south_in_flit[4][1]   = north_out_flit[5][1];
   assign south_in_valid[4][1]  = north_out_valid[5][1];
   assign north_out_ready[5][1]   = south_in_ready[4][1];


   assign west_in_flit[5][1]     = east_out_flit[5][0];
   assign west_in_valid[5][1]    = east_out_valid[5][0];
   assign east_out_ready[5][0] = west_in_ready[5][1];
   assign east_in_flit[5][0]   = west_out_flit[5][1];
   assign east_in_valid[5][0]  = west_out_valid[5][1];
   assign west_out_ready[5][1]   = east_in_ready[5][0];



   /* Connecting (5,2) */
   assign north_in_flit[5][2]     = south_out_flit[4][2];
   assign north_in_valid[5][2]    = south_out_valid[4][2];
   assign south_out_ready[4][2] = north_in_ready[5][2];
   assign south_in_flit[4][2]   = north_out_flit[5][2];
   assign south_in_valid[4][2]  = north_out_valid[5][2];
   assign north_out_ready[5][2]   = south_in_ready[4][2];


   assign west_in_flit[5][2]     = east_out_flit[5][1];
   assign west_in_valid[5][2]    = east_out_valid[5][1];
   assign east_out_ready[5][1] = west_in_ready[5][2];
   assign east_in_flit[5][1]   = west_out_flit[5][2];
   assign east_in_valid[5][1]  = west_out_valid[5][2];
   assign west_out_ready[5][2]   = east_in_ready[5][1];



   /* Connecting (5,3) */
   assign north_in_flit[5][3]     = south_out_flit[4][3];
   assign north_in_valid[5][3]    = south_out_valid[4][3];
   assign south_out_ready[4][3] = north_in_ready[5][3];
   assign south_in_flit[4][3]   = north_out_flit[5][3];
   assign south_in_valid[4][3]  = north_out_valid[5][3];
   assign north_out_ready[5][3]   = south_in_ready[4][3];


   assign west_in_flit[5][3]     = east_out_flit[5][2];
   assign west_in_valid[5][3]    = east_out_valid[5][2];
   assign east_out_ready[5][2] = west_in_ready[5][3];
   assign east_in_flit[5][2]   = west_out_flit[5][3];
   assign east_in_valid[5][2]  = west_out_valid[5][3];
   assign west_out_ready[5][3]   = east_in_ready[5][2];



   /* Connecting (5,4) */
   assign north_in_flit[5][4]     = south_out_flit[4][4];
   assign north_in_valid[5][4]    = south_out_valid[4][4];
   assign south_out_ready[4][4] = north_in_ready[5][4];
   assign south_in_flit[4][4]   = north_out_flit[5][4];
   assign south_in_valid[4][4]  = north_out_valid[5][4];
   assign north_out_ready[5][4]   = south_in_ready[4][4];


   assign west_in_flit[5][4]     = east_out_flit[5][3];
   assign west_in_valid[5][4]    = east_out_valid[5][3];
   assign east_out_ready[5][3] = west_in_ready[5][4];
   assign east_in_flit[5][3]   = west_out_flit[5][4];
   assign east_in_valid[5][3]  = west_out_valid[5][4];
   assign west_out_ready[5][4]   = east_in_ready[5][3];



   /* Connecting (5,5) */
   assign north_in_flit[5][5]     = south_out_flit[4][5];
   assign north_in_valid[5][5]    = south_out_valid[4][5];
   assign south_out_ready[4][5] = north_in_ready[5][5];
   assign south_in_flit[4][5]   = north_out_flit[5][5];
   assign south_in_valid[4][5]  = north_out_valid[5][5];
   assign north_out_ready[5][5]   = south_in_ready[4][5];


   assign west_in_flit[5][5]     = east_out_flit[5][4];
   assign west_in_valid[5][5]    = east_out_valid[5][4];
   assign east_out_ready[5][4] = west_in_ready[5][5];
   assign east_in_flit[5][4]   = west_out_flit[5][5];
   assign east_in_valid[5][4]  = west_out_valid[5][5];
   assign west_out_ready[5][5]   = east_in_ready[5][4];



   /* Connecting (5,6) */
   assign north_in_flit[5][6]     = south_out_flit[4][6];
   assign north_in_valid[5][6]    = south_out_valid[4][6];
   assign south_out_ready[4][6] = north_in_ready[5][6];
   assign south_in_flit[4][6]   = north_out_flit[5][6];
   assign south_in_valid[4][6]  = north_out_valid[5][6];
   assign north_out_ready[5][6]   = south_in_ready[4][6];


   assign west_in_flit[5][6]     = east_out_flit[5][5];
   assign west_in_valid[5][6]    = east_out_valid[5][5];
   assign east_out_ready[5][5] = west_in_ready[5][6];
   assign east_in_flit[5][5]   = west_out_flit[5][6];
   assign east_in_valid[5][5]  = west_out_valid[5][6];
   assign west_out_ready[5][6]   = east_in_ready[5][5];



   /* Connecting (5,7) */
   assign north_in_flit[5][7]     = south_out_flit[4][7];
   assign north_in_valid[5][7]    = south_out_valid[4][7];
   assign south_out_ready[4][7] = north_in_ready[5][7];
   assign south_in_flit[4][7]   = north_out_flit[5][7];
   assign south_in_valid[4][7]  = north_out_valid[5][7];
   assign north_out_ready[5][7]   = south_in_ready[4][7];


   assign west_in_flit[5][7]     = east_out_flit[5][6];
   assign west_in_valid[5][7]    = east_out_valid[5][6];
   assign east_out_ready[5][6] = west_in_ready[5][7];
   assign east_in_flit[5][6]   = west_out_flit[5][7];
   assign east_in_valid[5][6]  = west_out_valid[5][7];
   assign west_out_ready[5][7]   = east_in_ready[5][6];

   assign east_in_flit[5][7]   = {flit_width{1'b0}};
   assign east_in_valid[5][7]  = {vchannels{1'b0}};
   assign east_out_ready[5][7] = {vchannels{1'b0}};




   /* Connecting (6,0) */
   assign north_in_flit[6][0]     = south_out_flit[5][0];
   assign north_in_valid[6][0]    = south_out_valid[5][0];
   assign south_out_ready[5][0] = north_in_ready[6][0];
   assign south_in_flit[5][0]   = north_out_flit[6][0];
   assign south_in_valid[5][0]  = north_out_valid[6][0];
   assign north_out_ready[6][0]   = south_in_ready[5][0];


   assign west_in_flit[6][0]   = {flit_width{1'b0}};
   assign west_in_valid[6][0]  = {vchannels{1'b0}};
   assign west_out_ready[6][0] = {vchannels{1'b0}};



   /* Connecting (6,1) */
   assign north_in_flit[6][1]     = south_out_flit[5][1];
   assign north_in_valid[6][1]    = south_out_valid[5][1];
   assign south_out_ready[5][1] = north_in_ready[6][1];
   assign south_in_flit[5][1]   = north_out_flit[6][1];
   assign south_in_valid[5][1]  = north_out_valid[6][1];
   assign north_out_ready[6][1]   = south_in_ready[5][1];


   assign west_in_flit[6][1]     = east_out_flit[6][0];
   assign west_in_valid[6][1]    = east_out_valid[6][0];
   assign east_out_ready[6][0] = west_in_ready[6][1];
   assign east_in_flit[6][0]   = west_out_flit[6][1];
   assign east_in_valid[6][0]  = west_out_valid[6][1];
   assign west_out_ready[6][1]   = east_in_ready[6][0];



   /* Connecting (6,2) */
   assign north_in_flit[6][2]     = south_out_flit[5][2];
   assign north_in_valid[6][2]    = south_out_valid[5][2];
   assign south_out_ready[5][2] = north_in_ready[6][2];
   assign south_in_flit[5][2]   = north_out_flit[6][2];
   assign south_in_valid[5][2]  = north_out_valid[6][2];
   assign north_out_ready[6][2]   = south_in_ready[5][2];


   assign west_in_flit[6][2]     = east_out_flit[6][1];
   assign west_in_valid[6][2]    = east_out_valid[6][1];
   assign east_out_ready[6][1] = west_in_ready[6][2];
   assign east_in_flit[6][1]   = west_out_flit[6][2];
   assign east_in_valid[6][1]  = west_out_valid[6][2];
   assign west_out_ready[6][2]   = east_in_ready[6][1];



   /* Connecting (6,3) */
   assign north_in_flit[6][3]     = south_out_flit[5][3];
   assign north_in_valid[6][3]    = south_out_valid[5][3];
   assign south_out_ready[5][3] = north_in_ready[6][3];
   assign south_in_flit[5][3]   = north_out_flit[6][3];
   assign south_in_valid[5][3]  = north_out_valid[6][3];
   assign north_out_ready[6][3]   = south_in_ready[5][3];


   assign west_in_flit[6][3]     = east_out_flit[6][2];
   assign west_in_valid[6][3]    = east_out_valid[6][2];
   assign east_out_ready[6][2] = west_in_ready[6][3];
   assign east_in_flit[6][2]   = west_out_flit[6][3];
   assign east_in_valid[6][2]  = west_out_valid[6][3];
   assign west_out_ready[6][3]   = east_in_ready[6][2];



   /* Connecting (6,4) */
   assign north_in_flit[6][4]     = south_out_flit[5][4];
   assign north_in_valid[6][4]    = south_out_valid[5][4];
   assign south_out_ready[5][4] = north_in_ready[6][4];
   assign south_in_flit[5][4]   = north_out_flit[6][4];
   assign south_in_valid[5][4]  = north_out_valid[6][4];
   assign north_out_ready[6][4]   = south_in_ready[5][4];


   assign west_in_flit[6][4]     = east_out_flit[6][3];
   assign west_in_valid[6][4]    = east_out_valid[6][3];
   assign east_out_ready[6][3] = west_in_ready[6][4];
   assign east_in_flit[6][3]   = west_out_flit[6][4];
   assign east_in_valid[6][3]  = west_out_valid[6][4];
   assign west_out_ready[6][4]   = east_in_ready[6][3];



   /* Connecting (6,5) */
   assign north_in_flit[6][5]     = south_out_flit[5][5];
   assign north_in_valid[6][5]    = south_out_valid[5][5];
   assign south_out_ready[5][5] = north_in_ready[6][5];
   assign south_in_flit[5][5]   = north_out_flit[6][5];
   assign south_in_valid[5][5]  = north_out_valid[6][5];
   assign north_out_ready[6][5]   = south_in_ready[5][5];


   assign west_in_flit[6][5]     = east_out_flit[6][4];
   assign west_in_valid[6][5]    = east_out_valid[6][4];
   assign east_out_ready[6][4] = west_in_ready[6][5];
   assign east_in_flit[6][4]   = west_out_flit[6][5];
   assign east_in_valid[6][4]  = west_out_valid[6][5];
   assign west_out_ready[6][5]   = east_in_ready[6][4];



   /* Connecting (6,6) */
   assign north_in_flit[6][6]     = south_out_flit[5][6];
   assign north_in_valid[6][6]    = south_out_valid[5][6];
   assign south_out_ready[5][6] = north_in_ready[6][6];
   assign south_in_flit[5][6]   = north_out_flit[6][6];
   assign south_in_valid[5][6]  = north_out_valid[6][6];
   assign north_out_ready[6][6]   = south_in_ready[5][6];


   assign west_in_flit[6][6]     = east_out_flit[6][5];
   assign west_in_valid[6][6]    = east_out_valid[6][5];
   assign east_out_ready[6][5] = west_in_ready[6][6];
   assign east_in_flit[6][5]   = west_out_flit[6][6];
   assign east_in_valid[6][5]  = west_out_valid[6][6];
   assign west_out_ready[6][6]   = east_in_ready[6][5];



   /* Connecting (6,7) */
   assign north_in_flit[6][7]     = south_out_flit[5][7];
   assign north_in_valid[6][7]    = south_out_valid[5][7];
   assign south_out_ready[5][7] = north_in_ready[6][7];
   assign south_in_flit[5][7]   = north_out_flit[6][7];
   assign south_in_valid[5][7]  = north_out_valid[6][7];
   assign north_out_ready[6][7]   = south_in_ready[5][7];


   assign west_in_flit[6][7]     = east_out_flit[6][6];
   assign west_in_valid[6][7]    = east_out_valid[6][6];
   assign east_out_ready[6][6] = west_in_ready[6][7];
   assign east_in_flit[6][6]   = west_out_flit[6][7];
   assign east_in_valid[6][6]  = west_out_valid[6][7];
   assign west_out_ready[6][7]   = east_in_ready[6][6];

   assign east_in_flit[6][7]   = {flit_width{1'b0}};
   assign east_in_valid[6][7]  = {vchannels{1'b0}};
   assign east_out_ready[6][7] = {vchannels{1'b0}};




   /* Connecting (7,0) */
   assign north_in_flit[7][0]     = south_out_flit[6][0];
   assign north_in_valid[7][0]    = south_out_valid[6][0];
   assign south_out_ready[6][0] = north_in_ready[7][0];
   assign south_in_flit[6][0]   = north_out_flit[7][0];
   assign south_in_valid[6][0]  = north_out_valid[7][0];
   assign north_out_ready[7][0]   = south_in_ready[6][0];

   assign south_in_flit[7][0]   = {flit_width{1'b0}};
   assign south_in_valid[7][0]  = {vchannels{1'b0}};
   assign south_out_ready[7][0] = {vchannels{1'b0}};

   assign west_in_flit[7][0]   = {flit_width{1'b0}};
   assign west_in_valid[7][0]  = {vchannels{1'b0}};
   assign west_out_ready[7][0] = {vchannels{1'b0}};



   /* Connecting (7,1) */
   assign north_in_flit[7][1]     = south_out_flit[6][1];
   assign north_in_valid[7][1]    = south_out_valid[6][1];
   assign south_out_ready[6][1] = north_in_ready[7][1];
   assign south_in_flit[6][1]   = north_out_flit[7][1];
   assign south_in_valid[6][1]  = north_out_valid[7][1];
   assign north_out_ready[7][1]   = south_in_ready[6][1];

   assign south_in_flit[7][1]   = {flit_width{1'b0}};
   assign south_in_valid[7][1]  = {vchannels{1'b0}};
   assign south_out_ready[7][1] = {vchannels{1'b0}};

   assign west_in_flit[7][1]     = east_out_flit[7][0];
   assign west_in_valid[7][1]    = east_out_valid[7][0];
   assign east_out_ready[7][0] = west_in_ready[7][1];
   assign east_in_flit[7][0]   = west_out_flit[7][1];
   assign east_in_valid[7][0]  = west_out_valid[7][1];
   assign west_out_ready[7][1]   = east_in_ready[7][0];



   /* Connecting (7,2) */
   assign north_in_flit[7][2]     = south_out_flit[6][2];
   assign north_in_valid[7][2]    = south_out_valid[6][2];
   assign south_out_ready[6][2] = north_in_ready[7][2];
   assign south_in_flit[6][2]   = north_out_flit[7][2];
   assign south_in_valid[6][2]  = north_out_valid[7][2];
   assign north_out_ready[7][2]   = south_in_ready[6][2];

   assign south_in_flit[7][2]   = {flit_width{1'b0}};
   assign south_in_valid[7][2]  = {vchannels{1'b0}};
   assign south_out_ready[7][2] = {vchannels{1'b0}};

   assign west_in_flit[7][2]     = east_out_flit[7][1];
   assign west_in_valid[7][2]    = east_out_valid[7][1];
   assign east_out_ready[7][1] = west_in_ready[7][2];
   assign east_in_flit[7][1]   = west_out_flit[7][2];
   assign east_in_valid[7][1]  = west_out_valid[7][2];
   assign west_out_ready[7][2]   = east_in_ready[7][1];



   /* Connecting (7,3) */
   assign north_in_flit[7][3]     = south_out_flit[6][3];
   assign north_in_valid[7][3]    = south_out_valid[6][3];
   assign south_out_ready[6][3] = north_in_ready[7][3];
   assign south_in_flit[6][3]   = north_out_flit[7][3];
   assign south_in_valid[6][3]  = north_out_valid[7][3];
   assign north_out_ready[7][3]   = south_in_ready[6][3];

   assign south_in_flit[7][3]   = {flit_width{1'b0}};
   assign south_in_valid[7][3]  = {vchannels{1'b0}};
   assign south_out_ready[7][3] = {vchannels{1'b0}};

   assign west_in_flit[7][3]     = east_out_flit[7][2];
   assign west_in_valid[7][3]    = east_out_valid[7][2];
   assign east_out_ready[7][2] = west_in_ready[7][3];
   assign east_in_flit[7][2]   = west_out_flit[7][3];
   assign east_in_valid[7][2]  = west_out_valid[7][3];
   assign west_out_ready[7][3]   = east_in_ready[7][2];



   /* Connecting (7,4) */
   assign north_in_flit[7][4]     = south_out_flit[6][4];
   assign north_in_valid[7][4]    = south_out_valid[6][4];
   assign south_out_ready[6][4] = north_in_ready[7][4];
   assign south_in_flit[6][4]   = north_out_flit[7][4];
   assign south_in_valid[6][4]  = north_out_valid[7][4];
   assign north_out_ready[7][4]   = south_in_ready[6][4];

   assign south_in_flit[7][4]   = {flit_width{1'b0}};
   assign south_in_valid[7][4]  = {vchannels{1'b0}};
   assign south_out_ready[7][4] = {vchannels{1'b0}};

   assign west_in_flit[7][4]     = east_out_flit[7][3];
   assign west_in_valid[7][4]    = east_out_valid[7][3];
   assign east_out_ready[7][3] = west_in_ready[7][4];
   assign east_in_flit[7][3]   = west_out_flit[7][4];
   assign east_in_valid[7][3]  = west_out_valid[7][4];
   assign west_out_ready[7][4]   = east_in_ready[7][3];



   /* Connecting (7,5) */
   assign north_in_flit[7][5]     = south_out_flit[6][5];
   assign north_in_valid[7][5]    = south_out_valid[6][5];
   assign south_out_ready[6][5] = north_in_ready[7][5];
   assign south_in_flit[6][5]   = north_out_flit[7][5];
   assign south_in_valid[6][5]  = north_out_valid[7][5];
   assign north_out_ready[7][5]   = south_in_ready[6][5];

   assign south_in_flit[7][5]   = {flit_width{1'b0}};
   assign south_in_valid[7][5]  = {vchannels{1'b0}};
   assign south_out_ready[7][5] = {vchannels{1'b0}};

   assign west_in_flit[7][5]     = east_out_flit[7][4];
   assign west_in_valid[7][5]    = east_out_valid[7][4];
   assign east_out_ready[7][4] = west_in_ready[7][5];
   assign east_in_flit[7][4]   = west_out_flit[7][5];
   assign east_in_valid[7][4]  = west_out_valid[7][5];
   assign west_out_ready[7][5]   = east_in_ready[7][4];



   /* Connecting (7,6) */
   assign north_in_flit[7][6]     = south_out_flit[6][6];
   assign north_in_valid[7][6]    = south_out_valid[6][6];
   assign south_out_ready[6][6] = north_in_ready[7][6];
   assign south_in_flit[6][6]   = north_out_flit[7][6];
   assign south_in_valid[6][6]  = north_out_valid[7][6];
   assign north_out_ready[7][6]   = south_in_ready[6][6];

   assign south_in_flit[7][6]   = {flit_width{1'b0}};
   assign south_in_valid[7][6]  = {vchannels{1'b0}};
   assign south_out_ready[7][6] = {vchannels{1'b0}};

   assign west_in_flit[7][6]     = east_out_flit[7][5];
   assign west_in_valid[7][6]    = east_out_valid[7][5];
   assign east_out_ready[7][5] = west_in_ready[7][6];
   assign east_in_flit[7][5]   = west_out_flit[7][6];
   assign east_in_valid[7][5]  = west_out_valid[7][6];
   assign west_out_ready[7][6]   = east_in_ready[7][5];



   /* Connecting (7,7) */
   assign north_in_flit[7][7]     = south_out_flit[6][7];
   assign north_in_valid[7][7]    = south_out_valid[6][7];
   assign south_out_ready[6][7] = north_in_ready[7][7];
   assign south_in_flit[6][7]   = north_out_flit[7][7];
   assign south_in_valid[6][7]  = north_out_valid[7][7];
   assign north_out_ready[7][7]   = south_in_ready[6][7];

   assign south_in_flit[7][7]   = {flit_width{1'b0}};
   assign south_in_valid[7][7]  = {vchannels{1'b0}};
   assign south_out_ready[7][7] = {vchannels{1'b0}};

   assign west_in_flit[7][7]     = east_out_flit[7][6];
   assign west_in_valid[7][7]    = east_out_valid[7][6];
   assign east_out_ready[7][6] = west_in_ready[7][7];
   assign east_in_flit[7][6]   = west_out_flit[7][7];
   assign east_in_valid[7][6]  = west_out_valid[7][7];
   assign west_out_ready[7][7]   = east_in_ready[7][6];

   assign east_in_flit[7][7]   = {flit_width{1'b0}};
   assign east_in_valid[7][7]  = {vchannels{1'b0}};
   assign east_out_ready[7][7] = {vchannels{1'b0}};







   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][0][vchannels-1:0]),
       .local_out_flit_o	(link0_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link0_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][0][vchannels-1:0]),
       .local_in_ready_o	(link0_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][0][vchannels-1:0]),
       .local_out_ready_i	(link0_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][0][vchannels-1:0]),
       .local_in_flit_i		(link0_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link0_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][1][vchannels-1:0]),
       .local_out_flit_o	(link1_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link1_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][1][vchannels-1:0]),
       .local_in_ready_o	(link1_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][1][vchannels-1:0]),
       .local_out_ready_i	(link1_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][1][vchannels-1:0]),
       .local_in_flit_i		(link1_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link1_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][2][vchannels-1:0]),
       .local_out_flit_o	(link2_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link2_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][2][vchannels-1:0]),
       .local_in_ready_o	(link2_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][2][vchannels-1:0]),
       .local_out_ready_i	(link2_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][2][vchannels-1:0]),
       .local_in_flit_i		(link2_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link2_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][3][vchannels-1:0]),
       .local_out_flit_o	(link3_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link3_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][3][vchannels-1:0]),
       .local_in_ready_o	(link3_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][3][vchannels-1:0]),
       .local_out_ready_i	(link3_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][3][vchannels-1:0]),
       .local_in_flit_i		(link3_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link3_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][4][vchannels-1:0]),
       .local_out_flit_o	(link4_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link4_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][4][vchannels-1:0]),
       .local_in_ready_o	(link4_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][4][vchannels-1:0]),
       .local_out_ready_i	(link4_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][4][vchannels-1:0]),
       .local_in_flit_i		(link4_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link4_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][5][vchannels-1:0]),
       .local_out_flit_o	(link5_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link5_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][5][vchannels-1:0]),
       .local_in_ready_o	(link5_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][5][vchannels-1:0]),
       .local_out_ready_i	(link5_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][5][vchannels-1:0]),
       .local_in_flit_i		(link5_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link5_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][6][vchannels-1:0]),
       .local_out_flit_o	(link6_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link6_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][6][vchannels-1:0]),
       .local_in_ready_o	(link6_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][6][vchannels-1:0]),
       .local_out_ready_i	(link6_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][6][vchannels-1:0]),
       .local_in_flit_i		(link6_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link6_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_0_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[0][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[0][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[0][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[0][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[0][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[0][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[0][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[0][7][vchannels-1:0]),
       .local_out_flit_o	(link7_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link7_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[0][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[0][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[0][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[0][7][vchannels-1:0]),
       .local_in_ready_o	(link7_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[0][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[0][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[0][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[0][7][vchannels-1:0]),
       .local_out_ready_i	(link7_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[0][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[0][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[0][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[0][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[0][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[0][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[0][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[0][7][vchannels-1:0]),
       .local_in_flit_i		(link7_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link7_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][0][vchannels-1:0]),
       .local_out_flit_o	(link8_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link8_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][0][vchannels-1:0]),
       .local_in_ready_o	(link8_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][0][vchannels-1:0]),
       .local_out_ready_i	(link8_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][0][vchannels-1:0]),
       .local_in_flit_i		(link8_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link8_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][1][vchannels-1:0]),
       .local_out_flit_o	(link9_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link9_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][1][vchannels-1:0]),
       .local_in_ready_o	(link9_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][1][vchannels-1:0]),
       .local_out_ready_i	(link9_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][1][vchannels-1:0]),
       .local_in_flit_i		(link9_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link9_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][2][vchannels-1:0]),
       .local_out_flit_o	(link10_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link10_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][2][vchannels-1:0]),
       .local_in_ready_o	(link10_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][2][vchannels-1:0]),
       .local_out_ready_i	(link10_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][2][vchannels-1:0]),
       .local_in_flit_i		(link10_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link10_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][3][vchannels-1:0]),
       .local_out_flit_o	(link11_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link11_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][3][vchannels-1:0]),
       .local_in_ready_o	(link11_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][3][vchannels-1:0]),
       .local_out_ready_i	(link11_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][3][vchannels-1:0]),
       .local_in_flit_i		(link11_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link11_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][4][vchannels-1:0]),
       .local_out_flit_o	(link12_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link12_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][4][vchannels-1:0]),
       .local_in_ready_o	(link12_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][4][vchannels-1:0]),
       .local_out_ready_i	(link12_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][4][vchannels-1:0]),
       .local_in_flit_i		(link12_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link12_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][5][vchannels-1:0]),
       .local_out_flit_o	(link13_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link13_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][5][vchannels-1:0]),
       .local_in_ready_o	(link13_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][5][vchannels-1:0]),
       .local_out_ready_i	(link13_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][5][vchannels-1:0]),
       .local_in_flit_i		(link13_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link13_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][6][vchannels-1:0]),
       .local_out_flit_o	(link14_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link14_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][6][vchannels-1:0]),
       .local_in_ready_o	(link14_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][6][vchannels-1:0]),
       .local_out_ready_i	(link14_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][6][vchannels-1:0]),
       .local_in_flit_i		(link14_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link14_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_1_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[1][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[1][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[1][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[1][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[1][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[1][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[1][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[1][7][vchannels-1:0]),
       .local_out_flit_o	(link15_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link15_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[1][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[1][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[1][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[1][7][vchannels-1:0]),
       .local_in_ready_o	(link15_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[1][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[1][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[1][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[1][7][vchannels-1:0]),
       .local_out_ready_i	(link15_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[1][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[1][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[1][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[1][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[1][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[1][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[1][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[1][7][vchannels-1:0]),
       .local_in_flit_i		(link15_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link15_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][0][vchannels-1:0]),
       .local_out_flit_o	(link16_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link16_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][0][vchannels-1:0]),
       .local_in_ready_o	(link16_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][0][vchannels-1:0]),
       .local_out_ready_i	(link16_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][0][vchannels-1:0]),
       .local_in_flit_i		(link16_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link16_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][1][vchannels-1:0]),
       .local_out_flit_o	(link17_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link17_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][1][vchannels-1:0]),
       .local_in_ready_o	(link17_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][1][vchannels-1:0]),
       .local_out_ready_i	(link17_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][1][vchannels-1:0]),
       .local_in_flit_i		(link17_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link17_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][2][vchannels-1:0]),
       .local_out_flit_o	(link18_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link18_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][2][vchannels-1:0]),
       .local_in_ready_o	(link18_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][2][vchannels-1:0]),
       .local_out_ready_i	(link18_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][2][vchannels-1:0]),
       .local_in_flit_i		(link18_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link18_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][3][vchannels-1:0]),
       .local_out_flit_o	(link19_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link19_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][3][vchannels-1:0]),
       .local_in_ready_o	(link19_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][3][vchannels-1:0]),
       .local_out_ready_i	(link19_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][3][vchannels-1:0]),
       .local_in_flit_i		(link19_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link19_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][4][vchannels-1:0]),
       .local_out_flit_o	(link20_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link20_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][4][vchannels-1:0]),
       .local_in_ready_o	(link20_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][4][vchannels-1:0]),
       .local_out_ready_i	(link20_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][4][vchannels-1:0]),
       .local_in_flit_i		(link20_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link20_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][5][vchannels-1:0]),
       .local_out_flit_o	(link21_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link21_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][5][vchannels-1:0]),
       .local_in_ready_o	(link21_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][5][vchannels-1:0]),
       .local_out_ready_i	(link21_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][5][vchannels-1:0]),
       .local_in_flit_i		(link21_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link21_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][6][vchannels-1:0]),
       .local_out_flit_o	(link22_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link22_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][6][vchannels-1:0]),
       .local_in_ready_o	(link22_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][6][vchannels-1:0]),
       .local_out_ready_i	(link22_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][6][vchannels-1:0]),
       .local_in_flit_i		(link22_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link22_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_2_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[2][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[2][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[2][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[2][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[2][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[2][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[2][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[2][7][vchannels-1:0]),
       .local_out_flit_o	(link23_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link23_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[2][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[2][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[2][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[2][7][vchannels-1:0]),
       .local_in_ready_o	(link23_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[2][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[2][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[2][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[2][7][vchannels-1:0]),
       .local_out_ready_i	(link23_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[2][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[2][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[2][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[2][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[2][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[2][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[2][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[2][7][vchannels-1:0]),
       .local_in_flit_i		(link23_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link23_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][0][vchannels-1:0]),
       .local_out_flit_o	(link24_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link24_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][0][vchannels-1:0]),
       .local_in_ready_o	(link24_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][0][vchannels-1:0]),
       .local_out_ready_i	(link24_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][0][vchannels-1:0]),
       .local_in_flit_i		(link24_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link24_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][1][vchannels-1:0]),
       .local_out_flit_o	(link25_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link25_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][1][vchannels-1:0]),
       .local_in_ready_o	(link25_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][1][vchannels-1:0]),
       .local_out_ready_i	(link25_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][1][vchannels-1:0]),
       .local_in_flit_i		(link25_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link25_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][2][vchannels-1:0]),
       .local_out_flit_o	(link26_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link26_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][2][vchannels-1:0]),
       .local_in_ready_o	(link26_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][2][vchannels-1:0]),
       .local_out_ready_i	(link26_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][2][vchannels-1:0]),
       .local_in_flit_i		(link26_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link26_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][3][vchannels-1:0]),
       .local_out_flit_o	(link27_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link27_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][3][vchannels-1:0]),
       .local_in_ready_o	(link27_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][3][vchannels-1:0]),
       .local_out_ready_i	(link27_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][3][vchannels-1:0]),
       .local_in_flit_i		(link27_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link27_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][4][vchannels-1:0]),
       .local_out_flit_o	(link28_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link28_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][4][vchannels-1:0]),
       .local_in_ready_o	(link28_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][4][vchannels-1:0]),
       .local_out_ready_i	(link28_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][4][vchannels-1:0]),
       .local_in_flit_i		(link28_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link28_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][5][vchannels-1:0]),
       .local_out_flit_o	(link29_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link29_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][5][vchannels-1:0]),
       .local_in_ready_o	(link29_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][5][vchannels-1:0]),
       .local_out_ready_i	(link29_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][5][vchannels-1:0]),
       .local_in_flit_i		(link29_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link29_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][6][vchannels-1:0]),
       .local_out_flit_o	(link30_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link30_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][6][vchannels-1:0]),
       .local_in_ready_o	(link30_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][6][vchannels-1:0]),
       .local_out_ready_i	(link30_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][6][vchannels-1:0]),
       .local_in_flit_i		(link30_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link30_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_3_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[3][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[3][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[3][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[3][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[3][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[3][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[3][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[3][7][vchannels-1:0]),
       .local_out_flit_o	(link31_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link31_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[3][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[3][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[3][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[3][7][vchannels-1:0]),
       .local_in_ready_o	(link31_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[3][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[3][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[3][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[3][7][vchannels-1:0]),
       .local_out_ready_i	(link31_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[3][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[3][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[3][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[3][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[3][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[3][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[3][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[3][7][vchannels-1:0]),
       .local_in_flit_i		(link31_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link31_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][0][vchannels-1:0]),
       .local_out_flit_o	(link32_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link32_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][0][vchannels-1:0]),
       .local_in_ready_o	(link32_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][0][vchannels-1:0]),
       .local_out_ready_i	(link32_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][0][vchannels-1:0]),
       .local_in_flit_i		(link32_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link32_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][1][vchannels-1:0]),
       .local_out_flit_o	(link33_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link33_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][1][vchannels-1:0]),
       .local_in_ready_o	(link33_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][1][vchannels-1:0]),
       .local_out_ready_i	(link33_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][1][vchannels-1:0]),
       .local_in_flit_i		(link33_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link33_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][2][vchannels-1:0]),
       .local_out_flit_o	(link34_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link34_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][2][vchannels-1:0]),
       .local_in_ready_o	(link34_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][2][vchannels-1:0]),
       .local_out_ready_i	(link34_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][2][vchannels-1:0]),
       .local_in_flit_i		(link34_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link34_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][3][vchannels-1:0]),
       .local_out_flit_o	(link35_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link35_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][3][vchannels-1:0]),
       .local_in_ready_o	(link35_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][3][vchannels-1:0]),
       .local_out_ready_i	(link35_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][3][vchannels-1:0]),
       .local_in_flit_i		(link35_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link35_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][4][vchannels-1:0]),
       .local_out_flit_o	(link36_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link36_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][4][vchannels-1:0]),
       .local_in_ready_o	(link36_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][4][vchannels-1:0]),
       .local_out_ready_i	(link36_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][4][vchannels-1:0]),
       .local_in_flit_i		(link36_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link36_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][5][vchannels-1:0]),
       .local_out_flit_o	(link37_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link37_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][5][vchannels-1:0]),
       .local_in_ready_o	(link37_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][5][vchannels-1:0]),
       .local_out_ready_i	(link37_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][5][vchannels-1:0]),
       .local_in_flit_i		(link37_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link37_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][6][vchannels-1:0]),
       .local_out_flit_o	(link38_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link38_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][6][vchannels-1:0]),
       .local_in_ready_o	(link38_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][6][vchannels-1:0]),
       .local_out_ready_i	(link38_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][6][vchannels-1:0]),
       .local_in_flit_i		(link38_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link38_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_4_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[4][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[4][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[4][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[4][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[4][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[4][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[4][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[4][7][vchannels-1:0]),
       .local_out_flit_o	(link39_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link39_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[4][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[4][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[4][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[4][7][vchannels-1:0]),
       .local_in_ready_o	(link39_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[4][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[4][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[4][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[4][7][vchannels-1:0]),
       .local_out_ready_i	(link39_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[4][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[4][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[4][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[4][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[4][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[4][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[4][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[4][7][vchannels-1:0]),
       .local_in_flit_i		(link39_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link39_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][0][vchannels-1:0]),
       .local_out_flit_o	(link40_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link40_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][0][vchannels-1:0]),
       .local_in_ready_o	(link40_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][0][vchannels-1:0]),
       .local_out_ready_i	(link40_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][0][vchannels-1:0]),
       .local_in_flit_i		(link40_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link40_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][1][vchannels-1:0]),
       .local_out_flit_o	(link41_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link41_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][1][vchannels-1:0]),
       .local_in_ready_o	(link41_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][1][vchannels-1:0]),
       .local_out_ready_i	(link41_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][1][vchannels-1:0]),
       .local_in_flit_i		(link41_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link41_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][2][vchannels-1:0]),
       .local_out_flit_o	(link42_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link42_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][2][vchannels-1:0]),
       .local_in_ready_o	(link42_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][2][vchannels-1:0]),
       .local_out_ready_i	(link42_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][2][vchannels-1:0]),
       .local_in_flit_i		(link42_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link42_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][3][vchannels-1:0]),
       .local_out_flit_o	(link43_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link43_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][3][vchannels-1:0]),
       .local_in_ready_o	(link43_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][3][vchannels-1:0]),
       .local_out_ready_i	(link43_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][3][vchannels-1:0]),
       .local_in_flit_i		(link43_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link43_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][4][vchannels-1:0]),
       .local_out_flit_o	(link44_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link44_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][4][vchannels-1:0]),
       .local_in_ready_o	(link44_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][4][vchannels-1:0]),
       .local_out_ready_i	(link44_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][4][vchannels-1:0]),
       .local_in_flit_i		(link44_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link44_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][5][vchannels-1:0]),
       .local_out_flit_o	(link45_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link45_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][5][vchannels-1:0]),
       .local_in_ready_o	(link45_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][5][vchannels-1:0]),
       .local_out_ready_i	(link45_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][5][vchannels-1:0]),
       .local_in_flit_i		(link45_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link45_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][6][vchannels-1:0]),
       .local_out_flit_o	(link46_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link46_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][6][vchannels-1:0]),
       .local_in_ready_o	(link46_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][6][vchannels-1:0]),
       .local_out_ready_i	(link46_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][6][vchannels-1:0]),
       .local_in_flit_i		(link46_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link46_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_5_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[5][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[5][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[5][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[5][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[5][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[5][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[5][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[5][7][vchannels-1:0]),
       .local_out_flit_o	(link47_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link47_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[5][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[5][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[5][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[5][7][vchannels-1:0]),
       .local_in_ready_o	(link47_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[5][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[5][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[5][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[5][7][vchannels-1:0]),
       .local_out_ready_i	(link47_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[5][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[5][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[5][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[5][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[5][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[5][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[5][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[5][7][vchannels-1:0]),
       .local_in_flit_i		(link47_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link47_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][0][vchannels-1:0]),
       .local_out_flit_o	(link48_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link48_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][0][vchannels-1:0]),
       .local_in_ready_o	(link48_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][0][vchannels-1:0]),
       .local_out_ready_i	(link48_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][0][vchannels-1:0]),
       .local_in_flit_i		(link48_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link48_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][1][vchannels-1:0]),
       .local_out_flit_o	(link49_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link49_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][1][vchannels-1:0]),
       .local_in_ready_o	(link49_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][1][vchannels-1:0]),
       .local_out_ready_i	(link49_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][1][vchannels-1:0]),
       .local_in_flit_i		(link49_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link49_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][2][vchannels-1:0]),
       .local_out_flit_o	(link50_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link50_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][2][vchannels-1:0]),
       .local_in_ready_o	(link50_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][2][vchannels-1:0]),
       .local_out_ready_i	(link50_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][2][vchannels-1:0]),
       .local_in_flit_i		(link50_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link50_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][3][vchannels-1:0]),
       .local_out_flit_o	(link51_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link51_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][3][vchannels-1:0]),
       .local_in_ready_o	(link51_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][3][vchannels-1:0]),
       .local_out_ready_i	(link51_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][3][vchannels-1:0]),
       .local_in_flit_i		(link51_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link51_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][4][vchannels-1:0]),
       .local_out_flit_o	(link52_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link52_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][4][vchannels-1:0]),
       .local_in_ready_o	(link52_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][4][vchannels-1:0]),
       .local_out_ready_i	(link52_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][4][vchannels-1:0]),
       .local_in_flit_i		(link52_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link52_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][5][vchannels-1:0]),
       .local_out_flit_o	(link53_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link53_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][5][vchannels-1:0]),
       .local_in_ready_o	(link53_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][5][vchannels-1:0]),
       .local_out_ready_i	(link53_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][5][vchannels-1:0]),
       .local_in_flit_i		(link53_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link53_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][6][vchannels-1:0]),
       .local_out_flit_o	(link54_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link54_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][6][vchannels-1:0]),
       .local_in_ready_o	(link54_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][6][vchannels-1:0]),
       .local_out_ready_i	(link54_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][6][vchannels-1:0]),
       .local_in_flit_i		(link54_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link54_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH,`SELECT_SOUTH}))
   u_router_6_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[6][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[6][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[6][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[6][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[6][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[6][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[6][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[6][7][vchannels-1:0]),
       .local_out_flit_o	(link55_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link55_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[6][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[6][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[6][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[6][7][vchannels-1:0]),
       .local_in_ready_o	(link55_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[6][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[6][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[6][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[6][7][vchannels-1:0]),
       .local_out_ready_i	(link55_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[6][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[6][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[6][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[6][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[6][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[6][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[6][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[6][7][vchannels-1:0]),
       .local_in_flit_i		(link55_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link55_in_valid_i[vchannels-1:0])
    );



   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_0
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][0][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][0][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][0][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][0][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][0][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][0][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][0][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][0][vchannels-1:0]),
       .local_out_flit_o	(link56_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link56_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][0][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][0][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][0][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][0][vchannels-1:0]),
       .local_in_ready_o	(link56_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][0][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][0][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][0][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][0][vchannels-1:0]),
       .local_out_ready_i	(link56_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][0][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][0][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][0][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][0][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][0][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][0][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][0][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][0][vchannels-1:0]),
       .local_in_flit_i		(link56_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link56_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_1
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][1][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][1][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][1][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][1][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][1][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][1][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][1][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][1][vchannels-1:0]),
       .local_out_flit_o	(link57_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link57_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][1][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][1][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][1][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][1][vchannels-1:0]),
       .local_in_ready_o	(link57_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][1][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][1][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][1][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][1][vchannels-1:0]),
       .local_out_ready_i	(link57_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][1][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][1][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][1][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][1][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][1][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][1][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][1][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][1][vchannels-1:0]),
       .local_in_flit_i		(link57_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link57_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_2
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][2][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][2][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][2][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][2][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][2][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][2][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][2][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][2][vchannels-1:0]),
       .local_out_flit_o	(link58_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link58_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][2][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][2][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][2][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][2][vchannels-1:0]),
       .local_in_ready_o	(link58_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][2][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][2][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][2][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][2][vchannels-1:0]),
       .local_out_ready_i	(link58_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][2][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][2][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][2][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][2][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][2][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][2][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][2][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][2][vchannels-1:0]),
       .local_in_flit_i		(link58_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link58_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_3
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][3][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][3][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][3][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][3][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][3][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][3][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][3][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][3][vchannels-1:0]),
       .local_out_flit_o	(link59_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link59_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][3][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][3][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][3][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][3][vchannels-1:0]),
       .local_in_ready_o	(link59_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][3][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][3][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][3][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][3][vchannels-1:0]),
       .local_out_ready_i	(link59_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][3][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][3][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][3][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][3][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][3][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][3][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][3][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][3][vchannels-1:0]),
       .local_in_flit_i		(link59_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link59_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_4
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][4][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][4][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][4][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][4][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][4][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][4][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][4][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][4][vchannels-1:0]),
       .local_out_flit_o	(link60_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link60_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][4][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][4][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][4][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][4][vchannels-1:0]),
       .local_in_ready_o	(link60_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][4][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][4][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][4][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][4][vchannels-1:0]),
       .local_out_ready_i	(link60_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][4][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][4][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][4][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][4][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][4][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][4][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][4][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][4][vchannels-1:0]),
       .local_in_flit_i		(link60_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link60_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST,`SELECT_EAST}))
   u_router_7_5
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][5][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][5][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][5][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][5][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][5][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][5][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][5][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][5][vchannels-1:0]),
       .local_out_flit_o	(link61_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link61_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][5][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][5][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][5][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][5][vchannels-1:0]),
       .local_in_ready_o	(link61_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][5][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][5][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][5][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][5][vchannels-1:0]),
       .local_out_ready_i	(link61_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][5][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][5][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][5][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][5][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][5][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][5][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][5][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][5][vchannels-1:0]),
       .local_in_flit_i		(link61_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link61_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL,`SELECT_EAST}))
   u_router_7_6
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][6][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][6][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][6][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][6][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][6][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][6][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][6][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][6][vchannels-1:0]),
       .local_out_flit_o	(link62_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link62_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][6][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][6][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][6][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][6][vchannels-1:0]),
       .local_in_ready_o	(link62_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][6][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][6][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][6][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][6][vchannels-1:0]),
       .local_out_ready_i	(link62_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][6][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][6][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][6][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][6][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][6][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][6][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][6][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][6][vchannels-1:0]),
       .local_in_flit_i		(link62_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link62_in_valid_i[vchannels-1:0])
    );

   lisnoc_router_2dgrid
   # (.num_dests(64),.vchannels(vchannels),.lookup({`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_NORTH,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_WEST,`SELECT_LOCAL}))
   u_router_7_7
     (
       .clk			(clk),
       .rst			(rst),
       .north_out_flit_o	(north_out_flit[7][7][flit_width-1:0]),
       .north_out_valid_o	(north_out_valid[7][7][vchannels-1:0]),
       .east_out_flit_o		(east_out_flit[7][7][flit_width-1:0]),
       .east_out_valid_o	(east_out_valid[7][7][vchannels-1:0]),
       .south_out_flit_o	(south_out_flit[7][7][flit_width-1:0]),
       .south_out_valid_o	(south_out_valid[7][7][vchannels-1:0]),
       .west_out_flit_o		(west_out_flit[7][7][flit_width-1:0]),
       .west_out_valid_o	(west_out_valid[7][7][vchannels-1:0]),
       .local_out_flit_o	(link63_out_flit_o[flit_width-1:0]),
       .local_out_valid_o	(link63_out_valid_o[vchannels-1:0]),
       .north_in_ready_o	(north_in_ready[7][7][vchannels-1:0]),
       .east_in_ready_o		(east_in_ready[7][7][vchannels-1:0]),
       .south_in_ready_o	(south_in_ready[7][7][vchannels-1:0]),
       .west_in_ready_o		(west_in_ready[7][7][vchannels-1:0]),
       .local_in_ready_o	(link63_in_ready_o[vchannels-1:0]),

       .north_out_ready_i	(north_out_ready[7][7][vchannels-1:0]),
       .east_out_ready_i	(east_out_ready[7][7][vchannels-1:0]),
       .south_out_ready_i	(south_out_ready[7][7][vchannels-1:0]),
       .west_out_ready_i	(west_out_ready[7][7][vchannels-1:0]),
       .local_out_ready_i	(link63_out_ready_i[vchannels-1:0]),
       .north_in_flit_i		(north_in_flit[7][7][flit_width-1:0]),
       .north_in_valid_i	(north_in_valid[7][7][vchannels-1:0]),
       .east_in_flit_i		(east_in_flit[7][7][flit_width-1:0]),
       .east_in_valid_i		(east_in_valid[7][7][vchannels-1:0]),
       .south_in_flit_i		(south_in_flit[7][7][flit_width-1:0]),
       .south_in_valid_i	(south_in_valid[7][7][vchannels-1:0]),
       .west_in_flit_i		(west_in_flit[7][7][flit_width-1:0]),
       .west_in_valid_i		(west_in_valid[7][7][vchannels-1:0]),
       .local_in_flit_i		(link63_in_flit_i[flit_width-1:0]),
       .local_in_valid_i	(link63_in_valid_i[vchannels-1:0])
    );



endmodule

`include "lisnoc_undef.vh"
