#include "GlipTcp.h"

#include <iostream>

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cassert>
#include <csignal>

using namespace std;

GlipTcp::GlipTcp() : mThreadReady(false), mConnected(false),
        mDataIn(1*1024*1024), mDataOut(1*1024*1024), mControl(256) {
}

void GlipTcp::init(int port, int width) {
    assert(!mThreadReady); // Make sure there is only one instance

    mPort = port;
    mNumBytes = width >> 3;
    pthread_create(&mThread, 0, &GlipTcp::thread, this);

    reset();
    signal(SIGPIPE, SIG_IGN);

    while(!mThreadReady) {}
}

int GlipTcp::reset() {
    /* Empty the buffers */
    uint8_t data;
    while (mControl.pop(data)) {};
    while (mDataIn.pop(data)) {};
    while (mDataOut.pop(data)) {};

    mControlItem.valid = false;
    mControlItem.partial = 0;
    mControlItem.value = 0;

    mReadItem.valid = false;
    mReadItem.partial = 0;
    mReadItem.value = 0;

    mWriteItem.valid = false;
    mWriteItem.partial = 0;
    mWriteItem.value = 0;

    return 0;
}

uint32_t GlipTcp::next_cycle() {
    int rv;
    uint32_t state = 0;

    if (!mConnected) {
        return state;
    }

    /* Check for new control message */
    assert(!mControlItem.valid);
    uint8_t data;
    for (int i = mControlItem.partial; i < 2; i++) {
        if (mControl.pop(data)) {
            mControlItem.value |= data << ((1 - i)*8);
            mControlItem.partial++;
        } else {
            break;
        }
    }

    if (mControlItem.partial == 2) {
        /* Control message is complete */
        mControlItem.valid = true;
        state |= CONTROL_AVAILABLE;
    }

    if (mReadItem.valid) {
        /* Still need to acknowledge current read */
        state |= INCOMING_AVAILABLE;
    } else {
        for (int i = mReadItem.partial; i < mNumBytes; i++) {
            if (mDataIn.pop(data)) {
                mReadItem.value |= data << ((mNumBytes - i - 1)*8);
                mReadItem.partial++;
            } else {
                break;
            }
        }

        if (mReadItem.partial == mNumBytes) {
            /* Read item is complete */
            mReadItem.valid = true;
            state |= INCOMING_AVAILABLE;
        }
    }

    if (!mWriteItem.valid) {
        /* We are ready to send an item */
        state |= OUTGOING_READY;
    } else {
        /* Try to send the item */
        for (int i = mWriteItem.partial; i < mNumBytes; i++) {
            uint8_t data;
            data = (mWriteItem.value >> ((mNumBytes - i - 1)*8)) & 0xff;
            if (mDataOut.push(data)) {
                mWriteItem.partial++;
            } else {
                break;
            }
        }

        if (mWriteItem.partial == mNumBytes) {
            /* The complete item was written */
            state |= OUTGOING_READY;
            mWriteItem.valid = false;
        }
    }

    return state;
}

uint32_t GlipTcp::control_msg() {
    uint32_t data = mControlItem.value;
    mControlItem.valid = false;
    mControlItem.partial = 0;
    return data;
}

uint64_t GlipTcp::readData() {
    assert(mReadItem.valid);
    return mReadItem.value;
}

void GlipTcp::readAck() {
    mReadItem.valid = false;
    mReadItem.partial = 0;
    mReadItem.value = 0;
}

void GlipTcp::writeData(uint64_t data) {
    assert(!mWriteItem.valid);

    mWriteItem.value = data;
    mWriteItem.valid = true;
    mWriteItem.partial = 0;
}

bool GlipTcp::connected() {
    return mConnected;
}

bool GlipTcp::listenSockets() {
    struct sockaddr_in servaddr;
    int rv;

    // Create sockets
    mSocketData.listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketData.listenSocket < 0) {
        cerr << "Cannot create data socket" << endl;
        return false;
    }

    mSocketControl.listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketControl.listenSocket < 0) {
        cerr << "Cannot create control socket" << endl;
        return false;
    }

    // Reuse existing sockets
    int enable = 1;
    rv = setsockopt(mSocketData.listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rv < 0) {
        cerr << "Cannot set data socket properties" << endl;
        return false;
    }
    rv = setsockopt(mSocketControl.listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rv < 0) {
        cerr << "Cannot set control socket properties" << endl;
        return false;
    }

    // bind address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(mPort);

    rv = bind(mSocketData.listenSocket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (rv < 0) {
        cerr << "Cannot bind data port" << endl;
        return false;
    }

    servaddr.sin_port = htons(mPort + 1);
    rv = bind(mSocketControl.listenSocket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (rv < 0) {
        cerr << "Cannot bind control port" << endl;
        return false;
    }

    // listen for incoming connections
    rv = listen(mSocketData.listenSocket, 1);
    if (rv < 0) {
        cerr << "Cannot listen on data port" << endl;
        return false;
    }

    rv = listen(mSocketControl.listenSocket, 1);
    if (rv < 0) {
        cerr << "Cannot listen on control port" << endl;
        return false;
    }

    return true;
}

void *GlipTcp::thread(void) {
    mThreadReady = true;

    if (listenSockets()) {
        cout << "Glip TCP DPI listening on port " << mPort <<
                " and " << (mPort + 1) << endl;
    } else {
        return (void*) -1;
    }

    while(true) {
        mSocketData.socket = -1;
        mSocketControl.socket = -1;

        uint8_t expect_read = 0;
        uint8_t expect_write = 0;

        int nmax;
        if (mSocketData.listenSocket > mSocketControl.listenSocket) {
            nmax = mSocketData.listenSocket + 1;
        } else {
            nmax = mSocketControl.listenSocket + 1;
        }

        while ((mSocketData.socket == -1) || (mSocketControl.socket == -1)) {
            fd_set waitfds;
            FD_ZERO(&waitfds);
            if (mSocketData.socket == -1) {
                FD_SET(mSocketData.listenSocket, &waitfds);
            }
            if (mSocketControl.socket == -1) {
                FD_SET(mSocketControl.listenSocket, &waitfds);
            }

            int activity = select( nmax + 1 , &waitfds , 0 , 0 , 0);

            if ((activity < 0) && (errno != EINTR)) {
                cerr << "Error while waiting for TCP connections" << endl;
                return (void*) -1;
            }

            if (FD_ISSET(mSocketData.listenSocket, &waitfds)) {
                mSocketData.socket = accept(mSocketData.listenSocket,
                                            &mSocketData.socketAddress,
                                            (socklen_t*)&mSocketData.socketAddressLength);
                if (mSocketData.socket < 0) {
                    cerr << "Error while accepting data connection" << endl;
                    return (void*) -1;
                }

                int status = fcntl(mSocketData.socket, F_GETFL, 0);
                status = fcntl(mSocketData.socket, F_SETFL, status | O_NONBLOCK);
            }

            if (FD_ISSET(mSocketControl.listenSocket, &waitfds)) {
                mSocketControl.socket = accept(mSocketControl.listenSocket,
                                               &mSocketControl.socketAddress,
                                               (socklen_t*)&mSocketControl.socketAddressLength);
                if (mSocketControl.socket < 0) {
                    cerr << "Error while accepting control connection" << endl;
                    return (void*) -1;
                }

                int status = fcntl(mSocketControl.socket, F_GETFL, 0);
                status = fcntl(mSocketControl.socket, F_SETFL, status | O_NONBLOCK);
            }
        }

        cout << "Client connected" << endl;
        reset();
        mConnected = true;

        while(true) {
            int rv;
            size_t sz = 4096;
            uint8_t data[4096];

            rv = read(mSocketControl.socket, data, sz);
            if (rv == -1) {
                if ((errno == EBADF) || (errno == EINVAL)) {
                    break;
                }
            } else if (rv == 0) {
                break;
            } else if (rv > 0) {
                for (int i = 0; i < rv; i++) {
                    while (!mControl.push(data[i])) {}
                }
            }

            rv = read(mSocketData.socket, data, sz);
            if (rv == -1) {
                if ((errno == EBADF) || (errno == EINVAL)) {
                    break;
                }
            } else if (rv == 0) {
                break;
            } else if (rv > 0) {
                for (int i = 0; i < rv; i++) {
                    while (!mDataIn.push(data[i])) {}
                }
            }

            size_t count = mDataOut.pop(data, sz);
            rv = write(mSocketData.socket, data, count);
            if (rv == -1) {
                if ((errno == EBADF) || (errno == EINVAL)) {
                    break;
                }
            } else {
                assert(rv == count);
            }

            usleep(100);
        }

        mConnected = false;
        reset();
        cout << "Disconnected" << endl;

    }
    return 0;
}

void* GlipTcp::thread(void* ctx) {
    GlipTcp *c = (GlipTcp *) ctx;
    return c->thread();
}
