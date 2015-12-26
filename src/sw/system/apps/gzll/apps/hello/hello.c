
#include <stdio.h>
#include <gzll.h>

#include <stdlib.h>

int main(int argc, char* argv[]) {
	char *kernel_version;
	printf("Hello World!\n");

	gzll_get_kernel_version(&kernel_version);

	printf("gzll version: %s\n", kernel_version);

	free(kernel_version);
}
