`include "uart_defines.v"

/*
the sequence of cmds are: 
 write process:write write reg opcode->write sensor addr->write sensor reg position->write num of transfered data->write start command  (wait for 5ms and then repeat writing for 8 sensors)->wait for finish of ranging
 read process:->write read reg opcode->write sensor addr->write sensor reg position->write read data num->wait for the coming of data (repeat reading process for 8 sensors)
   
 ***note:the frequency of system determines the waiting time, so be careful when changing system freq!**
 */


module soccerboard_tile(/*AUTOARG*/
   // Outputs
   soccertx_pad_o, noc_in_ready, noc_out_flit, noc_out_valid,
   // Inputs
   clk, clk_uart, rst, soccerrx_pad_i, noc_in_flit, noc_in_valid,
   noc_out_ready
   );
   
   parameter VCHANNELS = 'bx;
   parameter USE_VCHANNEL = 'bx;
   parameter ID = 'bx;
   
   parameter ph_dest_width = 5;
   parameter ph_cls_width = 3;
   parameter ph_src_width = 5;
   
   parameter destination = 0;
   parameter pkt_class = 1;
   
   parameter noc_data_width = 32;
   parameter noc_type_width = 2;
   parameter UART_BAUD_RATE = 250000;
   localparam NOC_FLIT_WIDTH = noc_data_width+noc_type_width;
   localparam uart_data_width = `UART_DATA_WIDTH;
   localparam uart_addr_width = `UART_ADDR_WIDTH;

   wire [NOC_FLIT_WIDTH-1:0]  soccer_out_flit;
   wire [VCHANNELS-1:0]       soccer_out_valid;
   wire [VCHANNELS-1:0]       soccer_in_ready;
   wire [NOC_FLIT_WIDTH-1:0]  soccer_in_flit;
   wire [VCHANNELS-1:0]       soccer_in_valid;
   wire [VCHANNELS-1:0]       soccer_out_ready;
   
   input clk, clk_uart;
   input rst;

   output soccertx_pad_o;
   input  soccerrx_pad_i;
    
   input      [NOC_FLIT_WIDTH-1:0]  noc_in_flit;
   input      [VCHANNELS-1:0]       noc_in_valid;
   output     [VCHANNELS-1:0]       noc_in_ready;
   output     [NOC_FLIT_WIDTH-1:0]  noc_out_flit;
   output     [VCHANNELS-1:0]       noc_out_valid;
   input      [VCHANNELS-1:0]       noc_out_ready;
 
   reg                              enable_sensor;
   reg                              nxt_enable_sensor;
   reg                              enable_servo;
   reg                              nxt_enable_servo;
   reg [15:0]                       servo_angle;
   reg [15:0]                       nxt_servo_angle;
   
   reg [4:0]                        state;
   reg [4:0]                        nxt_state;  

   reg                              valid;
   reg [7:0]                        cmd;

   reg [31:0]                        counter;
   reg [31:0]                        nxt_counter;   
   reg [7:0]                        sensor_addr;
   reg [7:0]                        nxt_sensor_addr;
 
   reg [31:0]                           counter_i2c;
   reg [31:0]                           nxt_counter_i2c;
 
   localparam STATE_IDLE = 0;
   localparam STATE_I2CWRREG = 1;
   localparam STATE_I2CWRADDR = 2;
   localparam STATE_I2CWRREGPOS = 3;
   localparam STATE_I2CWRNUM = 4;
   localparam STATE_I2CWRCMD = 5;
   localparam STATE_I2CWRSW = 6;  
   localparam STATE_I2CWAIT=7;
   localparam STATE_I2CRDREG = 8;
   localparam STATE_I2CRDADDR = 9;
   localparam STATE_I2CRDREGPOS =10;
   localparam STATE_I2CRDNUM = 11;
   localparam STATE_I2CEND = 12;
   localparam STATE_SERVOWRREG_L = 13;
   localparam STATE_SERVOWRADDR_L = 14;
   localparam STATE_SERVOWRREGPOS_L = 15;
   localparam STATE_SERVOWRNUM_L = 16;
   localparam STATE_SERVOWRCMD_L = 17;
   localparam STATE_SERVOWAIT_L = 18;
   localparam STATE_SERVOWRREG = 19;
   localparam STATE_SERVOWRADDR = 20;
   localparam STATE_SERVOWRREGPOS = 21;
   localparam STATE_SERVOWRNUM = 22;
   localparam STATE_SERVOWRCMD = 23;
   localparam STATE_SERVOWAIT = 24;   
    
   assign soccer_in_valid = {{VCHANNELS-1{1'b0}},valid};
   assign soccer_in_flit =  {2'b11,24'h0,cmd};
   assign soccer_out_ready = noc_out_ready;
   assign noc_out_flit =    {soccer_out_flit[33:16],sensor_addr,soccer_out_flit[7:0]};
   assign noc_out_valid =   {VCHANNELS{enable_sensor|enable_servo}} & soccer_out_valid;
   assign noc_in_ready =    soccer_in_ready;

   
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         enable_sensor <= 0;
         enable_servo<=0;
         servo_angle<=16'd1500;
         counter <= 0;
         sensor_addr<=8'he0;
         counter_i2c<=0;
      end else begin
         state <= nxt_state;
         enable_sensor <= nxt_enable_sensor;
         enable_servo<=nxt_enable_servo;
         servo_angle<=nxt_servo_angle;
         counter <= nxt_counter;
         counter_i2c<=nxt_counter_i2c;
         sensor_addr<=nxt_sensor_addr;
      end // else: !if(rst)
   end // always @ (posedge clk)
 
  always @(*) begin
      nxt_enable_sensor = enable_sensor;
      nxt_enable_servo = enable_servo;
      nxt_servo_angle=servo_angle;
      if (noc_in_valid) begin
         nxt_enable_sensor=noc_in_flit[0];
         nxt_enable_servo=noc_in_flit[1];
         nxt_servo_angle=noc_in_flit[17:2];
      end
   end
   
   always @(*) begin
      valid =  1'b0;
      cmd = 8'hx;
      nxt_state= state;
      nxt_counter = counter;
      nxt_sensor_addr=sensor_addr;
      nxt_counter_i2c=counter_i2c;
      
      case(state)
        STATE_IDLE: begin
           if (enable_sensor) 
             begin
             nxt_state = STATE_I2CWRREG;
             end 
           else if(enable_servo)
             begin
                nxt_state = STATE_SERVOWRREG_L;
             end
           else begin
             nxt_state = STATE_IDLE;
           end
        end

        STATE_SERVOWRREG: begin
           valid = 1'b1;
           cmd=8'h1B;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRADDR;
           end else begin
              nxt_state = STATE_SERVOWRREG;
           end
        end

        STATE_SERVOWRREG_L: begin
           valid = 1'b1;
           cmd=8'h1B;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRADDR_L;
           end else begin
              nxt_state = STATE_SERVOWRREG_L;
           end
        end

        STATE_SERVOWRADDR: begin
           valid = 1'b1;
           cmd=8'hc2;//address of sensor
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRREGPOS;
           end else begin
              nxt_state = STATE_SERVOWRADDR;
           end
        end
        STATE_SERVOWRADDR_L: begin
           valid = 1'b1;
           cmd=8'hc2;//address of sensor
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRREGPOS_L;
           end else begin
              nxt_state = STATE_SERVOWRADDR_L;
           end
        end

        STATE_SERVOWRREGPOS: begin
           valid = 1'b1;
           cmd=8'h3e;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRNUM;
           end else begin
              nxt_state = STATE_SERVOWRREGPOS;
           end
        end

        STATE_SERVOWRREGPOS_L: begin
           valid = 1'b1;
           cmd=8'h3d;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRNUM_L;
           end else begin
              nxt_state = STATE_SERVOWRREGPOS_L;
           end
        end

        STATE_SERVOWRNUM: begin
           valid = 1'b1;
           cmd=8'h01;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRCMD;
           end else begin
              nxt_state = STATE_SERVOWRNUM;
           end
        end

        STATE_SERVOWRNUM_L: begin
           valid = 1'b1;
           cmd=8'h01;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWRCMD_L;
           end else begin
              nxt_state = STATE_SERVOWRNUM_L;
           end
        end

        STATE_SERVOWRCMD: begin
           valid = 1'b1;
           cmd=servo_angle[15:8];
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWAIT;
           end else begin
              nxt_state = STATE_SERVOWRCMD;
           end
        end

        STATE_SERVOWRCMD_L: begin
           valid = 1'b1;
           cmd=servo_angle[7:0];
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_SERVOWAIT_L;
           end else begin
              nxt_state = STATE_SERVOWRCMD_L;
           end
        end

        STATE_SERVOWAIT:begin
           if(counter_i2c>=240000) begin//wait for 5ms so that the qfix can finish I2C protocol
              nxt_counter_i2c=0;
              nxt_state=STATE_IDLE;
              //angle=0;
           end // if (counter_i2c>=480000)
           else begin
              nxt_counter_i2c=counter_i2c+1;
              nxt_state=STATE_SERVOWAIT;
           end // else: !if(counter_i2c>=480000)
        end

        STATE_SERVOWAIT_L:begin
           if(counter_i2c>=240000) begin//wait for 5ms so that the qfix can finish I2C protocol
              nxt_counter_i2c=0;
              nxt_state=STATE_SERVOWRREG;
              //angle=0;
           end // if (counter_i2c>=480000)
           else begin
              nxt_counter_i2c=counter_i2c+1;
              nxt_state=STATE_SERVOWAIT_L;
           end // else: !if(counter_i2c>=480000)
        end
        
        STATE_I2CWRREG: begin
           valid = 1'b1;
           cmd=8'h1B;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CWRADDR;
           end else begin
              nxt_state = STATE_I2CWRREG;
           end
        end

        STATE_I2CWRADDR: begin
           valid = 1'b1;
           cmd=sensor_addr;//address of sensor
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CWRREGPOS;
           end else begin
              nxt_state = STATE_I2CWRADDR;
           end
        end

        STATE_I2CWRREGPOS: begin
           valid = 1'b1;
           cmd=8'h00;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CWRNUM;
           end else begin
              nxt_state = STATE_I2CWRREGPOS;
           end
        end

        STATE_I2CWRNUM: begin
           valid = 1'b1;
           cmd=8'h01;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CWRCMD;
           end else begin
              nxt_state = STATE_I2CWRNUM;
           end
        end
        STATE_I2CWRCMD: begin
           valid = 1'b1;
           cmd=8'h51;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CWRSW;
           end else begin
              nxt_state = STATE_I2CWRCMD;
           end
        end
        STATE_I2CWRSW:begin
           if(counter_i2c>=240000) begin//wait for 5ms so that the qfix can finish I2C protocol
              nxt_counter_i2c=0;
              if(sensor_addr==8'hee)
              begin
                nxt_sensor_addr=8'he0;
                nxt_state=STATE_I2CWAIT;
              end
              else
              begin
                nxt_sensor_addr=sensor_addr+2;
                nxt_state=STATE_I2CWRREG;
              end
           end // if (counter_i2c>=480000)
           else begin
              nxt_counter_i2c=counter_i2c+1;
              nxt_state=STATE_I2CWRSW;
           end // else: !if(counter_i2c>=480000)
           
           end

        STATE_I2CWAIT: begin
           if(counter>=1920000)//because the freq of system is 48MHZ, the required waiting time is 65ms, previous uart cmds last nearly 40 ms already, here only wait for 40 ms
             begin
             nxt_counter=0;
              nxt_state = STATE_I2CRDREG;
                end
           else
             begin
             nxt_counter=counter+1;
              nxt_state = STATE_I2CWAIT;
           end
        end

        STATE_I2CRDREG: begin
           valid = 1'b1;
           cmd=8'h0a;
           if(soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CRDADDR;
           end else begin
              nxt_state = STATE_I2CRDREG;
           end
        end
        
        STATE_I2CRDADDR: begin
           valid = 1'b1;
           cmd=sensor_addr;//address of sensor
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CRDREGPOS;
           end else begin
              nxt_state = STATE_I2CRDADDR;
           end
        end
        
        STATE_I2CRDREGPOS: begin
           valid = 1'b1;
           cmd=8'h02;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CRDNUM;
           end else begin
              nxt_state = STATE_I2CRDREGPOS;
           end
        end
        STATE_I2CRDNUM: begin
           valid = 1'b1;
           cmd=8'h02;
           if (soccer_in_ready[USE_VCHANNEL]) begin
              nxt_state = STATE_I2CEND;
           end else begin
              nxt_state = STATE_I2CRDNUM;
           end
        end

  
        STATE_I2CEND: begin
           if(counter_i2c>=240000) begin//wait for Qfix to finish I2C protocol   Q: need to add wait for valid signal?
              nxt_counter_i2c=0;
              if(sensor_addr==8'hee)
             begin
                nxt_sensor_addr=8'he0;
                nxt_state=STATE_IDLE;
             end
           else
             begin
                nxt_sensor_addr=sensor_addr+2;
                nxt_state=STATE_I2CRDREG;
             end
           end // if (noc_out_valid==1'b1)
           else begin
              nxt_counter_i2c=nxt_counter_i2c+1;
              nxt_state=STATE_I2CEND;
           end // else: !if(noc_out_valid==1'b1)
         end // case: STATE_I2CEND
        
        default:
          nxt_state = STATE_IDLE;
      endcase   
   end
 
   uart_tile
   #(.UART_BAUD_RATE(UART_BAUD_RATE),.USE_VCHANNEL(USE_VCHANNEL),.ID(ID),.pkt_class(pkt_class))
     soccerboard_uart(
                      // Outputs
         .stx_pad_o                     (soccertx_pad_o),
         .noc_in_ready                  (soccer_in_ready[VCHANNELS-1:0]),
         .noc_out_flit                  (soccer_out_flit[NOC_FLIT_WIDTH-1:0]),
         .noc_out_valid                 (soccer_out_valid[VCHANNELS-1:0]),
         // Inputs
         .clk                               (clk),
                      .clk_uart(clk_uart),
         .rst                               (rst),
         .srx_pad_i                     (soccerrx_pad_i),
         .noc_in_flit                   (soccer_in_flit[NOC_FLIT_WIDTH-1:0]),
         .noc_in_valid                  (soccer_in_valid[VCHANNELS-1:0]),
         .noc_out_ready                 (soccer_out_ready[VCHANNELS-1:0]));   
         

  endmodule