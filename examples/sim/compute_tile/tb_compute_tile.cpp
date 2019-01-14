#include "Vtb_compute_tile__Syms.h"
#include "Vtb_compute_tile__Dpi.h"

#include <VerilatedToplevel.h>
#include <VerilatedControl.h>

#include <Tracer.h>

#include <ctime>
#include <cstdlib>

using namespace simutilVerilator;

VERILATED_TOPLEVEL(tb_compute_tile,clk, rst)

int main(int argc, char *argv[])
{
    tb_compute_tile ct("TOP");

    VerilatedControl &simctrl = VerilatedControl::instance();
    simctrl.init(ct, argc, argv);

    bool trace = false;

    for (size_t i = 0; i < argc; i++) {
      if (strcmp(argv[i], "--trace") == 0) {
        trace = true;
      }
    }

    if (trace) {
      Tracer::instance().init(false);
    }

    simctrl.addMemory("TOP.tb_compute_tile.u_compute_tile.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl");
    simctrl.setMemoryFuncs(do_readmemh, do_readmemh_file);
    simctrl.run();

    return 0;
}
