@defgroup backend_cypressfx2-examples-ztex_213_loopback ZTEX USB-FPGA 2.13 Loopback Demo
@ingroup backend_cypressfx2-logic

This demo implements a loopback in the device. The LEDs of the debug
board (LED2) are driven by a counter and indicate the speed and amount
of transfers. You need to set the DIP switches 3 and 4 to ON to have the LEDs
connected.

Prerequisites
-------------
Before you get started with this demo, make sure that you have the necessary
hardware ready and the software installed and configured.

* a [ZTEX USB-FPGA 2.13](http://www.ztex.de/usb-fpga-2/usb-fpga-2.13.e.html)
  board
* the [ZTEX SDK](http://www.ztex.de/firmware-kit/)
* have GLIP compiled and installed

You also need a synthesis toolchain.
* In this demo, we support *either*
  [Xilinx Vivado](http://www.xilinx.com/products/design-tools/vivado/)
  (the free WebPack edition is sufficient), version 2014.1 or higher
* *or* [Xilinx ISE](http://www.xilinx.com/products/design-tools/ise-design-suite/)
  (again, the free WebPack edition is sufficient), together with Synopsys
  Synplify (commercial license required)

Synthesize the design
---------------------
In the end we want to get from the Verilog sources to a bitstream. In this demo,
we show two ways of doing that: using Xilinx Vivado and using Synopsys Synplify
together with Xilinx ISE. The other two options, using Synplify with Vivado and
using ISE alone should be possible as well but are not shown here.

### Vivado ###
* In a console, type

      $> cd GLIP_DIR/src/backend_cypressfx2/logic/demo/ztex_213_loopback
      $> vivado -source loopback_vivado.tcl

  You need to replace `GLIP_DIR` with the directory containing your GLIP
  checkout.

* Click on `Generate Bitstream`. Answer `Yes` when asked to run other steps
  before the bitstream generation.
* The bitstream is located at
  `loopback/loopback.runs/impl_1/ztex_213_loopback.bit`

### Synplify and ISE ###
* Open Synplify
* Click on the left side on `Open Project...` -> `Existing Project...`, select
  the `loopback_synplify.prj` file and click on `Open`
* Click on `Run` in the top-left corner to synthesize the design. Wait for it to
  finish.
* Generate the bitstream by running on the console

      $> cd GLIP_DIR/src/backend_cypressfx2/logic/demo/ztex_213_loopback
      $> bitgen -w rev_1/pr_1/ztex_213_loopback.ncd

* The bitstream is located at `rev_1/pr_1/ztex_213_loopback.bit`.

Build and flash the FX2 firmware
--------------------------------

The Cypress FX2 chip on the ZTEX boards needs firmware to work properly.
Connect the board to your PC and execute the following steps to build and
flash the firmware.

    $> cd GLIP_DIR/src/backend_cypressfx2/fw/ztex
    $> make fw-213.ihx
    $> make flash-213

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
