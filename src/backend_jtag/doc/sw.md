@defgroup backend_jtag-sw JTAG libglip backend
@ingroup backend_jtag
The JTAG backend connects talks to your logic over a JTAG interface using
a custom protocol. The JTAG transmissions are handled by OpenOCD. This page
explains the configuration options as well as important information regarding
the use of OpenOCD.

Configuration Options
=====================
Use the "jtag" backend name with glip_new(). The following options are
available:

<table>
  <caption><code>jtag</code> backend options</caption>
  <tr>
    <td><code>port</code></td>
    <td>port where OpenOCD runs on (default: <code>6666</code>)</td>
  </tr>
  <tr>
    <td><code>hostname</code></td>
    <td>
      hostname of the PC running OpenOCD (default: <code>localhost</code>)
    </td>
  </tr>
  <tr>
    <td><code>oocd_conf_interface</code></td>
    <td>
      <i>(Required if OpenOCD is started by GLIP.)</i>
      OpenOCD configuration file for the JTAG adapter (aka. "interface").

      Example for the Olimex ARM-USB-TINY-H adapter
      <code>oocd_conf_interface=interface/ftdi/olimex-arm-usb-tiny-h.cfg</code>
    </td>
  <tr>
    <td><code>oocd_conf_target</code></td>
    <td>
      <i>(Only relevant if OpenOCD is started by GLIP, optional.)</i>
      OpenOCD target configuration file.

      By default, we pass the GLIP
      specific configuration file <code>glip.cfg</code>, but you can
      override that here. You can also use this option to include other
      OpenOCD configuration files. For this feature, write a custom
      configuration file and include the <code>glip.conf</code> file
      as needed.
    </td>
  </tr>
  </tr>
</table>

OpenOCD configuration search path
---------------------------------
The <code>oocd_conf_*</code> configuration options can be passed an an absolute
or a relative path to a configuration file in the same way they would be passed
to the OpenOCD <code>-f</code> option.

Relative paths are resolved using an extended OpenOCD search path, which results
in the following path:
<ol>
  <li>The current working directory</li>
  <li>
    GLIP's <code>$PKGDATADIR</code>, usually
    <code>/usr/share/glip</code>
  </li>
  <li>
    The directory <code>.openocd</code> in your home directory.
  </li>
  <li>
    OpenOCD's script directory in <code>$PKGDATADIR/scripts</code>,
    usually <code>/usr/share/openocd/scripts</code>.
  </li>
</ol>

OpenOCD
=======
This backend relies on OpenOCD for the data transmission. OpenOCD needs to
be installed and working on the machine where the JTAG cable is connected.
GLIP then connects to OpenOCD over TCP. The hostname and port where OpenOCD
is running can be specified using backend options (see above).

GLIP can also start OpenOCD itself if it is needed. For this feature to
work, please consider the following notes:

- Set the configuration options `hostname` and `port` to `localhost`
  and `6666`, respectively. This equals the default values, i.e. you can
  simply not pass those options at all.
- Make sure OpenOCD is properly installed and the `openocd` binary is in
  your `PATH`.
- Specify at least the `oocd_conf_interface` backend option and pass the
  path to a interface configuration file of OpenOCD, usually shipped with
  OpenOCD in `/usr/share/openocd/scripts/interface`.

@note OpenOCD contains for many FT2232-based adapters two possible backends:
  the `ftdi` ones using the newer open source FTDI driver, and the older
  `ft2232` ones. By default, OpenOCD only builds the newer backends. Those
  backends have also shown to be faster, and are preferred.
