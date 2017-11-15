@defgroup backend_uart-examples-vcu108_stress_test VCU108 UART Stress Test Demo
@ingroup backend_uart-logic

This demo implements a stress test in the device.

@note The VCU108 board provides only a rather slow UART chip. In theory,
it should be able to provide up to 2 MBaud/s, but in our testing only speeds
up to roughly 1 MBaud/s work reliably. However, it is also possible to use a
PmodUSBUART module to achieve a higher speed. By default the demo uses such a
module, connected to the bottom row of J52, which can reach up to 3 MBaud/s.

Prerequisites
-------------

For this demo you need:

* [Xilinx VCU108 Evaluation Kit](https://www.xilinx.com/products/boards-and-kits/ek-u1-vcu108-g.html)

* [Xilinx Vivado](http://www.xilinx.com/products/design-tools/vivado/)
  version 2016.3 or higher
  
Optional:

* [Pmod USBUART: USB to UART interface](http://store.digilentinc.com/pmod-usbuart-usb-to-uart-interface/)


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

    cd GLIP_SOURCES/src/backend_uart/logic/demo/vcu108_stress_test
    make vivado-project
    vivado vivado/vcu108_stress_test.xpr

In vivado run the implementation and generate the bitstream.


Configure the FPGA
------------------

When the bitstream has been succesfully generated, open the Hardware
Manager in Vivado, turn the board on and program the device.


Execute the stress test tool
----------------------------

Execute the GLIP tool "I/O Stress Test" that measures the performance when
streaming to/from the device.

The VC108 board provides a UART to serial bridge using the a Silicon Labs
CP2105 Dual USB to UART Bridge Controller. This device provides two serial ports
over one USB connection. Our demo uses the *lower numbered* serial port.

First, prepare the connection:

* Connect the "USB UART" labeled Mini-USB port on the board to your PC.
* Two new serial devices will appear, usually named `/dev/ttyUSB0` and
  `/dev/ttyUSB1`.
* We use the ttyUSB device with the *higher* number, i.e. `/dev/ttyUSB1`.
* Check with `ls -l /dev/ttyUSB1` that the file permissions allow you to read
  and write from this device. Otherwise use chmod to temporarily change the
  permissions, or use a udev rule to make these changes permanent.

Now you can run the loopback measurement tool:

    glip_io_stress_test -b uart -ospeed=921600,device=/dev/ttyUSB1

You should get around 75 KByte/s when running this test.

When using the PmodUSBUART module you will only see one serial device, usually
`/dev/ttyUSB0`. In this case run the loopback measurement tool:

    glip_io_stress_test -b uart -ospeed=3000000,device=/dev/ttyUSB0

You should get around 250 KByte/s when running this test.

### Notes for debugging

* We have experienced trouble with some USB hubs. If you cannot get a reliable
  connection to the board, try connecting the board directly to your PC.
* To find the right device file, run `dmesg` after connecting the board. You
  should see an output like this:

      usb 1-1.3: new full-speed USB device number 7 using ehci-pci
      usb 1-1.3: New USB device found, idVendor=10c4, idProduct=ea70
      usb 1-1.3: New USB device strings: Mfr=1, Product=2, SerialNumber=5
      usb 1-1.3: Product: CP2105 Dual USB to UART Bridge Controller
      usb 1-1.3: Manufacturer: Silicon Labs
      usb 1-1.3: SerialNumber: 006277B8
      cp210x 1-1.3:1.0: cp210x converter detected
      usb 1-1.3: cp210x converter now attached to ttyUSB0
      cp210x 1-1.3:1.1: cp210x converter detected
      usb 1-1.3: cp210x converter now attached to ttyUSB1
      cp210x ttyUSB1: failed to set baud rate to 300

    The second cp210x device name is the one we're looking for, ttyUSB1 in this case.
    The first one, ttyUSB0 is connected to the system management chip of the VCU108
    baord. You can confirm that by opening a terminal application like cutecom,
    set the baudrate to 115200 baud and connect to /dev/ttyUSB0. After pressing
    ENTER, you should see a menu of options to configure the VCU108 board.
