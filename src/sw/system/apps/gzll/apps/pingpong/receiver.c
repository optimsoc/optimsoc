#include <stdio.h>
#include <assert.h>

#include <gzll.h>

void main() {
	int rv;
	gzll_node_id sender_node;

	printf("Receiver started\n");

	rv = gzll_lookup_nodeid("sender-0", &sender_node);
	assert(rv == 0);

	printf("Looked up receiver id: %d\n", sender_node);
}
