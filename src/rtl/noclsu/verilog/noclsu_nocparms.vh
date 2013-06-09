/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * REVIEW: Description missing
 *
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

parameter ph_dest_width = 5;
parameter ph_class_width = 2;
parameter ph_lsu_class = 2'b00;

localparam flit_data_width = 32;
localparam ph_dest_msb = flit_data_width - 1;
localparam ph_dest_lsb = ph_dest_msb - ph_dest_width + 1;

localparam ph_src_msb = ph_dest_lsb - 1;
localparam ph_src_lsb = ph_src_msb - ph_dest_width + 1;

localparam ph_class_msb = ph_src_lsb - 1;
localparam ph_class_lsb = ph_class_msb - ph_class_width + 1;

localparam ph_msgtype_width = 2;
localparam ph_msgtype_msb = ph_class_lsb - 1;
localparam ph_msgtype_lsb = ph_msgtype_msb - ph_msgtype_width + 1;

localparam msgtype_readreq   = 2'd0;
localparam msgtype_readresp  = 2'd1;
localparam msgtype_writereq  = 2'd2;
localparam msgtype_writeresp = 2'd3;

localparam ph_type_width = 1;
localparam ph_type_msb = ph_msgtype_lsb - 1;
localparam ph_type_lsb = ph_type_msb - ph_type_width + 1;

localparam type_i = 1'b0;
localparam type_d = 1'b1;

localparam ph_size_width = 1;
localparam ph_size_msb = ph_type_lsb - 1;
localparam ph_size_lsb = ph_size_msb - ph_size_width + 1;

localparam size_single = 1'b0;
localparam size_burst = 1'b1;

localparam ph_bsel_width = 4;
localparam ph_bsel_msb = ph_size_lsb - 1;
localparam ph_bsel_lsb = ph_bsel_msb - ph_bsel_width + 1;
