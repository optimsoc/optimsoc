#ifndef __GZLL_H__
#define __GZLL_H__

#include <stdint.h>

void gzll_get_kernel_version(char **version_string);

typedef uint32_t gzll_node_id;
gzll_node_id gzll_self(void);
int gzll_lookup_nodeid(char *name, gzll_node_id *nodeid);

typedef struct gzll_mp_endpoint* gzll_mp_endpoint_t;

int gzll_mp_endpoint_get(gzll_mp_endpoint_t *ep, gzll_node_id node,
                         uint32_t port);

#endif
