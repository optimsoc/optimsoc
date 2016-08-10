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

#include "glip_tcp_toplevel.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

SC_MODULE_EXPORT(glip_tcp_toplevel);

/**
 * Method: Synchronous FIFO interface
 */
void glip_tcp_toplevel::fifo_if_proc()
{
    if (m_int_rst) {
        // don't do anything while in reset
        return;
    }

    // Logic -> Host
    if (fifo_out_valid.read()) {
        m_output_queue.push(fifo_out_data);
    }
    fifo_out_ready = (m_output_queue.size() <= OUTPUT_BUF_SIZE);

    // Host -> Logic
    if (m_input_queue.empty()) {
        // not necessary, but easier for debugging
        fifo_in_data = 0x0000;
    } else {
        fifo_in_data = m_input_queue.front();
    }
    fifo_in_valid = !m_input_queue.empty();
    if (fifo_in_ready && !m_input_queue.empty()) {
        m_input_queue.pop();
    }
}

/**
 * Method: Synchronous reset
 */
void glip_tcp_toplevel::reset_proc()
{
    m_int_rst = (m_trigger_rst || rst);

    if (m_int_rst) {
        com_rst = true;

        fifo_out_ready = false;
        fifo_in_valid = false;

        // clear internal FIFO buffers
        while (!m_input_queue.empty()) m_input_queue.pop();
        while (!m_output_queue.empty()) m_output_queue.pop();

        m_trigger_rst = false;
    } else {
        com_rst = false;
    }
}

/**
 * Method: Synchronous handling of the control signals
 */
void glip_tcp_toplevel::ctrl_signal_proc()
{
    // logic reset
    if (m_trigger_ctrl_rst) {
        ctrl_logic_rst = true;
        m_trigger_ctrl_rst = false;
    } else {
        ctrl_logic_rst = false;
    }
}

/**
 * Thread: Send/receive data from/to the TCP data channel
 *
 * The incoming data is stored in |m_input_queue|, the outgoing data is read
 * from |m_output_queue|.
 */
void glip_tcp_toplevel::tcp_data_proc()
{
    int listenfd = listen_socket(DEFAULT_PORT_DATA);
    if (listenfd == -1) {
        return;
    }

    while (1) {
        int connfd = wait_for_connection(listenfd);
        if (connfd == -1) {
            return;
        }
        m_chan_data_ok = true;

        // wait for both communication channels to be established
        while (!m_chan_ctrl_ok) {
            wait(100, SC_NS);
        }

        // reset the internal state, and inform the logic of it (com rst)
        m_trigger_rst = true;

        // continuously send and receive data
        uint16_t buffer[1024];
        memset(&buffer, 0, sizeof(buffer));

        while (1) {
            // don't do anything while in reset
            while (m_int_rst) {
                wait(100, SC_NS);
            }

            // send
            if (!m_output_queue.empty()) {
                unsigned int send_size = m_output_queue.size();
                if (send_size > (sizeof(buffer) / 2)) {
                    send_size = sizeof(buffer) / 2;
                }
                for (unsigned int i = 0; i < send_size; i++) {
                    buffer[i] = m_output_queue.front();
                    m_output_queue.pop();
                }

                write(connfd, (void*) buffer, send_size * 2);
            }

            // receive
            ssize_t nbytes = recv(connfd, (void*) buffer, sizeof(buffer),
                                  MSG_DONTWAIT);
            if (nbytes == 0) {
                printf("Connection closed\n");
                fflush(stdout);
                close(connfd);
                m_chan_data_ok = false;
                break;
            } else if (nbytes < 0) {
                wait(100, SC_NS);
            } else {
                for (int i = 0; i < nbytes / 2; i++) {
                    m_input_queue.push(buffer[i]);
                }
            }
        }
    }
}

/**
 * Thread: Interface to the control TCP channel
 */
void glip_tcp_toplevel::tcp_ctrl_proc()
{
    int listenfd = listen_socket(DEFAULT_PORT_DATA + 1);
    if (listenfd == -1) {
        return;
    }

    while (1) {
        int connfd = wait_for_connection(listenfd);
        if (connfd == -1) {
            return;
        }
        m_chan_ctrl_ok = true;


        // wait for both communication channels to be established
        while (!m_chan_data_ok) {
            wait(100, SC_NS);
        }

        // on the control channel, we only receive data
        uint16_t buffer[1024];
        memset(&buffer, 0, sizeof(buffer));

        while (1) {
            // don't do anything while in reset
            while (m_int_rst) {
                wait(100, SC_NS);
            }

            // receive
            ssize_t nbytes = recv(connfd, (void*) buffer, sizeof(buffer),
                                  MSG_DONTWAIT);
            if (nbytes == 0) {
                printf("Connection closed\n");
                fflush(stdout);
                close(connfd);
                m_chan_ctrl_ok = false;
                break;
            } else if (nbytes < 0) {
                wait(100, SC_NS);
            } else {
                for (int i = 0; i < nbytes / 2; i++) {
                    if (buffer[i] == CTRL_MSG_LOGIC_RESET) {
                        m_trigger_ctrl_rst = true;
                    }
                }
            }
        }

    }
}

/**
 * Create a TCP socket and listen to it
 *
 * @param port the port to listen on
 * @return the file descriptor of the socket
 */
int glip_tcp_toplevel::listen_socket(unsigned int port)
{
    int listenfd;
    struct sockaddr_in servaddr;
    int rv;

    // create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
        return -1;
    }

    // reuse existing socket to avoid socket timeout
    int enable = 1;
    rv = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rv == -1) {
        fprintf(stderr, "Unable to mark socket reusable: %s\n", strerror(errno));
    }

    // bind address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    rv = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (rv == -1) {
        fprintf(stderr, "Cannot bind port %d: %s\n", port, strerror(errno));
        return -1;
    }

    // listen for incoming connections
    rv = listen(listenfd, 1);
    if (rv == -1) {
        fprintf(stderr, "Listen error: %s\n", strerror(errno));
        return -1;
    }

    printf("Now listening on port %d for incoming connections.\n", port);
    fflush(stdout);

    return listenfd;
}

/**
 * Wait for a incoming TCP connection
 *
 * @param listenfd listening socket descriptor
 * @return the connection descriptor
 */
int glip_tcp_toplevel::wait_for_connection(int listenfd)
{
    int connfd;

    int nready;
    fd_set rset, allset;
    socklen_t clilen;
    struct sockaddr_in cliaddr;

    rset = allset;

    printf("Waiting for incoming connection ...\n");
    fflush(stdout);

    // we first wait for the listenfd to be available for reading to avoid
    // blocking in the accept() call, which freezes ModelSim
    // XXX: this is not really perfect, as we sometimes get into accept()
    // below anyway and then ModelSim hangs again.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200;
    FD_ZERO(&rset);
    FD_SET(listenfd, &rset);
    nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
    while (!FD_ISSET(listenfd, &rset)) {
        wait(100, SC_NS);
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
    }

    // accept incoming connection
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);
    if (connfd == -1) {
        fprintf(stderr, "Unable to accept incoming connection: %s\n",
                strerror(errno));
        return -1;
    }

    printf("New client connected: %s:%d\n", inet_ntoa(cliaddr.sin_addr),
           ntohs(cliaddr.sin_port));
    fflush(stdout);

    return connfd;
}
