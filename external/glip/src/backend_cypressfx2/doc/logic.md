@defgroup backend_cypressfx2-logic Cypress FX2 FPGA Logic
@ingroup backend_cypressfx2

The GLIP logic provides the interface between the Cypress FX2 chip and the user
logic. Since the FX2 chip already provides a FIFO interface, the logic is rather
minimal and consists mainly of the required clock-domain crossing
infrastructure.

The Verilog toplevel module `glip_cypressfx2_toplevel` implements the
[common GLIP logic interface](@ref logicif). The following additional ports are
needed for the external interface towards the FX2 chip. All signals with the
`_n` suffix are active low, following the FX2 default. Be careful as the output
polarity can be changed in the FX2 firmware! USB Endpoint 2 (EP2) is the OUT
endpoint, transferring data from the host to the device. USB Endpoint 6 (EP6) is
configured as IN endpoint, transferring data from the device to the host PC.

| Port Name     | Width    | Direction | Description                       |
|---------------|:--------:|:---------:|-----------------------------------|
| fx2_ifclk     | 1        | IN        | interface clock (supplied by FX2) |
| fx2_com_rst   | 1        | IN        | communication reset               |
| fx2_logic_rst | 1        | IN        | logic reset                       |
| fx2_fd        | 16       | INOUT     | data bus                          |
| fx2_sloe_n    | 1        | OUT       | enable FX2 output on data bus     |
| fx2_slrd_n    | 1        | OUT       | read strobe                       |
| fx2_slwr_n    | 1        | OUT       | write strobe                      |
| fx2_fifoadr   | 2        | OUT       | select FIFO                       |
| fx2_flaga_n   | 1        | IN        | EP2 empty flag                    |
| fx2_flagb_n   | 1        | IN        | FIFOADR selected FIFO full flag   |
| fx2_flagc_n   | 1        | IN        | EP6 full flag                     |
| fx2_flagd_n   | 1        | IN        | EP6 almost full flag              |

