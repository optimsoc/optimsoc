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

	gzll_mp_endpoint_t local_ep;

	gzll_mp_endpoint_create(&local_ep, 0, OPTIMSOC_MP_EP_CHANNEL, 4, 4);
}
