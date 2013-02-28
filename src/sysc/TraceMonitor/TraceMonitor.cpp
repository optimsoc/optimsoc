#include "TraceMonitor.h"

#include <iostream>

void TraceMonitor::monitor() {
    if (*m_wb_freeze==0) {
        if ((*m_wb_insn >> 24) == 0x15) {
            unsigned int K = *m_wb_insn & 0xffff;
            uint32_t val = *m_r3;

            switch (K) {
            case 0:
                // normal nop..
                break;
            case 1:
                sc_stop();
                break;
            case 4:
                if (m_newline) {
                    if (ofstdout) fprintf(ofstdout,"[%16s] ",sc_time_stamp().to_string().c_str());
                    m_newline = false;
                }
                if (ofstdout) fprintf(ofstdout,"%c",(char)val);
                if ((char)val=='\n') {
                    m_newline = true;
                }
                break;
#include "traceevents.h"
            default:
                if (ofevents) fprintf(ofevents,"[%16s] Event %x: %x\n",sc_time_stamp().to_string().c_str(),K,val);
                break;
            }
	    fflush(ofevents);
        }
    }
}

void TraceMonitor::setWbPc(uint32_t *pc) {
    m_wb_pc = pc;
}

void TraceMonitor::setWbInsn(uint32_t *wb_insn) {
    m_wb_insn = wb_insn;
}

void TraceMonitor::setWbFreeze(uint8_t *wb_freeze) {
    m_wb_freeze = wb_freeze;
}

void TraceMonitor::setR3(uint32_t *r3) {
    m_r3 = r3;
}

void TraceMonitor::setEventTraceFile(std::string name) {
    if (!ofevents) {
        ofevents = fopen(name.c_str(),"w");
    }
}

void TraceMonitor::setStdoutFile(std::string name) {
    if (!ofstdout) {
        ofstdout = fopen(name.c_str(),"w");
    }
}
