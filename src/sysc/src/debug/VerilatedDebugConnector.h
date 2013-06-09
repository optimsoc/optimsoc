#ifndef VERILATEDDEBUGCONNECTOR_H_
#define VERILATEDDEBUGCONNECTOR_H_

#include <systemc.h>

#include "DebugModule.h"
#include "DebugConnector.h"

#include <vector>

class VerilatedDebugConnector: public DebugConnector
{
public:
    sc_out<bool> rst_sys;
    sc_out<bool> rst_cpu;

    SC_CTOR(VerilatedDebugConnector);

    void start();
    void stop();

};

#endif /* VERILATEDDEBUGCONNECTOR_H_ */
