/* Copyright (c) 2017 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static int _numlinks;
static uint32_t **_buffer;
static size_t *_pos;
static FILE *_fh;
static void _emit_event_context(FILE *fh, int link, uint16_t id,
				uint64_t timestamp, uint32_t header);
static void _write_metadata(char* path);

extern "C" {  
  void* noc_tracer_init(int numlinks) {
    _numlinks = numlinks;
    _buffer = (uint32_t**) calloc(sizeof(uint32_t*), numlinks);
    _pos = (size_t*) calloc(sizeof(size_t), numlinks);
    
    for (int p = 0; p < numlinks; p++) {
      _buffer[p] = (uint32_t*) malloc(sizeof(uint32_t)*128);
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char folder[64];
    strftime(folder, 60, "ctf-%Y%m%d-%H%M%S", tm);
    
    mkdir(folder, 0777);
    _write_metadata(folder);

    char noctrace[64];
    snprintf(noctrace, 64, "%s/noc", folder);
    _fh = fopen(noctrace, "w");
  }

  void* noc_tracer_trace(int link,
			 uint32_t flit,
			 int last,
			 uint64_t timestamp) {
    assert(link < _numlinks);
    assert(_pos[link] < 127);
    
    _buffer[link][_pos[link]++] = flit;
    if (last) {
      uint32_t header = _buffer[link][0];
      uint8_t cls = (header >> 24) & 0x7;

      if (cls == 0x0) {
	// mp buffer messages
	uint32_t payload_len = _pos[link] - 1;

	_emit_event_context(_fh, link, 3, timestamp, header);
	fwrite(&_buffer[link][0], 4, 1, _fh);
	fwrite(&payload_len, 4, 1, _fh);
	for (size_t p = 1; p < _pos[link]; p++) {
	  fwrite(&_buffer[link][p], 4, 1, _fh);
	}
      } else if (cls == 0x7) {
	// mp buffer control
	if ((_buffer[link][0] & 0x1) == 0) {
	  // request
	  _emit_event_context(_fh, link, 1, timestamp, header);
	} else {
	  // response
	  uint8_t status = (header >> 1) & 0x1;
	  _emit_event_context(_fh, link, 2, timestamp, header);
	  fwrite(&status, 1, 1, _fh);
	}
      } else {
	uint32_t payload_len = _pos[link] - 1;

	_emit_event_context(_fh, link, 0, timestamp, header);
	fwrite(&_buffer[link][0], 4, 1, _fh);
	fwrite(&payload_len, 4, 1, _fh);
	for (size_t p = 1; p < _pos[link]; p++) {
	  fwrite(&_buffer[link][p], 4, 1, _fh);
	}
      }

      fflush(_fh);
      _pos[link] = 0;
    }
  }

}

static void _emit_event_context(FILE *fh, int link, uint16_t id,
				uint64_t timestamp, uint32_t header) {
  uint8_t src = (header >> 19) & 0x1f;
  uint8_t dest = (header >> 27) & 0x1f;
  uint16_t link_short = (uint16_t) link;

  fwrite(&timestamp, 8, 1, fh);
  fwrite(&id, 2, 1, fh);
  fwrite(&link_short, 2, 1, fh);
  fwrite(&src, 1, 1, fh);
  fwrite(&dest, 1, 1, fh);
}

static const char *_metadata = "/* CTF 1.8 */\n\n"
  "typealias integer {\n"
  "  size = 64;\n"
  "  signed = false;\n"
  "  align = 8;\n"
  "} := uint64_t;\n"
  "\n"
  "typealias integer {\n"
  "  size = 32;\n"
  "  signed = false;\n"
  "  align = 8;\n"
  "} := uint32_t;\n"
  "\n"
  "typealias integer {\n"
  "    size = 16;\n"
  "    signed = false;\n"
  "    align = 8;\n"
  "} := uint16_t;\n"
  "\n"
  "typealias integer {\n"
  "    size = 8;\n"
  "    signed = false;\n"
  "    align = 8;\n"
  "} := uint8_t;\n"
  "\n"
  "trace {\n"
  "    major = 1;\n"
  "    minor = 8;\n"
  "    byte_order = le;\n"
  "};\n"
  "\n"
  "stream {\n"
  "    event.header := struct {\n"
  "        uint64_t timestamp;\n"
  "        uint16_t id;\n"
  "    };\n"
  "    event.context := struct {\n"
  "        uint16_t link;\n"
  "	uint8_t src;\n"
  "	uint8_t dest;\n"
  "    };\n"
  "};\n"
  "\n"
  "event {\n"
  "    id = 0;\n"
  "    name = 'unknown';\n"
  "    fields := struct {\n"
  "        uint32_t header;\n"
  "	uint32_t length;\n"
  "	uint32_t payload[length];\n"
  "    };\n"
  "};\n"
  "\n"
  "event {\n"
  "    id = 1;\n"
  "    name = 'mpbuffer_control_req';\n"
  "};\n"
  "\n"
  "event {\n"
  "    id = 2;\n"
  "    name = 'mpbuffer_control_resp';\n"
  "    fields := struct {\n"
  "        uint8_t status;\n"
  "    };\n"
  "};\n"
  "\n"
  "event {\n"
  "    id = 3;\n"
  "    name = 'mpbuffer_message';\n"
  "    fields := struct {\n"
  "        uint32_t header;\n"
  "	uint32_t length;\n"
  "	uint32_t payload[length];\n"
  "    };\n"
  "};\n";

    
static void _write_metadata(char* path) {
  char *fname = (char*) malloc(strlen(path)+11);
  snprintf(fname, strlen(path)+10, "%s/metadata", path);

  FILE *fh = fopen(fname, "w");
  fwrite(_metadata, strlen(_metadata), 1, fh);
  fclose(fh);
}
