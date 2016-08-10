/* Copyright (c) 2014-2015 by the author(s)
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
 * ============================================================================
 *
 * Utility functions used in GLIP and its backends.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "util.h"
#include "glip-protected.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Open a socket connection
 *
 * This function blocks until the connection is established and creates a
 * blocking socket.
 *
 * @param[in]  ctx the library context
 * @param[in]  hostname the hostname to connect to
 * @param[in]  port the port to connect to
 * @param[out] sfd the resulting socket file descriptor (if the connection
 *                 was possible)
 * @return 0 if the connection was successful
 * @return any other value indicates an error
 *
 * @see gl_util_sock_nonblock()
 */
int gl_util_connect_to_host(struct glip_ctx *ctx, const char *hostname,
                            unsigned int port, int *sfd)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd_out;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
    hints.ai_protocol = 0;          /* Any protocol */

    char port_c[8];
    if (port > 9999999) {
        err(ctx, "Port %u out of bounds [0..9999999]\n", port);
        return -1;
    }
    snprintf(port_c, sizeof(port_c), "%7u", port);

    int s = getaddrinfo(hostname, port_c, &hints, &result);
    if (s != 0) {
        err(ctx, "getaddrinfo failed for %s:%d: %s\n",
            hostname, port, gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int sfd_tmp = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd_tmp == -1) {
            /* connection failed, try the next one */
            dbg(ctx, "Unable to use connection: %s\n", strerror(errno));
            continue;
        }

        if (connect(sfd_tmp, rp->ai_addr, rp->ai_addrlen) == 0) {
            /* connection successful */
            sfd_out = sfd_tmp;
            break;
        } else {
            dbg(ctx, "connect() failed: %s\n", strerror(errno));
        }

        close(sfd_tmp);
    }

    if (rp == NULL) {
        info(ctx, "Could not connect to %s:%u\n", hostname, port);
        return -1;
    }

    freeaddrinfo(result);

    *sfd = sfd_out;
    return 0;
}

/**
 * Make a file descriptor non-blocking
 *
 * @param ctx the library context
 * @param fd the file descriptor to make non-blocking
 * @return 0 on success
 * @return any other value indicates an error
 */
int gl_util_fd_nonblock(struct glip_ctx *ctx, int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        err(ctx, "Unable to get file descriptor flags: %s\n", strerror(errno));
        return -1;
    }
    flags |= O_NONBLOCK;
    int rv = fcntl(fd, F_SETFL, flags);
    if (rv == -1) {
        err(ctx, "Unable to set file descriptor flags: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * Start a program and attach to the STDIN and STDOUT streams
 *
 * This function is used in similar situations as popen(), but differs in some
 * ways:
 * - The program is executed directly, not through a shell.
 * - STDERR is redirected to STDOUT (@p outfd)
 * - The process id of the child is returned. With the PID the process can be
 *   killed properly.
 *
 * @param file the file to execute. A relative file name is searched in $PATH,
 *             see man execvp() for details.
 * @param argv the arguments passed to the program. Make sure that the first
 *             argument is the name of the executable.
 * @param infd the STDIN file descriptor of the child. Set to NULL if unused.
 * @param outfd the STDOUT/STDERR file descriptor of the child. Set to NULL if
 *              unused.
 * @return the child process PID
 * @return an value <= 0 indicates an error
 *
 * @see gl_util_pclose()
 */
int gl_util_popen(const char *file, char *const argv[], int *infd, int *outfd)
{
    int pfd_stdin[2], pfd_stdout[2], pfd_stderr[2];
    pid_t cpid;

    if (pipe(pfd_stdin) || pipe(pfd_stdout) || pipe(pfd_stderr)) {
        return -1;
    }

    cpid = fork();
    if (cpid == -1) {
        return -1;
    }

    if (cpid == 0) {
        /* in the child */
        dup2(pfd_stdin[0], STDIN_FILENO);
        dup2(pfd_stdout[1], STDOUT_FILENO);
        /* send stdout to stderr */
        dup2(pfd_stdout[1], STDERR_FILENO);

        /* close unused descriptors in child */
        close(pfd_stdin[0]);
        close(pfd_stdin[1]);
        close(pfd_stdout[0]);
        close(pfd_stdout[1]);
        close(pfd_stderr[0]);
        close(pfd_stderr[1]);

        execvp(file, argv);
        perror("execvp");
        exit(1);
    } else {
        /* close unused descriptors in parent */
        close(pfd_stdin[0]);
        close(pfd_stdout[1]);

        close(pfd_stderr[0]);
        close(pfd_stderr[1]);


        if (!infd) {
            close(pfd_stdin[1]);
        } else {
            *infd = pfd_stdin[1];
        }

        if (!outfd) {
            close(pfd_stdout[0]);
        } else {
            *outfd = pfd_stdout[0];
        }

        return cpid;
    }
}


/**
 * Terminate a process and close the pipes
 *
 * Send SIGHUP (first) and SIGKILL (if SIGHUP didn't work) to the process @p pid
 * and wait for it to terminate.
 *
 * @param pid the PID of the process to close
 * @return
 *
 * @see gl_util_pclose()
 */
int gl_util_pclose(pid_t pid)
{
    int status;

    /* send HUP and wait for process to finish */
    kill(pid, SIGHUP);
    for (int i = 0; i < 10; i++) {
        waitpid(pid, &status, WNOHANG);
        if (WIFEXITED(status)) {
            return 0;
        }
        usleep(10*1000); /* 10 ms */
    }

    /* send KILL and try again */
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return 0;
    }

    return -1;
}
