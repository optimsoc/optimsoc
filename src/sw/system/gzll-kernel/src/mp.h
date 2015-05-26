#ifndef __MP_H__
#define __MP_H__

#include <optimsoc-mp.h>

struct gzll_endpoint_table {
    uint32_t len;
    optimsoc_mp_endpoint_handle ep[0];
};

void endpoint_table_init(struct gzll_endpoint_table **table);

void endpoint_table_add(struct gzll_endpoint_table *table, uint32_t port,
                        optimsoc_mp_endpoint_handle endpoint);

#endif
