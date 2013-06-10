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

    VerilatedDebugConnector(sc_module_name nm, uint16_t systemid);
    typedef DebugConnector SC_CURRENT_USER_MODULE;

    void start();
    void stop();

};

#endif /* VERILATEDDEBUGCONNECTOR_H_ */
