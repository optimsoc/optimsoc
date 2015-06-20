#include "obj_dir/Vtb_system_2x2_cccc__Syms.h"

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

    typedef tracemon SC_CURRENT_USER_MODULE;
    tracemon(sc_module_name name, unsigned int from = 0,
	     unsigned int to = 0) : sc_module(name), clk("clk"),
      u_from(from), u_to(to) {
        vcd = NULL;
        SC_METHOD(dump);
        sensitive << clk;
    }

    void dump()
    {
      unsigned int timestamp = sc_time_stamp().value() / 1000;
      if (vcd && (timestamp > u_from) &&
	  ((u_to == 0) || (timestamp < u_to))) {
            vcd->dump((uint32_t) timestamp);
        }
    }

    VerilatedVcdC *vcd;

    unsigned int u_from, u_to;
};

int sc_main(int argc, char *argv[])
{
    bool standalone;
    unsigned int from, to;

    standalone = ((argc > 1) &&
                  (strncmp(argv[1], "standalone", 10) == 0));

    from = (argc > 2) ? atoi(argv[2]) : 0;
    to = (argc > 3) ? atoi(argv[3]) : 0;

    srand48((unsigned int) time(0));

    Verilated::commandArgs(argc, argv);

    Vtb_system_2x2_cccc system("system");

    sc_signal<bool> rst_sys;
    sc_signal<bool> rst_cpu;
    sc_clock clk("clk", 5, SC_NS);
    sc_signal<bool> cpu_stall;

    system.clk(clk);
    system.rst_cpu(rst_cpu);
    system.rst_sys(rst_sys);
    system.cpu_stall(cpu_stall);
    
    system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__3__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();

#ifdef VCD_TRACE
    tracemon trace("trace", from, to);
    trace.clk(clk);

    Verilated::traceEverOn(true);

    VerilatedVcdC vcd;
    system.trace(&vcd, 99, 0);
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
      stm->setEnable(&system.v->trace_enable[i]);
      stm->setInsn(&system.v->trace_insn[i]);
      stm->setPC(&system.v->trace_pc[i]);
      stm->setR3(&system.v->trace_r3[i]);
      stm->setCoreId(i);
      stm->clk(clk);
    }

    sc_start();

    return 0;
}
