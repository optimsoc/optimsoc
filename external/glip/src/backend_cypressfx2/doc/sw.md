@defgroup backend_cypressfx2-sw Cypress FX2 libglip backend
@ingroup backend_cypressfx2

The Cypress FX2 backend connects over a USB 2.0 connection to targets with
a Cypress EZ-USB FX2 USB chip.

Configuration Options
=====================

Use the "cypressfx2" backend name with glip_new(). The following options
are available:

<table>
  <caption><code>cypressfx2</code> backend options</caption>
  <tr>
    <td><code>usb_dev_bus</code></td>
    <td>the USB bus the device is found on</td>
  </tr>
  <tr>
    <td><code>usb_dev_addr</code></td>
    <td>the address of the device on <code>usb_dev_bus</code></td>
  </tr>
  <tr>
    <td><code>usb_vid</code></td>
    <td>the USB vendor ID of the device (for auto-discovery)</td>
  </tr>
  <tr>
    <td><code>usb_pid</code></td>
    <td>the USB product ID of the device (for auto-discovery)</td>
  </tr>
  <tr>
    <td><code>usb_manufacturer</code></td>
    <td>the USB manufacturer string of the device (for auto-discovery)</td>
  </tr>
  <tr>
    <td><code>usb_product</code></td>
    <td>the USB product name of the device (for auto-discovery)</td>
  </tr>
</table>

USB Device Auto-Discovery
-------------------------

The used USB device can either be auto-discovered or manually specified. Both
mechanisms can be configured through options.

To <b>manually specify</b> a specific USB device to be used, set the options
`usb_dev_bus` and `usb_dev_addr`.
`usb_dev_bus` specifies the bus the device is connected to,
`usb_dev_addr` specifies the address of the device on the given bus.

To <b>automatically discover</b> the used USB device, a filter with supported
devices is necessary. This filter is applied to some fields of the device
descriptor, namely the vendor id (idVendor), the product id (idProduct), the
product name string (iProduct) and the manufacturer name string (iManufacturer).
It is possible to either specify a custom filter, or to rely on a built-in
list of well-known devices.

To set a custom filter the following options are used:
- `usb_vid`: the vendor ID
- `usb_pid`: the product ID
- `usb_manufacturer`: the manufacturer name string (not matched if omitted)
- `usb_product`: the product name string (not matched if omitted)

If no option is specified, the built-in list of devices is tried.

@note When using the automatic device discovery based on filters it is
      possible that more than one device matches the filter. In this case any
      device might be used, i.e. the used device is unspecified. The only way
      to prevent this is to manually specify a device based on bus and
      address.

Device Permissions
==================

The user running the application using GLIP needs to have sufficient
permissions to read and write from the USB device. This can be done
through a simple

    sudo chmod a+rw /dev/bus/usb/<BUS>/<ADDR>

after obtaining the bus and address information of the target device, e.g.
through the `lsusb` command. A more permanent way of setting those
permissions is to write a custom udev rule, usually based on the USB VID/PID
combination. The Internet has plenty of examples of such udev rules
available.
