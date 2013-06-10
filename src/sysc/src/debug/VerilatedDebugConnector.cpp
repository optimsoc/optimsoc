#include "VerilatedDebugConnector.h"

VerilatedDebugConnector::VerilatedDebugConnector(sc_module_name nm, uint16_t systemid) :
        DebugConnector(nm, systemid)
{
}

void VerilatedDebugConnector::start()
{
    rst_sys = 0;
    rst_cpu = 0;
}

void VerilatedDebugConnector::stop()
{
    rst_sys = 1;
    rst_cpu = 1;
}
