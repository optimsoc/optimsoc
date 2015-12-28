/* Copyright (c) 2015 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * This is the definition file. All Verilog macros are defined here.
 * Please note, that it is not intended to be used for configuration
 * (which should be done via parameters) but more for specific
 * cosntants, that might change over longer time periods.
 *
 * Author(s):
 *    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *    Andreas Lankes <andreas.lankes@tum.de>
 *    Michael Tempelmeier <michael.tempelmeier@tum.de>
 */

`define FLIT16_WIDTH 18

// Type of flit
// The coding is chosen, so that
// type[0] signals that this is the first flit of a packet
// type[1] signals that this is the last flit of a packet

`define FLIT16_TYPE_MSB (`FLIT16_WIDTH - 1)
`define FLIT16_TYPE_WIDTH 2
`define FLIT16_TYPE_LSB (`FLIT16_TYPE_MSB - `FLIT16_TYPE_WIDTH + 1)
`define FLIT16_DATA_WIDTH `FLIT16_WIDTH - `FLIT16_TYPE_WIDTH

// same as FLIT_TYPE_* in lisnoc_def.v
`define FLIT16_TYPE_PAYLOAD 2'b00
`define FLIT16_TYPE_HEADER  2'b01
`define FLIT16_TYPE_LAST    2'b10
`define FLIT16_TYPE_SINGLE  2'b11

// This is the flit content size
`define FLIT16_CONTENT_WIDTH 16
`define FLIT16_CONTENT_MSB   15
`define FLIT16_CONTENT_LSB    0

// The following fields are only valid for header flits
`define FLIT16_DEST_WIDTH 5
// destination address field of header flit
`define FLIT16_DEST_MSB `FLIT16_CONTENT_MSB
`define FLIT16_DEST_LSB `FLIT16_DEST_MSB - `FLIT16_DEST_WIDTH + 1

// packet type field  of header flit
`define PACKET16_CLASS_MSB (`FLIT16_DEST_LSB - 1)
`define PACKET16_CLASS_WIDTH 3
`define PACKET16_CLASS_LSB (`PACKET16_CLASS_MSB - `PACKET16_CLASS_WIDTH + 1)

// class defines
`define PACKET16_CLASS_CONTROL 3'b010
`define PACKET16_CLASS_32 3'b111

//some global defines
`define USB_DEST 5'b00000
`define MAX_NOC16_PACKET_LENGTH 32
`define LD_MAX_NOC16_PACKET_LENGTH 8
