Debugging
=========

Run the program with GDB
------------------------
Before `make install` is called all compiled binaries in the source tree are wrapped by libtool.
To run GDB on them, use the following command line:

.. code-block:: sh

   # general
   libtool --mode=execute gdb --args YOUR_TOOL

   # for example
   libtool --mode=execute gdb --args ./src/tools/osd-target-run/osd-target-run -e ~/src/baremetal-apps/hello/hello.elf -vvv

GDB helpers
-----------

GDB can call functions in the program binary when the program is stopped (e.g. when a breakpoint hit).
This can be used to dump useful information from data structures in a readable form.

.. code-block:: none

   # dump a DI packet
   (gdb) p osd_packet_dump(pkg, stdout)
   Packet of 5 data words:
   DEST = 4096, SRC = 1, TYPE = 2 (OSD_PACKET_TYPE_EVENT), TYPE_SUB = 0
   Packet data (including header):
     0x1000
     0x0001
     0x8000
     0x0000
     0x0000
   $5 = void

Profiling
---------
Generating an execution profile of the OSD tools can be helpful to reduce the CPU consumption, increase throughput, etc.
To generate a profile we recommend the Linux ``perf`` tool.
(gprof doesn't play nice with ZeroMQ.)

Setup perf
""""""""""
Install perf using the package manager of your distribution.

.. code-block:: sh

   # install perf
   # for Ubuntu/Debian
   sudo apt-get install perf
   # for SUSE
   sudo zypper install perf

Running perf as non-root user requires a couple kernel settings to be changed.

.. code-block:: sh

   # Give access to kernel pointers
   sudo sh -c " echo 0 > /proc/sys/kernel/kptr_restrict"

   # Give access to perf events
   sudo sh -c " echo 0 > /proc/sys/kernel/perf_event_paranoid"

   # Make kernel permissions permanent after the next reboot (optional)
   echo "kernel.kptr_restrict=0\nkernel.perf_event_paranoid=0" >> /etc/sysctl.conf"

Profile application
"""""""""""""""""""
First, build the application as usual.
Then, from inside the source tree, run the tool with ``perf record`` to collect an execution profile.

.. code-block:: sh

   # example: profile osd-target-run (without call stacks)
   libtool --mode=execute perf record ./src/tools/osd-target-run/osd-target-run -e ~/src/baremetal-apps/hello/hello.elf --systrace -vvv

   # the same with call stacks
   libtool --mode=execute perf record -g ./src/tools/osd-target-run/osd-target-run -e ~/src/baremetal-apps/hello/hello.elf --systrace -vvv

Evaluate profile
""""""""""""""""
``perf record`` stores the recorded events in a file called ``perf.data`` in the directory it was called in.
To display this data nicely run ``perf report``.

.. code-block:: sh

   perf report
