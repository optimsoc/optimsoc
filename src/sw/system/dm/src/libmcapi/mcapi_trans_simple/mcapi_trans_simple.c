/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */
#include <stdlib.h>
#include <stdio.h>

#include <optimsoc.h>
#include <optimsoc-sysconfig.h>

#include <optimsoc-baremetal.h>
#include <mcapi_trans.h>

#ifdef RUNTIME
#include <optimsoc-runtime.h>
#endif

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************/
// Specific endpoint handling
/*******************************************************************/

// The endpoint data structure holds the actual buffer and the flags
// It can only hold one message at the moment
struct endpoint {
    volatile unsigned int *buffer;
    volatile unsigned int cur_message_size;
    volatile unsigned int cur_message_ptr;
    volatile unsigned int flags;
#ifdef RUNTIME
    volatile unsigned int waiting;
    volatile thread_t waiting_thread;
#endif
};

#define EP_FLAG_READY 1
#define EP_FLAG_ALLOC 2
#define EP_FLAG_MSG   4

// The endpoint handle contains the metainformation of an endpoint
// and a pointer to the actual endpoint if local
struct endpoint_handle {
    enum { LOCAL, REMOTE } type;
    struct endpoint  *ep;
    mcapi_domain_t   domain;
    mcapi_node_t     node;
    mcapi_port_t     port;
};

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
    e->next = NULL;

    if (eplist_first==NULL) {
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
struct endpoint_handle *endpoint_get(mcapi_domain_t domain, mcapi_node_t node, mcapi_port_t port) {
    struct endpoint_handle *eph = NULL;
    struct endpoint_handle_listelem *lit;
    for (lit=eplist_first;lit!=NULL;lit=lit->next) {
        if ((lit->handle->domain == domain) &&
                (lit->handle->node == node) &&
                (lit->handle->port == port)) {
            eph = lit->handle;
            break;
        }
    }
    return eph;
}

/// Verify that endpoint handle is valid
struct endpoint_handle *endpoint_verify(struct endpoint_handle *eph) {
    struct endpoint_handle_listelem *lit;
    for (lit=eplist_first;lit!=NULL;lit=lit->next) {
        if (lit->handle == eph) {
            return eph;
        }
    }

    return NULL;
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
            eplist_first = NULL;
            eplist_last = NULL;
        } else {
            eplist_first = eplist_first->next;
        }
        return;
    }

    for (lit = eplist_first;lit != NULL;lit=lit->next) {
        if (lit->next &&
                (lit->next->handle == eph) ) {
            free(lit->next);
            lit->next = lit->next->next;
            return;
        }
        lit = lit->next;
    }
}

mcapi_boolean_t endpoint_generate_portnum(mcapi_uint_t *port) {
    struct endpoint_handle_listelem *lit;
    for (mcapi_uint_t p = 0;p<MCAPI_MAX_ENDPOINTS;p++) {
        unsigned int found = 0;
        for (lit=eplist_first;lit!=NULL;lit=lit->next) {
            if (lit->handle->port == p) {
                found = 1;
                break;
            }
        }
        if (found==0) {
            *port = p;
            return MCAPI_TRUE;
        }
    }

    return MCAPI_FALSE;
}

// Indicates whether the transport has been initialized
unsigned int initialized = 0;

mcapi_domain_t mcapi_domainid;
mcapi_node_t mcapi_nodenum;

// The protocol for the MCAPI message passing uses control
// messages via the NoC.
struct {
    uint32_t buffer[16];
    volatile unsigned int done;
} ctrl_request;

#define CTRL_REQUEST_MSB (OPTIMSOC_SRC_LSB-1)
#define CTRL_REQUEST_LSB (CTRL_REQUEST_MSB-2)
#define CTRL_REQUEST_GETEP_REQ      0
#define CTRL_REQUEST_GETEP_RESP     1
#define CTRL_REQUEST_MSG_ALLOC_REQ  2
#define CTRL_REQUEST_MSG_ALLOC_RESP 3
#define CTRL_REQUEST_MSG_DATA       4
#define CTRL_REQUEST_5              5
#define CTRL_REQUEST_6              6
#define CTRL_REQUEST_7              7
// HINT: if you add more, you have to adjust LSB!

#define CTRL_REQUEST_ACK 1
#define CTRL_REQUEST_NACK 0

#define EXTRACT(x,msb,lsb) ((x>>lsb) & ~(~0 << (msb-lsb+1)))
#define SET(x,v,msb,lsb) (((~0 << (msb+1) | ~(~0 << lsb))&x) | ((v & ~(~0<<(msb-lsb+1))) << lsb))

// The following handler is called by the message interrupt service routine
void mcapi_trans_recvctl_handler(unsigned int* buffer,int len) {
    // Extract sender information
    mcapi_node_t src = EXTRACT(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
    // Extract request type
    int req = EXTRACT(buffer[0],CTRL_REQUEST_MSB,CTRL_REQUEST_LSB);

    // Reply buffer
    uint32_t rbuffer[5];

    // Handle the respective request
    switch (req) {
    case CTRL_REQUEST_GETEP_REQ:
    {
        // This is the request to get an endpoint handle
        // Flit 1: node number
        // Flit 2: port number

        // Return the get endpoint response to sender
        rbuffer[0] = (src << OPTIMSOC_DEST_LSB) |
                (1 << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_GETEP_RESP << CTRL_REQUEST_LSB);

        // Get endpoint handle for <thisdomain,node,port> where
        // this domain is the tile id
        struct endpoint_handle *eph = endpoint_get(optimsoc_get_tileid(),buffer[1],buffer[2]);

        // If valid numbers and endpoint handle found
        if (buffer[1] < MCA_MAX_NODES &&
                buffer[2] < MCAPI_MAX_ENDPOINTS &&
                (eph!=NULL)) {
            // Return endpoint
            rbuffer[1] = (unsigned int) eph->ep;
        } else {
            // Signal this is an invalid endpoint
            rbuffer[1] = (int) -1;
        }
        //OPTIMSOC_REPORT(0x301,rbuffer[1]);
        optimsoc_mp_simple_send(2,rbuffer);
        break;
    }
    case CTRL_REQUEST_MSG_ALLOC_REQ:
    {
        rbuffer[0] = (src << OPTIMSOC_DEST_LSB) |
                (1 << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_ALLOC_RESP << CTRL_REQUEST_LSB);
        struct endpoint *ep = (struct endpoint*) buffer[1];
        unsigned int size = buffer[2];
        if (ep->flags & EP_FLAG_READY) {
            // Set size of current message and reset pointer
            ep->cur_message_size = size; // in byte
            ep->cur_message_ptr  = 0;

            // Clear ready flag
            ep->flags &= ~EP_FLAG_READY;
            // Set allocated flag
            ep->flags |= EP_FLAG_ALLOC;
            // Return acknowledge
            rbuffer[1] = CTRL_REQUEST_ACK;
            //OPTIMSOC_REPORT(0x301,rbuffer[1])
            //OPTIMSOC_REPORT(0x302,size)
        } else {
            // Return error
            rbuffer[1] = CTRL_REQUEST_NACK;
            //OPTIMSOC_REPORT(0x301,rbuffer[1])
        }
        optimsoc_mp_simple_send(2,rbuffer);
        break;
    }
    case CTRL_REQUEST_MSG_DATA:
    {
        struct endpoint *ep = (struct endpoint*) buffer[1];
        // Write data to buffer
        for (int i=0;i<len-2;i++) {
            ep->buffer[ep->cur_message_ptr+i] = buffer[i+2];
        }

        ep->cur_message_ptr += len-2;
        if (ep->cur_message_ptr==(ep->cur_message_size+3)>>2) {
            // Message is complete: set flags
            ep->flags &= ~EP_FLAG_ALLOC;
            ep->flags |= EP_FLAG_MSG;

#ifdef RUNTIME
            if (ep->waiting) {
                thread_resume(ep->waiting_thread);
                ep->waiting = 0;
            }
#endif
        }
        break;
    }
    case CTRL_REQUEST_GETEP_RESP:
    case CTRL_REQUEST_MSG_ALLOC_RESP:
        // Forward the responses to the handler
        ctrl_request.buffer[0] = buffer[0];
        ctrl_request.buffer[1] = buffer[1];
        ctrl_request.buffer[2] = buffer[2];
        ctrl_request.buffer[3] = buffer[3];
        ctrl_request.buffer[4] = buffer[4];
        ctrl_request.done = 1;
        break;
    default:
        printf("Unknown request: %d\n",req);
        break;
    }
}


mcapi_boolean_t mcapi_trans_get_node_num(mcapi_node_t* node_num) {
    *node_num = 0;
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_get_domain_num(mcapi_domain_t* domain_num)
{
    *domain_num = optimsoc_get_tileid();
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_set_node_num(mcapi_uint_t node_num)
{
    return MCAPI_FALSE;
}


/****************** error checking queries *************************/
/* checks if the given node is valid */
mcapi_boolean_t mcapi_trans_valid_node(mcapi_uint_t node_num)
{
    if (node_num<4) {
        return MCAPI_TRUE;
    } else {
        return MCAPI_FALSE;
    }
}

/* checks to see if the port_num is a valid port_num for this system */
mcapi_boolean_t mcapi_trans_valid_port(mcapi_uint_t port_num)
{
    if (port_num == MCAPI_PORT_ANY)
        return MCAPI_TRUE;
    else
        return ((port_num < MCAPI_MAX_ENDPOINTS) ? MCAPI_TRUE : MCAPI_FALSE);
}

/* checks if the endpoint handle refers to a valid endpoint */
mcapi_boolean_t mcapi_trans_valid_endpoint (mcapi_endpoint_t endpoint)
{
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) endpoint;
    if (endpoint_verify(eph)!=eph) {
        return MCAPI_FALSE;
    } else {
        return MCAPI_TRUE;
    }
}

/* checks if the channel is open for a given endpoint */
mcapi_boolean_t mcapi_trans_endpoint_channel_isopen (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_channel_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan receive handle */
mcapi_boolean_t mcapi_trans_pktchan_recv_isopen (mcapi_pktchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_pktchan_recv_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given pktchan send handle */
mcapi_boolean_t mcapi_trans_pktchan_send_isopen (mcapi_pktchan_send_hndl_t send_handle)
{
    printf("mcapi_trans_pktchan_send_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan receive handle */
mcapi_boolean_t mcapi_trans_sclchan_recv_isopen (mcapi_sclchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_sclchan_recv_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the channel is open for a given sclchan send handle */
mcapi_boolean_t mcapi_trans_sclchan_send_isopen (mcapi_sclchan_send_hndl_t send_handle)
{
    printf("mcapi_trans_sclchan_send_isopen not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given endpoint is owned by the given node */
mcapi_boolean_t mcapi_trans_endpoint_isowner (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_isowner not implemented!\n");
    return MCAPI_FALSE;
}



channel_type mcapi_trans_channel_type (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_channel_type not implemented!\n");
    return 0;
}



mcapi_boolean_t mcapi_trans_channel_connected  (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_channel_connected not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_recv_endpoint (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_recv_endpoint not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_send_endpoint (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_send_endpoint not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given endpoints have compatible attributes */
mcapi_boolean_t mcapi_trans_compatible_endpoint_attributes  (mcapi_endpoint_t send_endpoint, mcapi_endpoint_t recv_endpoint)
{
    printf("mcapi_trans_compatible_endpoint_attributes not implemented!\n");
    return MCAPI_FALSE;
}



/* checks if the given channel handle is valid */
mcapi_boolean_t mcapi_trans_valid_pktchan_send_handle( mcapi_pktchan_send_hndl_t handle)
{
    printf("mcapi_trans_valid_pktchan_send_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_pktchan_recv_handle( mcapi_pktchan_recv_hndl_t handle)
{
    printf("mcapi_trans_valid_pktchan_recv_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_send_handle( mcapi_sclchan_send_hndl_t handle)
{
    printf("mcapi_trans_valid_sclchan_send_handle not implemented!\n");
    return MCAPI_FALSE;
}


mcapi_boolean_t mcapi_trans_valid_sclchan_recv_handle( mcapi_sclchan_recv_hndl_t handle)
{
    printf("mcapi_trans_valid_sclchan_recv_handle not implemented!\n");
    return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_initialized (mca_domain_t domain_id,mca_node_t node_id)
{
    return ((initialized>0) ? MCAPI_TRUE : MCAPI_FALSE);
}

mcapi_uint32_t mcapi_trans_num_endpoints()
{
    return eplist_localnum;
}

mcapi_boolean_t mcapi_trans_valid_priority(mcapi_priority_t priority)
{
    // For the moment we just accept all priorities
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_connected(mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_connected not implemented!\n");
    return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_valid_status_param (mca_status_t* mcapi_status){
    printf("mcapi_trans_valid_status_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_version_param (mcapi_info_t* mcapi_version)
{
    printf("mcapi_trans_valid_version_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_buffer_param (void* buffer)
{
    // Why should a pointer be invalid?
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_request_param (mcapi_request_t* request)
{
    printf("mcapi_trans_valid_request_param not implemented!\n");
    return MCAPI_TRUE;
}

mcapi_boolean_t mcapi_trans_valid_size_param (size_t* size)
{
    printf("mcapi_trans_valid_size_param not implemented!\n");
    return MCAPI_TRUE;
}

/****************** initialization *************************/
/* initialize the transport layer */
mcapi_boolean_t mcapi_trans_initialize(mca_domain_t domain_id,
        mcapi_node_t node_num,
        const mcapi_node_attributes_t* node_attrs)
{
    optimsoc_mp_simple_init();
    // Add handler so that received message are treated correctly
    // Class 1: control messages
    optimsoc_mp_simple_addhandler(1,&mcapi_trans_recvctl_handler);

    mcapi_domainid = domain_id;
    mcapi_nodenum = node_num;

    // Initialize endpoint list
    eplist_first = NULL;
    eplist_last = NULL;
    eplist_localnum = 0;
    eplist_remotenum = 0;

    initialized = 1;

    return MCAPI_TRUE;
}



/****************** tear down ******************************/
mcapi_boolean_t mcapi_trans_finalize() {
    printf("mcapi_trans_finalize not implemented!\n");
    return MCAPI_FALSE;
}



/****************** endpoints ******************************/
/* create endpoint <node_num,port_num> and return it's handle */
mcapi_boolean_t mcapi_trans_endpoint_create(mcapi_endpoint_t *endpoint,  mcapi_uint_t port_num, mcapi_boolean_t anonymous)
{
    OPTIMSOC_REPORT(0x300,port_num)

    // If any port can be selected, find a free one
    if (port_num == MCAPI_PORT_ANY) {
        if(endpoint_generate_portnum(&port_num) == MCAPI_FALSE) {
            // status = MCAPI_ERR_ENDP_NOPORTS; // Not asked for..
            return MCAPI_FALSE;
        }
    }

    struct endpoint *ep = malloc(sizeof(struct endpoint));
    assert(ep!=NULL);

    ep->buffer = malloc(MCAPI_MAX_MSG_SIZE*4);
    assert(ep->buffer!=NULL);

    struct endpoint_handle *eph = malloc(sizeof(struct endpoint_handle));
    assert(eph!=NULL);

    eph->domain = optimsoc_get_tileid();
    eph->node = 0;
    eph->port = port_num;
    eph->type = LOCAL;
    eph->ep = ep;

    endpoint_add(eph);

    ep->flags = EP_FLAG_READY;
    ep->cur_message_ptr = 0;
    ep->cur_message_size = 0;
#ifdef RUNTIME
    ep->waiting = 0;
#endif

    *endpoint = (mcapi_endpoint_t) eph;

    //printf("endpoint created (@%p) domain: %d, node: %d, port: %d\n",eph,eph->domain,eph->node,eph->port);

    OPTIMSOC_REPORT(0x301,eph)

    return MCAPI_TRUE;
}


/* non-blocking get endpoint for the given <node_num,port_num> and set endpoint parameter to it's handle */
void mcapi_trans_endpoint_get_i(  mcapi_endpoint_t* endpoint, mca_domain_t domain_id,mcapi_uint_t node_num,
        mcapi_uint_t port_num,mcapi_request_t* request,
        mca_status_t* mcapi_status)
{
    printf("mcapi_trans_endpoint_get_i not implemented!\n");
}

/* blocking get endpoint for the given <node_num,port_num> and return it's handle */
/***************************************************************************
NAME:mcapi_trans_endpoint_get
DESCRIPTION:blocking get endpoint for the given <node_num,port_num>
PARAMETERS:
   endpoint - the endpoint handle to be filled in
   node_num - the node id
   port_num - the port id
RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE indicating success or failure
 ***************************************************************************/
void mcapi_trans_endpoint_get(mcapi_endpoint_t *endpoint,
        mcapi_domain_t domain_id,
        mcapi_uint_t node_num,
        mcapi_uint_t port_num)
{
    struct endpoint_handle *eph;

    OPTIMSOC_REPORT(0x302,domain_id);
    OPTIMSOC_REPORT(0x303,node_num);
    OPTIMSOC_REPORT(0x304,port_num);

    // Try to find endpoint in local database
    eph = endpoint_get(domain_id,node_num,port_num);

#ifndef RUNTIME
    unsigned int timeout_insns = 1000;
#endif

    if (eph==NULL) {
        // Endpoint is not locally known
        struct endpoint *ep;

        do {
            // Try to retrieve from remote
            // We do this as long as we do not get a valid handle back (-1)
            ctrl_request.buffer[0] = (domain_id << OPTIMSOC_DEST_LSB) |
                    (1 << OPTIMSOC_CLASS_LSB) |
                    (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                    (CTRL_REQUEST_GETEP_REQ << CTRL_REQUEST_LSB);
            ctrl_request.buffer[1] = node_num;
            ctrl_request.buffer[2] = port_num;
            ctrl_request.done = 0;
            optimsoc_mp_simple_send(3,ctrl_request.buffer);

            while (ctrl_request.done == 0) { }

            ep = (struct endpoint*) ctrl_request.buffer[1];

            if ((int)ep==-1) {
#ifdef RUNTIME
                thread_yield();
#else
                for (int t=0;t<timeout_insns;t++) { asm __volatile__("l.nop 0x0"); }
                timeout_insns = timeout_insns * 10; // somewhat arbitrary..
#endif
            }

        } while ((int)ep==-1);

        // Create the respective handle
        eph = malloc(sizeof(struct endpoint_handle));
        eph->domain = domain_id;
        eph->node = node_num;
        eph->port = port_num;
        eph->ep = ep;
        eph->type = REMOTE;

        endpoint_add(eph);
    }

    //printf("Got endpoint @%p: %p\n",eph,eph->ep);

    *endpoint = (mcapi_endpoint_t) eph;

    OPTIMSOC_REPORT(0x305,eph);
}



/* delete the given endpoint */
void mcapi_trans_endpoint_delete( mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_delete not implemented!\n");
}



/* get the attribute for the given endpoint and attribute_num */
void mcapi_trans_endpoint_get_attribute(
        mcapi_endpoint_t endpoint,
        mcapi_uint_t attribute_num,
        void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status)
{
    printf("mcapi_trans_endpoint_get_attribute not implemented!\n");
}



/* set the given attribute on the given endpoint */
void mcapi_trans_endpoint_attribute_set( mcapi_endpoint_t endpoint, mcapi_uint_t attribute_num, const void* attribute, size_t attribute_size)
{
    printf("mcapi_trans_endpoint_attribute_set not implemented!\n");
}




/****************** msgs **********************************/
void mcapi_trans_msg_send_i( mcapi_endpoint_t  send_endpoint,
        mcapi_endpoint_t  receive_endpoint,
        const char* buffer, size_t buffer_size,
        mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_msg_send_i not implemented!\n");
}

mcapi_boolean_t mcapi_trans_msg_send( mcapi_endpoint_t  send_endpoint,
        mcapi_endpoint_t  receive_endpoint,
        const char* buffer, size_t buffer_size)
{
    struct endpoint_handle *recvep = (struct endpoint_handle *) receive_endpoint;

    OPTIMSOC_REPORT(0x306,send_endpoint)
    OPTIMSOC_REPORT(0x307,receive_endpoint)
    OPTIMSOC_REPORT(0x308,buffer_size)

#ifndef RUNTIME
    unsigned int timeout_insns = 1000;
#endif

    // Wait until receive_endpoint is ready to receive (allocate)
    do {
        // Try to retrieve from remote
        // We do this as long as we do not get a valid handle back (-1)
        ctrl_request.buffer[0] = (recvep->domain << OPTIMSOC_DEST_LSB) |
                (1 << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_ALLOC_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) recvep->ep;
        ctrl_request.buffer[2] = (unsigned int) buffer_size;
        ctrl_request.done = 0;

        optimsoc_mp_simple_send(3,ctrl_request.buffer);

        //OPTIMSOC_REPORT(0x322,0)

        while (ctrl_request.done == 0) {}

        //OPTIMSOC_REPORT(0x322,1)

        if (ctrl_request.buffer[1]==CTRL_REQUEST_NACK) {
            OPTIMSOC_REPORT(0x309,1)
#ifdef RUNTIME
            thread_yield();
#else
            for (int t=0;t<timeout_insns;t++) { asm __volatile__("l.nop 0x0"); }
            timeout_insns = timeout_insns * 10; // somewhat arbitrary..
#endif
        }

    } while (ctrl_request.buffer[1]==CTRL_REQUEST_NACK);

    //OPTIMSOC_REPORT(0x322,2)

    // TODO: what if size%4!=0?
    assert(buffer_size%4==0);
    unsigned int words = (buffer_size+3)>>2;
    unsigned int wordsperpacket = _optimsoc_noc_maxpacketsize-2;

    for (int i=0;i<words;i=i+wordsperpacket) {
        ctrl_request.buffer[0] = (recvep->domain << OPTIMSOC_DEST_LSB) |
                (1 << OPTIMSOC_CLASS_LSB) |
                (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                (CTRL_REQUEST_MSG_DATA << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) recvep->ep;

        int size = words - i;
        if (size>wordsperpacket)
            size = wordsperpacket;

        for (int d=0;d<size;d++) {
            ctrl_request.buffer[2+d] = ((unsigned int *)buffer)[i+d];
        }

        OPTIMSOC_REPORT(0x30a,size)
        optimsoc_mp_simple_send(2+size,ctrl_request.buffer);
    }

    return MCAPI_TRUE;
}

void mcapi_trans_msg_recv_i( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_msg_recv_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_msg_recv( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, size_t* received_size)
{
    struct endpoint *ep;
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) receive_endpoint;
    ep = eph->ep;
    int size;

    OPTIMSOC_REPORT(0x30b,receive_endpoint);

#ifdef RUNTIME
    if ((ep->flags & EP_FLAG_MSG) == 0) {
        // There is no message waiting, wait for event
        ep->waiting = 1;
        ep->waiting_thread = thread_self();
        thread_suspend();
    }
#else
    while((ep->flags & EP_FLAG_MSG)==0) { }
#endif

    size = ep->cur_message_size;

    if (size > buffer_size) {
      *received_size = 0;
      return MCAPI_FALSE;
    }

    OPTIMSOC_REPORT(0x30c,size);

    for (int i=0;i<size;i++) {
        ((char*)buffer)[i] = ((char*)ep->buffer)[i];
    }

    *received_size = size;

    ep->flags |= EP_FLAG_READY;
    ep->flags &= ~EP_FLAG_MSG;

    return MCAPI_TRUE;
}



mcapi_uint_t mcapi_trans_msg_available( mcapi_endpoint_t receive_endpoint)
{
    struct endpoint *ep;
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) receive_endpoint;
    ep = eph->ep;
    int size;

    OPTIMSOC_REPORT(0x619,ep->flags);

    if((ep->flags & EP_FLAG_MSG)==0) {
      return 0;
    } else {
      size = ep->cur_message_size;
      return size;
    }
}



/****************** channels general ****************************/

/****************** pkt channels ****************************/
void mcapi_trans_pktchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_connect_i not implemented!\n");
}



void mcapi_trans_pktchan_recv_open_i( mcapi_pktchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_open_i not implemented!\n");
}



void mcapi_trans_pktchan_send_open_i( mcapi_pktchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_open_i not implemented!\n");
}



void  mcapi_trans_pktchan_send_i( mcapi_pktchan_send_hndl_t send_handle,
        const void* buffer, size_t size,
        mcapi_request_t* request,
        mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_i not implemented!\n");
}



mcapi_boolean_t  mcapi_trans_pktchan_send( mcapi_pktchan_send_hndl_t send_handle,
        const void* buffer, size_t size)
{
    printf("mcapi_trans_pktchan_send not implemented!\n");
    return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_i( mcapi_pktchan_recv_hndl_t receive_handle,  void** buffer, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_pktchan_recv( mcapi_pktchan_recv_hndl_t receive_handle, void** buffer, size_t* received_size)
{
    printf("mcapi_trans_pktchan_recv not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_pktchan_available( mcapi_pktchan_recv_hndl_t   receive_handle)
{
    printf("mcapi_trans_pktchan_available not implemented!\n");
    return 0;
}



mcapi_boolean_t mcapi_trans_pktchan_free( void* buffer)
{
    printf("mcapi_trans_pktchan_free not implemented!\n");
    return MCAPI_FALSE;
}



void mcapi_trans_pktchan_recv_close_i( mcapi_pktchan_recv_hndl_t  receive_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_close_i not implemented!\n");
}



void mcapi_trans_pktchan_send_close_i( mcapi_pktchan_send_hndl_t  send_handle,mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_send_close_i not implemented!\n");
}



/****************** scalar channels ****************************/
void mcapi_trans_sclchan_connect_i( mcapi_endpoint_t  send_endpoint, mcapi_endpoint_t  receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_connect_i not implemented!\n");
}



void mcapi_trans_sclchan_recv_open_i( mcapi_sclchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_recv_open_i not implemented!\n");
}



void mcapi_trans_sclchan_send_open_i( mcapi_sclchan_send_hndl_t* send_handle, mcapi_endpoint_t  send_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_send_open_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_sclchan_send( mcapi_sclchan_send_hndl_t send_handle,  uint64_t dataword, uint32_t size)
{
    printf("mcapi_trans_sclchan_send not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_sclchan_recv( mcapi_sclchan_recv_hndl_t receive_handle,uint64_t *data,uint32_t size)
{
    printf("mcapi_trans_sclchan_recv not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_uint_t mcapi_trans_sclchan_available_i( mcapi_sclchan_recv_hndl_t receive_handle)
{
    printf("mcapi_trans_sclchan_available_i not implemented!\n");
    return 0;
}



void mcapi_trans_sclchan_recv_close_i( mcapi_sclchan_recv_hndl_t  recv_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_recv_close_i not implemented!\n");
}



void mcapi_trans_sclchan_send_close_i( mcapi_sclchan_send_hndl_t send_handle,mcapi_request_t* mcapi_request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_sclchan_send_close_i not implemented!\n");
}



/****************** test,wait & cancel ****************************/
mcapi_boolean_t mcapi_trans_test_i( mcapi_request_t* request, size_t* size,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_test_i not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_wait( mcapi_request_t* request, size_t* size,
        mca_status_t* mcapi_status,
        mca_timeout_t timeout)
{
    printf("mcapi_trans_wait not implemented!\n");
    return MCAPI_FALSE;
}



int mcapi_trans_wait_first( size_t number, mcapi_request_t** requests, size_t* size)
{
    printf("mcapi_trans_wait_first not implemented!\n");
    return 0;
}



void mcapi_trans_cancel( mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_cancel not implemented!\n");
}


/***************************************************************************
  NAME:mcapi_trans_display_state
  DESCRIPTION: This function is useful for debugging.  If the handle is null,
   we'll print out the state of the entire database.  Otherwise, we'll print out
   only the state of the endpoint that the handle refers to.
  PARAMETERS: 
     handle
  RETURN VALUE: none
 ***************************************************************************/
void mcapi_trans_display_state (void* handle)
{
    printf("mcapi_trans_display_state not implemented!\n");
}


/***************************************************************************
  NAME:mcapi_trans_node_init_attributes
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_init_attributes(
        mcapi_node_attributes_t* mcapi_node_attributes,
        mcapi_status_t* mcapi_status
) {

    printf("mcapi_trans_node_init_attributes not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_node_get_attribute
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_get_attribute(
        mcapi_domain_t domain_id,
        mcapi_node_t node_id,
        mcapi_uint_t attribute_num,
        void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status) {

    printf("mcapi_trans_node_get_attribute not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_node_set_attribute
  DESCRIPTION: 
  PARAMETERS: 
  RETURN VALUE: 
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_node_set_attribute(
        mcapi_node_attributes_t* mcapi_node_attributes,
        mcapi_uint_t attribute_num,
        const void* attribute,
        size_t attribute_size,
        mcapi_status_t* mcapi_status
){
    printf("mcapi_trans_node_set_attribute not implemented!\n");
    return MCAPI_FALSE;

}

/***************************************************************************
  NAME: mcapi_trans_endpoint_exists
  DESCRIPTION: checks if an endpoint has been created for this port id
  PARAMETERS: port id
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_endpoint_exists (mcapi_domain_t domain_id,
        uint32_t port_num)
{
    if (port_num == MCAPI_PORT_ANY)
        return MCAPI_FALSE;

    // TODO: Does this involve checking remotely?
    struct endpoint_handle *eph;
    eph = endpoint_get(domain_id,0,port_num);

    return ((eph!=NULL) ? MCAPI_TRUE : MCAPI_FALSE);

}

/***************************************************************************
  NAME: mcapi_trans_valid_endpoints
  DESCRIPTION: checks if the given endpoint handles refer to valid endpoints
  PARAMETERS: endpoint1, endpoint2
  RETURN VALUE: MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_valid_endpoints (mcapi_endpoint_t endpoint1,
        mcapi_endpoint_t endpoint2)
{
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) endpoint1;
    if (endpoint_verify(eph)!=eph) {
        return MCAPI_FALSE;
    }
    eph = (struct endpoint_handle*) endpoint2;
    if (endpoint_verify(eph)!=eph) {
        return MCAPI_FALSE;
    }
    return MCAPI_TRUE;
}

/***************************************************************************
  NAME: mcapi_trans_valid_request_handle
  DESCRIPTION:checks if the given request handle is valid 
        This is the parameter test/wait/cancel pass in to be processed.
  PARAMETERS: request
  RETURN VALUE:MCAPI_TRUE/MCAPI_FALSE
 ***************************************************************************/
mcapi_boolean_t mcapi_trans_valid_request_handle (mcapi_request_t* request)
{
    printf("mcapi_trans_valid_request_handle not implemented!\n");
    return MCAPI_FALSE;
}

/***************************************************************************
  NAME:mcapi_trans_wait_any
  DESCRIPTION:Tests if any of the requests have completed yet (blocking).
      Note: the request is now cleared if it has been completed or cancelled.
  PARAMETERS: 
    send_handle -
    request -
    mcapi_status -
  RETURN VALUE:
 ***************************************************************************/
unsigned mcapi_trans_wait_any(size_t number, mcapi_request_t** requests, size_t* size,
        mcapi_status_t* mcapi_status,
        mcapi_timeout_t timeout)
{
    printf("mcapi_trans_wait_any not implemented!\n");
    return MCA_RETURN_VALUE_INVALID;
}


#endif

#ifdef __cplusplus
extern }
#endif /* __cplusplus */
