#include "VerilatedSTM.h"

#include "DebugConnector.h"

#define STM_VERSION 0

struct stm_trace
{
public:
    uint32_t coreid;
    uint32_t timestamp;
    uint16_t id;
    uint32_t value;

    stm_trace()
    {
        m_data = new char[getSize()];
        assert(m_data != 0);
    }

    int getSize()
    {
        return 4 + 4 + 2 + 4;
    }
    char* getPacket()
    {
        memcpy(&m_data[0], (void*) &coreid, 4);
        memcpy(&m_data[4], (void*) &timestamp, 4);
        memcpy(&m_data[8], (void*) &id, 2);
        memcpy(&m_data[10], (void*) &value, 4);
        return m_data;
    }
private:
    char *m_data;
};

VerilatedSTM::VerilatedSTM(sc_module_name name, DebugConnector *dbgconn) :
        DebugModule(name, dbgconn)
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
    struct stm_trace trace;
    trace.coreid = m_coreid;
    if (*m_wb_freeze != 0) {
        return;
    }
    if ((*m_wb_insn >> 24) == 0x15) {
        unsigned int K = *m_wb_insn & 0xffff;
        uint32_t val = *m_r3;

        if (K > 0) {
            trace.timestamp = sc_time_stamp().value() / 1000;
            trace.id = K;
            trace.value = val;
            m_dbgconn->sendTrace(this, trace.getPacket(), trace.getSize());
        }
    }
}
