@defgroup backend_cypressfx2-examples-ztex_115_loopback ZTEX USB-FPGA 1.15d Loopback Demo
@ingroup backend_cypressfx2-logic

This demo implements a loopback in the device.

Prerequisites
-------------
Before you get started with this demo, make sure that you have the necessary
hardware ready and the software installed and configured.

* a [ZTEX USB-FPGA 1.15d](http://www.ztex.de/usb-fpga-1/usb-fpga-1.15.e.html)
  board
* the [ZTEX SDK](http://www.ztex.de/firmware-kit/)
* have GLIP compiled and installed

You also need a synthesis toolchain.
* [Xilinx ISE](http://www.xilinx.com/products/design-tools/ise-design-suite/)
  (the free WebPack edition is sufficient), together with Synopsys
  Synplify (commercial license required)

Synthesize the design
---------------------
In the end we want to get from the Verilog sources to a bitstream. In this demo,
we use Synopsys Synplify together with Xilinx ISE.

* Open Synplify
* Click on the left side on `Open Project...` -> `Existing Project...`, select
  the `loopback_synplify.prj` file and click on `Open`
* Click on `Run` in the top-left corner to synthesize the design. Wait for it to
  finish.
* Generate the bitstream by running on the console

      $> cd GLIP_DIR/src/backend_cypressfx2/logic/demo/ztex_115_loopback
      $> bitgen -w rev_1/pr_1/ztex_115_loopback.ncd

* The bitstream is located at `rev_1/pr_1/ztex_115_loopback.bit`.

Build and flash the FX2 firmware
--------------------------------

The Cypress FX2 chip on the ZTEX boards needs firmware to work properly.
Connect the board to your PC and execute the following steps to build and
flash the firmware.

    $> cd GLIP_DIR/src/backend_cypressfx2/fw/ztex
    $> make fw-115.ihx
    $> make flash-115

The firmware is written to the EEPROM and does not need to be reloaded after you
unplug your board (until you have overwritten it).

Configure the FPGA
------------------

When the bitstream has been succesfully generated, you can configure the FPGA
with through the Cypress FX2 chip using the ZTEX tools. Adjust the path to the
bitstream as given in the section above.

    $> FWLoader -c -rf -uf PATH/TO/BITSTREAM.bit

Execute the loopback measurement tool
--------------------------------------

Execute the GLIP tool "Loopback Mesasure" that measures the loopback
performance.

    $> glip_loopback_measure -b cypressfx2 -ousb_vid=0x221a,usb_pid=0x0100

The performance should be around 20 MByte/s bidirectional.

