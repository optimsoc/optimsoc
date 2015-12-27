#include "obj_dir/Vtutorial_02.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

int main(int argc, char **argv, char **env) {
  int i;
  int clk;

  Vtutorial_02 top;

  Verilated::traceEverOn(true);
  VerilatedVcdC tf;
  top.trace (&tf, 99);
  tf.open("tutorial_02.vcd");

  top.clk = 1;
  top.rst = 0;

  for (i=0; i<100; i++) {
    top.rst = (i < 4);

    tf.dump (i);
    top.clk = !top.clk;
    top.eval ();

    if (Verilated::gotFinish()) {
      exit(0);
    }
  }

  tf.close();

  exit(0);
}
