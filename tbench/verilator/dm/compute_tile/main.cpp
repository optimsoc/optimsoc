#include "obj_dir/Vtb_compute_tile__Syms.h"

#include "VerilatedDebugConnector.h"
#include "VerilatedSTM.h"

#include <verilated_vcd_c.h>

SC_MODULE(tracemon) {
    sc_in<bool> clk;

    SC_CTOR(tracemon) : clk("clk") {
        vcd = NULL;
        SC_METHOD(dump);
        sensitive << clk;
    }

    void dump()
    {
        if (vcd) {
            vcd->dump((uint32_t)(sc_time_stamp().value() / 1000));
        }
    }

    VerilatedVcdC *vcd;
};

int sc_main(int argc, char *argv[])
{
    Vtb_compute_tile ct("CT");

    sc_signal<bool> rst_sys;
    sc_signal<bool> rst_cpu;
    sc_clock clk("clk", 5, SC_NS);
    sc_signal<bool> cpu_stall;

    ct.clk(clk);
    ct.rst_cpu(rst_cpu);
    ct.rst_sys(rst_sys);
    ct.cpu_stall(cpu_stall);

    ct.v->u_compute_tile->u_ram->memory->do_readmemh();

#ifdef VCD_TRACE
    tracemon trace("trace");
    trace.clk(clk);

    Verilated::traceEverOn(true);

    VerilatedVcdC vcd;
    ct.trace(&vcd, 99, 0);
    vcd.open("sim.vcd");
    trace.vcd = &vcd;
#endif

    VerilatedDebugConnector debugconn("DebugConnector");
    debugconn.rst_sys(rst_sys);
    debugconn.rst_cpu(rst_cpu);

    VerilatedSTM stm("STM", &debugconn);
    debugconn.registerDebugModule(&stm);
    stm.setWbInsn(&ct.v->u_compute_tile->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_insn);
    stm.setWbFreeze(&ct.v->u_compute_tile->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_freeze);
    stm.setR3(&ct.v->u_compute_tile->u_core0->u_cpu->or1200_cpu->or1200_rf->rf_a->mem[3]);
    stm.setCoreId(0);

    stm.clk(clk);

    sc_start();

    return 0;
}

