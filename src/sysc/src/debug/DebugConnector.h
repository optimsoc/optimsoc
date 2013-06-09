#ifndef DEBUGCONNECTOR_H_
#define DEBUGCONNECTOR_H_

#include <systemc.h>
#include "DebugModule.h"
#include <vector>

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
    SC_CTOR(DebugConnector);

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
     * itself, but is called with raw data. This data is packaged in a
     * MSGTYPE_TRACE type packet and delivered via the tcp connection.
     *
     * \param mod  The sending module
     * \param data The (serialized) trace event
     * \param size The size of the (serialized) trace event
     */
    void sendTrace(DebugModule* mod, char *data, unsigned int size);

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

    /** Vector of registered debug modules */
    std::vector<DebugModule*> m_debugModules;
};

#endif /* DEBUGCONNECTOR_H_ */
