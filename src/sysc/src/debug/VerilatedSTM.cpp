/* Copyright (c) 2013 by the author(s)
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
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "VerilatedSTM.h"

#include "DebugConnector.h"
#include "TracePacket.h"

#define STM_VERSION 0

VerilatedSTM::VerilatedSTM(sc_module_name name, DebugConnector *dbgconn) :
        DebugModule(name, dbgconn), m_insn(NULL), m_enable(NULL),
        m_coreid(0), m_r3(NULL), m_pc(NULL)
{
    SC_METHOD(monitor);
    sensitive << clk.neg();
}

uint16_t VerilatedSTM::getType()
{
    return DBGTYPE_STM;
}

uint16_t VerilatedSTM::getVersion()
{
    return STM_VERSION;
}

uint16_t VerilatedSTM::write(uint16_t address, uint16_t size, char* data)
{
    return 0;
}

uint16_t VerilatedSTM::read(uint16_t address, uint16_t *size, char** data)
{
    return 0;
}

void VerilatedSTM::monitor()
{
    assert(m_pc && m_insn && m_enable);
    STMTracePacket packet;
    packet.coreid = m_coreid;
    if (*m_enable == 0) {
        return;
    }
    if ((*m_insn >> 24) == 0x15) {
        unsigned int K = *m_insn & 0xffff;
        uint32_t val = *m_r3;

        if (K > 0) {
            packet.timestamp = sc_time_stamp().value() / 1000;
            packet.id = K;
            packet.value = val;
            m_dbgconn->sendTrace(this, packet);
        }
    } else if (*m_insn == 0x24000000) {
        packet.timestamp = sc_time_stamp().value() / 1000;
        packet.id = 0x11;
        packet.value = *m_pc;
        m_dbgconn->sendTrace(this, packet);
    } else if (((*m_pc & 0xf00) != 0) &&
               ((*m_pc & 0xff) == 0) &&
               ((*m_pc & 0xfffff000) == 0)) {
            packet.timestamp = sc_time_stamp().value() / 1000;
            packet.id = 0x10;
            packet.value = (*m_pc & 0xf00) >> 8;
            m_dbgconn->sendTrace(this, packet);
    }
}
