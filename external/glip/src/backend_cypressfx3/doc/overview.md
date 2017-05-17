@defgroup backend_cypressfx3 Cypress FX3
@ingroup backend


The Cypress FX3 (internal name: `cypressfx3`) connects over USB 3.0 to a target device with a [Cypress EZ-USB FX3 Chip](http://www.cypress.com/products/ez-usb-fx3-superspeed-usb-30-peripheral-controller) on it or attached to it.

Supported Features
==================
- Number of channels: 1 (fixed)
- FIFO width: 16 or 32 bit


Components
==========

To make use of this backend, a number of components are required, as shown in the
graphic below.

            Host PC                Cypress FX3              Target FPGA
    -----------  ----------       ------------  -----------------------------------
    |         |  |        |       |          |  | --------------   -------------- |
    | libglip |--| libusb |--USB--| firmware |--| | GLIP logic | - | Your Logic | |
    |         |  |        |       |          |  | --------------   -------------- |
    -----------  ----------       ------------  -----------------------------------

On the host PC, the `cypressfx3` backend of libglip internally uses [libusb
1.0](http://libusb.info) for all USB communication. This library is available on
all major Linux distributions and does not need any kernel components.

The data is then transferred as USB 3.0 bulk transfers to the Cypress FX3 chip,
the control signals as USB control transfers. This chip contains a small CPU and
needs a special firmware for configuration. This firmware is provided in
different flavors by GLIP.

A 32 bit wide parallel interface (plus some control lines)
connects the Cypress FX3 chip with your FPGA. For this FPGA GLIP
provides a small piece of glue logic, which then provides the common
GLIP logic FIFO interface with a width of 16 or 32 bit.

For details and usage instructions for the individual components, see these
pages:
- @ref backend_cypressfx3-sw "libglip Backend"
- @ref backend_cypressfx3-fw
- @ref backend_cypressfx3-logic "FPGA Logic"
