/* Copyright 2017 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_HOST_CONTROLLER_H
#define MOCK_HOST_CONTROLLER_H

#include <check.h>

#include <osd/osd.h>
#include <osd/packet.h>
#include <czmq.h>

void mock_host_controller_setup(void);
void mock_host_controller_teardown(void);

osd_result mock_host_controller_queue_data_packet(const struct osd_packet *pkg);
void mock_host_controller_expect_reg_write(unsigned int src,
                                           unsigned int dest,
                                           unsigned int reg_addr,
                                           uint16_t exp_write_value);
void mock_host_controller_expect_reg_read(unsigned int src,
                                          unsigned int dest,
                                          unsigned int reg_addr,
                                          uint16_t ret_value);
void mock_host_controller_expect_reg_read_noresp(unsigned int src,
                                                 unsigned int dest,
                                                 unsigned int reg_addr);
void mock_host_controller_expect_mod_describe(unsigned int src,
                                              unsigned int dest,
                                              uint16_t vendor, uint16_t type,
                                              uint16_t version);
void mock_host_controller_expect_mgmt_req(const char* cmd, const char* resp);
void mock_host_controller_expect_diaddr_req(unsigned int diaddr);
void mock_host_controller_expect_data_req(struct osd_packet *req, struct osd_packet *resp);
void mock_host_controller_wait_for_event_tx(void);
void mock_host_controller_wait_for_requests(void);
#endif // MOCK_HOST_CONTROLLER_H
