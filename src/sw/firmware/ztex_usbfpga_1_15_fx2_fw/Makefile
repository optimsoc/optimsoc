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

all: fx2_fw.ihx

fx2_fw.ihx: fx2_fw.c
	bmpsdcc.sh fx2_fw.c "-I $(ZTEX_INCLUDE)"

clean:
	rm -f fx2_fw.{asm,ihx,lnk,lst,map,mem,rel,rst,sym,tmp.c}

flash: fx2_fw.ihx
	FWLoader -c -uu fx2_fw.ihx -ue fx2_fw.ihx
	FWLoader -p

