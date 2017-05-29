@defgroup backend_cypressfx3-examples-vcu108_loopback VCU108 Cypress FX3 (USB 3.0) Loopback Demo
@ingroup backend_cypressfx3-logic

Prerequisites
-------------

For this demo you need:

* [Xilinx Virtex UltraScale FPGA VCU108 Evaluation Kit](https://www.xilinx.com/products/boards-and-kits/ek-u1-vcu108-g.html)

* [Xilinx Vivado](http://www.xilinx.com/products/design-tools/vivado/)

* [CYUSB3KIT-003 EZ-USB® FX3™ SuperSpeed Explorer Kit](http://www.cypress.com/documentation/development-kitsboards/cyusb3kit-003-ez-usb-fx3-superspeed-explorer-kit)

* [CYUSB3ACC-005 FMC Interconnect Board for the EZ-USB® FX3™ SuperSpeed Explorer Kit](http://www.cypress.com/documentation/development-kitsboards/cyusb3acc-005-fmc-interconnect-board-ez-usb-fx3-superspeed)

To prepare, plug the Explorer Kit into the FMC Interconnect Board.
*Do not* connect the two boards to the VCU108 FPGA board just yet.


Download and compile glip
-------------------------

If you have not done this before you need to download and build glip:

    git clone https://github.com/TUM-LIS/glip.git
    cd glip
    ./autogen.sh
    mkdir build; cd build
    ../configure --enable-cypressfx3
    make
    sudo make install

Synthesize the design
---------------------

Synthesize the example design with Xilinx Vivado.

    cd GLIP_SOURCES/src/backend_cypressfx3/logic/demo/vcu108_loopback
    make vivado-project-32
    vivado vcu108_loopback_32/vcu108_loopback.xpr

In Vivado run the implementation and generate the bitstream.


Configure the FPGA
------------------

Before you turn on the VCU108 board, *disconnect* the FMC Interconnect board together with the FX3 board.

When the bitstream has been succesfully generated, open the Hardware Manager in Vivado, turn the board on and program the FPGA.


Flash the Cypress FX3 firmware
------------------------------

Before being used the first time, the FX3 chip must be flashed with our firmware.
Please refer to @ref backend_cypressfx3-fw for instructions how to do that.
It's best to program the firmware to the EEPROM to save yourself from the hassle of doing these steps again.


Check the jumpers on the FX3 board
----------------------------------

The FX3 board contains three jumpers to configure it.
Ensure that you have set the jumpers to the following settings:

 * Jumper J2 (VIO1-3) is not set (open)
 * Jumper J3 (VBUS JUMPER) is set (closed)
 * Jumper J4 (PMODE) is not set (open)
 * Jumper J5 (SRAM ENABLE) is not set (open)


Connect the FX3 board to the FPGA
---------------------------------

While the FPGA is board is powered-on and flashed with the bitstream, plug the FX3 board into the "FMC HPC1" (J2) connector on the VCU108 board.
(It's the one closer to the power plug.)


Execute the loopback measurement tool
-------------------------------------

As final (and long awaited) step, execute the GLIP tool "Loopback Measure" that measures the loopback performance by sending data to the device and reading back from it.
Run the loopback measurement tool like this to transfer 10 GB of data to the FPGA and back.

    glip_loopback_measure -b cypressfx3 -s 1000000000


If you plugged the FX3 chip into an USB 3.0 port on your PC, you should get a performance of around 190 MByte/s bidirectional (i.e. 190 MByte/s sending and 190 MByte/s receiving at the same time), or 3 GBps overall (net).
Note, however, that this data rate assumes that only this test occupies the USB bus on the host.
If other transfers are active at the same time (e.g. an external hard disk you're transferring data to), the data rate will be lower.
