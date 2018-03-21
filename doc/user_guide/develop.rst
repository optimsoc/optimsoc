****************
Develop OpTiMSoC
****************

After you have worked through some, or even all, of the tutorials in the previous chapter, you're now ready to bring your own ideas to live using OpTiMSoC.
This chapter gives you a quick introduction on how to setup your development environment, like editors and the revision control system, and how to contribute back to the OpTiMSoC project.

We assumed in this whole tutorial that you are working on Linux.
While it is certainly possible to use Windows or OS~X for development, we cannot provide help for those systems and you're on your own.

Building Hardware
=================

When building software, engineers have gotten used to tools like ``make``, CMake and similar build systems.
Such build tools ensure that all dependencies of a software project are met, and then start the various tools (such as the compiler, linker, etc.) to produce the output files, e.g. the program binary.
In the hardware world, no standard tool for this job exists.
A new, but very promising contestent in this sector is is *FuseSoC*.

FuseSoC allows developers to write *core files*: short declarative files in an INI-like format that describe which components are required to build a hardware design.
When you look around in ``$OPTIMSOC/soc/hw`` you'll find such core files for all components that make up the SoC.
But the core files not only describe the modules inside the SoC design, they are also used to describe the toplevel SoC.

For example, let's have a look at the file \path{$OPTIMSOC_SOURCE/examples/sim/compute_tile/compute_tile_sim.core} inside the OpTiMSoC source tree (it's not installed!).
In there you find all dependencies that are needed to build the system with only one compute tile.
You also find the toplevel files that are used to simulate the system in Verilator and in XSIM (the Vivado built-in simulator).

The great benefit of using FuseSoC is that you can now simply compile and run the system with one simple command.

Before we start, two notes:

- We set an environment variable (``$FUSESOC_CORES``) during the installation that makes FuseSoC find all OpTiMSoC hardware modules. You do not need to add a special configuration for this. However, the examples inside ``$OPTIMSOC_SOURCE`` are not part of this search path.
- You can call ``fusesoc`` from any directory. We recommend **not** calling FuseSoC from inside your source directory. (This allows you to just delete the build folder and retain a clean source folder.)

So let's look at a couple of examples how to build a SoC hardware with fusesoc.

.. note:: All the examples require an OpTiMSoC source tree to be available at ``$OPTIMSOC_SOURCE``.

- Build and run a Verilator-based simulation of a single compute tile.

  .. code:: sh

     cd some/directory
     fusesoc --cores-root $OPTIMSOC_SOURCE/examples sim optimsoc:examples:compute_tile_sim

- Only build a Verilator simulation of a single compute tile

  .. code:: sh

     fusesoc --cores-root $OPTIMSOC_SOURCE/examples sim --build-only optimsoc:examples:compute_tile_sim

- Set the parameter ``NUM_CORES`` to 2 to create a system with two CPU cores inside the compute tile.
  You can have a look inside the top-level source file \path{$OPTIMSOC_SOURCE/examples/sim/compute_tile/tb_compute_tile.sv} for other parameters that are available.

  .. code:: sh

     fusesoc --cores-root $OPTIMSOC_SOURCE/examples sim --build-only optimsoc:examples:compute_tile_sim --NUM_CORES 2

- Synthesize a 2x2 system with four compute tiles for the Nexys 4 DDR board using Xilinx Vivado.
  This step requires Vivado to be installed and working, and a lot of time (approx. 30 minutes, depending on your machine).

  .. code:: sh

     fusesoc --cores-root $OPTIMSOC_SOURCE/examples build optimsoc:examples:system_allct_nexys4ddr --XDIM 2 --YDIM 2

- Now flash the bitstream that the previous step generated to the FPGA.

  .. code:: sh

     fusesoc --cores-root $OPTIMSOC_SOURCE/examples pgm optimsoc:examples:system_allct_nexys4ddr

Choosing an Editor/IDE
======================     

When editing code, an editor or IDE usually comes handy.
While there is clearly no "best" or even "recommended" editor or IDE, we will present two or our choices here, together with some settings that make working on OpTiMSoC a pleasant experience.
Feel free to adapt these recommendations to your personal preferences!

Eclipse
-------

Eclipse gives you a nice and integrated development across the different parts of the code base by using a couple of plugins.
But be aware, Eclipse likes memory and is not exactly "lightweight", but if you have enough memory available (in the area of 500~MB for Eclipse) it can be a very powerful and productive choice.

Installation and Basic Setup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

First of all, get Eclipse itself. Go to http://www.eclipse.org/downloads/ and get the "Eclipse IDE for C/C++ Developers" package or install it from your distribution's package manager.
All the following steps were tested with Eclipse Kepler (4.3).

Now start Eclipse and first go to *Help - Check for Updates*.
Install all available updates.

For Verilog syntax highlighting we use a plugin called "VEditor".
Go to "Help - Install New Software..." In the field "Work with" enter the URL of the installation site: http://veditor.sourceforge.net/update.
Now press the return key and after a couple of seconds, the entry "VEditor Plugin" appears below.
Select it and click on the "Next" button until the installation is finished.
To complete the process you need to restart Eclipse.

Every project has different preferences regarding the styling of the code.
Therefore every editor can be configured to some extend.
First, we'll set the general settings for Eclipse, and then for Verilog and C.

Start by clicking on *Window - Preferences* inside Eclipse.
There, choose *Editors - Text Editors*.
You should set the following settings:

- Check *Insert spaces for tabs*
- Check *Show print margin*
- Set the *Print margin column* to 80
- Check *Show line numbers*

Just leave the other settings as they are, or change them to your liking.

For the Verilog settings, go to *Verilog/VHDL Editor - Code Style*.
There, select *Space as Indent Character* and set the *Indent Size* to 3.

For the C style used in liboptimsochost and other libraries we have prepared a settings file.
Go to *C/C++ - Code Style - Formatter* and click on *Import...*, choose the settings file ``doc/resources/optimsoc-eclipse-cdt-codestyle.xml`` inside the OpTiMSoC source tree.
Now you should have a new profile "OpTiMSoC" in the list of profiles.
Choose this one for all work on the C code.

Creating the OpTiMSoC HDL Project
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now that all the basic settings are in place, we can create the projects inside Eclipse.
First, we will create a project for editing the HDL (Verilog) code.

In the *Project Explorer* (on the left side), right click and select *New - Project...*.
A new dialog window shows.
In this window, select *Verilog/VHDL - Verilog/VHDL project* and click *Next*.
Now enter a project name, e.g. "OpTiMSoC".
Uncheck the option *Use default location* and click on *Browse* to choose your OpTiMSoC source directory (the location where you cloned the Git repository to).

The OpTiMSoC source tree not only contains RTL code, but also the necessary software components like liboptimsochost and the OpTiMSoC GUI.
This code is better edited in a separate project and should be excluded from the project you just created.
To do that, choose *Project - Properties* from the main menu.
Navigate to *Resource - Resource Filters* and click on the *Add...* button.
There, choose the following settings:

- Choose *Exclude all* in the group *Filter type*
- Choose *Folders* in the group *Applies to*
- Check *All children (recursive)*
- In the group *File and Folder Attributes* select *Project Relative Path* *matches* ``src/sw``.

Now click on *OK* to finish editing the filter.

Then repeat the steps above to create a new resource filter but as path use ``src/sysc`` this time.

After you're done with the second filter, click *OK* again to close the dialog.

Creating a C Project
^^^^^^^^^^^^^^^^^^^^

Eclipse is also a great choice for editing C code.
As an example, we'll setup Eclipse for the OpTiMSoC baremetal library, ``libbaremetal``.

In the main menu, click on *File - New - Project*.
A dialog window is shown. There, nagivate to *C/C++ - Makefile Project with Existing Code* and click on the *Next* button.
Type ``libbaremetal`` as *Project Name* and click on *Browse...* to select the source code location of the library.
It should be inside your OpTiMSoC code in the folder ``src/soc/sw/baremetal-libs``.
Uncheck *C++* in the *Languages* group and select *GNU Autotools Toolchain* in the box below.
Now click on *Finish* to close the dialog.

Before you start coding, double-check if the code style settings are correct.
Select the newly created liboptimsochost project from the *Project Navigator* on the left, right-click and choose *Properties*.
Nagivate to *C/C++ General - Formatter* and check if ``OpTiMSoC`` is selected as style.
If not, click on *Enable project specific settings* and choose ``OpTiMSoC`` from the list (if there is no such entry, go back to the basic Eclipse setup and import the style file properly).

Emacs
-----

.. todo:: This section will be added shortly.

Verilog-mode
------------

Sometimes, writing Verilog means writing the same information twice in
different places of a source file, one example being the port of a module. To
save you as developer some typing time, a tool called *Verilog-mode* has been
invented. It allows you to specify comments inside your code where information
should be placed, and this information is then calculated and inserted
automatically. For more information about what it is and how it works, see
http://www.veripool.org/wiki/verilog-mode/.

Verilog-mode is used extensively throughout the project. Even though using it
is not required (the sources can be edited and compiled without it just fine),
it will save you a lot of time during development and is highly recommended.

Installation is rather easy, as it comes bundled with GNU Emacs. Simply install
Emacs as described above and you're ready to go. To support our coding style,
you will need to adjust the Emacs configuration (even though it is the Emacs
configuration, it also configures Verilog-mode).

Open the file ``~/.emacs`` and add the following lines at the end:

.. code:: lisp
	  
   (add-hook 'verilog-mode-hook '(lambda ()
     ;; Don't auto-insert spaces after ";"
     (setq verilog-auto-newline nil)
     ;; Don't indent with tabs!
     (setq indent-tabs-mode nil)))
   (add-hook 'verilog-mode-hook '(lambda ()
     ;; Remove any tabs from file when saving
     (add-hook 'write-file-functions (lambda()
       (untabify (point-min) (point-max))
       nil))))

If you also use Emacs as your code editor, Verilog-mode is already
enabled and you can read through the `documentation
<http://www.veripool.org/wiki/verilog-mode/Documentation>` to learn
how to use it.

Verilog-mode in Eclipse
^^^^^^^^^^^^^^^^^^^^^^^
Even if you use Eclipse, you do not need to switch editors to get the
benefits of Verilog-mode; you can run Verilog-mode in batch mode to
resolve all the AUTO comments. This will require some manual setup, but
afterwards it can be used quite easily.

First, you need to figure out where your ``verilog-mode.el`` or
``verilog-mode.elc`` file is located. If you want to use the
Verilog-mode which is part of your Emacs installation, it is probably located
somewhere in ``/usr/share/emacs``, e.g.
``/usr/share/emacs/24.3/lisp/progmodes/verilog-mode.elc`` on Ubuntu 14.04.
You can run

.. code:: sh

   $> find /usr/share/emacs -name 'verilog-mode.el*'

to search for it. If you found it, write down the path as we'll need it later.
If you installed Verilog-mode from source, just note the path where you put
your ``verilog-mode.el`` file (e.g. somewhere in your home directory).

In Eclipse, we will setup Verilog-mode as "Builder". To do so, click in the
main menu on *Project - Properties* and nagivate to
*Builders*. There, click on the *New...* button and select
*Program* as configuration type in the shown dialog. After pressing
*OK*, enter "verilog-mode" into the field *Name*. In the *Main*
tab, write ``/usr/bin/emacs`` into the field *Location*. Leave the field
*Working Directory* empty and enter the following string into the field
*Arguments*:

.. code:: sh
	  
   --batch --no-site-file -u ${env_var:USER}
   -l /usr/share/emacs/24.3/lisp/progmodes/verilog-mode.elc
   "${selected_resource_loc}" -f verilog-auto -f save-buffer

Replace the path to the ``verilog-mode.el`` or ``verilog-mode.elc``
file with your own path you found out above.

Now, switch to the tab *Refresh*, check the box *Refresh resources
upon completion* and select *The selected resource*. Since we don't need
to change anything in the last two tabs, you can now close the dialog by
clicking on the *OK* button and on *OK* again to close the project
properties dialog.

To test if it all works, navigate to
``src/rtl/compute_tile_dm/verilog/compute_tile_dm.v`` and change the word
"Outputs" in the comment right at the beginning of the file to something
else. Then press CTRL-B (or go to *Project - Build All*) and
after a couple of seconds, you should see the word "Outputs" restored and
some output messages in the *Console* view at the bottom. Also check if
there were no tabs inserted (e.g. at the instantiation of ``u_core0``). If
there are tabs then you probably did not setup your ``~/.emacs`` file
correctly.
