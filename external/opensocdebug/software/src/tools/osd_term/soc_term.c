/*
 * Copyright (c) 2014, Linaro Limited
 *               2016, Stefan Wallentowitz <stefan@wallentowitz.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static const char prog_name[] = "osd_term";

static void usage(void)
{
    fprintf(stderr, "Usage: %s <socket>\n", prog_name);
    exit(1);
}

static int open_fd(const char *socket_path)
{
    struct sockaddr_un remote;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, socket_path);
    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    return fd;
}

static bool write_buf(int fd, const void *buf, size_t count)
{
    const uint8_t *b = buf;
    size_t num_written = 0;

    while (num_written < count) {
        ssize_t res = write(fd, b + num_written, count - num_written);

        if (res == -1)
            return false;

        num_written += res;
    }
    return true;
}

static void serve_fd(int fd)
{
    uint8_t buf[512];
    struct pollfd pfds[2];

    memset(pfds, 0, sizeof(pfds));
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = fd;
    pfds[1].events = POLLIN;

    struct termios old = {0};
    if (tcgetattr(STDIN_FILENO, &old) < 0)
            perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");


    while (true) {
        int n;

        if (poll(pfds, 2, -1) == -1)
            err(1, "poll");

        if (pfds[0].revents & POLLIN) {
            n = read(STDIN_FILENO, buf, 1);
            if (n == -1)
                err(1, "read stdin");
            if (n == 0)
                errx(1, "read stdin EOF");

            /* TODO handle case when this write blocks */
            if (!write_buf(fd, buf, n)) {
                warn("write_buf fd");
                break;
            }
        }

        if (pfds[1].revents & POLLIN) {
            n = read(fd, buf, sizeof(buf));
            if (n == -1) {
                warn("read fd");
                break;
            }
            if (n == 0) {
                warnx("read fd EOF");
                break;
            }

            if (!write_buf(STDOUT_FILENO, buf, n))
                err(1, "write_buf stdout");
        }
    }
}

int main(int argc, char *argv[])
{
    char *socket;

    switch (argc) {
    case 2:
        socket = argv[1];
        break;
    default:
        usage();
    }

    int fd = open_fd(socket);

    serve_fd(fd);
    if (close(fd))
        err(1, "close");
    fd = -1;
}
