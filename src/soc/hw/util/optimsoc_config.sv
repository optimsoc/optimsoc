/* Copyright (c) 2016-2017 by the author(s)
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

package optimsoc_config;

   import optimsoc_functions::*;

   typedef enum { EXTERNAL, PLAIN } lmem_style_t;

   typedef struct packed {
      // System configuration
      integer     NUMTILES;
      integer     NUMCTS;
      logic [63:0][15:0] CTLIST;
      integer            CORES_PER_TILE;
      integer            GMEM_SIZE;
      integer            GMEM_TILE;

      // NoC-related configuration
      logic              NOC_ENABLE_VCHANNELS;

      // Tile configuration
      integer            LMEM_SIZE;
      lmem_style_t       LMEM_STYLE;
      logic              ENABLE_BOOTROM;
      integer            BOOTROM_SIZE;
      logic              ENABLE_DM;
      integer            DM_BASE;
      integer            DM_SIZE;
      logic              ENABLE_PGAS;
      integer            PGAS_BASE;
      integer            PGAS_SIZE;

      // CPU core configuration
      logic              CORE_ENABLE_FPU;
      logic              CORE_ENABLE_PERFCOUNTERS;

      // Network adapter configuration
      logic              NA_ENABLE_MPSIMPLE;
      logic              NA_ENABLE_DMA;
      logic              NA_DMA_GENIRQ;
      integer            NA_DMA_ENTRIES;

      // Debug configuration
      logic              USE_DEBUG;
      logic              DEBUG_STM;
      logic              DEBUG_CTM;
      logic              DEBUG_DEM_UART;
      integer            DEBUG_SUBNET_BITS;
      integer            DEBUG_LOCAL_SUBNET;
      integer            DEBUG_ROUTER_BUFFER_SIZE;
      integer            DEBUG_MAX_PKT_LEN;
   } base_config_t;

   typedef struct        packed {
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
      logic              NOC_ENABLE_VCHANNELS;
      //  -> derived
      integer            NOC_FLIT_WIDTH;
      integer            NOC_CHANNELS;

      // Tile configuration
      integer            LMEM_SIZE;
      lmem_style_t       LMEM_STYLE;
      logic              ENABLE_BOOTROM;
      integer            BOOTROM_SIZE;
      logic              ENABLE_DM;
      integer            DM_BASE;
      integer            DM_SIZE;
      logic              ENABLE_PGAS;
      integer            DM_RANGE_WIDTH;
      integer            DM_RANGE_MATCH;
      integer            PGAS_BASE;
      integer            PGAS_SIZE;
      integer            PGAS_RANGE_WIDTH;
      integer            PGAS_RANGE_MATCH;

      // CPU core configuration
      logic              CORE_ENABLE_FPU;
      logic              CORE_ENABLE_PERFCOUNTERS;

      // Network adapter configuration
      logic              NA_ENABLE_MPSIMPLE;
      logic              NA_ENABLE_DMA;
      logic              NA_DMA_GENIRQ;
      integer            NA_DMA_ENTRIES;

      // Debug configuration
      logic              USE_DEBUG;
      logic              DEBUG_STM;
      logic              DEBUG_CTM;
      logic              DEBUG_DEM_UART;
      integer            DEBUG_SUBNET_BITS;
      integer            DEBUG_LOCAL_SUBNET;
      integer            DEBUG_ROUTER_BUFFER_SIZE;
      integer            DEBUG_MAX_PKT_LEN;
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
      derive_config.NOC_ENABLE_VCHANNELS = conf.NOC_ENABLE_VCHANNELS;
      derive_config.LMEM_SIZE = conf.LMEM_SIZE;
      derive_config.LMEM_STYLE = conf.LMEM_STYLE;
      derive_config.ENABLE_BOOTROM = conf.ENABLE_BOOTROM;
      derive_config.BOOTROM_SIZE = conf.BOOTROM_SIZE;
      derive_config.ENABLE_DM = conf.ENABLE_DM;
      derive_config.DM_BASE = conf.DM_BASE;
      derive_config.DM_SIZE = conf.DM_SIZE;
      derive_config.ENABLE_PGAS = conf.ENABLE_PGAS;
      derive_config.PGAS_BASE = conf.PGAS_BASE;
      derive_config.PGAS_SIZE = conf.PGAS_SIZE;
      derive_config.CORE_ENABLE_FPU = conf.CORE_ENABLE_FPU;
      derive_config.CORE_ENABLE_PERFCOUNTERS = conf.CORE_ENABLE_PERFCOUNTERS;
      derive_config.NA_ENABLE_MPSIMPLE = conf.NA_ENABLE_MPSIMPLE;
      derive_config.NA_ENABLE_DMA = conf.NA_ENABLE_DMA;
      derive_config.NA_DMA_GENIRQ = conf.NA_DMA_GENIRQ;
      derive_config.NA_DMA_ENTRIES = conf.NA_DMA_ENTRIES;
      derive_config.USE_DEBUG = conf.USE_DEBUG;
      derive_config.DEBUG_STM = conf.DEBUG_STM;
      derive_config.DEBUG_CTM = conf.DEBUG_CTM;
      derive_config.DEBUG_DEM_UART = conf.DEBUG_DEM_UART;
      derive_config.DEBUG_SUBNET_BITS = conf.DEBUG_SUBNET_BITS;
      derive_config.DEBUG_LOCAL_SUBNET = conf.DEBUG_LOCAL_SUBNET;
      derive_config.DEBUG_ROUTER_BUFFER_SIZE = conf.DEBUG_ROUTER_BUFFER_SIZE;
      derive_config.DEBUG_MAX_PKT_LEN = conf.DEBUG_MAX_PKT_LEN;

      // Derive the other parameters
      derive_config.TOTAL_NUM_CORES = conf.NUMCTS * conf.CORES_PER_TILE;

      derive_config.DM_RANGE_WIDTH = conf.ENABLE_DM ? 32-clog2_width(conf.DM_SIZE) : 1;
      derive_config.DM_RANGE_MATCH = conf.DM_BASE >> (32-derive_config.DM_RANGE_WIDTH);
      derive_config.PGAS_RANGE_WIDTH = conf.ENABLE_PGAS ? 32-clog2_width(conf.PGAS_SIZE) : 1;
      derive_config.PGAS_RANGE_MATCH = conf.PGAS_BASE >> (32-derive_config.PGAS_RANGE_WIDTH);

      derive_config.DEBUG_MODS_PER_CORE = (int'(conf.DEBUG_STM) + int'(conf.DEBUG_CTM)) * int'(conf.USE_DEBUG);
      derive_config.DEBUG_MODS_PER_TILE = conf.USE_DEBUG *
                                          (1 /* MAM */
                                           + int'(conf.DEBUG_DEM_UART)
                                           + derive_config.DEBUG_MODS_PER_CORE * conf.CORES_PER_TILE);
      derive_config.DEBUG_NUM_MODS = conf.USE_DEBUG *
                                     (1 /* SCM */
                                      + conf.NUMCTS * derive_config.DEBUG_MODS_PER_TILE);

      // Those are supposed to be variables, but are constant at least for now
      derive_config.NOC_CHANNELS = 2;
      derive_config.NOC_FLIT_WIDTH = 32;
   endfunction // DERIVE_CONFIG

endpackage // optimsoc
