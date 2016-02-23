#include "obj_dir/Vtb_system_2x2_cccc__Syms.h"

#include <VerilatedToplevel.h>
#include <VerilatedControl.h>

#include <verilated_vcd_c.h>

#include <ctime>
#include <cstdlib>

#ifndef NUMCORES
#define NUMCORES 1
#endif

using namespace optimsoc;

VERILATED_TOPLEVEL(tb_system_2x2_cccc)

int main(int argc, char *argv[])
{
    tb_system_2x2_cccc system("System");

    VerilatedControl &simctrl = VerilatedControl::instance();
    simctrl.init(system, argc, argv);

    simctrl.addMemory("System.v.u_system.gen_ct[0].u_ct.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("System.v.u_system.gen_ct[1].u_ct.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("System.v.u_system.gen_ct[2].u_ct.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("System.v.u_system.gen_ct[3].u_ct.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.setMemoryFuncs(do_readmemh, do_readmemh_file);

    simctrl.run();

    return 0;
}
