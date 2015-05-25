#include <stdio.h>
#include <assert.h>

#include <gzll.h>

void main() {
	int rv;
	gzll_node_id receiver_node;
	printf("Sender started, node id: %d\n", gzll_self());

	rv = gzll_lookup_nodeid("receiver-0", &receiver_node);
	assert(rv == 0);

	printf("Looked up receiver id: %d\n", receiver_node);
}
