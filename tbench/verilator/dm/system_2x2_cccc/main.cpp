#include "obj_dir/Vtb_system_2x2_cccc__Syms.h"

#include <debug/VerilatedDebugConnector.h>
#include <debug/VerilatedSTM.h>

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
    Vtb_system_2x2_cccc system("system");

    sc_signal<bool> rst_sys;
    sc_signal<bool> rst_cpu;
    sc_clock clk("clk", 5, SC_NS);
    sc_signal<bool> cpu_stall;

    system.clk(clk);
    system.rst_cpu(rst_cpu);
    system.rst_sys(rst_sys);

    system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();
    system.v->u_system->gen_ct__BRA__3__KET____DOT__u_ct->u_ram->sp_ram->gen_sram_sp_impl__DOT__u_impl->do_readmemh();

#ifdef VCD_TRACE
    tracemon trace("trace");
    trace.clk(clk);

    Verilated::traceEverOn(true);

    VerilatedVcdC vcd;
    system.trace(&vcd, 99, 0);
    vcd.open("sim.vcd");
    trace.vcd = &vcd;
#endif

    VerilatedDebugConnector debugconn("DebugConnector", 2);
    debugconn.rst_sys(rst_sys);
    debugconn.rst_cpu(rst_cpu);

    for (int i = 0; i < 4; i++) {
      char *stm_name = (char*) malloc(8);
      snprintf(stm_name, 7, "STM%d", i);
      
      VerilatedSTM * stm = new VerilatedSTM(stm_name, &debugconn);
      debugconn.registerDebugModule(stm);
      stm->setEnable(&system.v->trace_stm_enable[i]);
      stm->setInsn(&system.v->trace_stm_insn[i]);
      stm->setR3(&system.v->trace_stm_r3[i]);
      stm->setCoreId(i);

      stm->clk(clk);
    }

    sc_start();

    return 0;
}

