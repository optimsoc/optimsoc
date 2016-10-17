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
 * Configuration struct
 *
 * This is the static configuration struct. It contains all global configuration
 * settings and is propagated through the hierarchy.
 *
 * There are two struct: The base_config_t struct contains all configuration
 * settings. It is used to setup the system. The config_t struct contains some
 * extra settings, which are derived from the base settings. You can call the
 * function derive_config to convert a base_config_t struct into a config_t.
 *
 * When you add a base variable you need to add it at three positions:
 *
 *  - In base_config_t
 *  - In config_t
 *  - The copy operation in derive_config
 *
 * If it is a derived setting, you only need to add it to the latter two.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

package optimsoc;

   typedef enum { DISTRIBUTED, PGAS } mem_access_t;
   typedef enum { EXTERNAL, PLAIN } lmem_style_t;

   typedef struct packed {
      // System configuration
      integer            NUMTILES;
      integer            NUMCTS;
      logic [63:0][15:0] CTLIST;
      integer            CORES_PER_TILE;
      integer            GMEM_SIZE;
      integer            GMEM_TILE;

      // NoC-related configuration
      integer            NOC_DATA_WIDTH;
      integer            NOC_TYPE_WIDTH;
      integer            NOC_VCHANNELS;
      integer            NOC_VC_MPSIMPLE;
      integer            NOC_VC_DMA_REQ;
      integer            NOC_VC_DMA_RESP;

      // Tile configuration
      mem_access_t       MEMORY_ACCESS;
      integer            LMEM_SIZE;
      lmem_style_t       LMEM_STYLE;

      // Debug configuration
      logic              USE_DEBUG;
      logic              DEBUG_STM;
      logic              DEBUG_CTM;
   } base_config_t;

   typedef struct packed {
      // System configuration
      integer            NUMTILES;
      integer            NUMCTS;
      logic [63:0][15:0] CTLIST;
      integer            CORES_PER_TILE;
      integer            GMEM_SIZE;
      integer            GMEM_TILE;
      //  -> derived
      integer            TOTAL_NUM_CORES;

      // NoC-related configuration
      integer            NOC_DATA_WIDTH;
      integer            NOC_TYPE_WIDTH;
      integer            NOC_VCHANNELS;
      integer            NOC_VC_MPSIMPLE;
      integer            NOC_VC_DMA_REQ;
      integer            NOC_VC_DMA_RESP;
      // -> derived
      integer            NOC_FLIT_WIDTH; // Must be DATA_WIDTH+TYPE_WIDTH

      // Tile configuration
      mem_access_t       MEMORY_ACCESS;
      integer            LMEM_SIZE;
      lmem_style_t       LMEM_STYLE;

      // Debug configuration
      logic              USE_DEBUG;
      logic              DEBUG_STM;
      logic              DEBUG_CTM;

      // -> derived
      integer            DEBUG_MODS_PER_CORE;
      integer            DEBUG_MODS_PER_TILE;
      integer            DEBUG_NUM_MODS;
   } config_t;

   function config_t derive_config(base_config_t conf);
      // Copy the basic parameters
      derive_config.NUMTILES = conf.NUMTILES;
      derive_config.NUMCTS = conf.NUMCTS;
      derive_config.CTLIST = conf.CTLIST;
      derive_config.CORES_PER_TILE = conf.CORES_PER_TILE;
      derive_config.GMEM_SIZE = conf.GMEM_SIZE;
      derive_config.GMEM_TILE = conf.GMEM_TILE;
      derive_config.NOC_DATA_WIDTH = conf.NOC_DATA_WIDTH;
      derive_config.NOC_TYPE_WIDTH = conf.NOC_TYPE_WIDTH;
      derive_config.NOC_VCHANNELS = conf.NOC_VCHANNELS;
      derive_config.NOC_VC_MPSIMPLE = conf.NOC_VC_MPSIMPLE;
      derive_config.NOC_VC_DMA_REQ = conf.NOC_VC_DMA_REQ;
      derive_config.NOC_VC_DMA_RESP = conf.NOC_VC_DMA_RESP;
      derive_config.MEMORY_ACCESS = conf.MEMORY_ACCESS;
      derive_config.LMEM_SIZE = conf.LMEM_SIZE;
      derive_config.LMEM_STYLE = conf.LMEM_STYLE;
      derive_config.USE_DEBUG = conf.USE_DEBUG;
      derive_config.DEBUG_STM = conf.DEBUG_STM;
      derive_config.DEBUG_CTM = conf.DEBUG_CTM;

      // Derive the other parameters
      derive_config.TOTAL_NUM_CORES = conf.NUMCTS * conf.CORES_PER_TILE;
      derive_config.NOC_FLIT_WIDTH = conf.NOC_DATA_WIDTH + conf.NOC_TYPE_WIDTH;
      derive_config.DEBUG_MODS_PER_CORE = (conf.DEBUG_STM + conf.DEBUG_CTM + 1) * conf.USE_DEBUG;	// + 1 for system diagnosis module
      derive_config.DEBUG_MODS_PER_TILE = (1 + derive_config.DEBUG_MODS_PER_CORE *
                                           conf.CORES_PER_TILE) * conf.USE_DEBUG;
      derive_config.DEBUG_NUM_MODS = (1 + conf.NUMCTS *
                                      derive_config.DEBUG_MODS_PER_TILE) * conf.USE_DEBUG;
   endfunction // DERIVE_CONFIG

endpackage // optimsoc
