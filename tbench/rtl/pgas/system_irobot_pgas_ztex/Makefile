#
# This file is part of OpTiMSoC.
#
# OpTiMSoC is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# OpTiMSoC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
#
# ============================================================================
#
# (c) 2012-2013 by the author(s)
#
# Author(s):
#    Philipp Wagner <philipp.wagner@tum.de>
#    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
#

all: build

work:
	if [ ! -d work ]; then vlib work; fi

build: work
	vlog -lint -f tb_system_irobot_pgas_ztex.vc
	sccom -Wall $(OPTIMSOC_SYSC)/src/debug/FX2EmulationDebugConnector.cpp \
		-I /usr/include/x86_64-linux-gnu/
	sccom -link

sim:
	vsim -voptargs=+acc -L unisims_ver -L xilinxcorelib_ver -L unimacro_ver \
		work.tb_system_irobot_pgas_ztex work.glbl

sim-noninteractive:
	vsim -c -voptargs=+acc -L unisims_ver -L xilinxcorelib_ver \
		work.tb_system_irobot_pgas_ztex work.glbl \
		-do "suppress 8233; run -all"

clean:
	rm -rf work
