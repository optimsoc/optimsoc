# OpTiMSoC Example Design: System with all compute tiles for the Xilinx VCU108 Evaluation Board

This is a example design of a system with all compute tiles in a mesh structure.
Each tile can have a configurable amount of CPU cores.
The memory distributed in each tile is mapped to one DDR memory on the board.

## Factsheet

| Name | Value |
| --- | ---|
| Design | `system_allct` |
| Frequency | 50 MHz |
| Board | [Xilinx Virtex UltraScale FPGA VCU108 Evaluation Kit](https://www.xilinx.com/products/boards-and-kits/ek-u1-vcu108-g.html) |
| Synthesis tool | Xilinx Vivado |
| Connectivity | UART or USB3 (see below) |

## Top-Level Design Parameters

| Parameter Name | Description | Values | Default Value |
|---|---|---|---|
| `HOST_IF` | Interface to the host | `usb3` or `uart` | `uart` |
| `UART0_SOURCE` | UART connection | `onboard` or `pmod` | `onboard` |
| `NUM_CORES` | Number of CPU cores per tile | any number, recommended between 1 and 4 | `1` |

All top-level design parameters can be set when running fusesoc to synthesize the design by passing them in the form `--PARAMETERNAME=PARAMETERVALUE`, e.g. `--UART0_SOURCE=onboard`.


## Usage
The following example shows how to build and run the design on the VCU108 board without any additional hardware, only using the connectivity options available on-board.
To do so this example makes use of the on-board UART connection, which is slow and unreliable at times.
If you plan to use the board more extensively, we recommend choosing either an external UART controller, or (preferred) the USB3 connectivity.

### Step 1: Load all tools
```sh
. path-to/optimsoc-environment.sh
. /opt/optimsoc/setup_prebuilt.sh
# also make sure Vivado is available in your path
```

### Step 2: Generate Bitstream
```sh
# run Vivado to generate the bitstream
# With HOST_IF=uart and UART0_SOURCE=onboard the design can be used without
# additional hardware.
fusesoc --cores-root $HOME/src/optimsoc/examples/fpga/vcu108/system_allct build optimsoc:examples:system_allct_vcu108 --UART0_SOURCE=onboard --HOST_IF=uart --XDIM 2 --YDIM 2 --LMEM_EXTERNAL
```

### Step 3: Connect Board
- Connect the Micro-USB programming cable (labeled USB JTAG, J106) to your PC
- Connect the Micro-USB UART cable to your PC (labeled USB UART, J4)
- Turn on the board

Now you should get two new UART devices on your PC, typically `/dev/ttyUSB0` and `/dev/ttyUSB1`.
Our UART connection is using the *higher-numbered device*, i.e. `/dev/ttyUSB1`.

The other device, i.e. `/dev/ttyUSB0` is connected to a onboard system controller.
You can confirm this by connecting with a terminal emulator (e.g. cutecom or GtkTerm) to `/dev/ttyUSB0` using 115200 baud, 8 data bits, no parity bit, no flow control.
After pressing any key you should see the System Controller Main Menu.

### Step 4: Program Bitstream
```sh
fusesoc --cores-root $HOME/src/optimsoc/examples/fpga/vcu108/system_allct pgm optimsoc:examples:system_allct_vcu108
```

### Step 5: Run software
```sh
osd-target-run -e ~/src/baremetal-apps/hello/hello.elf -b uart -o device=/dev/ttyUSB1,speed=921600 --coretrace --systrace -vvv --verify
```

## Connectivity
This example design supports three ways to connect the board to the PC (for loading the memories and tracing, that is. Programming the bitstream happens through another interface.)

### USB3
The connectivity option with the highest bandwidth is to use an external Cypress FX3 starter kit connected to one of the HPC connectors on the board.
To make use of the board you need to first (and once) flash it with custom firmware and set the jumpers to the right position.
This process which is documented at the [GLIP page.](http://www.glip.io/group__backend__cypressfx3-examples-vcu108__loopback.html).

Set the design parameters as follows: `HOST_IF=usb3`.

### Onboard UART
The VCU108 board has an onboard dual UART over USB chip.
Unfortunately, this chip is only usable for baud rates below 1 MBaud and no reasonable flow control.

Set the design parameters as follows: `HOST_IF=uart` and `UART0_SOURCE=onboard`.
Connect in `osd-target-run` with the `uart` backend and pass as GLIP backend option `speed=3000000`.


### External UART over PMOD
To get faster (3 MBaud) and more reliable UART connection than the one provided by the onboard UART controller you can connect a UART to USB chip to the PMOD ports available on the board.
We recommend the [Digilent USBUART pmod](https://store.digilentinc.com/pmod-usbuart-usb-to-uart-interface/).
Connect this board to bottom row of the J52 PMOD connector on the board.

Set the design parameters as follows: `HOST_IF=uart` and `UART0_SOURCE=pmod`.
Connect in `osd-target-run` with the `uart` backend and pass as GLIP backend option `speed=921600`.
