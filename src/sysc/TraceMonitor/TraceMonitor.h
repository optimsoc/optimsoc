#ifndef TRACEMONITOR_H_
#define TRACEMONITOR_H_

#include <systemc.h>
#include <stdio.h>
#include <string>

SC_MODULE(TraceMonitor) {
public:
    sc_in<bool> clk;
    SC_CTOR(TraceMonitor) : ofevents(NULL), ofstdout(NULL) {
        SC_METHOD(monitor);
        sensitive << clk.neg();

        m_newline = true;
    }

    void monitor();
    void setWbPc(uint32_t *wb_pc);
    void setWbInsn(uint32_t *wb_insn);
    void setWbFreeze(uint8_t *wb_insn);
    void setR3(uint32_t *r3);
    void setRFwaddr(uint8_t *waddr);
    void setRFwen(uint8_t *wen);
    void setRFwdata(uint32_t *wdata);

    void setEventTraceFile(std::string name);
    void setStdoutFile(std::string name);

    uint32_t *m_wb_pc;
    uint32_t *m_wb_insn;
    uint8_t *m_wb_freeze;
    uint32_t *m_r3;

    bool m_newline;
    FILE *ofevents;
    FILE *ofstdout;
};

#endif /* TRACEMONITOR_H_ */
