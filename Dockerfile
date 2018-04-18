# Dockerfile used for CI builds in Travis
FROM ubuntu:16.04

# make sources available in docker image
RUN mkdir -p /src
ADD . /src
WORKDIR /src

# Install OS dependencies as in the user guide
RUN apt-get update
RUN ./tools/install-build-deps.sh

# Python dependencies
RUN pip3 install pytest fusesoc

# target/installation directory
RUN mkdir -p /opt/optimsoc

# Get prebuilts: GCC for or1k and verilator
RUN curl -s https://raw.githubusercontent.com/optimsoc/prebuilts/master/optimsoc-prebuilt-deploy.py -o /tmp/optimsoc-prebuilt-deploy.py \
    && chmod a+x /tmp/optimsoc-prebuilt-deploy.py \
    && /tmp/optimsoc-prebuilt-deploy.py -d /opt/optimsoc all

# Execute the build and install it
RUN /bin/bash -c "source /opt/optimsoc/setup_prebuilt.sh \
    && ./tools/build.py --verbose --without-examples-fpga --with-examples-sim --with-docs \
    && make install INSTALL_TARGET=/opt/optimsoc/latest"

# Test the build
RUN /bin/bash -c "source /opt/optimsoc/setup_prebuilt.sh \
    && source /opt/optimsoc/latest/optimsoc-environment.sh \
    && pytest -s -v test/systemtest/test_tutorial.py::TestTutorial"

