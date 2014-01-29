#include <mcapi.h>
#include <mca_config.h>  /* for MAX_ defines */

#include <stdio.h> // for printf
#include <assert.h>


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

/* checks if the given endpoint is owned by the given node */
mcapi_boolean_t mcapi_trans_endpoint_isowner (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_endpoint_isowner not implemented!\n");
    return MCAPI_FALSE;
}

mcapi_boolean_t mcapi_trans_recv_endpoint (mcapi_endpoint_t endpoint)
{
    printf("mcapi_trans_recv_endpoint not implemented!\n");
    return MCAPI_FALSE;
}



mcapi_boolean_t mcapi_trans_send_endpoint (mcapi_endpoint_t endpoint)
{
    struct endpoint_handle *eph = (struct endpoint_handle *) endpoint;

    assert(eph->type == LOCAL);

    if ((eph->ep->flags & EP_FLAG_DIRECTION) == EP_FLAG_DIRECTION_SEND) {
        return MCAPI_TRUE;
    } else {
        return MCAPI_FALSE;

    }
}

/* checks if the given endpoints have compatible attributes */
mcapi_boolean_t mcapi_trans_compatible_endpoint_attributes  (mcapi_endpoint_t send_endpoint, mcapi_endpoint_t recv_endpoint)
{
    return MCAPI_TRUE;
}

/* create endpoint <node_num,port_num> and return it's handle */
mcapi_boolean_t mcapi_trans_endpoint_create(mcapi_endpoint_t *endpoint,  mcapi_uint_t port_num, mcapi_boolean_t anonymous)
{
    // If any port can be selected, find a free one
    if (port_num == MCAPI_PORT_ANY) {
        if(endpoint_generate_portnum(&port_num) == MCAPI_FALSE) {
            // status = MCAPI_ERR_ENDP_NOPORTS; // Not asked for..
            return MCAPI_FALSE;
        }
    }

    struct endpoint_handle *eph = endpoint_create();
    eph->domain = optimsoc_get_tileid();
    eph->node = 0;
    eph->port = port_num;
    eph->type = LOCAL;

    endpoint_add(eph);

    *endpoint = (mcapi_endpoint_t) eph;

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

    eph = endpoint_get(domain_id, node_num, port_num);

    *endpoint = (mcapi_endpoint_t) eph;
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

mcapi_uint32_t mcapi_trans_num_endpoints()
{
    return endpoints_localnum();
}
