/* Copyright (c) 2016 by the author(s)
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
 * ============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#ifndef __HELP_STRINGS_H__
#define __HELP_STRINGS_H__

static const char* help_param =
        "Usage: osd-cli <parameters>\n"
        "\n"
        "Parameters:\n"
        "  -h, --help                  Print this help\n"
        "  -s <file>, --source=<file>  Read commands from file at start\n"
        "  -b <file>, --batch=<file>   Read commands from file and exit\n"
        "  --python                    Interpret -s and -b as python script\n";

static const char* help_cmd =
        "Available commands:\n"
        "  help        Print this help\n"
        "  <cmd> help  Print help for command\n"
        "  quit        Exit the command line\n"
        "  reset       Reset the system\n"
        "  start       Start the processor cores\n"
        "  mem         Access memory\n"
        "  ctm         Configure core trace module\n"
        "  stm         Configure software trace module\n"
        "  terminal    Start terminal for device emulation module\n"
        "  wait        Wait for given seconds\n";

static const char* help_reset =
        "Available parameters:\n"
        "  -halt       Halt processor cores until 'start'\n";

static const char* help_start =
        "Start cores after 'reset', no parameters\n";

static const char* help_mem =
        "Available subcommands:\n"
        "  help        Print this help\n"
        "  test        Run memory tests\n"
        "  loadelf     Load an elf to memory\n";

static const char* help_mem_loadelf =
        "Usage: mem loadelf <file> <memid> [-verify]\n"
        "  file     Filename to load\n"
        "  memid    Module identifier of memory\n"
        "  -verify  Verify memory content by reading it back\n";

static const char* help_stm =
        "Available subcommands:\n"
        "  help        Print this help\n"
        "  log         Log STM events to file\n";

static const char* help_stm_log =
        "Usage: stm log <file> <stmid>\n"
        "  file   Filename to log to\n"
        "  stmid  STM to receive logs from\n";

static const char* help_ctm =
        "Available subcommands:\n"
        "  help        Print this help\n"
        "  log         Log CTM events to file\n";

static const char* help_ctm_log =
        "Usage: ctm log <file> <ctmid> [<elffile>]\n"
        "  file     Filename to log to\n"
        "  ctmid    CTM to receive logs from\n"
        "  elffile  ELF file to load symbols from, optional\n";

static const char* help_terminal =
        "Usage: terminal <id>\n"
        "  id  DEM-UART to use\n";

static const char* help_wait =
        "Usage: wait <n>\n"
        "  n  Number of seconds\n";

#endif
