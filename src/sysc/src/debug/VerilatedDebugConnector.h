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
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef VERILATEDDEBUGCONNECTOR_H_
#define VERILATEDDEBUGCONNECTOR_H_

#include <systemc.h>

#include "DebugModule.h"
#include "DebugConnector.h"

#include <vector>

/**
 * Debug connector for verilated simulations
 *
 * This connector is used with verilated simulations and has two modi:
 *  - It can run standalone and write events to an event file
 *  - It can start a TCP server and wait for the host software to connect
 */
class VerilatedDebugConnector: public DebugConnector
{
public:
    sc_in<bool> clk;      /*!< system clk */
    sc_out<bool> rst_sys; /*!< system reset */
    sc_out<bool> rst_cpu; /*!< cpu reset */

    /**
     * Constructor
     *
     * Set name, system id and enable standalone mode (default start server)
     */
    VerilatedDebugConnector(sc_module_name nm, uint16_t systemid, bool standalone = false);
    typedef VerilatedDebugConnector SC_CURRENT_USER_MODULE;

    virtual void start(); /*!< Overload start function to start the SystemC simulation */
    virtual void stop(); /*!< Overload stop function to stop the SystemC simulation */
    virtual void resetSystem(); /*!< Overload reset function to reset the SystemC simulation */

    virtual void connection(); /*!< Overload connection to handle standalone mode */

    /**
     * Overloaded send trace
     *
     * Handle trace event in standalone mode or forward to host via TCP connection.
     */
    virtual bool sendTrace(DebugModule *mod, TracePacket &packet);
private:
    // All for standalone
    bool m_standalone; /*!< This is a standalone simulation */
    std::vector<std::ofstream*> m_standalone_stdout; /*!< stdout files */
    std::vector<std::ofstream*> m_standalone_events; /*!< event files */
    std::vector<bool> m_standalone_finished;
    unsigned int m_standalone_finished_count;
};

#endif /* VERILATEDDEBUGCONNECTOR_H_ */
