#include "obj_dir/Vtb_system_2x2_cccc__Syms.h"

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
    Vtb_system_2x2_cccc system("system");

    sc_signal<bool> rst_sys;
    sc_signal<bool> rst_cpu;
    sc_clock clk("clk", 5, SC_NS);
    sc_signal<bool> cpu_stall;

    system.clk(clk);
    system.rst_cpu(rst_cpu);
    system.rst_sys(rst_sys);

    system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_ram->memory->do_readmemh();
    system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_ram->memory->do_readmemh();
    system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_ram->memory->do_readmemh();
    system.v->u_system->gen_ct__BRA__3__KET____DOT__u_ct->u_ram->memory->do_readmemh();

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

    VerilatedSTM stm0("STM0", &debugconn);
    debugconn.registerDebugModule(&stm0);
    stm0.setWbInsn(&system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_insn);
    stm0.setWbFreeze(&system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_freeze);
    stm0.setR3(&system.v->u_system->gen_ct__BRA__0__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_rf->rf_a->mem[3]);
    stm0.setCoreId(0);

    VerilatedSTM stm1("STM1", &debugconn);
    debugconn.registerDebugModule(&stm1);
    stm1.setWbInsn(&system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_insn);
    stm1.setWbFreeze(&system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_freeze);
    stm1.setR3(&system.v->u_system->gen_ct__BRA__1__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_rf->rf_a->mem[3]);
    stm1.setCoreId(1);

    VerilatedSTM stm2("STM2", &debugconn);
    debugconn.registerDebugModule(&stm2);
    stm2.setWbInsn(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_insn);
    stm2.setWbFreeze(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_freeze);
    stm2.setR3(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_rf->rf_a->mem[3]);
    stm2.setCoreId(2);

    VerilatedSTM stm3("STM3", &debugconn);
    debugconn.registerDebugModule(&stm3);
    stm3.setWbInsn(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_insn);
    stm3.setWbFreeze(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_ctrl->wb_freeze);
    stm3.setR3(&system.v->u_system->gen_ct__BRA__2__KET____DOT__u_ct->u_core0->u_cpu->or1200_cpu->or1200_rf->rf_a->mem[3]);
    stm3.setCoreId(3);


    stm0.clk(clk);
    stm1.clk(clk);
    stm2.clk(clk);
    stm3.clk(clk);

    sc_start();

    return 0;
}

