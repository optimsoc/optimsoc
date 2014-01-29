#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */

#include <assert.h>
#include <stdio.h>


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

    void * address = control_msg_alloc(recvep, buffer_size);

    control_msg_data(recvep, address, buffer, buffer_size);

    return MCAPI_TRUE;
}

void mcapi_trans_msg_recv_i( mcapi_endpoint_t  receive_endpoint,  char* buffer, size_t buffer_size, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_msg_recv_i not implemented!\n");
}



mcapi_boolean_t mcapi_trans_msg_recv(mcapi_endpoint_t  receive_endpoint,
                                     char* buffer, size_t buffer_size,
                                     size_t* received_size)
{
    struct endpoint *ep;
    struct endpoint_handle *eph;

    eph = (struct endpoint_handle*) receive_endpoint;
    ep = eph->ep;

    endpoint_msg_recv(ep, buffer, buffer_size, received_size);

    if (received_size > 0) {
        return MCAPI_TRUE;
    } else {
        return MCAPI_FALSE;
    }
}



mcapi_uint_t mcapi_trans_msg_available( mcapi_endpoint_t receive_endpoint)
{
    assert(1==0);
    /*
    struct endpoint *ep;
    struct endpoint_handle *eph;
    eph = (struct endpoint_handle*) receive_endpoint;
    ep = eph->ep;
    int size;

    if (ep->bread_ind == ep->bwrite_ind) {
    return 0;
    } else {
    size = ((MCAPI_EP_BUFFER_SIZE - ep->bread_ind) + ep->bwrite_ind) & MCAPI_EP_BUFFER_MASK;
    return size;
    }*/
}

