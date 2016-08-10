@defgroup backend_uart-sw UART libglip backend
@ingroup backend_uart

The UART backend connects over a serial line, using standard Linux
capabilities.

Configuration Options
=====================

Use the "uart" backend name with glip_new(). The following options are
available:

<table>
  <caption><code>uart</code> backend options</caption>
  <tr>
    <td><code>device</code></td>
    <td>The device name, default <code>/dev/ttyUSB0</code></td>
  </tr>
  <tr>
    <td><code>speed</code></td>
    <td>Baud rate, default <code>115200</code></td>
  </tr>
</table>
