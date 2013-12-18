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

#include "FX2EmulationDebugConnector.h"
#include <errno.h>

SC_MODULE_EXPORT(FX2EmulationDebugConnector);

void FX2EmulationDebugConnector::drivefd()
{
    if (!fx2_sloe) {
        // Drive output
        if (!input_queue.empty()) {
            fx2_fd = input_queue.front();
        } else {
            fx2_fd = "zzzzzzzzzzzzzzzz";
        }
    } else {
        fx2_fd = "zzzzzzzzzzzzzzzz";
    }
}

void FX2EmulationDebugConnector::interface()
{
    if (in_reset)
        return;
    if (!fx2_slrd && !input_queue.empty()) {
        input_queue.pop();
        if (!fx2_sloe && !input_queue.empty()) {
            fx2_fd = input_queue.front();
        }
    } else if (!fx2_slwr) {
        short int v = fx2_fd.read().to_int();
        output_queue.push(v);
    }
    fx2_flaga = !input_queue.empty();
    fx2_flagb = true;
    fx2_flagc = true;
    fx2_flagd = true;
}

void FX2EmulationDebugConnector::handler()
{
    int port = 23000;
    int listenfd, connfd;
    int nready;
    fd_set rset, allset;
    unsigned short buffer[1000];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("Cannot bind, errno == %d\n", errno);
        fflush(stdout);
        return;
    }

    if (listen(listenfd, 1) == -1) {
        printf("Listen error");
        fflush(stdout);
        return;
    }

    printf("Listening on port %d\n", port);
    fflush(stdout);

    bzero((void*) &buffer, 2000);

    for (;;) {
        rset = allset;

        printf("Wait for new connection\n");
        fflush(stdout);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200;
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
        while (!FD_ISSET(listenfd, &rset)) {
            wait(100, SC_NS);
            //        printf("Wait for new connection\n");
            FD_ZERO(&rset);
            FD_SET(listenfd, &rset);
            nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
        }

        printf("listening socket readable\n");
        fflush(stdout);

        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);

        printf("new client: %s, port %d\n", inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port));
        fflush(stdout);

        wait(1, SC_US);

        for (;;) {
            // If there is something to send we send it
            if (!output_queue.empty() && (output_queue.size() >= 256)) {
                int size = 256;
                for (int i = 0; i < size; i++) {
                    buffer[i] = output_queue.front();
                    fflush(stdout);
                    output_queue.pop();
                }

                write(connfd, (void*) buffer, size * 2);
            }

            // Now we see if something arrived for us
            int nbytes = recv(connfd, (void*) buffer, 512, MSG_DONTWAIT);
            if (nbytes == 0) {
                printf("Connection closed\n");
                fflush(stdout);
                close(connfd);
                break;
            } else if (nbytes < 0) {
                wait(100, SC_NS);
            } else {
                for (int i = 0; i < nbytes / 2; i++) {
                    input_queue.push(buffer[i]);
                }
            }

        }
    }
}

void FX2EmulationDebugConnector::ctrlhandler()
{
    int port = 23001;
    int listenfd, connfd;
    int nready;
    fd_set rset, allset;
    unsigned short buffer[1000];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    reset = true;
    in_reset = true;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("Cannot bind\n");
        fflush(stdout);
        return;
    }

    if (listen(listenfd, 1) == -1) {
        printf("Listen error");
        fflush(stdout);
        return;
    }

    printf("Listening on port %d\n", port);
    fflush(stdout);

    bzero((void*) &buffer, 2000);

    for (;;) {
        rset = allset;

        printf("Wait for new connection\n");
        fflush(stdout);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200;
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
        while (!FD_ISSET(listenfd, &rset)) {
            wait(100, SC_NS);
            //        printf("Wait for new connection\n");
            FD_ZERO(&rset);
            FD_SET(listenfd, &rset);
            nready = select(listenfd + 1, &rset, NULL, NULL, &tv);
        }

        printf("listening socket readable\n");
        fflush(stdout);

        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);

        printf("new client: %s, port %d\n", inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port));
        fflush(stdout);

        reset = false;
        in_reset = false;

        wait(1, SC_US);

        for (;;) {
            // Now we see if something arrived for us
            int nbytes = recv(connfd, (void*) buffer, 512, MSG_DONTWAIT);
            if (nbytes == 0) {
                printf("Connection closed\n");
                fflush(stdout);
                close(connfd);
                reset = true;
                in_reset = true;
                while (!input_queue.empty())
                    input_queue.pop();
                while (!output_queue.empty())
                    output_queue.pop();
                break;
            } else if (nbytes < 0) {
                wait(100, SC_NS);
            } else {
                for (int i = 0; i < nbytes / 2; i++) {
                    // TODO: implement reset
                }
            }

        }
    }
}
