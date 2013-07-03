set OPTIMSOC_RTL [get_env OPTIMSOC_RTL]
set LISNOC_RTL [get_env LISNOC_RTL]
set SYNDIR [file dirname [info script]]

# Base OpTiMSoC system
add_file -verilog "$OPTIMSOC_RTL/bootrom/verilog/bootrom.v"
add_file -verilog "$OPTIMSOC_RTL/compute_tile_pgas/verilog/compute_tile_pgas.v"
add_file -verilog "$OPTIMSOC_RTL/compute_tile_pgas/verilog/ct_select.v"
add_file -verilog "$OPTIMSOC_RTL/noclsu/verilog/noclsu_tile.v"
add_file -verilog "$OPTIMSOC_RTL/noclsu/verilog/noclsu_memory.v"
add_file -verilog "$OPTIMSOC_RTL/sram/verilog/wb_sram_sp.v"
add_file -verilog "$OPTIMSOC_RTL/sram/verilog/sram_sp.v"
add_file -verilog "$OPTIMSOC_RTL/sram/verilog/sram_sp_impl_xilinx_spartan6.v"
add_file -verilog "$OPTIMSOC_RTL/sram/verilog/wb2sram.v"
add_file -verilog "$OPTIMSOC_RTL/memory_tile_pgas/verilog/memory_tile_pgas.v"
add_file -verilog "$OPTIMSOC_RTL/networkadapter_ct/verilog/networkadapter_ct.v"
add_file -verilog "$OPTIMSOC_RTL/networkadapter_ct/verilog/networkadapter_conf.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_alu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_amultp2_32x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_cfgr.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_cpu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_ctrl.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dc_fsm.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dc_ram.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dc_tag.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dc_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_defines.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dmmu_tlb.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dmmu_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dpram_32x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dpram_256x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_dpram.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_du.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_except.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_addsub.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_arith.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_div.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_fcmp.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_intfloat_conv.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_mul.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_post_norm_addsub.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_post_norm_div.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_post_norm_intfloat_conv.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_post_norm_mul.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_pre_norm_addsub.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_pre_norm_div.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu_pre_norm_mul.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_fpu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_freeze.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_genpc.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_gmultp2_32x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_ic_fsm.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_ic_ram.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_ic_tag.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_ic_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_if.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_immu_tlb.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_immu_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_iwb_biu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_lsu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_mem2reg.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_mult_mac.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_operandmuxes.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_pic.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_pm.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_qmem_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_reg2mem.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_rf.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_rfram_generic.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_sb_fifo.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_sb.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_32_bw.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_32x24.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_64x14.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_64x22.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_64x24.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_128x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_256x21.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_512x20.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_1024x8.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_1024x32_bw.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_1024x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_2048x8.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_2048x32_bw.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram_2048x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_spram.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_sprs.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_top.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_tpram_32x32.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_tt.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_wb_biu.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_wbmux.v"
add_file -verilog "$OPTIMSOC_RTL/or1200mp/verilog/or1200_xcv_ram32x8d.v"
add_file -verilog "$OPTIMSOC_RTL/or1200_module/verilog/or1200_module.v"
add_file -verilog "$OPTIMSOC_RTL/wb_cas_unit/verilog/wb_cas_fsm.v"
add_file -verilog "$OPTIMSOC_RTL/wb_cas_unit/verilog/wb_cas_unit.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_rqacksync.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_fifo.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_sync_2ff.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_rdptr_empty.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_wrptr_full.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_sync_wr2rd.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_sync_rd2wr.v"
add_file -verilog "$OPTIMSOC_RTL/cdc/verilog/cdc_fifo_mem.v"

# LISNoC
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_initiator.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_initiator_nocreq.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_initiator_nocresp.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_initiator_wbreq.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_request_table.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_target.v"
add_file -verilog "$LISNOC_RTL/dma/lisnoc_dma_wbinterface.v"
add_file -verilog "$LISNOC_RTL/mp_simple/lisnoc_mp_simple.v"
add_file -verilog "$LISNOC_RTL/mp_simple/lisnoc_mp_simple_wb.v"
add_file -verilog "$LISNOC_RTL/infrastructure/lisnoc_packet_buffer.v"
add_file -verilog "$LISNOC_RTL/lisnoc_arb_rr.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_fifo.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_2dgrid.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_arbiter.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_input.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_input_route.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_output.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_output_arbiter.v"

# debug system
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/global_timestamp_provider.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/dbgnoc_conf_if.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/tcm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/ctm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/itm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/itm_dbgnoc_if.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/debug_data_sr.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/itm_trace_collector.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/itm_trace_compression.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/itm_trace_qualificator.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/stm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/stm_dbgnoc_if.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/stm_data_sr.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/stm_trace_collector.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/nrm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/nrm_statistics_collector.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/nrm_link_statistics_collector.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/nrm_dbgnoc_if.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/ncm.v"
add_file -verilog "$OPTIMSOC_RTL/debug_system/verilog/debug_system.v"

# LISNoC parts required by the debug system
add_file -verilog "$LISNOC_RTL/rings/lisnoc_uni_ring.v"
add_file -verilog "$LISNOC_RTL/router/lisnoc_router_uni_ring.v"
add_file -verilog "$LISNOC_RTL/lisnoc16/lisnoc16_fifo.v"
add_file -verilog "$LISNOC_RTL/infrastructure/lisnoc_vc_multiplexer.v"
add_file -verilog "$LISNOC_RTL/lisnoc16/converter/lisnoc16_converter_16to32.v"
add_file -verilog "$LISNOC_RTL/lisnoc16/converter/lisnoc16_converter_32to16.v"

# System
add_file -ucf "$SYNDIR/system_irobot_pgas_ztex.ucf"
add_file -constraint "$SYNDIR/system_irobot_pgas_ztex.sdc"

add_file -verilog "$SYNDIR/system_irobot_pgas_ztex.v"
add_file -verilog "$OPTIMSOC_RTL/system_irobot_pgas/verilog/system_irobot_pgas.v"

add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_tile.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/noc2char.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/char2uart.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/char2noc.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_top.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_wb.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_regs.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_debug_if.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_transmitter.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_sync_flops.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_receiver.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_rfifo.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_tfifo.v"
add_file -verilog "$OPTIMSOC_RTL/uart_tile/verilog/uart_raminfr.v"

add_file -verilog "$OPTIMSOC_RTL/soccerboard_tile/verilog/soccerboard_tile.v"

add_file -verilog "$OPTIMSOC_RTL/clockmanager/verilog/clockmanager_ztex115.v"
add_file -verilog "$OPTIMSOC_RTL/clockmanager/verilog/clk_opt_ctrl.v"
add_file -verilog "$OPTIMSOC_RTL/clockmanager/verilog/dcm_clkgen_ctrl.v"

add_file -verilog "$LISNOC_RTL/lisnoc16/usb/lisnoc16_usb_packet_buffer.v"
add_file -verilog "$OPTIMSOC_RTL/usb_dbg_if/verilog/usb_dbg_if.v"
add_file -verilog "$OPTIMSOC_RTL/usb_dbg_if/verilog/fx2_usb_comm.v"
add_file -xilinx "$SYNDIR/fifo_usb_dual.ngc"
add_file -verilog "$SYNDIR/fifo_usb_dual.v"
add_file -xilinx "$SYNDIR/fifo_usb_to_noc.ngc"
add_file -verilog "$SYNDIR/fifo_usb_to_noc.v"
add_file -verilog "$LISNOC_RTL/lisnoc16/usb/lisnoc16_usb_to_noc.v"
add_file -verilog "$LISNOC_RTL/lisnoc16/usb/lisnoc16_usb_from_noc.v"


################################################################################
# implementation: "rev_1_15b_bram"
################################################################################

impl -add rev_1_15b_bram -type fpga

# implementation attributes
set_option -vlog_std sysv
set_option -project_relative_includes 1
set_option -include_path "$OPTIMSOC_RTL/;$OPTIMSOC_RTL/uart_tile/verilog/;$OPTIMSOC_RTL/or1200mp/verilog/;$OPTIMSOC_RTL/debug_system/verilog/;$LISNOC_RTL/;$LISNOC_RTL/dma/;$LISNOC_RTL/lisnoc16/;$LISNOC_RTL/lisnoc16/usb/;$LISNOC_RTL/lisnoc16/converter/;$SYNDIR;$OPTIMSOC_RTL/uart_tile/verilog;"

# if you set OPTIMSOC_USE_DDR2 use the corresponding UCF file above
set_option -hdl_define -set "OPTIMSOC_MT_PLAIN OPTIMSOC_CTRAM_PLAINBETTER OR1200_BOOT_ADR=32'h00000100"

# pr_1 attributes
set_option -job pr_1 -add par
set_option -job pr_1 -option enable_run 1
set_option -job pr_1 -option run_backannotation 0

# device options
set_option -technology Spartan6
set_option -part XC6SLX75
set_option -package CSG484
set_option -speed_grade -3
set_option -part_companion ""

# compilation/mapping options
set_option -use_fsm_explorer 0
set_option -top_module "system_irobot_pgas_ztex"

# mapper_options
set_option -frequency auto
set_option -write_verilog 0
set_option -write_vhdl 0

# xilinx_options
set_option -enhanced_optimization 0

# Xilinx Spartan3
set_option -run_prop_extract 1
set_option -maxfan 10000

# I/O insertation must be disabled for DDR2 memory
# set OPTIMSOC_MANUAL_IOBUF above if you do this!
#set_option -disable_io_insertion 1
set_option -disable_io_insertion 0

set_option -pipe 1
set_option -retiming 0
set_option -update_models_cp 0
set_option -fixgatedclocks 3
set_option -fixgeneratedclocks 3
set_option -no_sequential_opt 1
set_option -resolve_multiple_driver 1

# Xilinx Spartan6
set_option -enable_prepacking 1

# NFilter
set_option -popfeed 0
set_option -constprop 0
set_option -createhierarchy 0

# Xilinx
set_option -fc_phys_opt 0

# sequential_optimization_options
set_option -symbolic_fsm_compiler 1

# Compiler Options
set_option -compiler_compatible 0
set_option -resource_sharing 1
set_option -multi_file_compilation_unit 1
set_option -ucf "$SYNDIR/system_irobot_pgas_ztex.ucf"

#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#set result format/file last
project -result_file "./rev_1_15b_bram/system_irobot_pgas_ztex115d_bram.edf"


################################################################################
# implementation: "rev_1_15d_bram"
################################################################################

impl -add rev_1_15d_bram -type fpga

# implementation attributes
set_option -vlog_std sysv
set_option -project_relative_includes 1
set_option -include_path "$OPTIMSOC_RTL/;$OPTIMSOC_RTL/uart_tile/verilog/;$OPTIMSOC_RTL/or1200mp/verilog/;$OPTIMSOC_RTL/debug_system/verilog/;$LISNOC_RTL/;$LISNOC_RTL/dma/;$LISNOC_RTL/lisnoc16/;$LISNOC_RTL/lisnoc16/usb/;$LISNOC_RTL/lisnoc16/converter/;$SYNDIR;$OPTIMSOC_RTL/uart_tile/verilog"

set_option -hdl_define -set "OPTIMSOC_MT_PLAIN OPTIMSOC_CTRAM_PLAINBETTER OR1200_BOOT_ADR=32'h00000100"

# pr_1 attributes
set_option -job pr_1 -add par
set_option -job pr_1 -option enable_run 1
set_option -job pr_1 -option run_backannotation 0

# device options
set_option -technology Spartan6
set_option -part XC6SLX150
set_option -package CSG484
set_option -speed_grade -3
set_option -part_companion ""

# compilation/mapping options
set_option -use_fsm_explorer 0
set_option -top_module "system_irobot_pgas_ztex"

# mapper_options
set_option -frequency auto
set_option -write_verilog 0
set_option -write_vhdl 0

# xilinx_options
set_option -enhanced_optimization 0

# Xilinx Spartan3
set_option -run_prop_extract 1
set_option -maxfan 10000

set_option -disable_io_insertion 0

set_option -pipe 1
set_option -retiming 0
set_option -update_models_cp 0
set_option -fixgatedclocks 3
set_option -fixgeneratedclocks 3
set_option -no_sequential_opt 1
set_option -resolve_multiple_driver 1

# Xilinx Spartan6
set_option -enable_prepacking 1

# NFilter
set_option -popfeed 0
set_option -constprop 0
set_option -createhierarchy 0

# Xilinx
set_option -fc_phys_opt 0

# sequential_optimization_options
set_option -symbolic_fsm_compiler 1

# Compiler Options
set_option -compiler_compatible 0
set_option -resource_sharing 1
set_option -multi_file_compilation_unit 1
set_option -ucf "$SYNDIR/system_irobot_pgas_ztex.ucf"

#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#set result format/file last
project -result_file "./rev_1_15d_bram/system_irobot_pgas_ztex115d_bram.edf"

#design plan options
