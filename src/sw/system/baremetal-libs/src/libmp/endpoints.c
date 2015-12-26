#include "endpoints.h"
#include "control.h"

#include <stdlib.h>
#include <assert.h>

// The endpoint handles are stored in a linked list with this element
struct endpoint_handle_listelem {
    struct endpoint_handle_listelem *next;
    struct endpoint_handle          *handle;
};

// Pointers to the actual list
struct endpoint_handle_listelem *eplist_first;
struct endpoint_handle_listelem *eplist_last;

// Counters of local and remote endpoints
unsigned int eplist_localnum;
unsigned int eplist_remotenum;


// Add an endpoint to the list by its handle
int endpoint_add(struct endpoint_handle *ep) {
    struct endpoint_handle_listelem *e = malloc(sizeof(struct endpoint_handle_listelem));
    e->handle = ep;
    e->next = 0;

    if (eplist_first==0) {
        eplist_first = e;
        eplist_last = e;
    } else {
        eplist_last->next = e;
        eplist_last = e;
    }

    if (ep->type == LOCAL) {
        eplist_localnum++;
    } else {
        eplist_remotenum++;
    }

    return 0;
}

// Get an endpoint from the list by its metainformation
struct endpoint_handle *endpoint_get(unsigned int domain, unsigned int node,
                                     unsigned int port) {
    struct endpoint_handle *eph = 0;

    // Try to find endpoint in local database
    struct endpoint_handle_listelem *lit;
    for (lit=eplist_first;lit!=0;lit=lit->next) {
        if ((lit->handle->domain == domain) &&
                (lit->handle->node == node) &&
                (lit->handle->port == port)) {
            eph = lit->handle;
            break;
        }
    }

    if (eph == 0 && (domain != optimsoc_get_tileid())) {
        // Endpoint is not locally known
        struct endpoint *ep = control_get_endpoint(domain, node, port);
        // Create the respective handle

        eph = malloc(sizeof(struct endpoint_handle));
        eph->domain = domain;
        eph->node = node;
        eph->port = port;
        eph->ep = ep;
        eph->type = REMOTE;

        endpoint_add(eph);

    }

    return eph;
}

/// Verify that endpoint handle is valid
struct endpoint_handle *endpoint_verify(struct endpoint_handle *eph) {
    struct endpoint_handle_listelem *lit;
    for (lit=eplist_first;lit!=0;lit=lit->next) {
        if (lit->handle == eph) {
            return eph;
        }
    }

    return 0;
}

// Delete an endpoint from the list
void endpoint_delete(struct endpoint_handle *eph) {
    struct endpoint_handle_listelem *lit;

    if (eph->type==LOCAL) {
        eplist_localnum--;
    } else {
        eplist_remotenum--;
    }

    if (eplist_first->handle == eph) {
        free(eplist_first);
        if (eplist_first==eplist_last) {
            eplist_first = 0;
            eplist_last = 0;
        } else {
            eplist_first = eplist_first->next;
        }
        return;
    }

    for (lit = eplist_first;lit != 0;lit=lit->next) {
        if (lit->next &&
                (lit->next->handle == eph) ) {
            free(lit->next);
            lit->next = lit->next->next;
            return;
        }
        lit = lit->next;
    }
}

int endpoint_generate_portnum(unsigned int *port) {
    struct endpoint_handle_listelem *lit;
    for (uint32_t p = 0;p<MAX_ENDPOINTS;p++) {
        unsigned int found = 0;
        for (lit=eplist_first;lit!=0;lit=lit->next) {
            if (lit->handle->port == p) {
                found = 1;
                break;
            }
        }
        if (found==0) {
            *port = p;
            return 0;
        }
    }

    return -1;
}

unsigned int endpoints_localnum() {
    return eplist_localnum;
}

void endpoints_init() {
    // Initialize endpoint list
    eplist_first = 0;
    eplist_last = 0;
    eplist_localnum = 0;
    eplist_remotenum = 0;
}

struct endpoint_handle *endpoint_create(uint32_t node, uint32_t port,
                                        endpoint_buffer_type buffer_type,
                                        uint32_t buffer_size,
                                        int overwrite_max_size) {

    struct endpoint *ep = malloc(sizeof(struct endpoint));
    assert(ep!=0);

    ep->buffer = calloc(1, sizeof(struct endpoint_buffer));
    assert(ep->buffer != 0);

    ep->buffer->data = malloc(buffer_size * sizeof(uint32_t*));
    assert(ep->buffer->data != 0);

    ep->buffer->data_size = malloc(buffer_size * sizeof(uint32_t*));
    assert(ep->buffer->data_size != 0);

    uint32_t max_element_size_bytes;
    uint32_t max_element_size_words;

    if (overwrite_max_size > 0) {
        max_element_size_bytes = overwrite_max_size;
    } else {
        if (buffer_type == MESSAGE) {
            max_element_size_bytes = MAX_MESSAGE_SIZE;
        } else {
            max_element_size_bytes = MAX_PACKET_SIZE;
        }
    }

    max_element_size_words = (max_element_size_bytes + 3) >> 2;

    uint32_t *datafield = malloc(max_element_size_bytes * buffer_size);
    assert(datafield);

    int i;
    for (i = 0; i < buffer_size; i++) {
        ep->buffer->data[i] = &datafield[i*max_element_size_words];
    }

    ep->buffer->size = buffer_size;
    ep->buffer->write_ptr = 0;
    ep->buffer->read_ptr = 0;

#ifdef RUNTIME
    ep->waiting_thread = 0;
#endif


    struct endpoint_handle *eph = malloc(sizeof(struct endpoint_handle));
    assert(eph!=0);

    eph->ep = ep;
    eph->domain = optimsoc_get_tileid();
    eph->node = node;
    eph->port = port;

    trace_ep_create(ep);
    endpoint_add(eph);

    return eph;
}

int endpoint_pop(struct endpoint *ep, uint32_t *ptr) {
    uint32_t newptr;

    // TODO: what about other concurrency
    do {
        *ptr = ep->buffer->read_ptr;
        newptr = *ptr + 1;
        if (ep->buffer->size <= newptr) {
            newptr -= ep->buffer->size;
        }
    } while (or1k_sync_cas((void*) &ep->buffer->read_ptr, *ptr, newptr) != *ptr);

    return 0;
}

int endpoint_push(struct endpoint *ep, uint32_t *ptr) {
    uint32_t newptr;

    // TODO: what about other concurrency
    do {
        *ptr = ep->buffer->write_ptr;
        newptr = *ptr + 1;
        if (ep->buffer->size <= newptr) {
            newptr -= ep->buffer->size;
        }
    } while (or1k_sync_cas((void*) &ep->buffer->write_ptr, *ptr, newptr) != *ptr);

    return 0;
}

int endpoint_empty(struct endpoint *ep) {
    return !endpoint_msg_available(ep);
}

int endpoint_full(struct endpoint *ep) {
    if (ep->buffer->write_ptr >= ep->buffer->read_ptr) {
        return ((ep->buffer->write_ptr - ep->buffer->read_ptr) == ep->buffer->size-1);
    } else {
        return ((ep->buffer->read_ptr - ep->buffer->write_ptr) == 1);
    }
}

int endpoint_alloc(struct endpoint *ep, uint32_t size, uint32_t *ptr) {

    if (ep->buffer->type == MESSAGE) {
        if (size > MAX_MESSAGE_SIZE) {
            return -1;
        }
    } else {
        if (size > MAX_PACKET_SIZE) {
            return -1;
        }
    }

    if (!endpoint_full(ep)) {
        // The sender can write to this address
        endpoint_push(ep, ptr);

        // Zero size (will also signal completeness)
        ep->buffer->data_size[*ptr] = 0;

        // Return acknowledge
        return 0;
    } else {
        // Return error
        return -1;
    }

}

void endpoint_write(struct endpoint *ep, uint32_t ptr, uint32_t offset,
                    uint32_t *buffer, uint32_t size) {
    // Write data to buffer
    for (int i=0; i<size; i++){
        ep->buffer->data[ptr][i+offset] = buffer[i];
    }
}

void endpoint_write_complete(struct endpoint *ep, uint32_t ptr, uint32_t size) {
    ep->buffer->data_size[ptr] = size;
}

int endpoint_msg_available(struct endpoint *ep) {
    if ((ep->buffer->write_ptr == ep->buffer->read_ptr) ||
            (ep->buffer->data_size[ep->buffer->read_ptr] == 0)) {
        return 0;
    } else {
        return 1;
    }

}

void endpoint_msg_recv(struct endpoint *ep, uint32_t *buffer,
                       uint32_t buffer_size, uint32_t *received) {

    while (!endpoint_msg_available(ep)) {}

    *received = ep->buffer->data_size[ep->buffer->read_ptr];

    if(*received > buffer_size) {
        *received = 0;
        return;
    }

    uint32_t wordsize = (*received + 3) >> 2;

    uint32_t ptr;
    endpoint_pop(ep, &ptr);

    for(int i=0; i<wordsize; i++) {
        buffer[i] = ep->buffer->data[ptr][i];
    }
}

uint32_t endpoint_channel_get_credit(struct endpoint *ep) {
    uint32_t wrptr = ep->buffer->write_ptr;
    uint32_t rdptr = ep->buffer->read_ptr;


    if (wrptr >= rdptr) {
        return (ep->buffer->size - 1 - wrptr + rdptr);
    } else {
        return (rdptr - 1 -wrptr);
    }
}

uint32_t endpoint_channel_get_fillstate(struct endpoint *ep) {
    return ep->buffer->size - endpoint_channel_get_credit(ep) - 1;
}

void endpoint_channel_add_credit(struct endpoint *ep, uint32_t credit) {
    uint32_t old_credit, new_credit;

    //    do {
        old_credit = ep->remotecredit;
        new_credit = old_credit + credit;
    //} while (cas(&ep->remotecredit, old_credit, new_credit) != old_credit);

    ep->remotecredit = new_credit;
}
