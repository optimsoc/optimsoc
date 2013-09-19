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
# (c) 2013 by the author(s)
#
# Author(s):
#    Philipp Wagner <philipp.wagner@tum.de>
#    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
#

# Do not set this variable in the Makefile, export it as environment variable!
# It is used by Synplify as well to determine the output directory of the
# synthesis.
OPTIMSOC_SYN_OUTDIR ?= .

# The commands synplify, bitgen and flash default to this target. If you want
# to explicitly specify a target, call the target directly (e.g. 
# "make bitgen_115b_bram")!
BOARD_TARGET ?= 115d_ddr

synplify: synplify_$(BOARD_TARGET)

bitgen: bitgen_$(BOARD_TARGET)

flash: flash_$(BOARD_TARGET)

synplify.prj:
	synplify -batch synplify_project.tcl

synplify_115b_ddr: synplify.prj
	synplify -batch synplify.prj -impl rev_1_15b_ddr

synplify_115d_bram: synplify.prj
	synplify -batch synplify.prj -impl rev_1_15d_bram

synplify_115d_ddr: synplify.prj
	synplify -batch synplify.prj -impl rev_1_15d_ddr

bitgen_115b_ddr: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15b_ddr/pr_1/system_2x2_cccc_ztex.bit

bitgen_115d_bram: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15d_bram/pr_1/system_2x2_cccc_ztex.bit

bitgen_115d_ddr: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15d_ddr/pr_1/system_2x2_cccc_ztex.bit

flash_115b_ddr: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15b_ddr/pr_1/system_2x2_cccc_ztex.bit
	FWLoader -v 0x221a 0x100 -f -uf $<

flash_115d_bram: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15d_bram/pr_1/system_2x2_cccc_ztex.bit
	FWLoader -v 0x221a 0x100 -f -uf $<

flash_115d_ddr: $(OPTIMSOC_SYN_OUTDIR)/rev_1_15d_ddr/pr_1/system_2x2_cccc_ztex.bit
	FWLoader -v 0x221a 0x100 -f -uf $<

%.bit: %.ncd
	bitgen -w $*.ncd

clean:
	rm synplify.prj

