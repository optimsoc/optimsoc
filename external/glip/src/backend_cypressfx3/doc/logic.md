@defgroup backend_cypressfx3-logic Cypress FX3 FPGA Logic
@ingroup backend_cypressfx3

The GLIP logic provides the interface between the GPIF-II of the Cypress FX3 chip
and the user logic. Since the GPIF-II already provides a FIFO interface, the
logic is rather minimal and consists mainly of the required clock-domain crossing
infrastructure and a state machine to compensate for the read/write delays of
the FX3.

The Verilog toplevel module `glip_cypressfx3_toplevel` implements the
[common GLIP logic interface](@ref logicif). The following additional ports are
needed for the external interface towards the GPIF-II of the FX3 chip. All
signals with the `_n` suffix are active low, following the FX3 default. Be
careful as the output polarity can be changed in the FX3 firmware!

| Port Name     | Width    | Direction | Description                                |
|---------------|:--------:|:---------:|--------------------------------------------|
| clk_io_100    | 1        | IN        | 100MHz I/O clock (forwarded to fx3_pclk)   |
| fx3_pclk      | 1        | OUT       | interface clock (supplied by FPGA)         |
| fx3_dq        | 16/32    | INOUT     | data bus. Can be 16 or 32 bit wide         |
| fx3_slcs_n    | 1        | OUT       | chip select                                |
| fx3_slwr_n    | 1        | OUT       | write strobe                               |
| fx3_sloe_n    | 1        | OUT       | enable FX3 output on data bus              |
| fx3_slrd_n    | 1        | OUT       | read strobe                                |
| fx3_pktend_n  | 1        | OUT       | packet end signal for short packet         |
| fx3_a         | 2        | OUT       | select GPIF-II socket                      |
| fx3_flaga_n   | 1        | IN        | GPIF-II socket 0 buffer full               |
| fx3_flagb_n   | 1        | IN        | GPIF-II socket 0 buffer almost full        |
| fx3_flagc_n   | 1        | IN        | GPIF-II socket 3 buffer empty              |
| fx3_flagd_n   | 1        | IN        | GPIF-II socket 3 buffer almost empty       |
| fx3_com_rst   | 1        | IN        | communication reset                        |
| fx3_logic_rst | 1        | IN        | logic reset                                |

The following parameters are available.

| Name         | Description                         |
|--------------|-------------------------------------|
| WIDTH        | Width of the FIFO (`fifo_*`) ports. Supported values: 16 and 32. Default: 16. You need to use the corresponding firmware for the chosen width! |
| BUFFER_DEPTH | Size of the input and output buffers on the FPGA in bytes. Default: 512 bytes |


USB Endpoint 1 (EP1) is the OUT endpoint, transferring data from the host to the
device, as well as the IN endpoint, transferring data from the device to the host
PC. P-port socket 3 of the GPIF-II is used for the USB OUT path, transferring
data to the FPGA, P-port socket 0 is used for the USB IN path, accepting data
from the FPGA. The data transfer between the USB endpoint and the P-ports is done
via automatic DMA transfers.

The following table lists the pins the different signals are mapped to on the FX3
board and the corresponding HPC connector pins.

| Port Name     | FX3 pin | HPC pin |
|---------------|:-------:|:-------:|
| fx3_pclk      | PCLK    | G6      |
| fx3_dq[0]     | DQ0     | H4      |
| fx3_dq[1]     | DQ1     | H7      |
| fx3_dq[2]     | DQ2     | H8      |
| fx3_dq[3]     | DQ3     | H10     |
| fx3_dq[4]     | DQ4     | H11     |
| fx3_dq[5]     | DQ5     | H13     |
| fx3_dq[6]     | DQ6     | H14     |
| fx3_dq[7]     | DQ7     | H16     |
| fx3_dq[8]     | DQ8     | H17     |
| fx3_dq[9]     | DQ9     | H19     |
| fx3_dq[10]    | DQ10    | H20     |
| fx3_dq[11]    | DQ11    | H22     |
| fx3_dq[12]    | DQ12    | H23     |
| fx3_dq[13]    | DQ13    | H25     |
| fx3_dq[14]    | DQ14    | H26     |
| fx3_dq[15]    | DQ15    | H28     |
| fx3_dq[16]    | DQ16    | H29     |
| fx3_dq[17]    | DQ17    | H31     |
| fx3_dq[18]    | DQ18    | H32     |
| fx3_dq[19]    | DQ19    | H34     |
| fx3_dq[20]    | DQ20    | H35     |
| fx3_dq[21]    | DQ21    | H37     |
| fx3_dq[22]    | DQ22    | H38     |
| fx3_dq[23]    | DQ23    | G2      |
| fx3_dq[24]    | DQ24    | G3      |
| fx3_dq[25]    | DQ25    | G9      |
| fx3_dq[26]    | DQ26    | G10     |
| fx3_dq[27]    | DQ27    | G12     |
| fx3_dq[28]    | DQ28    | G13     |
| fx3_dq[29]    | DQ29    | G15     |
| fx3_dq[30]    | DQ30    | G16     |
| fx3_dq[31]    | DQ31    | G18     |
| fx3_slcs_n    | CTL0    | G19     |
| fx3_slwr_n    | CTL1    | G21     |
| fx3_sloe_n    | CTL2    | G22     |
| fx3_slrd_n    | CTL3    | G24     |
| fx3_pktend_n  | CTL7    | G30     |
| fx3_a[0]      | CTL12   | G37     |
| fx3_a[1]      | CTL11   | G36     |
| fx3_flaga_n   | CTL4    | G25     |
| fx3_flagb_n   | CTL5    | G27     |
| fx3_flagc_n   | CTL6    | G28     |
| fx3_flagd_n   | CTL8    | G31     |
| fx3_com_rst   | CTL9    | G33     |
| fx3_logic_rst | CTL10   | G34     |

