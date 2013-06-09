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
