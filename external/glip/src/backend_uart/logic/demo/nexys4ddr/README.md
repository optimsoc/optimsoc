@defgroup backend_uart-examples-nexys4ddr_loopback Nexys4 DDR UART Loopback Demo
@ingroup backend_uart-logic

This demo implements a loopback in the device. The seven segment
display is used to show the current throughput in the loopback.

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
	../configure
	make
	sudo make install

Synthesize the design
---------------------

In the glip repository go to `src/backend_uart/logic/demo/nexys4ddr`
and run:

    make vivado-project
    vivado vivado/nexys4ddr.xpr

In vivado run the implementation and generate the bitstream.


Configure the FPGA
------------------

When the bitstream has been succesfully generated, open the Hardware
Manager, turn the board on and program the device.

Execute the loopback measurement tool
--------------------------------------

Execute the GLIP tool "Loopback Measure" that measures the loopback
performance.

    glip_loopback_measure -b uart -ospeed=3000000,device=/dev/ttyUSB0

If the UART device cannot be found at `/dev/ttyUSB0` try others like
`/dev/ttyUSB1` etc. If you can see the device, but cannot open, check
the permissions.

The performance should be 300 kByte/s bidirectional.

