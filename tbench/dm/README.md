# Distributed Memory Systems

## Systems without diagnosis system
For most software development in simulation, the diagnosis system support is
not required, as all signals and traces are available on the simulation
machine.

All data coming from `printf()` calls in the software ("simulated stdout") are
written to the file `stdout`.
To obtain a full instruction trace, set the parameter `ENABLE_TRACE` in the
top-level design file.

The following distributed memory systems without diagnosis system are
available:

 * `compute_tile`: A single compute tile without NoC

 * `compute_tile-dual`: A compute tile with two cores

 * `compute_tile-quad`: A compute tile with four cores

 * `compute_tile-octa`: A compute tile with eight cores

 * `system_2x2_cccc`: A 2x2 mesh NoC with four compute tiles

 * `system_2x2_cccc_dual`: The same system with two cores per tile

 * `system_2x2_cccc_quad`: The same system with four cores per tile

## Systems with integrated diagnosis system

Designs for synthesis usually include the diagnosis system to get insight into
the system behavior. For development of the diagnosis tools, or to use the same
tools (based on the liboptimsochost library, such as optimsocgui), simulations
with diagnosis support are available. Instead of using a board-specific
communication method (such as USB) to transfer the diagnosis data to the
developer PC, the simulations use the TCP backend of GLIP.

 * `system_2x2_cccc_diasys`: The `system_2x2_cccc` with added diagnosis
   support.
