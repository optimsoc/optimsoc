/* Copyright (c) 2014-2017 by the author(s)
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

#ifndef __OPTIMSOC_MP_H__
#define __OPTIMSOC_MP_H__

#include <stdint.h>
#include <stdlib.h>

/**
 * \defgroup libmp Message passing library
 *
 * This is the message passing library for inter-tile communication in
 * OpTiMSoC. It abstracts from the hardware and serves as a generic
 * API. The implementation choses the best available hardware.
 *
 * Each communication is between two endpoints, which are a tuple:
 * (tile, node, port). The application has to manage nodes and port by
 * itself and tiles are denoted by rank of the compute tile.
 *
 * There are two types of communication: Connection-less communication
 * with messages is best suited for adhoc communication between
 * multiple communication partners. The protocol for
 * connection-oriented channel is more efficient if the communication
 * relations are fixed, such as in streaming applications. Here
 * channels establish a fixed communication relation and flow control
 * can be implemented more efficient.
 *
 * \{
 */

typedef struct endpoint_handle* optimsoc_mp_endpoint_handle;

typedef enum {
	OPTIMSOC_MP_SUCCESS = 0,
	OPTIMSOC_MP_ERR_NOT_INITIALIZED,
	OPTIMSOC_MP_ERR_NO_MEMORY,
	OPTIMSOC_MP_ERR_INVALID_PARAMETER,
	OPTIMSOC_MP_ERR_GENERIC,
	OPTIMSOC_MP_ERR_CANNOT_RESOLVE,
	OPTIMSOC_MP_ERR_ALIGNMENT
} optimsoc_mp_result_t;

typedef enum {
	OPTIMSOC_MP_MGMT_DOMAIN_IS_TILE = 0
} optimsoc_mp_mgmt_type_t;

/**
 * Initialize the message passing subsystem
 *
 * \return 0 on success, an error code otherwise
 */
optimsoc_mp_result_t optimsoc_mp_initialize(optimsoc_mp_mgmt_type_t mgmt_type);

/**
 * Create an endpoint
 *
 * Creates a message passing endpoint in this tile for a specific node.
 * Other endpoints can send messages or connect to the endpoint via the
 * <app, task, port> tuple.
 *
 * The buffer can either be connection-less or
 * connection-oriented. When connection-oriented you need to connect
 * two endpoints to a channel first using
 * optimsoc_mp_channel_connect().
 *
 * The buffer_size is the number of messages or packets that can be used. If
 * messages or packets are larger than the default size, the maximum element
 * size can be overwritten for this endpoint.
 *
 * \param endpoint Pointer to the handle to initialize
 * \param app Application ID
 * \param node Node ID
 * \param port Port of the remote endpoint
 * \param type Type of the endpoint
 * \param buffer_size Number of messages the endpoint accepts
 * \param overwrite_max_size Set the maximum message size or 0 for the default
 * \return 0 if success, error code otherwise
 */

typedef enum {
	OPTIMSOC_MP_EP_DEFAULTS = 0,
	OPTIMSOC_MP_EP_CHANNEL = 1, /*!< Connection-oriented communication */
	OPTIMSOC_MP_EP_BUFFERED = 2, /*!< Send buffered */
	OPTIMSOC_MP_EP_NORECV = 4, /*!< Only used for sending */
} optimsoc_endpoint_attr_t;

optimsoc_mp_result_t optimsoc_mp_endpoint_create(optimsoc_mp_endpoint_handle *endpoint,
		uint32_t domain, uint32_t node, uint32_t port,
		uint32_t buffer_size, size_t msg_size, optimsoc_endpoint_attr_t attr);

/**
 * Retrieve a remote endpoint
 *
 * Before communicating with a remote endpoint, an endpoint handle
 * must be configured. This function initializes the endpoint for the
 * remote endpoint.
 *
 * This function blocks until the remote endpoint is available.
 *
 * \param endpoint Handle to initialize
 * \param app ID of the application of the remote endpoint
 * \param node ID of the node of the remote endpoint
 * \param port Port of the remote endpoint
 * \return Return 0 on success, an error code otherwise
 */
optimsoc_mp_result_t optimsoc_mp_endpoint_get(optimsoc_mp_endpoint_handle *endpoint,
        uint32_t domain, uint32_t node, uint32_t port);

/**
 * Send a message to a remote endpoint
 *
 * A local endpoint is used to store the data locally for an
 * asynchronous transfer if supported. The local endpoint must be
 * created by optimsoc_mp_endpoint_create() with type
 * OPTIMSOC_MP_EP_CONNECTIONLESS. The remote endpoint must be
 * retrieved with optimsoc_mp_endpoint_get().
 *
 * The function blocks until the data is stored in another data
 * structure or successfully transfer, i.e., the input buffer can be
 * safely reused.
 *
 * \param from Local endpoint
 * \param to Remote endpoint
 * \param data Data to transfer
 * \param size Size of the data to transfer
 * \return 0 on success, an error code otherwise
 */
optimsoc_mp_result_t optimsoc_mp_msg_send(optimsoc_mp_endpoint_handle from,
                         optimsoc_mp_endpoint_handle to, uint32_t *data,
                         uint32_t size);

/**
 * Receive a message from a local endpoint
 *
 * Receive a message from the local endpoint. The endpoint must be
 * configured by optimsoc_mp_endpoint_create() with type
 * OPTIMSOC_MP_EP_CONNECTIONLESS.
 *
 * The function blocks until a message was received.
 *
 * \param ep Local endpoint to receive from
 * \param buffer Output data buffer
 * \param buffer_size Size of the buffer
 * \param received_size The actual written data size
 * \return 0 on success, an error code otherwise
 */
int optimsoc_mp_msg_recv(optimsoc_mp_endpoint_handle ep, uint8_t *buffer,
                         uint32_t buffer_size, uint32_t *received_size);

int optimsoc_mp_channel_connect(optimsoc_mp_endpoint_handle from,
        optimsoc_mp_endpoint_handle to);

int optimsoc_mp_channel_send(optimsoc_mp_endpoint_handle from,
        optimsoc_mp_endpoint_handle to,
        uint8_t* buffer, uint32_t size);

int optimsoc_mp_channel_recv(optimsoc_mp_endpoint_handle ep,
        uint8_t* buffer, uint32_t buffer_size,
        uint32_t *received_size);

// Channel migration
int optimsoc_mp_channel_pause(optimsoc_mp_endpoint_handle ep);

int optimsoc_mp_channel_continue(optimsoc_mp_endpoint_handle ep);

char* optimsoc_mp_error_string(int errno);

typedef enum {
	TRACE_MP_FUNC_CALLS        = 0x00000001,
	TRACE_MP_LOWLVL_FUNC_CALLS = 0x00000002,
	TRACE_MP_PROTO         = 0x00000004,
	TRACE_MP_ALL               = 0xffffffff
} optimsoc_mp_trace_config_t;

void optimsoc_mp_trace_config_set(optimsoc_mp_trace_config_t config);

optimsoc_mp_trace_config_t optimsoc_mp_trace_config_get();

/**
 * \}
 */

#endif
