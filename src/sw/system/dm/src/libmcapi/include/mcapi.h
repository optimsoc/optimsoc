/*
 * mcapi.h
 * V1.301, May 2010
*/

#ifndef MCAPI_H
#define MCAPI_H

#include <stddef.h>					/* Required for size_t */
#include "mca.h"

 /* The implementation_spec.h header file is vendor/implementation specific,
 * and should contain declarations and definitions specific to a particular
 * implementation.
 *
 * This file must be provided by each implementation.
 *
 * It MUST contain type definitions for the following types:
 *
 * mcapi_endpoint_t;			Note: The endpoint identifier must be topology unique.
 * mca_request_t;
 * mcapi_pktchan_recv_hndl_t;
 * mcapi_pktchan_send_hndl_t;
 * mcapi_sclchan_send_hndl_t;
 * mcapi_sclchan_recv_hndl_t;
 *
 * These types must be either pointers or 32 bit scalars, allowing simple arithmetic equality comparison (a == b).
 * Implementers may which of these type are used.
 *
 * It MUST contain the following definition:
 * mcapi_param_t;
 *
 * It MUST contain the following definitions:
 *
 * Number of MCAPI reserved ports, starting at port 0. Reserved ports can be used for implementation specific purposes.
 *
 * MCAPI_NUM_RESERVED_PORTS				1	Number of reserved ports starting at port 0
 *
 *
 * Implementation defined MCAPI MIN and MAX values.
 *
 * Implementations may parameterize implementation specific max values,
 * smaller that the MCAPI max values. Implementations must specify what
 * those smaller values are and how they are set.
 *
 * MCAPI_MAX_DOMAIN				Maximum value for domain
 * MCAPI_MAX_NODE				Maximum value for node
 * MCAPI_MAX_PORT				Maximum value for port
 * MCAPI_MAX_MESSAGE_SIZE		Maximum message size
 * MCAPI_MAX_PACKET_SIZE		Maximum packet size
 *
 * Implementations may parameterize implementation specific priority min value
 * and set the number of reserved ports. Implementations must specify what
 * those values are and how they are set.
 *
 * MCAPI_MIN_PORT				Minimum value for port
 * MCAPI_MIN_PRORITY			Minimum priority value
 *
 */

   #include "mcapi_impl_spec.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * MCAPI type definitions
 * (Additional typedefs under the attribute and initialization sections below)
 */
typedef mca_int_t			mcapi_int_t;
typedef mca_int8_t			mcapi_int8_t;
typedef mca_int16_t			mcapi_int16_t;
typedef mca_int32_t			mcapi_int32_t;
typedef mca_int64_t			mcapi_int64_t;
typedef mca_uint_t			mcapi_uint_t;
typedef mca_uint8_t			mcapi_uint8_t;
typedef mca_uint16_t		mcapi_uint16_t;
typedef mca_uint32_t		mcapi_uint32_t;
typedef mca_uint64_t		mcapi_uint64_t;
typedef mca_boolean_t		mcapi_boolean_t;
typedef mca_domain_t		mcapi_domain_t;
typedef mca_node_t			mcapi_node_t;
typedef unsigned int    	mcapi_port_t;				//Changed from int to unsigned int
typedef mca_status_t		mcapi_status_t;
  //MR: typedef mca_request_t		mcapi_request_t;
typedef mca_timeout_t		mcapi_timeout_t;
typedef unsigned int		mcapi_priority_t;


/* The following constants are not implementation defined */
#define MCAPI_VERSION					1103		/* Version 1.103 (major # + minor # (3-digit)) */
#define MCAPI_TRUE						MCA_TRUE
#define MCAPI_FALSE						MCA_FALSE
#define MCAPI_NULL						MCA_NULL	  /* MCAPI Zero value */
#define MCAPI_PORT_ANY					  (~0)		/* Create endpoint using the next available port */
#define	MCAPI_TIMEOUT_INFINITE		(~0)		/* Wait forever, no timeout */
#define	MCAPI_TIMEOUT_IMMEDIATE		  0			/* Return immediately, with success or failure */
#define MCAPI_NODE_INVALID 				(~0)		/* Return value for	invalid node */
#define MCAPI_DOMAIN_INVALID			(~0)		/* Return value for	invalid domain */
#define MCAPI_RETURN_VALUE_INVALID (~0)		/* Invalid return value */
#define MCAPI_MAX_PRORITY				    0			/* Maximum priority value */

#define MCAPI_MAX_STATUS_SIZE 32
/*
 * MCAPI Status codes
 */
enum mcapi_status_codes {
	MCAPI_SUCCESS = 1,				// Indicates operation was successful
	MCAPI_PENDING,					// Indicates operation is pending without errors
	MCAPI_TIMEOUT,					// The operation timed out
	MCAPI_ERR_PARAMETER,			// Incorrect parameter
	MCAPI_ERR_DOMAIN_INVALID,		// The parameter is not a valid domain
	MCAPI_ERR_NODE_INVALID,			// The parameter is not a valid node
	MCAPI_ERR_NODE_INITFAILED,		// The MCAPI node could not be initialized
	MCAPI_ERR_NODE_INITIALIZED,		// MCAPI node is already initialized
	MCAPI_ERR_NODE_NOTINIT,			// The MCAPI node is not initialized
	MCAPI_ERR_NODE_FINALFAILED,		// The MCAPI could not be finalized
	MCAPI_ERR_PORT_INVALID,			// The parameter is not a valid port
	MCAPI_ERR_ENDP_INVALID,			// The parameter is not a valid endpoint descriptor
	MCAPI_ERR_ENDP_EXISTS,			// The endpoint is already created
	MCAPI_ERR_ENDP_GET_LIMIT,		// Endpoint get reference count is to high
	MCAPI_ERR_ENDP_DELETED,			// The endpoint has been deleted
	MCAPI_ERR_ENDP_NOTOWNER,		// An endpoint can only be deleted by its creator
	MCAPI_ERR_ENDP_REMOTE,			// Certain operations are only allowed on the node local endpoints
  MCAPI_ERR_ENDP_NOPORTS,
  MCAPI_ERR_ENDP_LIMIT,
	MCAPI_ERR_ATTR_INCOMPATIBLE,	// Connection of endpoints with incompatible attributes not allowed
	MCAPI_ERR_ATTR_SIZE,			// Incorrect attribute size
	MCAPI_ERR_ATTR_NUM,				// Incorrect attribute number
	MCAPI_ERR_ATTR_VALUE,			// Incorrect attribute vale
	MCAPI_ERR_ATTR_NOTSUPPORTED,	// Attribute not supported by the implementation
	MCAPI_ERR_ATTR_READONLY,		// Attribute is read only
	MCAPI_ERR_MSG_LIMIT,			// The message size exceeds the maximum size allowed by the MCAPI implementation
	MCAPI_ERR_MSG_TRUNCATED,		// The message size exceeds the buffer size
	MCAPI_ERR_CHAN_OPEN,			// A channel is open, certain operations are not allowed
	MCAPI_ERR_CHAN_TYPE,			// Attempt to open a packet/scalar channel on an endpoint that has been connected with a different channel type
	MCAPI_ERR_CHAN_DIRECTION,		// Attempt to open a send handle on a port that was connected as a receiver, or vice versa
	MCAPI_ERR_CHAN_CONNECTED,		// A channel connection has already been established for one or both of the specified endpoints
	MCAPI_ERR_CHAN_OPENPENDING,		// An open request is pending
	MCAPI_ERR_CHAN_NOTOPEN,			// The channel is not open (cannot be closed)
	MCAPI_ERR_CHAN_INVALID,			// Argument is not a channel handle
	MCAPI_ERR_PKT_LIMIT,			// The packet size exceeds the maximum size allowed by the MCAPI implementation
	MCAPI_ERR_SCL_SIZE,				// Incorrect scalar size
	MCAPI_ERR_TRANSMISSION,			// Transmission failure
	MCAPI_ERR_PRIORITY,				// Incorrect priority level
	MCAPI_ERR_BUF_INVALID,			// Not a valid buffer descriptor
	MCAPI_ERR_MEM_LIMIT,			// Out of memory
	MCAPI_ERR_REQUEST_INVALID,		// Argument is not a valid request handle
	MCAPI_ERR_REQUEST_LIMIT,		// Out of request handles
	MCAPI_ERR_REQUEST_CANCELLED,	// The request was already canceled
	MCAPI_ERR_WAIT_PENDING,			// A wait is pending
        MCAPI_ERR_GENERAL,
	MCAPI_STATUSCODE_END			// This should always be last
};

/*
 * Node attribute numbers
 *
 */
enum mcapi_node_attribute_numbers {
	MCAPI_NODE_ATTR_TYPE,					/* Node type */
	MCAPI_NODE_ATTR_END						/* This should always be last */
};

/* MCAPI_NODE_ATTR_TYPE */
typedef mcapi_uint_t mcapi_node_attr_type_t;

/* Node Types */
enum mcapi_node_attribute_types {
	MCAPI_NODE_ATTR_TYPE_REGULAR,			/* Default - Regular node type */
	MCAPI_NODE_ATTR_TYPE_END				/* This should always be last */
};

/*
 * Endpoint attribute numbers
 *
 * Some of the endpoint attributes must have the same value for two endpoints to be connected by a channel. Those
 * attributes are identified below with "Channel compatibility attribute"
 *
 */
enum mcapi_endp_attribute_numbers {
	MCAPI_ENDP_ATTR_TYPE,					/* Endpoint type, message, packet channel or scalar channel */
	MCAPI_ENDP_ATTR_GET_COUNT,				/* Number of gets outstanding */
	MCAPI_ENDP_ATTR_GET_SOURCE,				/* Source id (domain, node) of endpoint "getter" */
	MCAPI_ENDP_ATTR_MAX_PAYLOAD_SIZE,		/* Maximum payload size - Channel compatibility attribute */
	MCAPI_ENDP_ATTR_BUFFER_TYPE,			/* Buffer type, FIFO - Channel compatibility attribute */
	MCAPI_ENDP_ATTR_MEMORY_TYPE,			/* Shared/local (0-copy), blocking or non-blocking on limit - Channel compatibility attribute */
	MCAPI_ENDP_ATTR_NUM_PRIORITIES,			/* Number of priorities - Channel compatibility attribute */
	MCAPI_ENDP_ATTR_PRIORITY,				/* Priority on connected endpoint - Channel compatibility attribute */
	MCAPI_ENDP_ATTR_NUM_SEND_BUFFERS,		/* Number of send buffers at the current endpoint priority level */
	MCAPI_ENDP_ATTR_NUM_RECV_BUFFERS,		/* Number of available receive buffers */
	MCAPI_ENDP_ATTR_ENDP_STATUS,			/* Endpoint status, connected, open etc. */
	MCAPI_ENDP_ATTR_TIMEOUT,				/* Timeout */
	MCAPI_ENDP_ATTR_END						/* This should always be last */
};

/* MCAPI endpoint Attributes
 * Implementations may designate some or all attributes as read-only
 *
 */

/* MCAPI_ENDP_ATTR_TYPE */
typedef mcapi_uint_t mcapi_endp_attr_type_t;

/* Endpoint Types */
#define MCAPI_ENDP_ATTR_TYPE_MESSAGE		0x00000001	/* Message type endpoint - Default */
#define MCAPI_ENDP_ATTR_TYPE_PKTCHAN		0x00000002	/* Packet channel type endpoint */
#define MCAPI_ENDP_ATTR_TYPE_SCLCHAN		0x00000004	/* Scalar channel type endpoint */

/* MCAPI_ATTR_GET_COUNT */
typedef mcapi_uint_t mcapi_endp_attr_get_count_t;	/* A counter tracking the number of outstanding gets
												   on the endpoint
												   Default = 0 */

/* MCAPI_ENDP_ATTR_GET_SOURCE */
typedef struct {
	mcapi_domain_t	domain_id;	/* Domain id - Default = MCAPI_DOMAIN_INVALID */
	mcapi_node_t	node_id;	/* Node id - Default = MCAPI_NODE_INVALID */
} mcapi_endp_attr_get_source_t;

/* MCAPI_ENDP_ATTR_MAX_PAYLOAD_SIZE */
typedef mcapi_int_t mcapi_endp_attr_max_payload_size_t;	/* Implementation defined default value */

/* MCAPI_ENDP_ATTR_BUFFER_TYPE */
typedef mcapi_uint_t mcapi_endp_attr_buffer_type_t;

/* Buffer Types */
enum mcapi_buffer_type {
	MCAPI_ENDP_ATTR_FIFO_BUFFER			/* Default */
};

/* MCAPI_ENDP_ATTR_MEMORY_TYPE */
typedef mcapi_uint_t mcapi_endp_attr_memory_type_t;

/* The type refers to both the memory's locality and behavior
 * Locality is local, shared and remote, defined in the lower two bytes of this attribute, mask = 0x0000ffff
 * Behavior defines behavior when no more memory is available, using the higher two bytes, mask = 0xffff0000
 * 		0x00000000
 * 		      ---- 	Locality
 * 		  ----		Behavior
 */

/* MCAPI Attribute Memory Locality Types
 * mask = 0x0000ffff
 */
enum mcapi_memory_type {
	MCAPI_ENDP_ATTR_LOCAL_MEMORY,		/* Zero copy operations not possible - Default */
	MCAPI_ENDP_ATTR_SHARED_MEMORY,		/* The user buffer provided by the sender is a shared memory buffer, zero copy possible */
	MCAPI_ENDP_ATTR_REMOTE_MEMORY
};

/* MCAPI Attribute Memory Behavior Types
 * The behavior applies to blocking receive functions.
 * mask = 0xffff0000
 */
#define MCAPI_ENDP_ATTR_BLOCK_ON_MEM_LIMIT	0x00000000	/* Block until memory becomes available - Default */
#define MCAPI_ENDP_ATTR_FAIL_ON_MEM_LIMIT	0x00010000	/* Fail if no memory available, status code = MCAPI_ERR_MEM_LIMIT */


/* MCAPI_ENDP_ATTR_NUM_PRIORITIES */
typedef mcapi_int_t mcapi_endp_attr_num_priorities_t;	/* Implementation defined default value */

/* MCAPI_ENDP_ATTR_PRIORITY */
typedef mcapi_uint_t mcapi_endp_attr_priority_t; 		/* A lower number means higher priority. A value of MCAPI_MAX_PRORITY (0) denotes the highest priority
														   MCAPI_MAX_PRORITY - Default */

/* MCAPI_ENDP_ATTR_NUM_SEND_BUFFERS */
typedef mcapi_int_t mcapi_endp_attr_num_send_buffers_t;	/* Implementation defined default value */

/* MCAPI_ENDP_ATTR_NUM_RECV_BUFFERS */
typedef mcapi_uint_t mcapi_endp_attr_num_recv_buffers_t; /* Number of received buffer available,
													   can for example be used for throttling.
													   Implementation defined default value  */

/* MCAPI_ENDP_ATTR_STATUS */
typedef mcapi_uint_t mcapi_endp_attr_status_t;

/*
 * MCAPI Endpoint Status Flags, are used to query the status of an endpoint, e.g. if it is connected and if so what type of channel, direction, etc.
 *
 * Note: The lower 16 bits are defined in mcapi.h whereas the upper 16 bits are reserved for implementation specific purposes and if used
 * must be defined in implementation_spec.h. It is therefore recommended that the upper 16 bits are masked off at the application level.
 *
 * 		0x00000000
 * 		      ---- 	mcapi.h
 * 		  ----		implementation_spec.h
 *
 * Default = 0x00000000
 *
 */
#define MCAPI_ENDP_ATTR_STATUS_CONNECTED		0x00000001	/* The endpoint is connected */
#define MCAPI_ENDP_ATTR_STATUS_OPEN				0x00000002	/* The channel is open */
#define MCAPI_ENDP_ATTR_STATUS_OPEN_PENDING		0x00000004	/* An open request is pending */
#define MCAPI_ENDP_ATTR_STATUS_CLOSE_PENDING	0x00000008	/* An close request is pending */
#define MCAPI_ENDP_ATTR_STATUS_PKTCHAN			0x00000001	/* The channel is a packet channel */
#define MCAPI_ENDP_ATTR_STATUS_SCLCHAN			0x00000002	/* The channel is a scalar channel */
#define MCAPI_ENDP_ATTR_STATUS_SEND				0x00000004	/* Endpoint is the send side of the channel */
#define MCAPI_ENDP_ATTR_STATUS_RECEIVE			0x00000008	/* Endpoint is the receive side of the channel */
#define MCAPI_ENDP_ATTR_STATUS_GET_PENDING		0x00000010	/* One or more endpoint get(s) are pending */

/* MCAPI_ENDP_ATTR_TIMEOUT */
typedef mcapi_timeout_t mcapi_endp_attr_timeout_t;	/* Timeout for blocking send and receive functions
												   Default = MCAPI_TIMEOUT_INFINITE and a value of
												   MCAPI_TIMEOUT_IMMEDIATE (or 0) means that the function will
												   return immediately, with success or failure*/


/*
 * MCAPI endpoint attribute #'s
 * The first range (64) is assigned to MCA
 * Organizations can apply to the MCA for endpoint attribute # ranges
 * One organization can be assigned multiple ranges.
 */
#define MCAPI_MAX_NUM_ENDP_ATTRS_MCA 	64		/* MCAPI endpoint attributes reserved for MCA */
#define MCAPI_MAX_NUM_ENDP_ATTRS_OTHER	32		/* MCAPI endpoint attributes reserved for other organizations */

/*
 * Macros for calculating starting and ending attribute numbers for an MCA assigned attribute range.
 *
 * x = assigned range. One organization can be assigned multiple ranges.
 */
#define mcapi_start_endp_attribute(x)  MCAPI_MAX_NUM_ENDP_ATTRS_MCA + (x * MCAPI_MAX_NUM_ENDP_ATTRS_OTHER)
#define mcapi_end_endp_attribute(x)  MCAPI_MAX_NUM_ENDP_ATTRS_MCA + (x + 1) * MCAPI_MAX_NUM_ENDP_ATTRS_OTHER

/*
 * MCAPI Initialization information
 * In addition to the MCAPI defined information implementations may
 * include implementation specific information.
 */
typedef struct
{
	mcapi_uint_t	mcapi_version;			/* MCAPI version, the three last (rightmost) hex digits are the minor number
												and those left of minor the major number */
	mcapi_uint_t	organization_id;		/* Implementation vendor/organization id */
	mcapi_uint_t	implementation_version; /* Vendor version, the three last (rightmost) hex digits are the minor number
												and those left of minor the major number */
	mcapi_uint_t	number_of_domains;		/* Number of domains in the topology */
	mcapi_uint_t	number_of_nodes;		/* Number of nodes in the domain, can be used for basic per domain topology discovery */
	mcapi_uint_t	number_of_ports;		/* Number of ports on the local node */
	impl_info_t 	*impl_info;				/* This structure has to be defined by the implementor in implementation_spec.h */
}mcapi_info_t;

/* MCAPI Organization specific reserved endpoint attribute numbers */
typedef struct
{
	mcapi_uint_t	organization_id;		/* Implementation vendor/organization id */
	mcapi_uint_t	start_endp_attr_num;	/* Starting MCA provided vendor specific attribute number */
	mcapi_uint_t	end_endp_attr_num;		/* Ending MCA provided vendor specific attribute number */
} mcapi__endp_attr_range_t;


/* In/out parameter indication macros */
#ifndef MCAPI_IN
#define MCAPI_IN const
#endif /* MCAPI_IN */

#ifndef MCAPI_OUT
#define MCAPI_OUT
#endif /* MCAPI_OUT */


/* Alignment macros */

#ifndef MCAPI_DECL_ALIGNED
#define MCAPI_DECL_ALIGNED MCA_DECL_ALIGNED
#endif

#ifndef MCAPI_BUF_ALIGN
#define MCAPI_BUF_ALIGN 0xff 
#endif

#ifndef LIB_BUILD

/*
 * Function prototypes
 */

/* Initialization, node and endpoint management */

extern void mcapi_initialize(
	MCAPI_IN mcapi_domain_t domain_id,
	MCAPI_IN mcapi_node_t node_id,
	MCAPI_IN mcapi_node_attributes_t* mcapi_node_attributes,
	MCAPI_IN mcapi_param_t* init_parameters,
	MCAPI_OUT mcapi_info_t* mcapi_info,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_finalize(
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mca_domain_t mcapi_domain_id_get(
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_node_t mcapi_node_id_get(
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_node_init_attributes(
	MCAPI_OUT mcapi_node_attributes_t* mcapi_node_attributes,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_node_get_attribute(
	MCAPI_IN mcapi_domain_t domain_id,
	MCAPI_IN mcapi_node_t node_id,
	MCAPI_IN mcapi_uint_t attribute_num,
	MCAPI_OUT void* attribute,
	MCAPI_IN size_t attribute_size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);
  
extern void mcapi_node_set_attribute(
                                MCAPI_OUT mcapi_node_attributes_t* mcapi_node_attributes,
                                MCAPI_IN mcapi_uint_t attribute_num,
                                MCAPI_IN void* attribute,
                                MCAPI_IN size_t attribute_size,
                                MCAPI_OUT mcapi_status_t* mcapi_status
                                );


extern mcapi_endpoint_t mcapi_endpoint_create(
	MCAPI_IN mcapi_port_t port_id,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_endpoint_get_i(
	MCAPI_IN mcapi_domain_t domain_id,
	MCAPI_IN mcapi_node_t node_id,
	MCAPI_IN mcapi_port_t port_id,
	MCAPI_OUT mcapi_endpoint_t* endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_endpoint_t mcapi_endpoint_get(
	MCAPI_IN mcapi_domain_t domain_id,
	MCAPI_IN mcapi_node_t node_id,
	MCAPI_IN mcapi_port_t port_id,
	MCAPI_IN mcapi_timeout_t timeout,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_endpoint_release(
		MCAPI_IN mcapi_endpoint_t endpoint,
		MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_endpoint_delete(
	MCAPI_IN mcapi_endpoint_t endpoint,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_endpoint_get_attribute(
	MCAPI_IN mcapi_endpoint_t endpoint,
	MCAPI_IN mcapi_uint_t attribute_num,
	MCAPI_OUT void* attribute,
	MCAPI_IN size_t attribute_size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_endpoint_set_attribute(
	MCAPI_IN mcapi_endpoint_t endpoint,
	MCAPI_IN mcapi_uint_t attribute_num,
	MCAPI_OUT const void* attribute,
	MCAPI_IN size_t attribute_size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);


/* Message functions */

extern void mcapi_msg_send_i(
	MCAPI_IN mcapi_endpoint_t send_endpoint,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_IN void* buffer,
	MCAPI_IN size_t buffer_size,
	MCAPI_IN mcapi_priority_t priority,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_msg_send(
	MCAPI_IN mcapi_endpoint_t send_endpoint,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_IN void* buffer,
	MCAPI_IN size_t buffer_size,
	MCAPI_OUT mcapi_priority_t priority,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_msg_recv_i(
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT void* buffer,
	MCAPI_IN size_t buffer_size,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_msg_recv(
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT void* buffer,
	MCAPI_IN size_t buffer_size,
	MCAPI_OUT size_t* received_size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint_t mcapi_msg_available(
	MCAPI_IN mcapi_endpoint_t receive_endoint,
	MCAPI_OUT mcapi_status_t* mcapi_status
);


/* Packet channel functions */

extern void mcapi_pktchan_connect_i(
	MCAPI_IN mcapi_endpoint_t send_endpoint,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_recv_open_i(
	MCAPI_OUT mcapi_pktchan_recv_hndl_t* receive_handle,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_send_open_i(
	MCAPI_OUT mcapi_pktchan_send_hndl_t* send_handle,
	MCAPI_IN mcapi_endpoint_t send_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_send_i(
	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle,
	MCAPI_IN void* buffer,
	MCAPI_IN size_t size,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_send(
	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle,
	MCAPI_IN void* buffer,
	MCAPI_IN size_t size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_recv_i(
	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle,
	MCAPI_OUT void** buffer,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_recv(
	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle,
	MCAPI_OUT void** buffer,
	MCAPI_OUT size_t* received_size,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint_t mcapi_pktchan_available(
	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_release(
                                  /* MR MCAPI_IN */ void* buffer,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_boolean_t mcapi_pktchan_release_test(
	MCAPI_IN void* buffer,
	MCAPI_OUT mcapi_status_t mcapi_status
);

extern void mcapi_pktchan_recv_close_i(
	MCAPI_IN mcapi_pktchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_pktchan_send_close_i(
	MCAPI_IN mcapi_pktchan_send_hndl_t send_handle,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);


/* Scalar channel functions */

extern void mcapi_sclchan_connect_i(
	MCAPI_IN mcapi_endpoint_t send_endpoint,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_recv_open_i(
	MCAPI_OUT mcapi_sclchan_recv_hndl_t* receive_handle,
	MCAPI_IN mcapi_endpoint_t receive_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_open_i(
	MCAPI_OUT mcapi_sclchan_send_hndl_t* send_handle,
	MCAPI_IN mcapi_endpoint_t  send_endpoint,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_uint64(
	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,
	MCAPI_IN mcapi_uint64_t dataword,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_uint32(
	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,
	MCAPI_IN mcapi_uint32_t dataword,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_uint16(
	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,
	MCAPI_IN mcapi_uint16_t dataword,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_uint8(
	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,
	MCAPI_IN mcapi_uint8_t dataword,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint64_t mcapi_sclchan_recv_uint64(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint32_t mcapi_sclchan_recv_uint32(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint16_t mcapi_sclchan_recv_uint16(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint8_t mcapi_sclchan_recv_uint8(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern mcapi_uint_t mcapi_sclchan_available(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_recv_close_i(
	MCAPI_IN mcapi_sclchan_recv_hndl_t receive_handle,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_sclchan_send_close_i(
	MCAPI_IN mcapi_sclchan_send_hndl_t send_handle,
	MCAPI_OUT mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);


/* Non-blocking management functions */

extern mcapi_boolean_t mcapi_test(
                                  /*MCAPI_IN*/ mcapi_request_t* request,
	MCAPI_OUT size_t* size,
 	MCAPI_OUT mcapi_status_t* mcapi_status);

extern mcapi_boolean_t mcapi_wait(
                                  /*MCAPI_IN*/ mcapi_request_t* request,
	MCAPI_OUT size_t* size,
	MCAPI_IN mcapi_timeout_t timeout,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern unsigned int mcapi_wait_any(
                                    MCAPI_IN size_t number,
                                    /*MCAPI_IN*/ mcapi_request_t** requests,
	MCAPI_OUT size_t* size,
	MCAPI_IN mcapi_timeout_t timeout,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

extern void mcapi_cancel(
                         /*MCAPI_IN*/ mcapi_request_t* request,
	MCAPI_OUT mcapi_status_t* mcapi_status
);

/* Convenience functions */
char* mcapi_display_status(mcapi_status_t status,char* status_message,size_t size);
void mcapi_set_debug_level(int d);

#endif /* LIB_BUILD */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MCAPI_H */

