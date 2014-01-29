#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */

#include <stdio.h> // for printf
#include <assert.h>

mcapi_boolean_t mcapi_trans_channel_connected  (mcapi_endpoint_t endpoint)
{
    /*struct endpoint_handle *eph = (struct endpoint_handle *) endpoint;

    if (eph->type == LOCAL) {
        if ((eph->ep->flags & EP_FLAG_CONNECTED) != 0) {
            return MCAPI_TRUE;
        } else {
            return MCAPI_FALSE;
        }
    } else {
        ctrl_request.buffer[0] = (eph->domain << OPTIMSOC_DEST_LSB) |
                                 (MCAPI_NOC_CLASS << OPTIMSOC_CLASS_LSB) |
                                 (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                                 (CTRL_REQUEST_CHAN_CONNECTED_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) eph->ep;
        ctrl_request.done = 0;

        optimsoc_mp_simple_send(2,ctrl_request.buffer);

        while (ctrl_request.done == 0) { }

        if (ctrl_request.buffer[1] == 0) {
            return MCAPI_FALSE;
        } else {
            return MCAPI_TRUE;
        }
    }*/
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

channel_type mcapi_trans_channel_type (mcapi_endpoint_t endpoint)
{
/*    struct endpoint_handle *eph = (struct endpoint_handle *) endpoint;

    assert(eph->type == LOCAL);

    if (eph->type == LOCAL) {
        if ((eph->ep->flags & EP_FLAG_CHANNELTYPE) == EP_FLAG_CHANNELTYPE_PACKET) {
            return MCAPI_PKT_CHAN;
        } else {
            return MCAPI_SCL_CHAN;
        }
    } else {
        return MCAPI_PKT_CHAN;
    }*/
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

mcapi_boolean_t mcapi_trans_connected(mcapi_endpoint_t endpoint)
{
    /*struct endpoint_handle *eph = (struct endpoint_handle *) endpoint;

    if (eph->type == LOCAL) {
        if ((eph->ep->flags & EP_FLAG_CONNECTED) != 0) {
            return MCAPI_TRUE;
        } else {
            return MCAPI_FALSE;
        }
    } else {
        ctrl_request.buffer[0] = (eph->domain << OPTIMSOC_DEST_LSB) |
                                 (MCAPI_NOC_CLASS << OPTIMSOC_CLASS_LSB) |
                                 (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                                 (CTRL_REQUEST_CHAN_CONNECTED_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = eph->ep;
        ctrl_request.done = 0;

        optimsoc_mp_simple_send(2,ctrl_request.buffer);

        while (ctrl_request.done == 0) { }

        if (ctrl_request.buffer[1] == 0) {
            return MCAPI_FALSE;
        } else {
            return MCAPI_TRUE;
        }
    }*/
}

/****************** pkt channels ****************************/
void mcapi_trans_pktchan_connect_i(mcapi_endpoint_t  send_endpoint,
                                   mcapi_endpoint_t  receive_endpoint,
                                   mcapi_request_t* request,
                                   mca_status_t* mcapi_status)
{
    assert(1);
    /*
    struct endpoint_handle *send_ep = (struct endpoint_handle *) send_endpoint;
    struct endpoint_handle *recv_ep = (struct endpoint_handle *) receive_endpoint;

    if (send_ep->type == LOCAL) {
        send_ep->ep->flags |= EP_FLAG_CONNECTED;
        send_ep->ep->flags |= EP_FLAG_DIRECTION_SEND;
    } else {
        ctrl_request.buffer[0] = (send_ep->domain << OPTIMSOC_DEST_LSB) |
                                 (MCAPI_NOC_CLASS << OPTIMSOC_CLASS_LSB) |
                                 (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                                 (CTRL_REQUEST_PKTCHAN_CONNECT_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) send_ep->ep;
        ctrl_request.buffer[2] = EP_FLAG_DIRECTION_SEND;
        ctrl_request.done = 0;

        optimsoc_mp_simple_send(3, ctrl_request.buffer);

        while (ctrl_request.done == 0) {}

        if (ctrl_request.buffer[1] != MCAPI_SUCCESS) {
            mcapi_status = ctrl_request.buffer[1];
            return;
        }
    }

    if (recv_ep->type == LOCAL) {
        recv_ep->ep->flags |= EP_FLAG_CONNECTED;
        send_ep->ep->flags |= EP_FLAG_DIRECTION_RECV;
    } else {
        // Try to retrieve from remote
        ctrl_request.buffer[0] = (recv_ep->domain << OPTIMSOC_DEST_LSB) |
                                 (MCAPI_NOC_CLASS << OPTIMSOC_CLASS_LSB) |
                                 (optimsoc_get_tileid() << OPTIMSOC_SRC_LSB) |
                                 (CTRL_REQUEST_PKTCHAN_CONNECT_REQ << CTRL_REQUEST_LSB);
        ctrl_request.buffer[1] = (unsigned int) recv_ep->ep;
        ctrl_request.buffer[2] = EP_FLAG_DIRECTION_RECV;
        ctrl_request.done = 0;

        optimsoc_mp_simple_send(3, ctrl_request.buffer);

        while (ctrl_request.done == 0) {}

        if (ctrl_request.buffer[1] != MCAPI_SUCCESS) {
            mcapi_status = ctrl_request.buffer[1];
            return;
        }
    }*/

}



void mcapi_trans_pktchan_recv_open_i( mcapi_pktchan_recv_hndl_t* recv_handle, mcapi_endpoint_t receive_endpoint, mcapi_request_t* request,mca_status_t* mcapi_status)
{
    printf("mcapi_trans_pktchan_recv_open_i not implemented!\n");
}



void mcapi_trans_pktchan_send_open_i(mcapi_pktchan_send_hndl_t* send_handle,
                                     mcapi_endpoint_t  send_endpoint,
                                     mcapi_request_t* request,
                                     mca_status_t* mcapi_status)
{
    assert(1);
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
