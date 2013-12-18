/* Copyright (c) 2013 by the author(s)
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
 * System-wide definitions for an OpTiMSoC system instance.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

// OR1200 boot address (reset vector)
`define OR1200_BOOT_ADR 32'h00000100

// Default SRAM implementation (unless overwritten in specific components)
`ifndef OPTIMSOC_SRAM_IMPLEMENTATION
 `define OPTIMSOC_SRAM_IMPLEMENTATION "XILINX_SPARTAN6"
`endif

// Number of virtual channels
`define VCHANNELS 3

// Assign virtual channels to services
`define VCHANNEL_LSU_REQ   'hx
`define VCHANNEL_LSU_RESP  'hx
`define VCHANNEL_DMA_REQ   0
`define VCHANNEL_DMA_RESP  1
`define VCHANNEL_MPSIMPLE  2

`define OPTIMSOC_XDIM 2
`define OPTIMSOC_YDIM 2

`define OPTIMSOC_MEMORYID 'bx

`define OPTIMSOC_CLOCK 48000000 // 48 MHz

//`define OPTIMSOC_DEBUG_NO_CLOCK_GATING
`define OPTIMSOC_DEBUG_ENABLE_ITM
`define OPTIMSOC_DEBUG_ENABLE_STM
//`define OPTIMSOC_DEBUG_ENABLE_NRM
//`define OPTIMSOC_DEBUG_ENABLE_NCM
`define OPTIMSOC_DEBUG_ENABLE_MAM

//`define OPTIMSOC_CLOCKDOMAINS
//`define OPTIMSOC_CDC_DYNAMIC
`define OPTIMSOC_CDC_DYN_DEFAULT 0
//`define OPTIMSOC_UART_LCD_ENABLE 0
