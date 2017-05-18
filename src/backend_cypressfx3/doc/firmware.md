@defgroup backend_cypressfx3-fw Cypress FX3 Firmware
@ingroup backend_cypressfx3

The Cypress FX3 chip needs firmware for its configuration. We use the
chip in the "Slave FIFO" mode which only forwards data between USB and
a 16 or 32 bit wide FIFO interface (depending on the firmware that is used).

## Flashing the FX3 firmware

Currently, the firmware part on the Fx3 is a bit messy, as a Cypress vendor tool is required.
The following steps flash the firmware.

 * Download the "EZ-USB FX3 SDK v1.3.3 for Linux" at http://www.cypress.com/file/139281.
   A registration is required.

 * Install the cyusb_linux GUI application

   ~~~sh
   mkdir fx3-install
   cd fx3-install
   tar -xf path/to/downloaded/FX3_SDK_Linux_v1.3.3.tar.gz
   tar -xf cyusb_linux_1.0.4.tar.gz
   cd cyusb_linux_1.0.4
   sudo ./install.sh
   ~~~

 * Unplug the FX3 board from the FPGA (if it is currently connected).

 * Disconnect (if already connected) the USB cable from the FX3 board.

 * Set the jumper J4 (PMODE) on the board to bring the device into programming mode.

 * Connect the FX3 board over the USB cable to your PC.

 * Run the cyusb_linux application to flash the firmware.

   ~~~sh
   LD_LIBRARY_PATH=/usr/local/lib cyusb_linux
   ~~~

 * In the GUI, select the correct device in the "List of devices" at the top.
   Verify that it the "Device Type" is "FX3".

 * Select the "Program" tab and there select the "FX3" subtab.

 * Select one of the "Download to ->" options.
   If you want to flash the firmware to the FX3 permanently, choose "I2C EEPROM". (The recommended default.)
   If you are debugging the firmware you can also choose "RAM". (If you choose "RAM", you need to re-flash whenever you disconnect the board from the power supply.)

 * Click on "Select File" and choose the firmware file from the GLIP source directory: $GLIP/src/backend_cypressfx3/fw/SlaveFifoSync32.img
   (use $GLIP/src/backend_cypressfx3/fw/SlaveFifoSync16.img for a 16 bit wide FIFO interface)

 * Click on "Start Download" to flash the firmware to the FX3.

 * If you've chosen the "I2C EEPROM" option, remove the jumper J4 (PMODE) again to let the FX3 boot with the firmware from EEPROM.

 * Close the cyusb_linux application again to free the device for other applications.

## Power cycle considerations

When powering (or resetting) the FX3 board the boot mode is determined by the PMODE pins.
These pins are also connected to the FPGA board. Hence, the FX3 must either be disconnected
from the FMC connector while being powered/resetted, or the FPGA must already be programmed
with the PMODE pins set to 'z1z' (which is done in the demo projects).

With the PMODE jumper (J4) set, the FX3 will boot from USB and can be programmed.
Without the jumper it will boot from the EEPROM.

## Building the Firmware

The firmware that is used with the glip is based on the 'FX3 Slave FIFO
Interface' example provided by Cypress.
To build the firmware from source code follow these steps (under Linux):

 * Download and install the Cypress EZ-USB FX3 SDK from http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit
 * Download the 'FX3 Slave FIFO Interface' example from http://www.cypress.com/documentation/application-notes/an65974-designing-ez-usb-fx3-slave-fifo-interface
 * Copy the directory "/FX3 Firmware/SlaveFifoSync" from the example to the directory $FX3_INSTALL_PATH/firmware/slavefifo_examples
 * Delete the subdirectories "Debug" and "Release" in "SlaveFifoSync"
 * Copy $GLIP/src/backend_cypressfx3/fw/SlaveFifoSync32.patch to $FX3_INSTALL_PATH/firmware/slavefifo_examples/SlaveFifoSync and apply the patch
   (to build the firmware with the 16 bit wide FIFO interface use $GLIP/src/backend_cypressfx3/fw/SlaveFifoSync16.patch instead)
 * Now you can import the project to the EZ-USB FX3 SDK and build the firmware

