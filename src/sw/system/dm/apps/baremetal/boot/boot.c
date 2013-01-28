#include <utils.h>
#include <sysconfig.h>
#include <mp_simple.h>

void main() {
  // Send a flit to the hostlink
  optimsoc_send_alive_message();
}
