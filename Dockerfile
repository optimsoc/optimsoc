FROM ubuntu:16.04

# Install OS dependencies as in the user guide
RUN apt-get update
RUN apt-get -y install texlive texlive-latex-extra texlive-fonts-extra
RUN apt-get -y install curl git build-essential autoconf automake \
    libtool pkg-config 
RUN apt-get -y install tcl libusb-1.0-0-dev libboost-dev \
    python3 python3-pip libreadline-dev python-dev python3-venv doxygen \
    latexmk python-cffi libffi-dev python3-packaging python3-yaml

# osd-sw dependencies, taken from osd-sw/install-build-deps.sh
RUN apt-get -y install \
      check \
      doxygen \
      python3 python3-venv python3-pip \
      tox \
      lcov valgrind \
      libzmq5 \
      libzmq3-dev \
      libzmq5-dbg \
      libczmq-dev \
      libczmq-dbg \
      xsltproc \
      libelf1 libelf-dev zlib1g zlib1g-dev
RUN pip3 install pytest


# install fusesoc
RUN pip3 install --upgrade pip
RUN pip3 install fusesoc

# make sources available in docker image
RUN mkdir -p /src
ADD . /src
WORKDIR /src

# target/installation directory
RUN mkdir -p /opt/optimsoc

# Get prebuilts: GCC for or1k and verilator
RUN curl -s https://raw.githubusercontent.com/optimsoc/prebuilts/master/optimsoc-prebuilt-deploy.py -o /tmp/optimsoc-prebuilt-deploy.py
RUN chmod a+x /tmp/optimsoc-prebuilt-deploy.py
RUN /tmp/optimsoc-prebuilt-deploy.py -d /opt/optimsoc all

# Execute the build and install it
RUN /bin/bash -c "source /opt/optimsoc/setup_prebuilt.sh \
    && ./tools/build.py --verbose --without-examples-fpga --with-examples-sim --with-docs \
    && make install INSTALL_TARGET=/opt/optimsoc/latest"

# Test the build
RUN /bin/bash -c "source /opt/optimsoc/setup_prebuilt.sh \
    && source /opt/optimsoc/latest/optimsoc-environment.sh \
    && pytest -s -v test/systemtest/test_tutorial.py::TestTutorial"

