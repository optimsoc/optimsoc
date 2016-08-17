//MAM with Wishbone interface
//Parameters as in MAM

import dii_package::dii_flit;
module mam_wb
    #(parameter                 DATA_WIDTH  = 16, // in bits, must be multiple of 16
    parameter                   ADDR_WIDTH  = 32,
    parameter MAX_PKT_LEN = 'x,
    parameter REGIONS     = 1,
    parameter MEM_SIZE0   = 'x,
    parameter BASE_ADDR0  = 'x,
    parameter MEM_SIZE1   = 'x,
    parameter BASE_ADDR1  = 'x,
    parameter MEM_SIZE2   = 'x,
    parameter BASE_ADDR2  = 'x,
    parameter MEM_SIZE3   = 'x,
    parameter BASE_ADDR3  = 'x,
    parameter MEM_SIZE4   = 'x,
    parameter BASE_ADDR4  = 'x,
    parameter MEM_SIZE5   = 'x,
    parameter BASE_ADDR5  = 'x,
    parameter MEM_SIZE6   = 'x,
    parameter BASE_ADDR6  = 'x,
    parameter MEM_SIZE7   = 'x,
    parameter BASE_ADDR7  = 'x)
    (input                      clk_i, rst_i,

    input                         dii_flit debug_in, output debug_in_ready,
    output                        dii_flit debug_out, input debug_out_ready,

    input [9:0]                   id,

    output reg                  stb_o,
    output reg                  cyc_o,
    input                       ack_i,
    output reg                  we_o,
    output reg [ADDR_WIDTH-1:0] addr_o,
    output reg [DATA_WIDTH-1:0] dat_o,
    input [DATA_WIDTH-1:0]      dat_i,
    output reg [2:0]            cti_o,
    output reg [1:0]            bte_o,
    output reg [SW-1:0]         sel_o
    );

    //Byte select width
    localparam SW = (DATA_WIDTH == 32) ? 4 :
                   (DATA_WIDTH == 16) ? 2 :
                   (DATA_WIDTH ==  8) ? 1 : 'hx;

    logic                       req_valid;
    logic                       req_ready;
    logic                       req_rw;
    logic [ADDR_WIDTH-1:0]      req_addr;
    logic                       req_burst;
    logic [13:0]                req_beats;

    logic                       write_valid;
    logic [DATA_WIDTH-1:0]      write_data;
    logic [DATA_WIDTH/8-1:0]    write_strb;
    logic                       write_ready;

    logic                      read_valid;
    logic     [DATA_WIDTH-1:0] read_data;
    logic                      read_ready;

    osd_mam #(
		.DATA_WIDTH(DATA_WIDTH),
		.ADDR_WIDTH(ADDR_WIDTH),
		.MAX_PKT_LEN(MAX_PKT_LEN),
		.BASE_ADDR0(BASE_ADDR0),
		.MEM_SIZE0(MEM_SIZE0))
	u_mam(.clk(clk_i), .rst(rst_i), .*);

	mam_wb_if #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH))
    u_mam_wb_if(.*);

endmodule

