# 2x2 Compute Tile System 2x2 Mesh NoC and Diagnosis System

This system contains four compute tiles in a 2x2 mesh Network-on-Chip
with each one core per tile. Diagnosis support is enabled and the GLIP TCP
backend is used for communication with liboptimsochost.


## Run the ModelSim Simulation

First build the simulation

    make build-modelsim

Then you should build an application from optimsoc/baremetal-apps,
e.g., hello

    make -C $OPTIMSOC_APPS/baremetal/hello

Then you need to link to the memory initialization file (`ct.vmem`)

    ln -s $OPTIMSOC_APPS/baremetal/hello/hello.vmem ct.vmem

Now you can run the simulation

    # with GUI (you need to click "Run -All" in the UI to start the simulation)
    make run-modelsim-gui

    # or without GUI
    make run-modelsim

After the simulation has started, you should see a console output like

    # Now listening on port 23000 for incoming connections.
    # Waiting for incoming connection ...
    # Now listening on port 23001 for incoming connections.
    # Waiting for incoming connection ...

This indicates that the simulation is now ready and you can connect with one
of the host tools: either the OpTiMSoC GUI, or the command line tool
`optimsoc_cli`.

Let's start with the command line tool. Open a new console (in addition to the
one running ModelSim) and start optimsoc_cli. You should get a list of all
available debug modules, and then a command prompt is waiting for you. We'll get
back to that one in a minute.

    optimsoc_cli -bdbgnoc -oconn=tcp -i

First, switch to the ModelSim console. You should see that a client connected
(the ports may differ):

    # New client connected: 127.0.0.1:59738
    # New client connected: 127.0.0.1:47606

Now go back to `optimsoc_cli` and type some commands into the prompt there.

    log_stm_trace stm.trace
    start

The first command starts a system trace (with events and stdout messages), and
the second command starts the CPUs and runs the application. (Type `help`
for more details.)

After a moment, the message `Software on all cores has terminated`
tells you it's time to close `optimsoc_cli` by typing `quit`. Have a look
at the `stm.trace` file for the logged output.


## Update GLIP
This design contains the GLIP communication interface. To update to a newer
version, copy the two GLIP toplevel files into this directory.

    cp GLIP_SRC/src/backend_tcp/logic/systemc/glip_tcp_toplevel.cpp .
    cp GLIP_SRC/src/backend_tcp/logic/systemc/glip_tcp_toplevel.h .

