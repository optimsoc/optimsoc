# Copyright (c) 2016 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author(s):
#   Philipp Wagner <philipp.wagner@tum.de>

version := $(shell tools/get-version.sh)

# Object directory: where all the build output goes
OBJDIR := objdir

# Build configuration
# Build and package the compiled examples (yes/no)
BUILD_EXAMPLES := yes
# Include FPGA bitstreams in the examples (yes/no)
# Requires Xilinx Vivado to be installed
BUILD_EXAMPLES_FPGA := yes
# Build documentation (yes/no)
BUILD_DOCS := yes

# Only for |make install|: use either INSTALL_PREFIX /or/ INSTALL_TARGET to
# choose an installation destination. In most cases use INSTALL_PREFIX.
# Installation prefix: the OpTiMSoC version will be appended as subdirectory.
INSTALL_PREFIX := /opt/optimsoc
# Full installation directory: OpTiMSoC will be installed into exactly that
# directory
INSTALL_TARGET := $(INSTALL_PREFIX)/$(version)


# Assemble arguments passed to tools/build.py
BUILD_ARGS = ''
ifeq ($(BUILD_DOCS),yes)
	BUILD_ARGS += --with-docs
else
	BUILD_ARGS += --without-docs
endif
ifeq ($(BUILD_EXAMPLES),yes)
	BUILD_ARGS += --with-examples-sim

	ifeq ($(BUILD_EXAMPLES_FPGA),yes)
		BUILD_ARGS += --with-examples-fpga
	else
		BUILD_ARGS += --without-examples-fpga
	endif
else
	BUILD_ARGS += --without-examples-sim --without-examples-fpga
endif


build:
	tools/build.py $(BUILD_ARGS) -o $(OBJDIR)

install:
	@test -d "$(OBJDIR)/dist" || (echo "Run make build first."; exit 1)
	mkdir -p $(INSTALL_TARGET)
	cp -rT $(OBJDIR)/dist $(INSTALL_TARGET)

dist:
	@test -d "$(OBJDIR)/dist" || (echo "Run make build first."; exit 1)
	tar -cz --directory $(OBJDIR) --exclude examples \
		--transform "s/dist/$(version)/" \
		-f $(OBJDIR)/optimsoc-$(version)-base.tar.gz dist

ifeq ($(BUILD_EXAMPLES),yes)
	tar -cz --directory $(OBJDIR) \
		--transform "s/dist/$(version)/" \
		-f $(OBJDIR)/optimsoc-$(version)-examples.tar.gz dist/examples
endif

	@echo
	@echo The binary distribution packages have been written to
	@echo $(OBJDIR)/optimsoc-$(version)-base.tar.gz

ifeq ($(BUILD_EXAMPLES),yes)
	@echo The examples have been written to
	@echo $(OBJDIR)/optimsoc-$(version)-examples.tar.gz
endif

clean:
	rm -rf $(OBJDIR)

test:
ifneq ($(BUILD_EXAMPLES_FPGA),yes)
	# only run verilator-based system tests, not the FPGA ones
	pytest -s -v test/systemtest/test_tutorial.py::TestTutorial
else
	pytest -s -v test/systemtest/test_tutorial.py
endif

.PHONY: build test

