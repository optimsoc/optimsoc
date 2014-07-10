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
    void setInsn(uint32_t *wb_insn)
    {
        m_insn = wb_insn;
    }
    void setPC(uint32_t *pc) {
        m_pc = pc;
    }
    void setEnable(uint8_t *enable)
    {
        m_enable = enable;
    }
    void setR3(uint32_t *r3)
    {
        m_r3 = r3;
    }

private:
    uint32_t m_coreid;
    uint32_t *m_insn;
    uint32_t *m_pc;
    uint8_t *m_enable;
    uint32_t *m_r3;
};

#endif /* VERILATEDSTM_H_ */
