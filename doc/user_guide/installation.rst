****************************
Installation & Configuration
****************************

Installing an OpTiMSoC release should not take more than 10 minutes (if you have a decent internet connection), so let's get started!

System Requirements
===================

Please first check the system requirements.

- Supported Linux distribution: Ubuntu 16.04 LTS on x86_64
- 20 GB disk space (mostly for Xilinx Vivado and other tools)
- 4 GB or more of RAM helps greatly (especially during FPGA synthesis)
- We recommend not using a VM, but running directly on the hardware.
  Using a VM is possible, but will result in significantly slower compilation and synthesis runs.
- (optional, only if you want to do FPGA synthesis) Xilinx Vivado |requirement_versions.vivado| or higher.
  The free WebPack edition is sufficient for most use cases.

.. note:: Some external tools, especially EDA tools for synthesis and simulators, might be required that are in some cases proprietary and cost some money.
   Although OpTiMSoC is developed at an university with access to many EDA tools, we aim to always provide tool flows and support for open and free tools.
   But especially when it comes to synthesis such alternatives are not available (yet).

Dependencies
============

In OpTiMSoC, we try to use only external dependencies which are known to be stable and readily available.
Some can be installed as distribution packages, and others can be downloaded as binaries from us.

Distribution packages
---------------------

First, install all required packages from Ubuntu.

.. note::

   These steps require root permissions.
   If you don't have these ask the system administrator to run this script for you.
   In many lab environments using OpTiMSoC the required packages have already been installed for you and you can skip this step.

.. code:: sh

   # This command should get all required build dependencies
   ./tools/install-build-deps.sh

   # If you do not plan to build the documentation you can save yourself
   # a bit of download time by using
   INSTALL_DOC_DEPS=no ./tools/install-build-deps.sh

   # optional, but highly recommended: a waveform viewer
   sudo apt-get -y install gtkwave

Python packages
---------------

We build our systems with `FuseSoC <https://github.com/olofk/fusesoc>`_, a package manager for RTL designs.
It automatically arranges the sources and writes project files for our systems.
At least FuseSoC version |requirement_versions.fusesoc| is required, you can install it from the `Python package index <https://pypi.org/project/fusesoc/>`_.
The steps below use the ``--user`` flag to install packages without root permissions.

.. code:: sh

   pip3 install --user --upgrade fusesoc

To run the tests shipped with OpTiMSoC pytest is required.
Install it through pip (the distribution packages are too old):

.. code:: sh

   pip3 install --user --upgrade pytest

.. note::

   The two `pip3` calls used the ``--user`` flag to install Python packages without requiring root permissions.
   In exchange you need to add the directory ``~/.local/bin`` to your ``PATH`` environment variable.
   To check and potentially fix this problem you can run this one-liner:

   ``fusesoc --version || echo 'PATH=~/.local/bin:$PATH' >> ~/.bashrc``

   After executing this line you need to log out and log in again to apply the changes.
   Then run ``fusesoc --version`` again. If it prints out a version number you're all set!

Verilator and toolchain
-----------------------

In addition to all things we've installed so far we need two things which are not available as Ubuntu packages right now:
a recent version of Verilator (>= |requirement_versions.verilator|), and the ``or1k-elf-multicore`` toolchain (compiler, C library, debugger, etc.).
Install them with our binary installation script:

.. code:: sh

   # if it does not exist yet: prepare the ~/optimsoc directory
   mkdir -p ~/optimsoc

   # download and install the prebuilt tools
   curl -O https://raw.githubusercontent.com/optimsoc/prebuilts/master/optimsoc-prebuilt-deploy.py
   python optimsoc-prebuilt-deploy.py -d $HOME/optimsoc verilator or1kelf

To use the prebuilt tools some environment variables need to be adjusted.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. code:: sh

   source ~/optimsoc/setup_prebuilt.sh

.. note::

   Automatically load the prebuilts in every new terminal session by adding it to your ``~/.bashrc`` file:

   .. code:: sh

      echo 'source ~/optimsoc/setup_prebuilt.sh' >> ~/.bashrc

Install OpTiMSoC
================

Now that all preparations are done, you are now ready to install the OpTiMSoC framework itself.
There are three options: you can install a prebuilt release package, a prebuilt nightly build, or you can build OpTiMSoC yourself from the sources.
We recommend starting with a binary release installation, and move to a custom-built version only after you verified that everything works.

Recommended: OpTiMSoC binary releases
-------------------------------------

.. ifconfig:: lastversion != version

   .. warning::

      This documentation was generated for a development version of OpTiMSoC newer than version |lastversion|.
      We only provide binaries for released versions, not for this development version.
      Please go to our documentation page at https://www.optimsoc.org/docs to read the documentation matching |lastversion|.

The most simple way to get started is with the release packages.
You can find the OpTiMSoC releases here: https://github.com/optimsoc/optimsoc/releases.
With the release you can find the distribution packages that can be extracted into any directory and used directly from there.
The recommended default is to install OpTiMSoC into ``~/optimsoc`` [1]_.

There are three packages:

- The ``base`` package contains the programs, libraries and tools to get started.
- The ``examples`` package contains prebuilt example systems (both in simulation and FPGA bitstreams) for the real quick start.
- For even more examples (more complex systems on larger FPGAs) check out the ``examples-ext`` package.

Additionally a ``src`` package exists, containing the source code used to build the other packages.
In many cases that package won't be needed: if you need access to the source code check it out from our git repository instead.

To install the OpTiMSoC release |lastversion| into ``~/optimsoc/framework`` run the following commands:

.. parsed-literal::

   wget |dl_base|
   wget |dl_examples|
   wget |dl_examples_ext|
   mkdir -p ~/optimsoc/framework/|lastversion|
   for f in optimsoc-|lastversion|-\*.tar.gz; do tar -xf $f -C ~/optimsoc/framework/|lastversion| --strip-components=1; done

To use OpTiMSoC multiple environment variables need to be set.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. parsed-literal::

   source ~/optimsoc/framework/|lastversion|/optimsoc-environment.sh

.. note:: Automatically load the OpTiMSoC environment in every new
   terminal session by adding it to your ``~/.bashrc`` file:

   .. parsed-literal::

      echo 'source ~/optimsoc/framework/|lastversion|/optimsoc-environment.sh' >> ~/.bashrc

Installation complete!
If you made any changes to the ``~/.bashrc`` file you need to log off and log in again to apply the changes.

You are now ready to go to the :doc:`tutorials`.


Alternative: Download a nightly version
---------------------------------------

Every night we produce a "nightly build" of OpTiMSoC with everything that has made it into the ``master`` development branch at that time.
These builds are automatically tested, but no further manual testing is performed.
Use these builds at your own risk!

You can download the latest nightly build from the `optisoc/nightly channel on Bintray <https://bintray.com/optimsoc/nightly/>`_.
The following script automates the process for you.

.. code:: sh

   # figure out the latest version of OpTiMSoC
   LV=$(curl -sL https://api.bintray.com/packages/optimsoc/nightly/optimsoc-src/versions/_latest  |  python -c 'import sys, json; print json.load(sys.stdin)["name"]')

   # get the OpTiMSoC framework
   curl -sLO https://dl.bintray.com/optimsoc/nightly/optimsoc-$LV-base.tar.gz

   # get all examples with all bitstreams
   curl -sLO https://dl.bintray.com/optimsoc/nightly/optimsoc-$LV-examples.tar.gz
   curl -sLO https://dl.bintray.com/optimsoc/nightly/optimsoc-$LV-examples-ext.tar.gz

   # OPTIONAL: get the source archive
   curl -sLO https://dl.bintray.com/optimsoc/nightly/optimsoc-$LV-src.tar.gz

   # Install into ~/optimsoc/framework/<version>
   mkdir -p ~/optimsoc/framework/$LV
   for f in optimsoc-$LV-*.tar.gz; do tar -xf $f -C ~/optimsoc/framework/$LV --strip-components=1; done


To use OpTiMSoC multiple environment variables need to be set.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**.
Replace ``YOUR_VERSION`` with the version you just downloaded.

.. code:: sh

   source ~/optimsoc/framework/YOUR_VERSION/optimsoc-environment.sh

.. note:: Automatically load the OpTiMSoC environment in every new
   terminal session by adding it to your ``~/.bashrc`` file:

   .. parsed-literal::

      echo 'source ~/optimsoc/framework/YOUR_VERSION/optimsoc-environment.sh' >> ~/.bashrc


Alternative: Build OpTiMSoC from sources
----------------------------------------

You can also build OpTiMSoC from the sources.
This options usually becomes standard if you start developing for or around OpTiMSoC.
The build is done from one git repository: https://github.com/optimsoc/optimsoc.

It is generally a good idea to understand git, especially when you plan to contribute to OpTiMSoC.
Nevertheless, we will give a more detailed explanation of how to get your personal copies of OpTiMSoC and (potentially) update them.

First get the sources from git.
In this guide we assume you place your OpTiMSoC git repository into ``~/src/optimsoc``.
We refer to this directory as ``$OPTIMSOC_SRC`` in this guide.
You can of course choose any location, just remember to use the correct path in the subsequent steps!

.. parsed-literal::

   mkdir -p ~/src
   cd ~/src

   git clone https://github.com/optimsoc/optimsoc.git
   cd optimsoc

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
You can either use OpTiMSoC directly from there (good during development), or copy it to a more suitable installation location in ``~/optimsoc/framework/VERSION`` by running

.. code:: sh

   make install

You can also modify the target directory using environment variables passed to ``make``.

- Use ``INSTALL_PREFIX`` to change the installation prefix from ``~/optimsoc/framework`` to something else.
  The installation will then go into ``INSTALL_PREFIX/VERSION``.
- Use ``INSTALL_TARGET`` to fully override the installation path.
  The installation will then go exactly into this directory.

.. code:: sh

   # use INSTALL_PREFIX to install into /opt/optimsoc (e.g. for shared installations)
   make install INSTALL_PREFIX=/opt/optimsoc

   # full control for special cases: use INSTALL_TARGET
   # to install into ~/optimsoc-testversion
   make install INSTALL_TARGET=~/optimsoc-testversion

Independent of which directory you chose, to use OpTiMSoC multiple environment variables need to be set.
This is done by running the following command **in every terminal session that you want to use OpTiMSoC in**:

.. code:: sh

   source YOUR_INSTALLATION_DIR/optimsoc-environment.sh

See the binary installation description above for information on how to make this change permanent.

OpTiMSoC is now ready to be used and you can continue with the :doc:`tutorials`.


.. [1] The tilde symbol (``~``) is a `shortcut for your home directory <https://en.wikipedia.org/wiki/Tilde#Directories_and_URLs>`_, e.g. ``/home/miriam``.
