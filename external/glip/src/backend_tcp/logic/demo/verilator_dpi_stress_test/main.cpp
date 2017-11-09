#include <verilated.h>
#include <verilated_vcd_c.h>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include "obj_dir/Vtb_verilator_dpi_stress_test.h"

#include "GlipTcp.h"

using namespace std;

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    vector<string> args(argv + 1, argv + argc);

    bool vcd_enable;
    vluint64_t end;

    for(vector<string>::iterator it = args.begin(); it != args.end(); ++it) {
      if(*it == "+vcd")
        vcd_enable = true;
      else if(it->find("+end=") == 0) {
        end = 10 * strtoul(it->substr(strlen("+end=")).c_str(), NULL, 10);
      }
    }


    Vtb_verilator_dpi_stress_test *top = new Vtb_verilator_dpi_stress_test();
    top->rst = 1;
    top->eval();

    // VCD dump
    VerilatedVcdC* vcd = new VerilatedVcdC;
    if(vcd_enable) {
        Verilated::traceEverOn(true);
        top->trace(vcd, 99);
        vcd->open("demo.vcd");
        vcd->dump(0);
    }

    vluint64_t time = 0;
    GlipTcp *glip = &GlipTcp::instance();
    while(!Verilated::gotFinish() && (time < end)) {
        if (!glip->connected()) {
            continue;
        }

        time += 5;

        if (time == 15) {
            top->rst = 0;
        }

        top->clk = 1 - top->clk;
        top->eval();

        if (vcd_enable) {
            vcd->dump(time);
        }
    }

    top->final();

    if (vcd_enable) {
        vcd->close();
    }

    return 0;
}
