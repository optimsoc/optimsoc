`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 24.06.2016 12:51:41
// Design Name: 
// Module Name: camera_tile
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module camera_tile(
    // Inputs
    clk, rst, noc_in_flit, noc_in_valid, noc_out_ready,
    // External Inputs
    PCLK, VSYNC, HREF, D,    
    // Outputs
    noc_in_ready, noc_out_flit, noc_out_valid,
    // External Outputs
    SIOC, SIOD, RESET, PWDN, XVCLK
    
    );

	parameter ID = 3;
    
    parameter noc_data_width = 32;
    parameter noc_type_width = 2;
    parameter noc_flit_width = noc_data_width + noc_type_width;
    
    parameter vchannels = 3;
    
    // WISHBONE bus width
    parameter wb_width = 32;
    // camera data bus width
    parameter d_width = 8;
    
    parameter NR_MASTERS = 2;
    parameter NR_SLAVES = 1;
    
    input clk, rst;
    input [noc_flit_width-1:0]  noc_in_flit;
    input [vchannels-1:0]       noc_in_valid;
    input [vchannels-1:0]       noc_out_ready;
    
    input PCLK, VSYNC, HREF;
    input [d_width-1:0] D; 
    
    output [vchannels-1:0]      noc_in_ready;
    output [noc_flit_width-1:0] noc_out_flit;
    output [vchannels-1:0]      noc_out_valid;
    
    output SIOC;
	output RESET;
	output PWDN; 
	output XVCLK;
    inout SIOD;
  
    wire [31:0]   busms_adr_o[0:NR_MASTERS-1];
    wire          busms_cyc_o[0:NR_MASTERS-1];
    wire [31:0]   busms_dat_o[0:NR_MASTERS-1];
    wire [3:0]    busms_sel_o[0:NR_MASTERS-1];
    wire          busms_stb_o[0:NR_MASTERS-1];
    wire          busms_we_o[0:NR_MASTERS-1];
    wire          busms_cab_o[0:NR_MASTERS-1];
    //wire [2:0]    busms_cti_o[0:NR_MASTERS-1];
    //wire [1:0]    busms_bte_o[0:NR_MASTERS-1];
    wire          busms_ack_i[0:NR_MASTERS-1];
    wire          busms_rty_i[0:NR_MASTERS-1];
    wire          busms_err_i[0:NR_MASTERS-1];
    wire [31:0]   busms_dat_i[0:NR_MASTERS-1];
    
    wire [wb_width-1:0] bussl_dat_o[0:NR_SLAVES-1];
    wire bussl_ack_o[0:NR_SLAVES-1];
    wire bussl_rty_o[0:NR_SLAVES-1];
    wire bussl_err_o[0:NR_SLAVES-1]; 
       
    wire [wb_width-1:0] bussl_adr_i[0:NR_SLAVES-1];
    wire bussl_cyc_i[0:NR_SLAVES-1];
    wire [wb_width-1:0] bussl_dat_i[0:NR_SLAVES-1];
    wire [3:0] bussl_sel_i[0:NR_SLAVES-1];
    wire bussl_stb_i[0:NR_SLAVES-1];
    wire bussl_we_i[0:NR_SLAVES-1]; 
    
    wire [wb_width*NR_MASTERS-1:0] busms_adr_o_flat;
    wire [wb_width*NR_MASTERS-1:0] busms_dat_o_flat;
    wire [NR_MASTERS-1:0] busms_cyc_o_flat;
    wire [NR_MASTERS-1:0] busms_stb_o_flat;
    wire [4*NR_MASTERS-1:0]  busms_sel_o_flat;
    wire [NR_MASTERS-1:0] busms_we_o_flat;
       
    wire [wb_width*NR_SLAVES-1:0]   bussl_dat_o_flat;
    wire [wb_width*NR_SLAVES-1:0]   bussl_ack_o_flat;
    wire [wb_width*NR_SLAVES-1:0]   bussl_rty_o_flat;
    wire [wb_width*NR_SLAVES-1:0]   bussl_err_o_flat;
        
    wire [NR_MASTERS-1:0]    busms_ack_i_flat;
    wire [NR_MASTERS-1:0]    busms_rty_i_flat;
    wire [NR_MASTERS-1:0]    busms_err_i_flat;
    wire [wb_width*NR_MASTERS-1:0] busms_dat_i_flat;
    
    wire [wb_width*NR_SLAVES-1:0] bussl_adr_i_flat;
    wire [0:NR_SLAVES-1] bussl_cyc_i_flat;
    wire [wb_width*NR_SLAVES-1:0] bussl_dat_i_flat;
    wire [4*NR_SLAVES-1:0] bussl_sel_i_flat;
    wire [0:NR_SLAVES-1] bussl_stb_i_flat;
    wire [0:NR_SLAVES-1] bussl_we_i_flat;
    
    generate
        genvar m;
        for (m = 0; m < NR_MASTERS; m = m + 1) begin : gen_busms_flat
            assign busms_adr_o_flat[32*(m+1)-1:32*m] = busms_adr_o[m];
            assign busms_cyc_o_flat[m] = busms_cyc_o[m];
            assign busms_dat_o_flat[32*(m+1)-1:32*m] = busms_dat_o[m];
            assign busms_sel_o_flat[4*(m+1)-1:4*m] = busms_sel_o[m];
            assign busms_stb_o_flat[m] = busms_stb_o[m];
            assign busms_we_o_flat[m] = busms_we_o[m];
            //assign busms_cab_o_flat[m] = busms_cab_o[m];      
            //assign busms_cti_o_flat[3*(m+1)-1:3*m] = busms_cti_o[m];
            //assign busms_bte_o_flat[2*(m+1)-1:2*m] = busms_bte_o[m];
            assign busms_ack_i[m] = busms_ack_i_flat[m];
            assign busms_rty_i[m] = busms_rty_i_flat[m];
            assign busms_err_i[m] = busms_err_i_flat[m];
            assign busms_dat_i[m] = busms_dat_i_flat[32*(m+1)-1:32*m];
        end
    endgenerate
    
    wire w_irq_new_frame;
    wire w_config_finished;
    wire [1:0] w_out_first_word;
           
    camera_module inst_camera_module (
        // Inputs
        .clk(clk),
        .rst(rst),
//        .wb_cyc_i(bussl_cyc_i[0]),
//        .wb_dat_i(bussl_dat_i[0][wb_width-1:0]),
//        .wb_stb_i(bussl_stb_i[0]),
//        .wb_sel_i(bussl_sel_i[0][3:0]),
//        .wb_adr_i(bussl_adr_i[0][wb_width-1:0]),
//        .wb_we_i(bussl_we_i[0]),

        .wb_cyc_i(bussl_cyc_i_flat),
        .wb_dat_i(bussl_dat_i_flat),
        .wb_stb_i(bussl_stb_i_flat),
        .wb_sel_i(bussl_sel_i_flat),
        .wb_adr_i(bussl_adr_i_flat),
        .wb_we_i(bussl_we_i_flat),
        
        .PCLK(PCLK),
        .VSYNC(VSYNC),
        .HREF(HREF),
        .PWDN(PWDN),
        .D(D),
        // Outputs
        .wb_dat_o(bussl_dat_o[0][wb_width-1:0]),
        .wb_ack_o(bussl_ack_o[0]),
        .wb_err_o(bussl_err_o[0]),
        .wb_rty_o(bussl_rty_o[0]),
        .irq_new_frame(w_irq_new_frame),
        .out_first_word(w_out_first_word),
        .config_finished(w_config_finished),
        .SIOC(SIOC),
        .SIOD(SIOD),
        .RESET(RESET),
        .XVCLK(XVCLK)
        );
        
    wb_bus_b3
        #(.MASTERS(NR_MASTERS),.SLAVES(NR_SLAVES),
           .S0_RANGE_WIDTH(4),.S0_RANGE_MATCH(4'hd))
        inst_wb_bus (
        // Inputs
        .clk_i(clk),
        // NOTE: in compute_tile_dm.v rst_sys is used instead of rst
        .rst_i(rst),
        .m_adr_i(busms_adr_o_flat),
        .m_dat_i(busms_dat_o_flat),
        .m_cyc_i(busms_cyc_o_flat),
        .m_stb_i(busms_stb_o_flat),
        .m_sel_i(busms_sel_o_flat),
        .m_we_i(busms_we_o_flat),
        .m_cti_i(),
        .m_bte_i(),
        .s_dat_i(bussl_dat_o[0]),
        .s_ack_i(bussl_ack_o[0]),
        .s_err_i(bussl_err_o[0]),
        .s_rty_i(bussl_rty_o[0]),

//        .s_dat_i(bussl_dat_o_flat),
//        .s_ack_i(bussl_ack_o_flat),
//        .s_err_i(bussl_err_o_flat),
//        .s_rty_i(bussl_rty_o_flat),

        .bus_hold(1'b0),
        // Outputs
        .m_dat_o(busms_dat_i_flat),
        .m_ack_o(busms_ack_i_flat),
        .m_err_o(busms_err_i_flat),
        .m_rty_o(busms_rty_i_flat),
//        .s_adr_o({bussl_adr_i[1], bussl_adr_i[0]}),
//        .s_dat_o({bussl_dat_i[1], bussl_dat_i[0]}),
//        .s_cyc_o({bussl_cyc_i[1], bussl_cyc_i[0]}),
//        .s_stb_o({bussl_stb_i[1], bussl_stb_i[0]}),
//        .s_sel_o({bussl_sel_i[1], bussl_sel_i[0]}),
//        .s_we_o({bussl_we_i[1], bussl_we_i[0]}),

        .s_adr_o(bussl_adr_i_flat),
        .s_dat_o(bussl_dat_i_flat),
        .s_cyc_o(bussl_cyc_i_flat),
        .s_stb_o(bussl_stb_i_flat),
        .s_sel_o(bussl_sel_i_flat),
        .s_we_o(bussl_we_i_flat),
        
        .s_cti_o(),
        .s_bte_o(),
        .snoop_adr_o(),
        .snoop_en_o(),
        .bus_hold_ack()
        );
    
    parameter vc_dma_req = 0;
    parameter vc_dma_resp = 1;
    parameter vc_config_mp = 2;      
        
    wire [vchannels-1:0] mod_out_ready;
    wire [vchannels-1:0] mod_out_valid;
    wire [noc_flit_width-1:0] mod_out_flit[0:vchannels-1];
    wire [vchannels-1:0] mod_in_ready;
    wire [vchannels-1:0] mod_in_valid;
    wire [noc_flit_width-1:0] mod_in_flit[0:vchannels-1];
    //wire [noc_flit_width*vchannels-1:0] mod_out_flit_flat;
     
       
    assign mod_in_valid = noc_in_valid;
    // DMA requests cannot be sent by camera tile
    assign mod_out_valid[vc_dma_req] = 1'b0;
    assign mod_out_flit[vc_dma_req] = 34'b0;
    // DMA responses cannot be received by camera tile
    assign mod_in_ready[vc_dma_resp] = 1'b0;
    assign noc_in_ready = mod_in_ready;
        
    generate
        genvar v;
        for (v=0;v<vchannels;v=v+1) begin
            assign mod_in_flit[v] = noc_in_flit;
            //assign mod_out_flit_flat[(v+1)*noc_flit_width-1:v*noc_flit_width] = mod_out_flit[v];
        end
    endgenerate        
        


            
    mp_to_wb_bridge 
	#(.TILEID(ID))
	inst_bridge (
        // Inputs
        .clk(clk),
        .rst(rst),
        .noc_out_ready(mod_out_ready[vc_config_mp]),
        .noc_in_flit(mod_in_flit[vc_config_mp]),
        .noc_in_valid(mod_in_valid[vc_config_mp]),
        .wb_dat_i(busms_dat_i[NR_MASTERS-1]),
        .wb_ack_i(busms_ack_i[NR_MASTERS-1]),
        .wb_err_i(busms_err_i[NR_MASTERS-1]),
        .wb_rty_i(busms_rty_i[NR_MASTERS-1]),
        // Outputs
        .wb_dat_o(busms_dat_o[NR_MASTERS-1]),
        .wb_we_o(busms_we_o[NR_MASTERS-1]),
        .wb_stb_o(busms_stb_o[NR_MASTERS-1]),
        .wb_adr_o(busms_adr_o[NR_MASTERS-1]),
        .wb_cyc_o(busms_cyc_o[NR_MASTERS-1]),
        .wb_sel_o(busms_sel_o[NR_MASTERS-1]),
        .noc_out_flit(mod_out_flit[vc_config_mp]),
        .noc_out_valid(mod_out_valid[vc_config_mp]),
        .noc_in_ready(mod_in_ready[vc_config_mp])
        );
    
        
    lisnoc_dma_target_cam 
	#(.tileid(ID))
	inst_dma_target (
        // Inputs
        .clk(clk),
        .rst(rst),
        .noc_out_ready(mod_out_ready[vc_dma_resp]),
        .noc_in_flit(mod_in_flit[vc_dma_req]),
        .noc_in_valid(mod_in_valid[vc_dma_req]),
        .wb_ack_i(busms_ack_i[0]),
        .wb_dat_i(busms_dat_i[0][wb_width-1:0]),
        .irq_new_frame(w_irq_new_frame),
        // Outputs
        .noc_out_flit(mod_out_flit[vc_dma_resp]),
        .noc_out_valid(mod_out_valid[vc_dma_resp]),
        .noc_in_ready(mod_in_ready[vc_dma_req]),
        .wb_cyc_o(busms_cyc_o[0]),
        .wb_stb_o(busms_stb_o[0]),
        .wb_we_o(busms_we_o[0]),
        .wb_dat_o(busms_dat_o[0][wb_width-1:0]),
        .wb_adr_o(busms_adr_o[0][wb_width-1:0]),
        .wb_sel_o(busms_sel_o[0][3:0]),
        .wb_cti_o(),
        .wb_bte_o(),
        .in_first_word(w_out_first_word)
        );
        
            
    lisnoc_router_output_arbiter 
        #(.vchannels(vchannels))
        inst_output_arbiter (
        // Inputs
        .clk(clk),
        .rst(rst),
        // vc_config_mp has highest priority -> is served first whenever there is config data
        .fifo_valid_i({(mod_out_valid[vc_dma_resp] && w_config_finished), mod_out_valid[vc_dma_req], mod_out_valid[vc_config_mp]}),
        .fifo_flit_i({mod_out_flit[vc_dma_resp], mod_out_flit[vc_dma_req], mod_out_flit[vc_config_mp]}),  
        .link_ready_i({noc_out_ready[vc_dma_resp], noc_out_ready[vc_dma_req], noc_out_ready[vc_config_mp]}),
        // Outputs
        .fifo_ready_o({mod_out_ready[vc_dma_resp], mod_out_ready[vc_dma_req], mod_out_ready[vc_config_mp]}),
        .link_valid_o({noc_out_valid[vc_dma_resp], noc_out_valid[vc_dma_req], noc_out_valid[vc_config_mp]}),
        .link_flit_o(noc_out_flit[noc_flit_width-1:0])
        
        // vc_config_mp has highest priority -> is served first whenever there is config data
//            .fifo_valid_i({mod_out_valid[vc_config_mp], mod_out_valid[vc_dma_req], (mod_out_valid[vc_dma_resp] && w_config_finished)}),
//            .fifo_flit_i({mod_out_flit[vc_config_mp], mod_out_flit[vc_dma_req],  mod_out_flit[vc_dma_resp]}),  
//            .link_ready_i({noc_out_ready[vc_config_mp], noc_out_ready[vc_dma_req], noc_out_ready[vc_dma_resp]}),
//            // Outputs
//            .fifo_ready_o({mod_out_ready[vc_config_mp], mod_out_ready[vc_dma_req], mod_out_ready[vc_dma_resp]}),
//            .link_valid_o({noc_out_valid[vc_config_mp], noc_out_valid[vc_dma_req], noc_out_valid[vc_dma_resp]}),
//            .link_flit_o(noc_out_flit[noc_flit_width-1:0])
        );         
       
endmodule
