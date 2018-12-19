****************
Porting OpTiMSoC
****************

OpTiMSoC already runs on a number of targets, like different FPGA boards and simulation environments.
This guide describes how to port OpTiMSoC to a new target which isn't supported yet.


Porting to an FPGA target
=========================

When porting to a new FPGA target, most work is required to get the off-chip interfaces working.
We do the porting in two steps: first, we ensure that we are able to communicate with the FPGA board.
Then, we port OpTiMSoC itself.

Establish a communication channel
---------------------------------

OpTiMSoC uses an abstraction library called `GLIP <http://www.glip.io>`_ to communicate with the FPGA board.
GLIP is an independent project, most resources on GLIP are on its own web site.
In this first step of porting, we leave OpTiMSoC aside and only port GLIP to the new target.

In GLIP, different communication methods in GLIP are handled by backends.
First, check if one of the backends is already supported for your board.
If that's the case, you're done.

If not, you need to either adapt an existing backend, or write a new one.
A good starting point for adaption are the UART and JTAG backends.
UART is an especially good choice if the target board has an UART to USB bridge (like the FT232), or a RS-232 driver (like the MAX232) available.
The GLIP JTAG backend only needs GPIO pins and does not use any on-board JTAG adapter which might be available (nor the various "soft" JTAG TAPs available in Xilinx and Altera FPGAs).
However, for JTAG you need a JTAG cable like the Olimex ARM-USB-TINY-H.
After you've chosen a backend, adapt it to work on your board by writing a lookback demo for your board.
Start by copying one of the existing loopback demos (in ``backend_XXX/logic/demo``).
Then add constraint file for your board to ``backend_XXX/logic/boards`` (usually you can find them on the board vendor's web site).
Now start modifying the demo you just copied and make it work.
In most cases, this involves changing the clock generation, and possibly exchanging implementation primitives like FIFOs.

In the end, you should have the ``loopback_measure`` demo application running on your board.
Create a pull request in the GLIP repository to get your changes reviewed and merged.
With the communication link now ready to go, switch back to the OpTiMSoC source tree.


.. note::

  For the next steps you can create a symlink to your modified GLIP version to ``$OPTIMSOC_SRC/externals/glip``, until all your changes are merged upstream.
  After your GLIP changes are merged upstream, you can run ``$OPTIMSOC_SRC/update_externals.sh`` to update the copy of GLIP inside the OpTiMSoC source tree.



Create a board support package
------------------------------

After you have ported GLIP, you're now ready to port OpTiMSoC itself.
OpTiMSoC abstracts many parts of the FPGA and board as "board support package" (BSP).
The BSPs live in ``$OPTIMSOC_SRC/external/extra_cores/boards`` and are synchronized with an `external repository <https://github.com/wallento/fusesoc_cores/>`_.
Until everything is working, we recommend editing the files directly in the OpTiMSoC source tree, and pushing them to the upstream wallento/fusesoc_cores repository as last step.

Porting a BSP involves work mostly in the areas of clocking and DDR memory interface.
The easiest way to develop the BSP is to directly test them on an OpTiMSoC system with only a single compute tile.
Before you start, equip yourself with a user guide of the board you're using and (if available) an example design.
The following instructions are necessarily vague, since the exact steps differ significantly between boards and devices.

1. Copy an existing compute_tile example as base four your changes.
   Use the most closely related board that you can find.
   For example, copy ``$OPTIMSOC_SRC/examples/fpga/nexys4ddr/compute_tile`` to ``$OPTIMSOC_SRC/examples/fpga/YOUR_BOARD/compute_tile``.

2. Rename the ``.core`` file and other files in the ``rtl/`` subdirectory to match your board name.

3. The toplevel design file is in the ``rtl/verilog`` directory.
   Inside it you'll find two things which need to be changed:

   - Use the appropriate GLIP toplevel module by replacing ``glip_BACKENDNAME_toplevel`` with the backend you made working in the previous step.
     Also set/adjust all required parameters, such as clocks.
   - ``u_board`` is an instance of the board support package.
     Most top-level ports are routed to this module.
     In return, the BSP provides a standardized interface for clock, reset, UART, and a AXI bus connected to the DDR memory.
     Change the name of the BSP module to match your board, before we continue creating this module.

4. Now it's time to create the BSP itself: go to ``$OPTIMSOC_SRC/external/extra_cores/boards`` and copy the directory from the closest related board.
   Also rename all files to match your board name.

5. If the board vendor provides a constraint files with pin locations, drop that file into the ``data/`` subdirectory of the BSP.
   Try to rename the pin names as little as possible to keep them consistent with the board vendor documentation.
   However, it's good practice in OpTiMSoC to make all input and output ports lower case.

6. Then start with the main work of porting: adjusting DRAM interface, the clock and routing of the UART pins.
   Open the toplevel module of the BSP, located in the ``rtl/verilog`` subdirectory of the BSP.

   - First adjust the DRAM interface.
     The easier way is to use your FPGA vendor's memory interface generator, such as Xilinx MIG, and let it create a memory interface which provides a AXI interface.
     If you didn't design the PCB yourself, find the information regarding the right DDR timings in the example design from a vendor, or in other documentation.
     For Xilinx devices, use the MIG to IP-XACT description file of the core (file ending ``.xci``).
     You can then include this file in the ``.core`` file and let Vivado create the IP core itself during synthesis.
     For other FPGA vendors, you're currently on your own: we don't have any experience them currently.

   - After you're done with the memory interface, adjust the clocking.
     In most cases, OpTiMSoC is clocked with a single 50 MHz clock.
     In Xilinx devices, this clock is generated by the MIG component, the off-chip oscillator is only routed to the MIG.
     In other devices, things might be different.
     Consult the FPGA and board user guides for details.

   - Finally, check if the board provides an UART interface chip somewhere (e.g. UART to USB or UART to RS232).
     If so, adjust the routing and signal names in the toplevel BSP file.

7. After you're happy with the initial changes, test them running a synthesis.

   - First, re-build OpTiMSoC itself to pick up the changes made in the ``external`` directory.

     .. code-block:: shell

       cd $OPTIMSOC_SRC
       ./tools/build.py --link-hw  --without-docs --without-examples-fpga --without-examples-sim

     This command will build all tools and link (instead of copy) all RTL/Verilog files to the ``$OPTIMSOC_SRC/objdir/dist``.
     In the following, you can edit RTL files in ``externals/`` without running this command again.

   - Ensure that you're using the OpTiMSoC version you've just built by running ``source $OPTIMSOC_SRC/objdir/dist/optimsoc-environment.sh``.
     Do this in every shell you're using (or add it to your ``~/.bashrc``).

   - Now synthesize the compute_tile example with fusesoc:

     .. code-block:: shell

        # use any directory outside of the source tree for synthesis
        mkdir $HOME/synthesis
        cd $HOME/synthesis

        fusesoc --verbose --cores-root $OPTIMSOC_SRC/examples/fpga/YOURBOARD build optimsoc:examples:YOURBOARD

     Most likely you will need to iterate a couple times until everything works as you want.
     For easier debugging, you can open the fusesoc-generated project file directly in your synthesis tool.
     You can find it in a subdirectory of ``$HOME/synthesis``.
     However, note that fusesoc **copies** all source files into the synthesis directory.
     If you make changes to the files there, you must **manually** copy them back to the OpTiMSoC source tree.


If have have successfully completed the previous steps: congratulations!
We know porting is a rather annoying job, things only get easier from now on!

As a final step, please create a pull request for the changes required for the ``external/fusesoc_cores`` parts, the GLIP changes and finally the OpTiMSoC changes itself.
The more working examples we have in our tree, the easier the porting will be for the next person.
If you run into any problem, please feel free to contact the OpTiMSoC developers!

