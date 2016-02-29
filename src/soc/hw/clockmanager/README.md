Clock and Reset Manager
=======================

The OpTiMSoC clock manager generates all necessary auxiliary clocks out of a
base clocks (the clock tree). The factor between the output clocks and the 
base clock is configurable through parameters. In addition, this module 
generates the reset sequence for the system.


Defines
-------

| Name                    | Description                                |
|-------------------------|--------------------------------------------|
| `OPTIMSOC_CLOCKDOMAINS` | Enable clock domain support                |
| `OPTIMSOC_IO_CLOCKS`    | Enable I/O clock support                   |
| `OPTISMOC_CDC_DYNAMIC`  | Enable dynamic clock domain crossing (CDC) |


General Parameters
------------------

| Parameter Name     | Default | Description                                       |
|--------------------|---------|---------------------------------------------------|
| `CLK_PERIOD`       | 5       | Period of the clock signal `clk` (in ns)          |
| `RST_WIDTH`        | 128     | Width of the reset signal (in cycles)             |
| `ENABLE_DDR_CLOCK` | 1       | Enable the DDR memory clock                       |
| `NUM_CT_CLOCKS`    | 1       | Number of generated compute tile clocks           |


Parameters for systems without clock domain support (OPTIMSOC_CLOCKDOMAINS not defined)
---------------------------------------------------------------------------------------

| Parameter Name     | Default | Description                                                   |
|--------------------|---------|---------------------------------------------------------------|
| `CLOCK_MUL`        | 2       | Compute tile clocks: Multiplier to the base clock             |
| `CLOCK_DIV`        | 2       | Divisor to the base clock for all output clocks (except I/O)  |


Parameters for systems with clock domain support (OPTIMSOC_CLOCKDOMAINS is defined)
-----------------------------------------------------------------------------------

| Parameter Name     | Default | Description                                       |
|--------------------|---------|---------------------------------------------------|
| `CT_CLOCKS_MUL`    | 2       | Compute tile clocks: Multiplier to the base clock |
| `CT_CLOCKS_DIV`    | 4       | Compute tile clocks: Divisor to the base clock    |
| `NOC_CLOCK_MUL`    | 3       | NoC: Multiplier to the base clock                 |
| `NOC_CLOCK_DIV`    | 4       | NoC: Divisor to the base clock                    |
| `DBG_CLOCK_MUL`    | 2       | Debug system: Multiplier to the base clock        |
| `DBG_CLOCK_DIV`    | 2       | Debug system: Divisor to the base clock           |


Parameters for systems with I/O clock support (OPTIMSOC_IO_CLOCKS is defined)
-----------------------------------------------------------------------------

| Parameter Name     | Default | Description                                       |
|--------------------|---------|---------------------------------------------------|
| `NUM_IO_CLOCKS`    | 1       | Number of generated I/O clocks                    |
| `IO_CLOCK0_MUL`    | 2       | I/O clock 0: Multiplier to the base clock         |
| `IO_CLOCK0_DIV`    | 2       | I/O clock 0: Divisor to the base clock            |
| `IO_CLOCK1_MUL`    | 2       | I/O clock 1: Multiplier to the base clock         |
| `IO_CLOCK1_DIV`    | 2       | I/O clock 1: Divisor to the base clock            |


Signals
-------

| Signal Name     | Direction | Width                 | Description                                             |
|-----------------|-----------|-----------------------|---------------------------------------------------------|
| `clk`           | IN        |                       | Base clock                                              |
| `rst`           | IN        |                       | System reset request                                    |
| `cpu_reset`     | IN        |                       | CPU reset request                                       |
| `cpu_start`     | IN        |                       | Start the CPUs (enable the clocks)                      |
| `sys_halt`      | IN        |                       | Request all system clocks to stop                       |
| `sys_is_halted` | OUT       |                       | All system clocks are stopped (in response to sys_halt) |
| `cdc_conf`      | IN        | [NUM_CT_CLOCKS*3-1:0] | CDC configuration (per CPU)                             |
| `cdc_enable`    | IN        | [NUM_CT_CLOCKS-1:0]   | enable clock domain crossing (per CPU)                  |
| `clk_ct`        | OUT       | [NUM_CT_CLOCKS-1:0]   | Compute tile clock                                      |
| `clk_dbg`       | OUT       |                       | Clock for the debug/diagnosis system                    |
| `clk_noc`       | OUT       |                       | NoC clock                                               |
| `clk_ddr`       | OUT       |                       | DDR memory clock (only if `ENABLE_DDR_CLOCK` is set)    |
| `clk_io`        | OUT       | [NUM_IO_CLOCKS-1:0]   | I/O clocks (only if `OPTIMSOC_IO_CLOCKS` is defined)    |
