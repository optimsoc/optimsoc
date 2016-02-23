#ifndef __VERILATEDTOPLEVEL_H__
#define __VERILATEDTOPLEVEL_H__

#include <verilated.h>

namespace optimsoc {

class VerilatedToplevel {
public:
    class SignalProxy {
    public:
        vluint8_t *signal;
        vluint8_t get() { return *signal; }
        void set(vluint8_t v) { *signal = v; }
    };

    SignalProxy sig_clk;
    SignalProxy sig_rst_sys;
    SignalProxy sig_rst_cpu;

    virtual void wrapEval() = 0;
    virtual void wrapTrace (VerilatedVcdC* tfp, int levels, int options=0) = 0;
};

}

#define VERILATED_TOPLEVEL(topname) \
        class topname : public optimsoc::VerilatedToplevel, \
            public V##topname { \
        public: \
            topname(const char* name) : V##topname(name) { \
                sig_clk.signal = &clk; \
                sig_rst_sys.signal = &rst_sys; \
                sig_rst_cpu.signal = &rst_cpu; \
            } \
            void wrapEval() { eval(); } \
            void wrapTrace(VerilatedVcdC* tfp, int levels, int options=0) { \
                trace(tfp, levels, options); \
            } \
        };

#endif
