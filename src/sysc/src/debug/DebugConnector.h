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

#ifndef DEBUGCONNECTOR_H_
#define DEBUGCONNECTOR_H_

#include <systemc.h>
#include <vector>

#include "DebugModule.h"
#include "TracePacket.h"

/**
 * @defgroup debug Debugging Infrastructure
 */

/**
 * The abstract base class to handle debug connections
 *
 * This is the abstract base class for all implementations of a simtcp debug
 * connector. Its purpose is to connect the debug modules to the debugging
 * software via TCP. This class implements the connection and the basic handling
 * of debug modules.
 *
 * \ingroup debug
 */
class DebugConnector : public ::sc_core::sc_module
{
public:
    /**
     * The constructor
     *
     * The constructor initializes its SystemC thread.
     */
    DebugConnector(sc_module_name nm, uint16_t systemid);
    typedef DebugConnector SC_CURRENT_USER_MODULE;


    /**
     * TCP connection thread
     *
     * This is the SystemC thread waiting for connections and delivering the
     * messages to the debug modules and replies back. It gets the messages
     * from the TCP connection and calls handleMessage on each received message.
     */
    virtual void connection();

    /**
     * Add a debug module to this connection.
     *
     * With the function the simulation can add a Debug Module to the debug
     * setup.
     *
     * \param mod The debug module to add
     */
    void registerDebugModule(DebugModule* mod);

    /**
     * Function to send a trace via TCP
     *
     * This function can be called by the debug modules to send trace events
     * via the TCP connection. It does not care about the actual trace event
     * itself, only the raw data from the TracePacket is used and and packaged
     * in a MSGTYPE_TRACE type packet and delivered via the TCP connection.
     *
     * \param mod    The sending module
     * \param packet The trace packet to send
     * \return operation successful?
     */
    virtual bool sendTrace(DebugModule *mod, TracePacket &packet);

protected:
    /**
     * Process an incoming message
     *
     * This is the function that processes the incoming messages.
     */
    void handleMessage(int type, uint8_t *payload, unsigned int paylen);

    /**
     * Stop current simulation
     *
     * This function is overloaded in the actual Debug connector and terminates
     * a running debug session.
     */
    virtual void stop() = 0;

    /**
     * Start simulation for debug
     *
     * This function is overloaded in the actual debug connector and starts a
     * debug session.
     */
    virtual void start() = 0;

    virtual void resetSystem() = 0;

    /** Vector of registered debug modules */
    std::vector<DebugModule*> m_debugModules;
private:
    int m_port; /*!< Port of the TCP connection. */
    int m_connectionfd; /*!< The socket of the TCP connection. */
    uint16_t m_systemid; /*!< ID of the system. */
};

#endif /* DEBUGCONNECTOR_H_ */
