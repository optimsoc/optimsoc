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

#ifndef MOCK_HOSTMOD_H
#define MOCK_HOSTMOD_H

#include <check.h>

#include <osd/osd.h>
#include <osd/hostmod.h>

#define MOCK_HOSTMOD_DIADDR 42

struct mock_hostmod_regaccess {
    uint64_t reg_val;
    uint16_t diaddr;
    uint16_t reg_addr;
    int reg_size_bit;
    int flags;
    osd_result retval;
};

struct mock_hostmod_event {
    struct osd_packet* pkg;
    osd_result retval;
};

void mock_hostmod_setup(void);
void mock_hostmod_teardown(void);

void mock_hostmod_expect_reg_read_raw(struct mock_hostmod_regaccess *exp);
void mock_hostmod_expect_reg_write_raw(struct mock_hostmod_regaccess *exp);
void mock_hostmod_expect_reg_read16(uint16_t reg_val, uint16_t diaddr,
                                    uint16_t reg_addr, osd_result retval);
void mock_hostmod_expect_mod_describe(uint16_t diaddr, uint16_t vendor,
                                      uint16_t type, uint16_t version);
void mock_hostmod_expect_reg_write16(uint16_t reg_val, uint16_t diaddr,
                                     uint16_t reg_addr, osd_result retval);
void mock_hostmod_expect_event_send(struct osd_packet *event_pkg,
                                    osd_result retval);
void mock_hostmod_expect_event_send_fromfile(const char* path);
void mock_hostmod_expect_event_receive(struct osd_packet *event_pkg,
                                       osd_result retval);
struct osd_hostmod_ctx* mock_hostmod_get_ctx();

#endif // MOCK_HOSTMOD_H
