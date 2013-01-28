/*
 * mtapi_optimsoc_spec.c
 *
 *  Created on: Nov 20, 2012
 *      Author: ga49qez
 */
#include "mtapi_optimsoc_spec.h"

const unsigned int NUM_ACTIONS = 1;
const unsigned int REQUEST_SEND_PORT = 1;
const unsigned int REQUEST_RECV_PORT = 2;
const unsigned int LISTENER_SEND_PORT = 3;
const unsigned int LISTENER_RECV_PORT = 4;
const unsigned int MAX_TIMEOUT = 500;

const mtapi_action_id_t ACTIONS[] = {FIBONACCI_ACTION_01};

const unsigned int NO_NODES = 2;
const mtapiRT_NodeInfo_t NODE_INFOS[] = {
		{0, 1, 1001, &ACTIONS[0]},
		{1, 2, 1001, &ACTIONS[0]}	};
