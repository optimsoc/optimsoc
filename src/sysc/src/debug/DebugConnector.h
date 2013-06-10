/*
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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
class DebugConnector: ::sc_core::sc_module
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
    void connection();

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
    bool sendTrace(DebugModule *mod, TracePacket &packet);

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

private:
    int m_port; /*!< Port of the TCP connection. */
    int m_connectionfd; /*!< The socket of the TCP connection. */
    uint16_t m_systemid; /*!< ID of the system. */

    /** Vector of registered debug modules */
    std::vector<DebugModule*> m_debugModules;
};

#endif /* DEBUGCONNECTOR_H_ */
