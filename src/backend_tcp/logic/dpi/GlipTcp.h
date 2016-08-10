/* Copyright (c) 2016 by the author(s)
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

#include <cstdint>
#include <pthread.h>
#include <sys/socket.h>
#include <list>
#include <cstdlib>
#include <cassert>

#include <boost/lockfree/spsc_queue.hpp>

using namespace std;
using namespace boost::lockfree;

/**
 * Glip TCP interface
 */
class GlipTcp {
public:
    /**
     * Singleton: Get instance
     *
     * We are using a singleton to enforce we have exactly one instance.
     * It is therefore currently allowed to have two Glip DPI interfaces
     * in a system. As the interface to the logic still allows that,
     * the singleton will become a regular class in the future.
     *
     * @return Instance of the Glip TCP interface
     */
    static GlipTcp& instance() {
        static GlipTcp inst;
        return inst;
    }

    /**
     * Constructor replacement
     *
     * As we currently use a singleton, this is called instead of it.
     *
     * It can only be called once
     *
     * @param port TCP port
     * @param width Bit with as multiple of 8 and <=64
     */
    void init(int port, int width);

    /** Reset the interface */
    int reset();

    /**
     * Check if client is connected
     *
     * @return Whether client is connected
     */
    bool connected();

    /**
     * Go to next cycle
     * This function is used to advance to the next cycle. It is the
     * combinational part of the interface. It is called on the negative
     * clock edge. It returns a bit mask of available operations:
     *  - bit 0 if a control message can be received
     *  - bit 1 if an incoming data item is available
     *  - bit 2 if an outgoing data item can be send
     *
     * @return Bit mask of available actions
     */
    uint32_t next_cycle();

    /**
     * This function is used to pop the current control message
     *
     * It is called at the negative clock edge to set the output registers
     * of the interface.
     *
     * @return 0x1 if logic reset is requested, 0 otherwise
     */
    uint32_t control_msg();

    /**
     * This function is used to get the current incoming data
     *
     * It is called at the negative clock edge to set the incoming data
     * item for this cycle. It does not pop the data item, which requires
     * a call to glip_tcp_read_ack.
     *
     * @return Current incoming data item
     */
    uint64_t readData();

    /**
     * Acknowledge read
     *
     * This function is called on a positive clock edge.
     */
    void readAck();

    /**
     * Write a data item in this cycle
     *
     * This function is called on a positive clock edge.
     *
     * @param data Data item to write
     */
    void writeData(uint64_t data);

    /** Bit mask that a control message is available */
    static const uint32_t CONTROL_AVAILABLE  = 0x1;
    /** Bit mask that incoming data is available */
    static const uint32_t INCOMING_AVAILABLE = 0x2;
    /** Bit mask that outgoing data can be send */
    static const uint32_t OUTGOING_READY     = 0x4;

private:
    /* Singleton */
    GlipTcp();

    /** TCP base port number */
    int mPort;
    /** Number of bytes in width */
    int mNumBytes;
    /** Thread instance on TCP socket */
    pthread_t mThread;
    /** Flag if thread is ready */
    volatile bool mThreadReady;
    /** Flag if client is connected */
    volatile bool mConnected;

    /**
     * TCP Socket
     */
    struct Socket {
        /** Socket to listen on */
        int listenSocket;
        /** Socket to handle data with */
        int socket;

        /** Client address */
        struct sockaddr socketAddress;
        /** Length of client address */
        int socketAddressLength;
    };

    /** Socket for data communication */
    struct Socket mSocketData;
    /** Socket for control communication */
    struct Socket mSocketControl;

    /** Queue for incoming data */
    spsc_queue<uint8_t> mDataOut;
    /** Queue for outgoing data */
    spsc_queue<uint8_t> mDataIn;
    /** Queue for control messages */
    spsc_queue<uint8_t> mControl;

    /**
     * Temporary storage
     *
     * This storage is used to temporarily hold a data item between
     * the negative clock edge and the positive clock edge. It is used
     * to track how much data is currently transferred from and to
     * a code. It is also used to signal the completion of a transfer.
     */
    struct TempStorage {
        /** The item is valid/complete */
        bool valid;
        /** Count up until item is complete */
        size_t partial;
        /** Data item */
        uint64_t value;
    };

    /** Item to temporarily store incoming data */
    struct TempStorage mReadItem;
    /** Item to temporarily store outgoing data */
    struct TempStorage mWriteItem;
    /** Item to temporarily store control message */
    struct TempStorage mControlItem;

    /**
     * Helper function that opens the listen sockets
     *
     * @return Success of listening
     */
    bool listenSockets();

    /** Thread function */
    void *thread();

    /** C wrapper for thread function */
    static void* thread(void* ctx);
};
