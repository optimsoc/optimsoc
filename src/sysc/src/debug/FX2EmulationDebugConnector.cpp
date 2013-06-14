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
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include "FX2EmulationDebugConnector.h"

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
