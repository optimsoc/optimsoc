/*
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#ifndef VERILATEDSTM_H_
#define VERILATEDSTM_H_

#include <systemc.h>
#include "DebugModule.h"

// configuration
// activate:
//   address 0: write (uint8_t) 1 to activate or 0 to de-activate

class VerilatedSTM: public DebugModule
{
public:
    sc_in<bool> clk;
    typedef VerilatedSTM SC_CURRENT_USER_MODULE;
    VerilatedSTM(sc_module_name nm, DebugConnector *dbgconn);

    virtual uint16_t getType();
    virtual uint16_t getVersion();
    virtual uint16_t write(uint16_t address, uint16_t size, char* data);
    virtual uint16_t read(uint16_t address, uint16_t *size, char** data);

    void monitor();

    void setCoreId(uint32_t coreid)
    {
        m_coreid = coreid;
    }
    void setWbInsn(uint32_t *wb_insn)
    {
        m_wb_insn = wb_insn;
    }
    void setWbFreeze(uint8_t *wb_freeze)
    {
        m_wb_freeze = wb_freeze;
    }
    void setR3(uint32_t *r3)
    {
        m_r3 = r3;
    }

private:
    uint32_t m_coreid;
    uint32_t *m_wb_insn;
    uint8_t *m_wb_freeze;
    uint32_t *m_r3;
};

#endif /* VERILATEDSTM_H_ */
