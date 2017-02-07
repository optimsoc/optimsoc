@defgroup backend_uart-examples-nexys4ddr_loopback Nexys4 DDR UART Loopback Demo
@ingroup backend_uart-logic

This demo implements a loopback in the device. The seven segment
display is used to show the current throughput in the loopback.

This demo assumes basic familiarity with Xilinx Vivado and using an FPGA.

Prerequisites
-------------

For this demo you need:

* [Digilent's Nexys 4 DDR Board](http://store.digilentinc.com/nexys-4-ddr-artix-7-fpga-trainer-board-recommended-for-ece-curriculum/)

* [Xilinx Vivado](http://www.xilinx.com/products/design-tools/vivado/)
  (the free WebPack edition is sufficient), version 2015.4 or higher


Download and compile glip
-------------------------

If you have not done this before you need to download and build glip:

    git clone https://github.com/TUM-LIS/glip.git
    cd glip
    ./autogen.sh
    mkdir build; cd build
    ../configure --enable-uart
    make
    sudo make install

Synthesize the design
---------------------

Synthesize the example design with Xilinx Vivado.

    cd GLIP_SOURCES/src/backend_uart/logic/demo/nexys4ddr
    make vivado-project
    vivado vivado/nexys4ddr.xpr

In vivado run the implementation and generate the bitstream.


Configure the FPGA
------------------

When the bitstream has been succesfully generated, open the Hardware
Manager in Vivado, turn the board on and program the device.

Execute the loopback measurement tool
--------------------------------------

Execute the GLIP tool "Loopback Measure" that measures the loopback
performance.

Before you start the measurement, make sure to set the DIP switches on
Nexys 4 DDR board below the 7-segment display all to OFF, i.e. they all point
towards the bottom of the PCB and away from the 7-segment display (this enables
loopback mode). The 7-segment display should now display all zeros.

The design is set to 12 MBaud/s in the toplevel verilog file, we
need to pass this speed to GLIP using the `speed` parameter. Run the loopback
measurement tool like this:

    glip_loopback_measure -b uart -ospeed=12000000,device=/dev/ttyUSB0


If the UART device cannot be found at `/dev/ttyUSB0` try others like
`/dev/ttyUSB1` etc. If you can see the device, but cannot open, check
the permissions.

The performance should be 1170 kByte/s bidirectional.
