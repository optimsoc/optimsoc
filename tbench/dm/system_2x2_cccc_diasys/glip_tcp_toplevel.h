/* Copyright (c) 2013-2014 by the author(s)
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

#ifndef GLIP_TCP_TOPLEVEL_H_
#define GLIP_TCP_TOPLEVEL_H_

#include <systemc.h>
#include <queue>


/**
 * A SystemC module to connect to any GLIP-enabled application
 *
 * The TCP connection uses port 23000 for data communication and port 23001 for
 * control information.
 */
SC_MODULE(glip_tcp_toplevel)
{
public:
    // GLIP FIFO Interface
    sc_in<uint16_t> fifo_out_data;
    sc_in<bool> fifo_out_valid;
    sc_out<bool> fifo_out_ready;
    sc_out<uint16_t> fifo_in_data;
    sc_out<bool> fifo_in_valid;
    sc_in<bool> fifo_in_ready;

    // Control Interface
    sc_out<bool> ctrl_logic_rst;

    // Clock and Reset
    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_out<bool> com_rst;

    SC_CTOR(glip_tcp_toplevel)
    {
        m_int_rst = false;
        m_trigger_rst = false;

        m_chan_data_ok = false;
        m_chan_ctrl_ok = false;

        // TCP interface
        SC_THREAD(tcp_data_proc);
        SC_THREAD(tcp_ctrl_proc);

        // FIFO logic interface
        SC_METHOD(fifo_if_proc);
        sensitive << clk.pos();

        // sync. reset handling
        SC_METHOD(reset_proc);
        sensitive << clk.pos();

        // control signals
        SC_METHOD(ctrl_signal_proc);
        sensitive << clk.pos();
    }

private:
    /**
     * Default data port; the control port will be DEFAULT_PORT_DATA + 1
     */
    static const unsigned int DEFAULT_PORT_DATA = 23000;

    // control messages
    static const unsigned int CTRL_MSG_LOGIC_RESET = 0x0001;

    /**
     * Size of the internal output buffer (in 16 bit words)
     */
    static const unsigned int OUTPUT_BUF_SIZE = 1024;

    bool m_chan_data_ok;
    bool m_chan_ctrl_ok;

    bool m_int_rst;
    bool m_trigger_rst;
    bool m_trigger_ctrl_rst;


    std::queue<uint16_t> m_input_queue;
    std::queue<uint16_t> m_output_queue;

    void tcp_data_proc();
    void tcp_ctrl_proc();

    void fifo_if_proc();
    void reset_proc();
    void ctrl_signal_proc();

    int wait_for_connection(int listenfd);
    int listen_socket(unsigned int port);
};

#endif /* GLIP_TCP_TOPLEVEL */
