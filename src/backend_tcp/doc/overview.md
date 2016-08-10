@defgroup backend_tcp TCP
@ingroup backend

The TCP backend (internal name: `tcp`) connects over a TCP channel to a
target. Currently, the we only provide a SystemC model as target, which can be
used to communicate with simulated systems, e.g. in ModelSim.

The backend uses two TCP channels, one for data communication, and one for
control traffic. The target acts as TCP server, which listens to two TCP ports
(23000 and 23001 by default), and the host connects to those ports.

It should be easily possible to extend this backend for different targets, for
example to communicate with a Zedboard.

Supported Features
==================
- Number of channels: 1 (fixed)
- FIFO width: 16 bit (fixed)
