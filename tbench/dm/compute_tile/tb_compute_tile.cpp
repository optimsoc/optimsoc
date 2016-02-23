#include "obj_dir/Vtb_compute_tile__Syms.h"
#include "obj_dir/Vtb_compute_tile__Dpi.h"

#include <VerilatedToplevel.h>
#include <VerilatedControl.h>

#include <verilated_vcd_c.h>

#include <ctime>
#include <cstdlib>

#ifndef NUMCORES
#define NUMCORES 1
#endif

using namespace optimsoc;

VERILATED_TOPLEVEL(tb_compute_tile)

int main(int argc, char *argv[])
{
    tb_compute_tile ct("CT");

    VerilatedControl &simctrl = VerilatedControl::instance();
    simctrl.init(ct, argc, argv);

    simctrl.addMemory("CT.v.u_compute_tile.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.setMemoryFuncs(do_readmemh, do_readmemh_file);

    simctrl.run();

    return 0;
}
