@defgroup backend_cypressfx2 Cypress FX2
@ingroup backend


The Cypress FX2 (internal name: `cypressfx2`) connects over USB 2.0 to a
target device with a [Cypress EZ-USB FX2LP Chip](http://www.cypress.com/?id=193)
(part numbers CY7C68013A/14/15/16A) on it.

Supported Features
==================
- Number of channels: 1 (fixed)
- FIFO width: 16 bit (fixed)


Components
==========

To make use of this backend, a number of components are required, as shown in the
graphic below.

            Host PC                Cypress FX2              Target FPGA
    -----------  ----------       ------------  -----------------------------------
    |         |  |        |       |          |  | --------------   -------------- |
    | libglip |--| libusb |--USB--| firmware |--| | GLIP logic | - | Your Logic | |
    |         |  |        |       |          |  | --------------   -------------- |
    -----------  ----------       ------------  -----------------------------------

On the host PC, the `cypressfx2` backend of libglip internally uses [libusb
1.0](http://libusb.info) for all USB communication. This library is available on
all major Linux distributions and does not need any kernel components.

The data is then transferred as USB 2.0 bulk transfers to the Cypress FX2 chip,
the control signals as USB control transfers. This chip contains a small CPU and
needs a special firmware for configuration. This firmware is provided in
different flavors by GLIP.

A 16 bit wide parallel interface (plus some control lines) connects the Cypress
FX2 chip with your FPGA. For this FPGA GLIP provides a small piece of glue
logic, which then provides the common GLIP logic FIFO interface.

For details and usage instructions for the individual components, see these
pages:
- @ref backend_cypressfx2-sw "libglip Backend"
- @ref backend_cypressfx2-fw
- @ref backend_cypressfx2-logic "FPGA Logic"


Performance
==========
The `cypressfx2` libglip backend has been carefully written to provide excellent
throughput even for small reads or writes (e.g. calling glip_read() with only 6
bytes of data at a time). In loopback tests (user application - FPGA - user
application) around 21 MByte/s bi-directional were achieved (i.e. sending 21
MByte/s and at the same time receiving 21 MByte/s). With the theoretical USB 2.0
data rate being 30 MByte/s bi-directional, the achieved data rate is very close
to the maximum net data rate.

This performance is achieved by using a double-buffered architecture. All data
to be written to the device is first put into a ring buffer, from where it is
read in periodic intervals and transferred in large blocks over the USB bus.
Data read from the device is handled in the same way. All operations between the
libglip API functions and the transfer backend are fully asynchronous with no
locking in between.

While this approach results in great throughput performance, the transfer
latency can be reduced. A new USB transfer is started if either a sufficient
amount of data is available (`USB_BUF_SIZE / 2 = 16 kByte`) or if a timeout
expires. This timeout has been set to 5 milliseconds (see
`USB_TRANSFER_RETRY_TIMEOUT_MS` in backend_cypressfx2.c), i.e. when writing
only a small amount of data to the target GLIP waits for 5 ms until the data
is sent off to the device. If you need a latency below this threshold, you can
lower the timeout value.
