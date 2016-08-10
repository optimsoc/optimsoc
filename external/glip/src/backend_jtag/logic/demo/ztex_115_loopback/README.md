@defgroup backend_jtag-examples-ztex_115_loopback ZTEX USB-FPGA 1.15 JTAG Loopback Demo
@ingroup backend_jtag-logic

This demo shows how to use the GLIP JTAG backend. The FGPA is only used as a
loopback device, which sends back all data it receives.

Prerequisites
-------------
Before you get started with this demo, make sure that you have the necessary
hardware ready and the software installed and configured.

* a [ZTEX USB-FPGA 1.15d](http://www.ztex.de/usb-fpga-1/usb-fpga-1.15.d.html)
  board
* an
  [Olimex TINY-USB-H](https://www.olimex.com/Products/ARM/JTAG/ARM-USB-TINY-H/)
  JTAG adapter
* have GLIP compiled and installed with the `--enable-jtag` configure option

You also need a synthesis toolchain. In this demo, we use
[Xilinx ISE](http://www.xilinx.com/products/design-tools/ise-design-suite/)
(the free WebPack edition is sufficient), together with Synopsys Synplify
(commercial license required). It should be possible without any trouble to use
ISE alone.

Synthesize the design
---------------------

A complete Makefile is included with this demo. To synthesize and flash the
design, run

    $> make synplify
    $> make bitstream

Hardware setup
--------------

The JTAG adapter is connected to standard I/O pins of the FPGA, which are
defined in the `boards/ztex_115/jtag.ucf` constraints file.

For this demo, connect the Olimex TINY-USB-H device to the FPGA using the pin
mapping in the table below. If those pins are not available in your design, you
can use any other I/O pin by modifying the constraints file accordingly.


JTAG Name | ZTEX 1.15d pin | FPGA pad | Olimex pin |
--------- | -------------- | -------- |------------|
tck       | B27            | D7       | 9          |
tms       | A26            | A8       | 7          |
tdi       | B26            | B8       | 5          |
tdo       | B29            | C5       | 13         |


All other JTAG pins (especially TRST) are not used, as their functionality can
be emulated using the other signals.

Further reading:
 - [Olimex adapter pin mapping](https://www.olimex.com/Products/ARM/JTAG/_resources/openocd-jtag-layout.png)
 - [ZTEX pin mapping](http://www.ztex.de/downloads/usb-fpga-1.xls)

Configure the FPGA
------------------

Two options exist to load the bitstream onto the FPGA on the ZTEX boards: either
by using the standard Xilinx tools ("Program Device" from Xilinx Impact), which
load the bitstream over JTAG, or by using the ZTEX tools, which load the
bitstream through a USB connection.

@note The bitstream is loaded through the FPGA's hardware JTAG port, which has
  nothing to do with the JTAG port the GLIP communication happens over. If you
  flash the bitstream over JTAG, and also use the GLIP JTAG, you will have two
  independent JTAG cables connected to your device!

### Using the ZTEX tools
If you use the board for the first time, you need to flash the Cypress FX2 USB
chip first with the ZTEX firmware, and then use the `FWLoader` utility to
configure the FPGA. Please consult the
@ref backend_cypressfx2-examples-ztex_213_loopback "ZTEX USB-FPGA 2.13 Loopback Demo"
for details. Instead of the GLIP firmware which contains additional
functionality for the USB data transfer (unused in this demo), you can also use the
[standalone firmware from ZTEX](http://wiki.ztex.de/doku.php?id=en:ztex_boards:ztex_fpga_boards:standalone).

Note that this method requires a USB connection between the ZTEX board and
the host PC.

To flash the device, run

    $> make flash


### Using Xilinx Impact
This method requires a JTAG connection between the board and your PC using a
standard JTAG adapter. More information is available from
[ZTEX](http://wiki.ztex.de/doku.php?id=en:ztex_boards:ztex_fpga_boards:jtag)
and in the ISE/Impact documentation from Xilinx.


Execute the loopback measurement tool
--------------------------------------

Execute the GLIP tool "Loopback Mesasure" that measures the loopback
performance.

    $> glip_loopback_measure -b jtag -o oocd_conf_interface=interface/ftdi/olimex-arm-usb-tiny-h.cfg
