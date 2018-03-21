#include "Vtb_system_allct__Syms.h"
#include "Vtb_system_allct__Dpi.h"

#include <VerilatedToplevel.h>
#include <VerilatedControl.h>

#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace simutilVerilator;

VERILATED_TOPLEVEL(tb_system_allct, clk, rst)

int main(int argc, char *argv[])
{
    int xdim, ydim;
    tb_system_allct sys("TOP");

    VerilatedControl &simctrl = VerilatedControl::instance();
    simctrl.init(sys, argc, argv);

    svSetScope(svGetScopeFromName ("TOP.tb_system_allct"));
    xdim = sys.getXDIM();
    ydim = sys.getYDIM();

    for (int x = 0; x < xdim; ++x) {
        for (int y = 0; y < ydim; ++y) {
            std::ostringstream stringStream;
            stringStream << "TOP.tb_system_allct.u_system.gen_cty[" << y << "].gen_ctx[" << x << "].u_ct.gen_sram.u_ram.sp_ram.gen_sram_sp_impl.u_impl";
            simctrl.addMemory(stringStream.str().c_str());
        }
    }
    simctrl.setMemoryFuncs(do_readmemh, do_readmemh_file);
    simctrl.run();

    return 0;
}
