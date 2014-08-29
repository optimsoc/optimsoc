#include <optimsoc-runtime.h>
#include <stdio.h>

int main() {
  optimsoc_runtime_boot();
}

void init() {
  printf("Init task is running\n");
  while (1) {}
}
