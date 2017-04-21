#include "Vtb_system_2x2_cccc__Syms.h"
#include "Vtb_system_2x2_cccc__Dpi.h"

#include <VerilatedToplevel.h>
#include <VerilatedControl.h>

#include <Tracer.h>

#include <ctime>
#include <cstdlib>

using namespace simutilVerilator;

VERILATED_TOPLEVEL(tb_system_2x2_cccc, clk, rst)

int main(int argc, char *argv[])
{
    tb_system_2x2_cccc ct("TOP");

    VerilatedControl &simctrl = VerilatedControl::instance();
    simctrl.init(ct, argc, argv);

    bool trace = false;
    bool tracenocfull = false;

    for (size_t i = 0; i < argc; i++) {
      if (strcmp(argv[i], "--trace") == 0) {
	trace = true;
      }
      if (strcmp(argv[i], "--trace-noc-full") == 0) {
	tracenocfull = true;
      }
    }

    if (trace) {
      Tracer::instance().init(tracenocfull);
    }

    simctrl.addMemory("TOP.tb_system_2x2_cccc.u_system.gen_ct[0].u_ct.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("TOP.tb_system_2x2_cccc.u_system.gen_ct[1].u_ct.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("TOP.tb_system_2x2_cccc.u_system.gen_ct[2].u_ct.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.addMemory("TOP.tb_system_2x2_cccc.u_system.gen_ct[3].u_ct.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.setMemoryFuncs(do_readmemh, do_readmemh_file);
    simctrl.run();

    return 0;
}
