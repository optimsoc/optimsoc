#include "obj_dir/Vtb_compute_tile__Syms.h"

#include "debug/VerilatedDebugConnector.h"
#include "debug/VerilatedSTM.h"

#include <verilated_vcd_c.h>

#include <ctime>
#include <cstdlib>

#ifndef NUMCORES
#define NUMCORES 1
#endif

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
    bool standalone;

    standalone = ((argc > 1) &&
                  (strncmp(argv[1], "standalone", 10) == 0));

    srand48((unsigned int) time(0));

    Verilated::commandArgs(argc, argv);

    Vtb_compute_tile ct("CT");

    sc_signal<bool> rst_sys;
    sc_signal<bool> rst_cpu;
    sc_clock clk("clk", 5, SC_NS);
    sc_signal<bool> cpu_stall;

    ct.clk(clk);
    ct.rst_cpu(rst_cpu);
    ct.rst_sys(rst_sys);
    ct.cpu_stall(cpu_stall);

    ct.v->u_compute_tile->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();

#ifdef VCD_TRACE
    tracemon trace("trace");
    trace.clk(clk);

    Verilated::traceEverOn(true);

    VerilatedVcdC vcd;
    ct.trace(&vcd, 99, 0);
    vcd.open("sim.vcd");
    trace.vcd = &vcd;
#endif

    VerilatedDebugConnector debugconn("DebugConnector", 0xc200, standalone);
    debugconn.clk(clk);
    debugconn.rst_sys(rst_sys);
    debugconn.rst_cpu(rst_cpu);

    for (int i = 0;  i < NUMCORES; i++) {
      char name[64];
      snprintf(name, 64, "STM%d", i);
      VerilatedSTM *stm = new VerilatedSTM(name, &debugconn);
      debugconn.registerDebugModule(stm);
      stm->setEnable(&ct.v->trace_enable[i]);
      stm->setInsn(&ct.v->trace_insn[i]);
      stm->setPC(&ct.v->trace_pc[i]);
      stm->setR3(&ct.v->trace_r3[i]);
      stm->setCoreId(i);
      stm->clk(clk);
    }

    sc_start();

    return 0;
}
