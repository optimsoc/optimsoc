*****************************
Installation & Configuration
*****************************

Installing an OpTiMSoC release should not take more than 10 minutes (if you have a decent internet connection), so let's get started!

System Requirements
===================

Please first check the system requirements.

- Supported Linux distribution: Ubuntu 16.04 LTS on x86_64
- 20 GB disk space (mostly for Xilinx Vivado and other tools)
- 4 GB or more of RAM helps greatly (especially during FPGA synthesis)
- We recommend not using a VM, but running directly on the hardware.
  Using a VM is possible, but will result in significantly slower compilation and synthesis runs.
- (optional, only if you want to do FPGA synthesis) Xilinx Vivado 2016.2 (the free WebPack edition is sufficient for most use cases)

.. note:: Some external tools, especially EDA tools for synthesis and simulators, might be required that are in some cases proprietary and cost some money.
   Although OpTiMSoC is developed at an university with access to many EDA tools, we aim to always provide tool flows and support for open and free tools.
   But especially when it comes to synthesis such alternatives are even not available.

Dependencies
============

In OpTiMSoC, we try to use only external dependencies which are known to be stable and readily available.
Some can be installed as distribution packages, and others can be downloaded as binaries from us.

First, install all required packages from Ubuntu.

.. code:: sh

   sudo apt-get -y install tcl libusb-1.0-0-dev \
     libboost-dev libelf-dev swig \
     python3 python3-pip python3-venv libreadline-dev python-dev \
     curl git build-essential autoconf automake \
     libtool pkg-config python-cffi libffi-dev

   # optional, but highly recommended: a waveform viewer
   sudo apt-get -y install gtkwave

We build our systems with FuseSoC_, a package manager for RTL designs.
It automatically arranges the sources and writes project files for our systems.
You can install it simply from the Python package index:

.. code:: sh

   sudo pip3 install "fusesoc>=1.6.1"

To run the tests shipped with OpTiMSoC pytest is required.
Install it through pip (the distribution packages are too old):

.. code:: sh

   sudo pip3 install pytest

Additionally, we need two things which are not available as Ubuntu packages right now: a recent version of Verilator, and the ``or1k-elf-multicore`` toolchain (compiler, C library, debugger, etc.).
Install them with our binary installation script:

.. code:: sh

   # if it does not exist yet: prepare the /opt/optimsoc directory
   sudo mkdir /opt/optimsoc
   sudo chown $USER /opt/optimsoc

   # download and install the prebuilt tools
   curl -O https://raw.githubusercontent.com/optimsoc/prebuilts/master/optimsoc-prebuilt-deploy.py
   sudo python optimsoc-prebuilt-deploy.py -d /opt/optimsoc verilator or1kelf

To use the prebuilt tools some environment variables need to be adjusted.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. code:: sh

   source /opt/optimsoc/setup_prebuilt.sh

.. note:: Automatically load the prebuilts in every new terminal session by adding it to your ``~/.bashrc`` file:

   .. code:: sh

      echo 'source /opt/optimsoc/setup_prebuilt.sh' >> ~/.bashrc

Install OpTiMSoC
================

Now that all preparations are done, you are now ready to install the OpTiMSoC framework itself.
There are two options: either, you can install a prebuilt release package, or you can build OpTiMSoC yourself from the sources.
We recommend starting with a binary release installation, and move to a custom-built version only after you verified that everything works.

Recommended: OpTiMSoC binary releases
-------------------------------------

The most simple way to get started is with the release packages.
You can find the OpTiMSoC releases here: https://github.com/optimsoc/sources/releases.
With the release you can find the distribution packages that can be extracted into any directory and used directly from there.
The recommended default is to install OpTiMSoC into ``/opt/optimsoc``.
There are two packages: the ``base`` package contains the programs, libraries and tools to get started.
The ``examples`` package contains prebuilt example systems (both in simulation and FPGA bitstreams) for the real quick start.

.. ifconfig:: lastversion != version

   .. note:: This documentation was generated for a development version and you cannot download prebuild packages for it.
      Some parts of this documentation will vary from the release documentation and examples not work anymore.
      Please refer to the documentation matching the last release that you can find here: https://optimsoc.org/docs.

To install the |lastversion| release into ``/opt/optimsoc``, run the following commands:

.. parsed-literal::

   wget |dl_base|
   wget |dl_examples|
   tar -xf optimsoc-|lastversion|-base.tar.gz -C /opt/optimsoc
   tar -xf optimsoc-|lastversion|-examples.tar.gz -C /opt/optimsoc

To use OpTiMSoC multiple environment variables need to be set.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. parsed-literal::

   cd /opt/optimsoc/|lastversion|
   source optimsoc-environment.sh

.. note:: Automatically load the OpTiMSoC environment in every new
   terminal session by adding it to your ``~/.bashrc`` file:

   .. parsed-literal::

      echo 'cd /opt/optimsoc/|lastversion|; source optimsoc-environment.sh' >> ~/.bashrc

Installation complete!

You are now ready to go to the :doc:`tutorials`.

Alternative: Build OpTiMSoC from sources
----------------------------------------

You can also build OpTiMSoC from the sources.
This options usually becomes standard if you start developing for or around OpTiMSoC.
The build is done from one git repository: https://github.com/optimsoc/sources.

It is generally a good idea to understand git, especially when you plan to contribute to OpTiMSoC.
Nevertheless, we will give a more detailed explanation of how to get your personal copies of OpTiMSoC and (potentially) update them.

First, you need some additional tools (the "build dependencies"):

.. code:: sh

   sudo apt-get -y install doxygen texlive texlive-latex-extra texlive-fonts-extra

Then get the sources from git:

.. parsed-literal::

   git clone https://github.com/optimsoc/sources.git optimsoc-sources
   cd optimsoc-sources
   # optional: checkout a release version
   git checkout |lastversion|

Now you're ready to build OpTiMSoC.

OpTiMSoC contains a Makefile which controls the whole build process.
Building is as simple as calling (inside the top-level source directory that you just got from git)

.. code:: sh

   make

By default this also builds the documentation, the Verilator examples and the FPGA bitstreams (which requires Xilinx Vivado to be working).
You can disable some features by passing variables to the ``Makefile``:

.. code:: sh

   # only build Verilator examples, but no bitstreams and no docs
   make BUILD_EXAMPLES=yes BUILD_EXAMPLES_FPGA=no BUILD_DOCS=no

If you need even more fine-grained control over the build process, call the build script ``tools/build.py`` directly.

Running ``tools/build.py --help`` will give you a list of all available options.

After the build process, all build artifacts are located in ``objdir/dist``.
You can either use OpTiMSoC directly from there (good during development), or copy it to a more suitable installation location in ``/opt/optimsoc/VERSION`` by running

.. code:: sh

   make install

You can also modify the target directory using environment variables passed to ``make``.
This is especially useful if you don't have enough permissions to write to ``/opt/optimsoc``.

- Use ``INSTALL_PREFIX`` to change the installation prefix from ``/opt/optimsoc`` to something else.
  The installation will then go into ``INSTALL_PREFIX/VERSION``.
- Use ``INSTALL_TARGET`` to fully override the installation path.
  The installation will then go exactly into this directory.

.. code:: sh

   # use INSTALL_PREFIX to install into ~/optimsoc/VERSION
   make install INSTALL_PREFIX=~/optimsoc

   # full control for special cases: use INSTALL_TARGET
   # to install into ~/optimsoc-testversion
   make install INSTALL_TARGET=~/optimsoc-testversion

Independent of which directory you chose, to use OpTiMSoC multiple environment variables need to be set.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. code:: sh

   cd YOUR_INSTALLATION_DIR
   source optimsoc-environment.sh

See the binary installation description above for information on how to make this change permanent.

OpTiMSoC is now ready to be used and you can continue with the :doc:`tutorials`.


.. _FuseSoC: https://github.com/olofk/fusesoc
