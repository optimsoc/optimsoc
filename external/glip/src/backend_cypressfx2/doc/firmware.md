@defgroup backend_cypressfx2-fw Cypress FX2 Firmware
@ingroup backend_cypressfx2

The Cypress FX2 chip needs firmware for its configuration. We use the chip in the "Slave FIFO" mode which only forwards data between USB and a 16 bit wide FIFO interface.

Depending on your board the FX2 chip might be used for other purposes as well, and an integration of the different firmware modules is necessary. One example are the ZTEX FPGA-USB boards, which also do FPGA configuration and SD-card access through the FX2. For those  boards special firmware is necessary, which we call "firmware flavors".

ZTEX Firmware
=============
The [ZTEX USB-FPGA boards](http://www.ztex.de/) are small modules containing a Xilinx FPGA (Spartan or Artix), DDR memory, a FX2 chip and other components. The firmware provided by GLIP also enables FPGA highspeed configuration through the FWLoader utility from the ZTEX SDK.

Supported Boards
----------------
- [ZTEX USB-FPGA 1.15 series](http://www.ztex.de/usb-fpga-1/usb-fpga-1.15.e.html)
- [ZTEX USB-FPGA 2.13 series](http://www.ztex.de/usb-fpga-2/usb-fpga-2.13.e.html)

Build Prerequisites
-------------------
You need to have two tools installed before building the firmware:

- [sdcc](http://sdcc.sourceforge.net/), the small device C compiler; it's available as package in many distributions
- the [ZTEX SDK](http://www.ztex.de/firmware-kit/), which includes tools such as FWLoader to flash the firmware and the FPGA bitstream

Build
-----
Switch to the `fw/ztex` directory, and run

~~~~~~~~~~~~~~~~~~~~~
# for the 2.13 series boards
$> make fw-213.ihx

# for the 1.15 series boards
$> make fw-115.ihx
~~~~~~~~~~~~~~~~~~~~~

Flash
-----
You can flash the FX2 firmware into volatile memory or into the EEPROM of the FX2 chip. The following commands flash it into the EEPROM, so that you have to do this step only once when using a new board.

~~~~~~~~~~~~~~~~~~~~~
# for the 2.13 series boards
$> make flash-213

# for the 1.15 series boards
$> make flash-115
~~~~~~~~~~~~~~~~~~~~~