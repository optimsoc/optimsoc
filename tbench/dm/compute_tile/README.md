# Compute Tile System

This is a simple system that only contains a single processor core in
one compute tile and no Network-on-Chip.

## Run Verilator Simulation

First build the simulation

    make build-verilator

Then you should build an application from optimsoc/baremetal-apps,
e.g., hello

    make -C ${OPTIMSOC_APPS}/baremetal/hello

Then you need to link to the memory initialization file (`ct.vmem`)

    ln -s ${OPTIMSOC_APPS}/baremetal/hello/hello.vmem ct.vmem

Now you can run the simulation

    ./tb_compute_tile standalone

After excution you can find the output

    cat stdout.0

that should read as

     Hello World! Core 0 of 1 in tile 0, my absolute core id is: 0

You can find all emitted events in `events.0`.

Finally you can also debug hardware using the vcd waveform

    ./tb_compute_tile-vcd standalone

and inspect the waveform

    gtkwave sim.vcd