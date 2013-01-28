#include <utils.h>
#include <sysconfig.h>
#include <mp_simple.h>

void init() {
  // Send a flit to the hostlink
  optimsoc_report_booted();
}
