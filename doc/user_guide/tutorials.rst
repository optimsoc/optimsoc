*********
Tutorials
*********

The best way to get started with OpTiMSoC after you've prepared your system as described in the previous chapter is to follow some of our tutorials.
They are written with two goals in mind: to introduce some of the basic concepts and nomenclature of manycore SoC, and to show you how those are implemented and can be used in OpTiMSoC.

Some of the tutorials (especially the first ones) build on top of each other, so it's recommended to do them in order.
Simply stop if you think you know enough to implement your own ideas!

Starting Small: Compute Tile and Embedded Software (Simulated)
==============================================================

It is a good starting point to simulate a single compute tile of a distributed memory system.
Therefore a simple example is included and demonstrates the general simulation approach and gives an insight in the software building process.

Simulating only a single compute tile is essentially an OpenRISC core plus memory and the network adapter, where all I/O of the network adapter is not functional in this test case.
It can therefore only be used to simulate local software.

You can find this example in ``$OPTIMSOC/examples/sim/compute_tile``.

In addition to the simulated SoC hardware, you also need software that runs on the system.
Our demonstration software is available in an extra repository:

.. parsed-literal::

   git clone https://github.com/optimsoc/baremetal-apps
   cd baremetal-apps

Build a simple "Hello World" example:

.. code:: sh

   cd hello
   make

You will then find the executable elf file as ``hello/hello.elf``.
Furthermore some other files are built.
They are essentially transformed versions of the ELF file, i.e. the software binary.

- ``hello.dis`` is the disassembly of the file
- ``hello.bin`` is the elf representation of the binary file
- ``hello.vmem`` is a textual copy of the binary file

Now you have everything you need to run the hello world example on a simulated SoC hardware:

.. code:: sh

   $OPTIMSOC/examples/sim/compute_tile/compute_tile_sim_singlecore --meminit=hello.vmem

And you'll get roughly this output:

::

   [                  22, 0] Software reset
   [               63128, 0] Terminated at address 0x0000e958 (status:          0)
   - ../src/optimsoc_trace_software_0/verilog/software_tracer.sv:104: Verilog $finish

Furthermore, you will find a file called ``stdout.000`` which shows the actual output:

::

   # OpTiMSoC trace_monitor stdout file
   # [TIME, CORE] MESSAGE
   [               39614, 0] Hello World! Core 0 of 1 in tile 0, my absolute core id is: 0
   [               48764, 0] There are 1 compute tiles:
   [               57162, 0]  rank 0 is tile 0

Congratulations, you've ran your first OpTiMSoC system!

.. note:: If you are already familiar with embedded systems or microcontrollers, you might wonder: how did the ``printf()`` output from the software get into the ``stdout.000`` file if there is no UART or anything similar?

   OpTiMSoC software makes excessive use of a useful part of the OpenRISC ISA.
   The "no operation" instruction ``l.nop`` has a parameter ``K`` in assembly.
   This can be used for simulation purposes. It can be used for instrumentation, tracing or special purposes as writing characters with minimal intrusion or simulation termination.

   The termination is forced with ``l.nop 0x1``.
   The instruction is observed and a trace monitor terminates when it was observed at all cores (shortly after ``main()`` returned).

   With this method you can simply provide constants to your simulation environments.
   For variables this method is extended by putting data in further registers (often ``r3``).
   This still is minimally intrusive and allows you to trace values.
   The printf is also done that way (see newlib):

   .. code:: c

      static void sim_putc(unsigned char c) {
          asm("l.addi r3,%0,0" : : "r" (c) : "r3");
          asm("l.nop 4");
      }

   This function is called from printf as write function.
   The trace monitor captures theses characters and puts them to the stdout file.

   You can easily add your own *traces* using a macro defined in ``$OPTIMSOC/soc/sw/include/baremetal/optimsoc-baremetal.h``:

   .. code:: c

      #define OPTIMSOC_TRACE(id,v)                \
         asm("l.addi r3,%0,0" : : "r" (v) : "r3"); \
         asm("l.nop %0": :"K" (id));

See the Waves
=============

One major benefit of simulating a SoC is the possibility to inspect every signal inside the hardware design quite easily.
When running a Verilator simulation, as we did in the previous step, you can simply add the ``--vcd`` command line option.
It instructs Verilator to write all signals into a file.
You can then start a waveform viewer, like GTKWave to display it.

.. code:: sh

   $OPTIMSOC/examples/sim/compute_tile/compute_tile_sim_singlecore --meminit=hello.vmem --vcd

This command will run the hello world example like it did before, but this time Verilator additionally writes a ``sim.vcd`` waveform file.
You can now view this file.

.. code:: sh

   gtkwave -o sim.vcd

The screenshot is similar to what you should see when running GTKWave.

.. image:: img/screenshot-gtkwave.png
   :width: 100%

On the left side you find a hierarchy of all signals in the system.
Add them to the wave view and explore all internals of a working SoC at your fingertips!
Can you find the program counter? The instruction and data caches? The branch predictor?

Going Multicore: Simulate a Multicore Compute Tile
==================================================

Next you might want to build an actual multicore system.
In a first step, you can just start simulations of compute tiles with multiple cores.

Inside ``$OPTIMSOC/examples/sim/compute_tile`` you'll find a dual-core version and a quad-core version of the system with just one compute tile that you just simulated in the previous step.
You can run those examples like you did before.

The first thing you observe: the simulation runs become longer.
After each run, inspect the ``stdout.*`` files.

Welcome to the multicore world!

Tiled Multicore SoC: Simulate a Small 2x2 Distributed Memory System
===================================================================

Next we want to run an actual NoC-based tiled multicore system-on-chip, with the examples you get ``system_2x2_cccc``.
The nomenclature in all pre-packed systems first denotes the dimensions and then the instantiated tiles, here ``cccc`` as four compute tiles.
In our pre-built example, each compute tile has two CPU cores, meaning you have eight CPU cores in total.

Execute it again to get the hello world experience:

.. code:: sh

   $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore --meminit=hello.vmem

In our simulation all cores in the four tiles run the same software.
Before you shout "that's boring": you can still write different code depending on which tile and core the software is executed.
A couple of functions are useful for that:

- :c:func:`optimsoc_get_numct`: The number of compute tiles in the system
- :c:func:`optimsoc_get_numtiles`: The number of tiles (of any type) in the system
- :c:func:`optimsoc_get_ctrank`: Get the rank of this compute tile in this system.
  Essentially this is just a number that uniquely identifies a compute tile.

There are more useful utility functions like those available, find them in the file ``$OPTIMSOC/soc/sw/include/baremetal/optimsoc-baremetal.h``.

A simple application that uses those functions to do message passing between the different tiles is ``hello_mpsimple``.
This program uses the simple message passing facilities of the network adapter to send messages.
All cores send a message to core 0.
If all messages have been received, core 0 prints a message "Received all messages. Hello World!".

.. code:: sh

   # start from the the baremetal-apps source code directory
   cd hello_mpsimple
   make
   $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore --meminit=hello_mpsimple.vmem

Have a look what the software does (you find the code in ``hello_mpsimple.c``).
Let's first check the output of core 0.

.. code:: sh

   $> cat stdout.000
   # OpTiMSoC trace_monitor stdout file
   # [TIME, CORE] MESSAGE
   [               42844, 0] Wait for 3 messages
   [               48734, 0] Received all messages. Hello World!

Finally, let's have a quick glance at a more realistic application: ``heat_mpsimple``.
You can find it in the same place as the previous applications, ``hello`` and ``hello_mpsimple``.
The application calculates the heat distribution in a distributed manner.
The cores coordinate their boundary regions by sending messages around.

Can you compile this application and run it?
Don't get nervous, the simulation can take a couple of minutes to finish.
Have a look at the source code and try to understand what's going on.
Also have a look at the ``stdout`` log files.
Core 0 will also print the complete heat distribution at the end.

Observing Software During Execution: The Debug System
=====================================================

Up to now, you have seen the output of the software that runs on your SoC.
And you had a look deep into the inner works of the SoC by looking at the waveforms.

In a real-world system, you need something in between: a way to observe the software as it executes on a chip, but without observing or understanding all the signals inside the hardware.
This is what the debug system provides: hardware inside the chip which allows you to observe what's going on during software execution.

OpTiMSoC also comes with an extensive debug system.
In this section, we'll have a look at this system, how it works and how you can use it to debug your applications.
But before diving into the details, we'll have a short discussion of the basics which are necessary to understand the system.

Many developers know debugging from their daily work.
Most of the time it involves running a program inside a debugger like GDB or Microsoft Visual Studio, setting a breakpoint at the right line of code, and stepping through the program from there on, running one instruction (or one line of code) at a time.
This technique is what we call run-control debugging.
While it works great for single-threaded programs, it cannot easily be applied to debugging parallel software running on possibly heterogeneous many-core SoC.
Instead, the debug support in OpTiMSoC mainly relies on tracing.
Tracing does not stop or otherwise influence the SoC itself; it only "records" what's going on during software execution, and transmits this data to the developer.

The debug system consists of two main parts: the hardware part runs on the OpTiMSoC system itself and collects all data.
The other part runs on a developer's PC (often also called host PC) and controls the debugging process and displays the collected data.

After this introduction, let's make use of the debug system to obtain various traces.
Just like in the previous examples, our SoC hardware is still running in Verilator.
This tutorial works best if you have multiple terminal windows open at the same time, as we'll need to have multiple programs running at the same time.

So, open a new terminal (or a new tab inside your terminal), and start the simulation of the SoC hardware.

.. code:: sh

   $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore_debug

The first and most common task using the debug system is to run a program (just like we did before with the ``--meminit`` parameter).
Open a second terminal (leave the first one running!) and type

.. code:: sh

   osd-target-run -e hello.elf -vvv

``The osd-target-run`` command takes a couple seconds to finish, so don't get nervious.
If everything goes to plan ``osd-target-run`` just does its job: run the provided ELF file ``hello.elf`` on all CPUs in the system.
To do that, it internally performs the following steps:

- Connect to the simulation over TCP (on port 23000 and 23001)
- Halt all CPUs
- Load all memories in the system (since this is a 2x2 system, there are four memories) with the ELF file
- Reset and start all CPUs
- Close the TCP connection

If you switch back to the first console where you started the simulation you should see something like this:

.. code:: text

   $> $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore_debug
   Glip TCP DPI listening on port 23000 and 23001
   [                  24, 0] Software reset
   [                  24, 1] Software reset
   [                  24, 2] Software reset
   [                  24, 3] Software reset
   Client connected
   Disconnected
   [             1035016, 0] Terminated at address 0x0000ee38 (status:          0)
   [             1035016, 1] Terminated at address 0x0000ee38 (status:          0)
   [             1035016, 2] Terminated at address 0x0000ee38 (status:          0)
   [             1035016, 3] Terminated at address 0x0000ee38 (status:          0)
   - ../src/optimsoc_trace_monitor_trace_monitor_0/verilog/trace_monitor.sv:94: Verilog $finish
   - ../src/optimsoc_trace_monitor_trace_monitor_0/verilog/trace_monitor.sv:94: Verilog $finish
   - ../src/optimsoc_trace_monitor_trace_monitor_0/verilog/trace_monitor.sv:94: Second verilog $finish, exiting

Just like in the previous examples you can see the output of the program runs as captured by the simulation software in the files ``stdout.NNN`` in the directory where you started the simulation.

Reading the ``stdout`` files works great as long as OpTiMSoC runs in simulation -- but how can you access the program's output when it runs on an FPGA?
The answer is called "system trace", and you'll learn more about that in the next section.

System Traces
-------------
System traces (sometimes also called instrumentation traces) give software developers a tool to instruct their software running on OpTiMSoC to send information into a "system trace log".
By default, all calls to ``printf()`` result in an entry in the system trace.
(See the discussion above for how this works.)
This system trace log can then be captured on the host and displayed.
To capture a system trace from the system we'll again use the ``osd-target-run`` tool:

.. code:: sh

   osd-target-run -e hello.elf --systrace -vvv

Just like before, ``osd-target-run`` initializes the memories and starts the CPUs.
It then starts recording system traces until you press CTRL-C to end the trace collection.
(Yes, you need to abort the program by pressing CTRL-C! It will not terminate itself.)
After roughly 20 seconds, you can press CTRL-C to stop collecting traces.
Now you can analyze the collected traces in the same directory you ran ``osd-target-run`` in.
The files ``systrace.print.NNNN.log`` contain the ``printf()`` output of the program.
These files are generated by analyzing the raw system log events, which are recorded in ``systrace.NNNN.log``.


Core Function Traces
--------------------

If you need more insight into a program than system traces provide, or want to get insight into a program which isn't instrumented to generate system traces, core function traces come to help.
These traces are recording every call of a function and every return from it, resulting in traces which allow you to understand which parts of your program have been called.

To obtain a core trace use the following command:

.. code:: sh

   osd-target-run -e hello.elf --coretrace -vvv

Just like in the previous example, you need to stop the trace collection by pressing CTRL-C.
You can then view the traces in the ``coretrace.NNNN.log`` files.

This completes our short trip through the debug system.
Knowing about it will be of great help when we move on to the next step: running OpTiMSoC on an FPGA.


Our SoC on an FPGA
==================

Welcome to the fun of real hardware!
Before we can get started, you need to clarify some prerequisites.

Prerequisites: FPGA board and Vivado
------------------------------------

This, of course, first means that you need borrow, buy or otherwise obtain an FPGA board.
In this tutorial, we use the Nexys 4 DDR board by Xilinx/Digilent.
It's not that expensive (of course, depending on your financial situation) and widely available.
If you need help obtaining one, let us know - maybe we can help out in some way.

Additionally you need to download and install the Xilinx Vivado tool (the cost-free WebPack license is sufficient).
We used the |requirement_versions.vivado| version when preparing this tutorial; we strongly recommend you also use this exact version.

Once you have obtained the FPGA board, connect it to the PC on the "PROG UART" USB connection.
You don't need to connect any additional power supply.

Programming the FPGA
--------------------

With the board connected, we can program (or "flash") the FPGA with our hardware design, the *bitstream*.
The OpTiMSoC release contains pre-built bitstreams for the single compute tile system, meaning we can start directly with programming the FPGA.

There are two ways to program the device: using the Vivado GUI, or using the command line.

Programming the FPGA with the Vivado GUI
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Open Vivado (e.g. by typing ``vivado`` into a terminal window)
- On the welcome screen, click on "Hardware Manager"
- Ensure that your Nexys 4 DDR board is plugged into your PC and is turned on.
- Click on "Open Target" in the green bar on the top, and then on "Auto Connect"
- Now click on "Program Device" in the same green bar and select the only option ``xc7a100t_0`` (that's the FPGA on the board).
- In the dialog window, select the bitstream file. We'll start directly with the larger 2x2 system, you can find the bitstream in
   ``$OPTISMOC/examples/fpga/nexys4ddr/compute_tile/compute_tile_nexys4ddr_singlecore.bit``.
- You can leave the other field "Debug probes file" empty.
- Click on "Program" to download the bitstream onto the FPGA.

After a couple of seconds, your FPGA contains the SoC hardware and is ready to be used.

Programming the FPGA on the Command Line
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   optimsoc-pgm-fpga $OPTIMSOC/examples/fpga/nexys4ddr/compute_tile/compute_tile_nexys4ddr_singlecore.bit xc7a100t_0


Connecting
----------

In the previous tutorials, we have already seen the debug infrastructure and connected to it over TCP.
We now use the same tools to connect to our SoC, but this time we connect to the FPGA using UART.
Fortunately, you don't need to connect any additional cables; the USB cable that you just used to program the FPGA is also the serial connection.

First, check which serial port was assigned to the board.
Usually the easiest way is to do a

.. code:: sh

   ls /dev/ttyUSB*

If you have only the Nexys 4 DDR board connected, you'll see only one device, e.g. ``/dev/ttyUSB1``.
Make note of this device name, and replace it accordingly in all the following steps in this tutorial.


Running Software
----------------


Now that you've connected to the system, can you run software on it?
Just like in the previous chapter we'll use the ``osd-target-run`` tool, this time passing it some paramters to connect to the FPGA instead to a simulation.

.. code:: sh

   osd-target-run -e hello.elf -b uart -o device=/dev/ttyUSB1,speed=12000000 --coretrace --systrace --verify -vvv
   # let it run for a couple of seconds, then press CTRL-C to stop collecting traces

When you run software, you'll notice two things: first, the output is the same as you've already seen when running the system in simulation.
But: it's much faster. The FPGA runs at 50 MHz, which is still quite slow compared to current desktop processors, but still much faster than the simulation.

Before we end, let's discuss one more topic which helps you in writing good software for OpTiMSoC: message passing.

Make Message Passing More Simple
================================

So far the example programs you have seen used the low level message passing buffers to exchange data between the tiles.
You may remember that exchanging this data involved forming and parsing messages including the low level network-on-chip details.

To abstract from these low level details and to encapsulate certain extensions OpTiMSoC comes with the message passing library (``libmp``).
It is a rather simple, straight-forward message passing API.
Two different styles of communication are supported: message-oriented and connection-oriented.
Message-oriented communication is preferred when you have spurious communication between many different communication partners.
Connection-oriented communication is preferred when you have a fixed setup of channels between communication partners.

In this part of the tutorial you will learn the basic usage of the message passing library using message-oriented communication.
In the ``baremetal-apps`` you can find the ``hello_mp`` example.
Inspecting ``hello_mp.c`` you can see that it is much less code than the low level example from before.

Lets have a look at how it works. It starts with initializing the hardware and software:

.. code:: c

   optimsoc_init(0);
   optimsoc_mp_initialize(0);

The parameters of those functions can be ignored for now.
After calling those functions you can use the message passing library.

Communication in the message passing library takes place between so called endpoints.
In the next step we create an endpoint in each tile:

.. code:: c

   optimsoc_mp_endpoint_handle ep;
   optimsoc_mp_endpoint_create(&ep, 0, 0, OPTIMSOC_MP_EP_CONNECTIONLESS, 2, 0);

:c:type:`optimsoc_mp_endpoint_handle` is the opaque type used to identify an endpoint in your code.
You create and initialize the endpoint by calling :c:func:`optimsoc_mp_endpoint_create` that takes a reference to this handle as first parameter.
The second and third parameter initialize the endpoint with a node and port.
Each endpoint is globally addressable with its ``(tile, node, port)`` identifier.
In our case the node 0 and port 0 endpoint is created in each tile.

The remaining parameters of :c:func:`optimsoc_mp_endpoint_create` configure the endpoint.
By using :c:type:`OPTIMSOC_MP_EP_CONNECTIONLESS` we create it to receive messages from arbitrary tiles.
The last two parameters configure the number of messages it can hold and the maximum message size (``0`` says it is the default).

Now the code of the example diverts again, all but tile 0 execute:

.. code:: c

   optimsoc_mp_endpoint_handle ep_remote;
   optimsoc_mp_endpoint_get(&ep_remote, 0, 0, 0);

   optimsoc_mp_msg_send(ep, ep_remote, (uint8_t*) &rank, sizeof(rank));

So what they do is to define a second endpoint.
But in this case it is not locally generated but points to a remote endpoint.
It is the one we want to send a message too: tile 0, node 0, port 0.
What happens under the hood it blocks until the remote endpoint is created and ready and than stores some information locally.
In the final step the software sends a word to the remote endpoint using the local endpoint for sending.

In tile zero the software waits to receive all messages using:

.. code:: c

   optimsoc_mp_msg_recv(ep, (uint8_t*) &remote, 4, &received);

You can now run the example using:

.. code:: sh

   # start from the the baremetal-apps source code directory
   cd hello_mp
   make
   $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_singlecore --meminit=hello_mp.vmem

::

   TOP.tb_system_2x2_cccc.u_system.gen_ct[0].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.ibus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[0].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.dbus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[1].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.ibus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[1].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.dbus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[2].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.ibus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[2].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.dbus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[3].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.ibus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   TOP.tb_system_2x2_cccc.u_system.gen_ct[3].u_ct.gen_cores[0].u_core.u_cpu.bus_gen.dbus_bridge: Wishbone bus IF is B3_REGISTERED_FEEDBACK
   [               50602, 1] Terminated at address 0x00011364 (status:          0)
   [               65212, 2] Terminated at address 0x00011364 (status:          0)
   [              169848, 3] Terminated at address 0x00011364 (status:          0)
   [              180834, 0] Terminated at address 0x00011364 (status:          0)

You can see that the cores in the tiles have terminated over a longer time frame.
Core 0 exits as last after printing to the output in ``stdout.000``:

::

   # OpTiMSoC trace_monitor stdout file
   # [TIME, CORE] MESSAGE
   [               72050, 0] Received from 1
   [               78792, 0] Received from 2
   [              179834, 0] Received from 3

Under the Hood: Simulation Tracing
==================================

So far we have used "printf-debuggging", the most popular way of debugging embedded programs.
It is pretty simple to add, but not very performant or structured.
We have introduced the concept of tracing before.
In OpTiMSoC we make excessive use of software instrumentation and other tracing techniques.

In this tutorial you will see how to use the tracing infrastructure of the verilator simulations to better understand what is happening.
We want to have a look at the messages exchanged between tiles and how they relate to software calls.

You must have ``babeltrace`` installed to use this tutorial.
The OpTiMSoC simulations are capable of generating traces in the Common Trace Format (CTF).
Just run the simulation from before again with tracing enabled:

.. code:: sh

   $OPTIMSOC/examples/sim/system_2x2_cccc/system_2x2_cccc_sim --meminit=hello_mp.vmem --trace

You will see that a directory has been created, named in the form ``ctf-yyyymmdd-hhmmss``.
The folder contains a timestamp so that you don't accidentally overwrite previous experiments.
There you can find two subfolders: ``noc`` contains a trace of the packets transmitted in the network-on-chip, and ``sw`` are the infamous software traces.

Let's have a look at the software trace first:

.. code:: sh

   babeltrace --clock-seconds ctf-yyyymmdd-hhmmss/sw/

Babeltrace will print a full trace of all events from all cores:

::

   [0.000000024] (+?.?????????) 0 reset: { cpu_id = 0 }
   [0.000000024] (+0.000000000) 0 reset: { cpu_id = 1 }
   [0.000000024] (+0.000000000) 0 reset: { cpu_id = 2 }
   [0.000000024] (+0.000000000) 0 reset: { cpu_id = 3 }
   [0.000032746] (+0.000032722) 0 ep_create: { cpu_id = 0 }, { handle = 98848 }
   [0.000032778] (+0.000000032) 0 ep_create: { cpu_id = 1 }, { handle = 98848 }
   [0.000032806] (+0.000000028) 0 ep_create: { cpu_id = 2 }, { handle = 98848 }
   [0.000032834] (+0.000000028) 0 ep_create: { cpu_id = 3 }, { handle = 98848 }
   [0.000034918] (+0.000002084) 0 exception_enter: { cpu_id = 1 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000034946] (+0.000000028) 0 exception_enter: { cpu_id = 2 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000034974] (+0.000000028) 0 exception_enter: { cpu_id = 3 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000036982] (+0.000002008) 0 exception_leave: { cpu_id = 1 }
   [0.000037010] (+0.000000028) 0 exception_leave: { cpu_id = 2 }
   [0.000037038] (+0.000000028) 0 exception_leave: { cpu_id = 3 }
   [0.000037140] (+0.000000102) 0 ep_get_enter: { cpu_id = 1 }, { domain = 0, node = 0, port = 0 }
   [0.000037168] (+0.000000028) 0 ep_get_enter: { cpu_id = 2 }, { domain = 0, node = 0, port = 0 }
   [0.000037196] (+0.000000028) 0 ep_get_enter: { cpu_id = 3 }, { domain = 0, node = 0, port = 0 }
   [0.000037626] (+0.000000430) 0 exception_enter: { cpu_id = 0 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000039582] (+0.000001956) 0 exception_enter: { cpu_id = 1 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000040616] (+0.000001034) 0 exception_enter: { cpu_id = 2 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000041176] (+0.000000560) 0 exception_leave: { cpu_id = 1 }
   [0.000041306] (+0.000000130) 0 ep_get_leave: { cpu_id = 1 }, { handle = 98848 }
   [0.000041638] (+0.000000332) 0 exception_enter: { cpu_id = 3 }, { cause = ( "INTERRUPT" : container = 8 ) }
   ... (we've skipped some output here) ...
   [0.000167144] (+0.000000258) 0 msg_data_enter: { cpu_id = 3 }, { handle = 99112, address = 0, size = 4 }
   [0.000167686] (+0.000000542) 0 exception_enter: { cpu_id = 0 }, { cause = ( "INTERRUPT" : container = 8 ) }
   [0.000168432] (+0.000000746) 0 msg_data_leave: { cpu_id = 3 }, { handle = 99112 }
   [0.000169726] (+0.000001294) 0 exception_leave: { cpu_id = 0 }

What you essentially see are two kinds of event messages.
There are the ``exception_enter`` and ``exception_leave`` events that are helpful to understand the impact of the handling of the message passing on the software execution.
All other events are emitted from the software actually and in the majority of cases represent the span of an interesting function call.

For example lets first have a look cpu 1.
At time 0.000037140 the event `ep_get_enter` is emitted, with the parameters to retrieve a remote endpoint.
In the source code of the library we find the implementation of the function :c:func:`optimsoc_mp_endpoint_get` ends up in:

.. code:: c

   struct endpoint *control_get_endpoint(uint32_t domain, uint32_t node, uint32_t port) {
     struct endpoint *ep;
     while (!optimsoc_mp_simple_ctready(domain, 0));
     trace_ep_get_req_begin(domain, node, port);

Later in that function the `ep_get_leave` is emitted (time 0.000041306).
Let us try to further understand what happens.
For that we have a look at the second trace:

.. code:: sh

   babeltrace --clock-seconds ctf-yyyymmdd-hhmmss/sw/

It generates output like this:

::

   [0.000034854] (+?.?????????) 0 mpbuffer_control_req: { link = 1, src = 1, dest = 0 }
   [0.000034872] (+0.000000018) 0 mpbuffer_control_resp: { link = 5, src = 0, dest = 1 }, { status = 1 }
   [0.000034882] (+0.000000010) 0 mpbuffer_control_req: { link = 1, src = 2, dest = 0 }
   [0.000034900] (+0.000000018) 0 mpbuffer_control_resp: { link = 9, src = 0, dest = 2 }, { status = 1 }
   [0.000034916] (+0.000000016) 0 mpbuffer_control_req: { link = 1, src = 3, dest = 0 }
   [0.000034940] (+0.000000024) 0 mpbuffer_control_resp: { link = 13, src = 0, dest = 3 }, { status = 1 }
   [0.000037588] (+0.000002648) 0 mp_getep_req: { link = 1, src = 1, dest = 0 }, { node = 0, port = 0 }
   [0.000037616] (+0.000000028) 0 mp_getep_req: { link = 1, src = 2, dest = 0 }, { node = 0, port = 0 }
   [0.000037650] (+0.000000034) 0 mp_getep_req: { link = 1, src = 3, dest = 0 }, { node = 0, port = 0 }
   [0.000039566] (+0.000001916) 0 mp_getep_resp_ack: { link = 7, src = 0, dest = 1 }, { handle = 98848 }
   [0.000040600] (+0.000001034) 0 mp_getep_resp_ack: { link = 11, src = 0, dest = 2 }, { handle = 98848 }
   [0.000041622] (+0.000001022) 0 mp_getep_resp_ack: { link = 15, src = 0, dest = 3 }, { handle = 98848 }
   [0.000044290] (+0.000002668) 0 mp_msgalloc_req: { link = 1, src = 1, dest = 0 }, { handle = 98848, size = 4 }
   [0.000045328] (+0.000001038) 0 mp_msgalloc_req: { link = 1, src = 2, dest = 0 }, { handle = 98848, size = 4 }
   [0.000045946] (+0.000000618) 0 mp_msgalloc_resp_ack: { link = 7, src = 0, dest = 1 }, { ptr = 1 }
   [0.000046352] (+0.000000406) 0 mp_msgalloc_req: { link = 1, src = 3, dest = 0 }, { handle = 98848, size = 4 }
   [0.000046990] (+0.000000638) 0 mp_msgalloc_resp_nack: { link = 11, src = 0, dest = 2 }
   [0.000047976] (+0.000000986) 0 mp_msgalloc_resp_nack: { link = 15, src = 0, dest = 3 }
   [0.000048418] (+0.000000442) 0 mp_msgdata: { link = 1, src = 1, dest = 0 }, { handle = 98848, address = 0, offset = 0 }
   [0.000048996] (+0.000000578) 0 mp_msgcomplete: { link = 1, src = 1, dest = 0 }, { handle = 98848, address = 0, size = 4 }
   [0.000058982] (+0.000009986) 0 mp_msgalloc_req: { link = 1, src = 2, dest = 0 }, { handle = 98848, size = 4 }
   [0.000059974] (+0.000000992) 0 mp_msgalloc_req: { link = 1, src = 3, dest = 0 }, { handle = 98848, size = 4 }
   [0.000060584] (+0.000000610) 0 mp_msgalloc_resp_ack: { link = 11, src = 0, dest = 2 }, { ptr = 1 }
   [0.000061600] (+0.000001016) 0 mp_msgalloc_resp_nack: { link = 15, src = 0, dest = 3 }
   [0.000063028] (+0.000001428) 0 mp_msgdata: { link = 1, src = 2, dest = 0 }, { handle = 98848, address = 1, offset = 0 }
   [0.000063606] (+0.000000578) 0 mp_msgcomplete: { link = 1, src = 2, dest = 0 }, { handle = 98848, address = 1, size = 4 }
   [0.000163576] (+0.000099970) 0 mp_msgalloc_req: { link = 1, src = 3, dest = 0 }, { handle = 98848, size = 4 }
   [0.000165214] (+0.000001638) 0 mp_msgalloc_resp_ack: { link = 15, src = 0, dest = 3 }, { ptr = 1 }
   [0.000167670] (+0.000002456) 0 mp_msgdata: { link = 1, src = 3, dest = 0 }, { handle = 98848, address = 0, offset = 0 }
   [0.000168248] (+0.000000578) 0 mp_msgcomplete: { link = 1, src = 3, dest = 0 }, { handle = 98848, address = 0, size = 4 }

The first messages are control messages that check if the message passing is properly setup on the remote.
After that you can track the remote call to retrieve the endpoint handle:

- At 0.000037140 we already observed how the software enters the function on core 1.
- At 0.000037588 the network message relating to the function call to core 0.
- At 0.000037626 we can see how the arrival of the message raises interrupt exception in the software trace at core 0.
- At 0.000039566 the acknowledgement response network message including the handle to the endpoint is sent by core 0.
- At 0.000039582 this acknowledgement leads to an interrupt exception in the software at core 1.
- At 0.000041176 the interrupt routine completes after processing the message and updating the information.
- At 0.000041306 the function call to retrieve the remote endpoint is left.

Run Linux on OpTiMSoC
=====================

Up to now all software running on OpTiMSoC was "baremetal" software, similar to software run on a microcontroller.
For many purposes "baremetal" software is sufficient.
However, if you want to write more advanced software an operating system (OS) can help: it provides task management (scheduling), separates resources between tasks, and provides standardized interfaces which are expected by many of today's applications (e.g. pthreads).
For many, the operating system of choice is Linux, and it's natively supported by OpTiMSoC.
This tutorial section explores how to build a Linux "image,". i.e. a binary which contains both the Linux kernel (the actual operating system), together with a root filesystem containing all userspace components.

.. code:: sh

   # get the OpTiMSoC buildroot configuration (a "br2-external tree")
   git clone https://github.com/optimsoc/optimsoc-buildroot.git
   cd optimsoc-buildroot
   OPTIMSOC_BUILDROOT_DIR=$PWD
   OPTIMSOC_BUILDROOT_VERSION=$(cat $OPTIMSOC_BUILDROOT_DIR/buildroot_version)
   cd .. # back to your source directory

   # get buildroot itself
   git clone https://git.busybox.net/buildroot
   cd buildroot
   git checkout $OPTIMSOC_BUILDROOT_VERSION
   make BR2_EXTERNAL=$OPTIMSOC_BUILDROOT_DIR optimsoc_computetile_singlecore_defconfig
   make

This leaves a file ``output/images/vmlinux`` in the buildroot directory, which is in fact a regular ELF file for OpenRISC, which can be loaded on the system like a baremetal application.
To see the output of the Linux during boot, and to have a console to interact with the Linux system we make use of the UART device emulation provided by Open SoC Debug, and built into the ``compute_tile`` designs.

To continue with this tutorial we use the ``compute_tile`` design with a with a single core and the debug system for the Nexys 4 DDR board.
You can find this design in the folder ``$OPTIMSOC/examples/fpga/nexys4ddr/compute_tile/compute_tile_nexys4ddr_singlecore``.

.. code:: sh

    # Program the FPGA on the Nexys 4 DDR board
    optimsoc-pgm-fpga $OPTIMSOC/examples/fpga/nexys4ddr/compute_tile/compute_tile_nexys4ddr_singlecore.bit xc7a100t_0

Now you can load the Linux image on the FPGA.
See notes earlier in this tutorial for a discussion on the correct parameters for ``osd-target-run``.

.. code:: sh

   osd-target-run -e YOUR_BUILDROOT_DIR/output/images/vmlinux -b uart -o device=/dev/ttyUSB1,speed=12000000 --systrace -vvv

Watch the output of this command.
If all goes well the output should contain a line similar to ``libosd: DEM-UART pseudo-terminal available at /dev/pts/19``.
Keep note of the device file (starting with ``/dev/pts/``), you'll need this path to connect to the Linux console on the OpTiMSoC system.

To connect, open a second terminal window on your machine, and use ``screen`` to connect to the remote console (use the appropriate device name as displayed by ``osd-target-run`` instead of ``/dev/pts/19``):

.. code:: sh

   screen /dev/pts/19

You should now see the output of Linux booting, and as soon as the boot process is done you can log into the system as ``root`` user (no password is required).
You can now interact with the system as it would be a normal Linux system.

If you have some time to spare, how about playing a round of pacman?

.. code:: sh

   # convince Linux that our console supports colors
   stty cols 80 rows 80
   export TERM=linux

   # and run pacman
   /usr/games/pacman4linux
